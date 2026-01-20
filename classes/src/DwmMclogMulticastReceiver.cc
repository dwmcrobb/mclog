//===========================================================================
//  Copyright (c) Daniel W. McRobb 2025
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. The names of the authors and copyright holders may not be used to
//     endorse or promote products derived from this software without
//     specific prior written permission.
//
//  IN NO EVENT SHALL DANIEL W. MCROBB BE LIABLE TO ANY PARTY FOR
//  DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES,
//  INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE,
//  EVEN IF DANIEL W. MCROBB HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
//  DAMAGE.
//
//  THE SOFTWARE PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND
//  DANIEL W. MCROBB HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
//  UPDATES, ENHANCEMENTS, OR MODIFICATIONS. DANIEL W. MCROBB MAKES NO
//  REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER
//  IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE,
//  OR THAT THE USE OF THIS SOFTWARE WILL NOT INFRINGE ANY PATENT,
//  TRADEMARK OR OTHER RIGHTS.
//===========================================================================

//---------------------------------------------------------------------------
//!  @file DwmMclogMulticastReceiver.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <algorithm>

#include "DwmSysLogger.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogKeyRequester.hh"
#include "DwmMclogMulticastReceiver.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MulticastReceiver::MulticastReceiver()
        : _config(), _fd(-1), _acceptLocal(true), _sinksMutex(), _sinks(),
          _thread(), _run(false),
          _sources(&_config.service.keyDirectory, &_sinks)
    {
      _stopfds[0] = -1;
      _stopfds[1] = -1;
    }

    //------------------------------------------------------------------------
    MulticastReceiver::~MulticastReceiver()
    {
      Close();
    }

    //------------------------------------------------------------------------
    bool MulticastReceiver::BindSocket()
    {
      bool  rc = false;
      int   on = 1;
      if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0) {
        if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == 0) {
          sockaddr_in  locAddr;
          memset(&locAddr, 0, sizeof(locAddr));
          locAddr.sin_family = PF_INET;
          locAddr.sin_port = htons(_config.mcast.dstPort);
          locAddr.sin_addr.s_addr = _config.mcast.groupAddr.Raw();
#ifndef __linux__
          locAddr.sin_len = sizeof(locAddr);
#endif
          if (::bind(_fd, (sockaddr *)&locAddr, sizeof(locAddr)) == 0) {
            rc = true;
          }
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool MulticastReceiver::JoinGroup()
    {
      struct ip_mreq group;
      group.imr_multiaddr.s_addr = _config.mcast.groupAddr.Raw();
      group.imr_interface.s_addr = _config.mcast.intfAddr.Raw();
      return (setsockopt(_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         (char *)&group, sizeof(group)) == 0);
    }

    //------------------------------------------------------------------------
    bool MulticastReceiver::Open(const Config & cfg, bool acceptLocal)
    {
      bool  rc = false;
      
      _config = cfg;
      _acceptLocal = acceptLocal;
      
      if (0 > _fd) {
        _fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (0 <= _fd) {
          if (BindSocket()) {
            if (JoinGroup()) {
              if (0 == pipe(_stopfds)) {
                _run = true;
                _thread = std::thread(&MulticastReceiver::Run, this);
                rc = true;
              }
            }
          }
          if (! rc) {
            Close();
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool MulticastReceiver::Restart(const Config & cfg)
    {
      Close();
      return Open(cfg, _acceptLocal);
    }
    
    //------------------------------------------------------------------------
    void MulticastReceiver::Close()
    {
      _run = false;
      if (_stopfds[1] >= 0) {
        char  stop;
        write(_stopfds[1], &stop, sizeof(stop));
      }
      if (_thread.joinable()) {
        _thread.join();
      }
      if (0 <= _fd) {
        ::close(_fd);
        _fd = -1;
      }

      //  Close the stop command pipe descriptors
      if (_stopfds[1] >= 0) {
        ::close(_stopfds[1]);  _stopfds[1] = -1;
      }
      if (_stopfds[0] >= 0) {
        ::close(_stopfds[0]);  _stopfds[0] = -1;
      }
      return;
    }
    
    //------------------------------------------------------------------------
    bool MulticastReceiver::AddSink(Thread::Queue<Message> *sink)
    {
      bool  rc = false;
      std::lock_guard  lck(_sinksMutex);
      auto  it = std::find_if(_sinks.cbegin(), _sinks.cend(),
                              [sink] (const auto & q)
                              { return (q == sink); });
      if (it == _sinks.cend()) {
        _sinks.push_back(sink);
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool MulticastReceiver::RemoveSink(Thread::Queue<Message> *sink)
    {
      bool  rc = false;
      std::lock_guard  lck(_sinksMutex);
      auto  it = std::find_if(_sinks.cbegin(), _sinks.cend(),
                              [sink] (const auto & q)
                              { return (q == sink); });
      if (it != _sinks.cend()) {
        _sinks.erase(it);
        rc = true;
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    void MulticastReceiver::ClearSinks()
    {
      std::lock_guard  lck(_sinksMutex);
      _sinks.clear();
      return;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void MulticastReceiver::Run()
    {
      Syslog(LOG_INFO, "MulticastReceiver thread started");

      if (0 <= _fd) {
        fd_set       fds;
        sockaddr_in  fromAddr;
        auto  reset_fds = [&] () -> void
        { FD_ZERO(&fds); FD_SET(_fd, &fds); FD_SET(_stopfds[0], &fds); };
        while (_run) {
          reset_fds();
          if (select(std::max(_fd,_stopfds[0]) + 1, &fds, nullptr, nullptr, nullptr) > 0) {
            if (FD_ISSET(_stopfds[0], &fds)) {
              break;
            }
            if (FD_ISSET(_fd, &fds)) {
              char  buf[1500];
              socklen_t  fromAddrLen = sizeof(fromAddr);
              ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                         (sockaddr *)&fromAddr,              
                                         &fromAddrLen);
              Ipv4Address  fromIP(fromAddr.sin_addr.s_addr);
              if ((recvrc > 0) && (_acceptLocal || (fromIP != _config.mcast.intfAddr))) {
                Syslog(LOG_DEBUG, "Received %lld bytes from %s:%hu",
                       recvrc, ((std::string)fromIP).c_str(),
                       ntohs(fromAddr.sin_port));
                _sources.ProcessPacket(fromAddr, buf, recvrc);
              }
            }
          }
        }
      }
      _run = false;
      Syslog(LOG_INFO, "MulticastReceiver thread done");
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
