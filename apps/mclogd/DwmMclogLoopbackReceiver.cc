//===========================================================================
//  Copyright (c) Daniel W. McRobb 2025, 2026
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
//!  @file DwmMclogLoopbackReceiver.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
  #include <unistd.h>
}

#include <cstring>

#include "DwmFormatters.hh"
#include "DwmIpv4Address.hh"
#include "DwmMclogLoopbackReceiver.hh"
#include "DwmMclogMessage.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    LoopbackReceiver::LoopbackReceiver()
        : _config(), _ifd(-1), _ifd6(-1), _run(false), _thread(),
          _sinksMutex(), _sinks()
    {}
    
    //------------------------------------------------------------------------
    bool LoopbackReceiver::Start(const Config & config)
    {
      _config = config;
      _run = true;
      if (0 == pipe(_stopfds)) {
        _thread = std::thread(&LoopbackReceiver::Run, this);
#if (defined(__FreeBSD__) || defined(__linux__))
        pthread_setname_np(_thread.native_handle(), "LoopbackRecv");
#endif
        Syslog(LOG_INFO, "LoopbackReceiver started");
        return true;
      }
      else {
        FSyslog(LOG_ERR, "LoopbackReceiver not started: pipe() failed ({})",
                strerror(errno));
      }
      return false;
    }

    //------------------------------------------------------------------------
    bool LoopbackReceiver::Restart(const Config & config)
    {
      Stop();
      return Start(config);
    }
    
    //------------------------------------------------------------------------
    void LoopbackReceiver::Stop()
    {
      _run = false;
      char  stop = 's';
      ::write(_stopfds[1], &stop, sizeof(stop));
      if (_thread.joinable()) {
        _thread.join();
        ::close(_stopfds[1]);  _stopfds[1] = -1;
        ::close(_stopfds[0]);  _stopfds[0] = -1;
        Syslog(LOG_INFO, "LoopbackReceiver stopped");
      }
      return;
    }

    //------------------------------------------------------------------------
    bool LoopbackReceiver::AddSink(MessageSink *sink)
    {
      bool  rc = false;
      std::lock_guard  lck(_sinksMutex);
      auto  it = std::find_if(_sinks.cbegin(), _sinks.cend(),
                              [sink] (const auto & q)
                              { return (sink == q); });
      if (it == _sinks.cend()) {
        _sinks.push_back(sink);
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool LoopbackReceiver::OpenIpv4Socket()
    {
      bool  rc = false;
      if (0 <= _ifd) {
        ::close(_ifd);  _ifd = -1;
      }
      _ifd = socket(PF_INET, SOCK_DGRAM, 0);
      if (0 <= _ifd) {
        struct sockaddr_in  sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = Ipv4Address("127.0.0.1").Raw();
        sockAddr.sin_port = htons(_config.loopback.port);
#ifndef __linux__
        sockAddr.sin_len = sizeof(sockAddr);
#endif
        if (0 == bind(_ifd, (sockaddr *)&sockAddr, sizeof(sockAddr))) {
          rc = true;
        }
        else {
          FSyslog(LOG_ERR, "LoopbackReceiver bind({},{}) failed: {}",
                  _ifd, sockAddr, strerror(errno));
          ::close(_ifd); _ifd = -1;
        }
      }
      else {
        FSyslog(LOG_ERR,
                "LoopbackReceiver socket(PF_INET,SOCK_DGRAM,0) failed: {}",
                strerror(errno));
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool LoopbackReceiver::OpenIpv6Socket()
    {
      bool  rc = false;
      if (0 <= _ifd6) {
        ::close(_ifd6);  _ifd6 = -1;
      }
      _ifd6 = socket(AF_INET6, SOCK_DGRAM, 0);
      if (0 <= _ifd6) {
        struct sockaddr_in6  sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin6_family = AF_INET6;
        sockAddr.sin6_addr = Ipv6Address("::1");
        sockAddr.sin6_port = htons(_config.loopback.port);
#ifndef __linux__
        sockAddr.sin6_len = sizeof(sockAddr);
#endif
        if (0 == bind(_ifd6, (sockaddr *)&sockAddr, sizeof(sockAddr))) {
          rc = true;
        }
        else {
          FSyslog(LOG_ERR, "LoopbackReceiver bind({},{}) failed: {}",
                  _ifd6, sockAddr, strerror(errno));
          ::close(_ifd6); _ifd6 = -1;
        }
      }
      else {
        FSyslog(LOG_ERR,
                "LoopbackReceiver socket(PF_INET,SOCK_DGRAM,0) failed: {}",
                strerror(errno));
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool LoopbackReceiver::DesiredSocketsOpen() const
    {
      bool  v4ok =
        (_config.loopback.ListenIpv4() ? (0 <= _ifd) : (0 > _ifd));
      if (! v4ok) {
        FSyslog(LOG_ERR, "LoopbackReceiver ipv4 socket not in desired state"
                " (_ifd == {})!", _ifd);
      }
      bool  v6ok =
        (_config.loopback.ListenIpv6() ? (0 <= _ifd6) : (0 > _ifd6));
      if (! v6ok) {
        FSyslog(LOG_ERR, "LoopbackReceiver ipv6 socket not in desired state"
                " (_ifd6 =- {})!", _ifd6);
      }
      return (v4ok && v6ok);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void LoopbackReceiver::Run()
    {
      Syslog(LOG_INFO, "LoopbackReceiver thread started");
#if (__APPLE__)
      pthread_setname_np("LoopbackReceiver");
#endif
      if (_config.loopback.ListenIpv4()) {
        OpenIpv4Socket();
      }
      if (_config.loopback.ListenIpv6()) {
        OpenIpv6Socket();
      }

      if (DesiredSocketsOpen()) {
        fd_set        fds;
        int           maxfd;
        sockaddr_in   fromAddr;
        sockaddr_in6  fromAddr6;
        
        auto  reset_fds = [&] () -> void
        {
          FD_ZERO(&fds);
          maxfd = 0;
          if (0 <= _ifd)  { FD_SET(_ifd, &fds); maxfd = std::max({_ifd, maxfd}); }
          if (0 <= _ifd6) { FD_SET(_ifd6, &fds); maxfd = std::max({_ifd6, maxfd}); }
          FD_SET(_stopfds[0], &fds);
          maxfd = std::max({_stopfds[0], maxfd});
        
        };
        char     buf[1500];
        Message  msg;
        while (_run) {
          reset_fds();
          int selectrc = select(maxfd, &fds, nullptr, nullptr, nullptr);
          if (selectrc > 0) {
            if ((0 <= _ifd) && FD_ISSET(_ifd, &fds)) {
              MessagePacket  pkt(buf, sizeof(buf));
              socklen_t      fromAddrLen = sizeof(fromAddr);
              if (pkt.RecvFrom(_ifd, &fromAddr) > 0) {
                while (msg.Read(pkt.Payload())) {
                  for (auto sink : _sinks) {
                    sink->PushBack(msg);
                  }
                }
              }
            }
            else if ((0 <= _ifd6) && FD_ISSET(_ifd6, &fds)) {
              MessagePacket  pkt(buf, sizeof(buf));
              socklen_t      fromAddrLen = sizeof(fromAddr6);
              if (pkt.RecvFrom(_ifd, &fromAddr6) > 0) {
                while (msg.Read(pkt.Payload())) {
                  for (auto sink : _sinks) {
                    sink->PushBack(msg);
                  }
                }
              }
            }
            else if (FD_ISSET(_stopfds[0], &fds)) {
              break;
            }
          }
        }
        if (0 <= _ifd)   { ::close(_ifd); _ifd = -1; }
        if (0 <= _ifd6)  { ::close(_ifd6); _ifd6 = -1; }
      }
      else {
        if (0 <= _ifd)   { ::close(_ifd); _ifd = -1; }
        if (0 <= _ifd6)  { ::close(_ifd6); _ifd6 = -1; }
        FSyslog(LOG_ERR, "LoopbackReceiver sockets not in desired state!");
      }
      _run = false;
      Syslog(LOG_INFO, "LoopbackReceiver thread done");
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
