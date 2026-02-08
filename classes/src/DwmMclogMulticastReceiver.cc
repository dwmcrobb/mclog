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
//!  @file DwmMclogMulticastReceiver.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MulticastReceiver implementation
//---------------------------------------------------------------------------

#include <algorithm>

#include "DwmFormatters.hh"
#include "DwmSysLogger.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogKeyRequester.hh"
#include "DwmMclogMulticastReceiver.hh"
#include "DwmMclogLogger.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MulticastReceiver::MulticastReceiver()
        : _config(), _fd(-1), _fd6(-1), _acceptLocal(true), _sinksMutex(),
          _sinks(), _thread(), _run(false),
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
          else {
            MCLOG(Severity::err, "bind({},{},{}) failed: {}",
                  _fd, locAddr, sizeof(locAddr), strerror(errno));
          }
        }
        else {
          MCLOG(Severity::err, "setsockopt({},SOL_SOCKET,SO_REUSEPORT)"
                " failed: {}", _fd, strerror(errno));
        }
      }
      else {
        MCLOG(Severity::err, "setsockopt({},SOL_SOCKET,SO_REUSEADDR)"
              " failed: {}", _fd, strerror(errno));
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool MulticastReceiver::BindSocket6()
    {
      bool  rc = false;
      int   on = 1;
      if (setsockopt(_fd6, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0) {
        if (setsockopt(_fd6, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == 0) {
          sockaddr_in6  locAddr;
          memset(&locAddr, 0, sizeof(locAddr));
          locAddr.sin6_family = PF_INET6;
          locAddr.sin6_port = htons(_config.mcast.dstPort);
          locAddr.sin6_addr = (in6_addr)_config.mcast.groupAddr6;
#ifndef __linux__
          locAddr.sin6_len = sizeof(locAddr);
#else
          locAddr.sin6_scope_id = if_nametoindex(_config.mcast.intfName.c_str());
#endif
          if (::bind(_fd6, (sockaddr *)&locAddr, sizeof(locAddr)) == 0) {
            rc = true;
          }
          else {
            MCLOG(Severity::err, "bind({},{},{}) failed: {}",
                  _fd6, locAddr, sizeof(locAddr), strerror(errno));
          }
        }
        else {
          MCLOG(Severity::err, "setsockopt({},SOL_SOCKET,SO_REUSEPORT)"
                " failed: {}", _fd6, strerror(errno));
        }
      }
      else {
        MCLOG(Severity::err, "setsockopt({},SOL_SOCKET,SO_REUSEADDR)"
              " failed: {}", _fd6, strerror(errno));
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool MulticastReceiver::JoinGroup()
    {
      bool  shouldJoin = ((_config.mcast.groupAddr != Ipv4Address())
                          && (_config.mcast.intfAddr != Ipv4Address()));
      if (shouldJoin) {
        struct ip_mreq group;
        group.imr_multiaddr.s_addr = _config.mcast.groupAddr.Raw();
        group.imr_interface.s_addr = _config.mcast.intfAddr.Raw();
        if (setsockopt(_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       (char *)&group, sizeof(group)) == 0) {
          MCLOG(Severity::info, "Joined group {} on {}",
                _config.mcast.groupAddr, _config.mcast.intfAddr);
          return true;
        }
        else {
          MCLOG(Severity::err, "setsockopt({},IPPROTO_IP, IP_ADD_MEMBERSHIP,"
                "{} {}) failed: {}", _fd, _config.mcast.groupAddr,
                _config.mcast.intfAddr, strerror(errno));
          return false;
        }
      }
      return true;
    }

    //------------------------------------------------------------------------
    bool MulticastReceiver::JoinGroup6()
    {
      bool  shouldJoin = ((! _config.mcast.intfName.empty())
                          && (_config.mcast.groupAddr6 != Ipv6Address()));
      bool  rc = false;
      if (shouldJoin) {
        struct ipv6_mreq  group;
        group.ipv6mr_multiaddr = _config.mcast.groupAddr6;
        group.ipv6mr_interface = if_nametoindex(_config.mcast.intfName.c_str());
        if (setsockopt(_fd6, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                       (char *)&group, sizeof(group)) == 0) {
          rc = true;
          MCLOG(Severity::info, "MulticastReceiver joined {} on interface {}",
                _config.mcast.groupAddr6, _config.mcast.intfName);
        }
        else {
          MCLOG(Severity::err, "MulticastReceiver failed to join {} on interface"
                " {}: {}", _config.mcast.groupAddr6, _config.mcast.intfName,
                strerror(errno));
        }
      }
      return (shouldJoin ? rc : true);
    }
    
    //------------------------------------------------------------------------
    bool MulticastReceiver::Open(const Config & cfg, bool acceptLocal)
    {
      bool  rc = false;
      
      _config = cfg;
      _acceptLocal = acceptLocal;

      bool shouldJoin4 = ((_config.mcast.groupAddr != Ipv4Address())
                          && (_config.mcast.intfAddr != Ipv4Address()));
      bool shouldJoin6 = ((! _config.mcast.intfName.empty())
                          && (_config.mcast.groupAddr6 != Ipv6Address())
                          && (_config.mcast.intfAddr6 != Ipv6Address()));
      if (! (shouldJoin4 || shouldJoin6)) {
        return true;
      }
      
      if (shouldJoin4) {
        if (0 > _fd) {
          _fd = socket(PF_INET, SOCK_DGRAM, 0);
          if (0 <= _fd) {
            if (! BindSocket()) {
              ::close(_fd);  _fd = -1;
            }
            else {
              if (! JoinGroup()) {
                ::close(_fd);  _fd = -1;
              }
            }
          }
        }
      }
      if (shouldJoin6) {
        if (0 > _fd6) {
          _fd6 = socket(PF_INET6, SOCK_DGRAM, 0);
          if (0 <= _fd6) {
            if (! BindSocket6()) {
              ::close(_fd6);  _fd6 = -1;
            }
            else {
              if (! JoinGroup6()) {
                ::close(_fd6);  _fd6 = -1;
              }
            }
          }
        }
      }
      if ((shouldJoin4 && (0 <= _fd))
          || (shouldJoin6 && (0 <= _fd6))) {
        if (0 == pipe(_stopfds)) {
          _run = true;
          _thread = std::thread(&MulticastReceiver::Run, this);
#if (defined(__FreeBSD__) || defined(__linux__))
          pthread_setname_np(_thread.native_handle(), "MulticastRecv");
#endif
          rc = true;
        }
      }
      if (! rc) {
        Close();
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
        ::close(_fd);  _fd = -1;
      }
      if (0 <= _fd6) {
        ::close(_fd6);  _fd6 = -1;
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
    bool MulticastReceiver::AddSink(MessageSink *sink)
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
    bool MulticastReceiver::RemoveSink(MessageSink *sink)
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
      MCLOG(Severity::info, "MulticastReceiver thread started");
#if (__APPLE__)
      pthread_setname_np("MulticastReceiver");
#endif
      if ((0 <= _fd) || (0 <= _fd6)) {
        fd_set        fds;
        sockaddr_in   fromAddr;
        sockaddr_in6  fromAddr6;
        int           maxfd;
        
        auto  reset_fds = [&] () -> void
        {
          FD_ZERO(&fds);
          if (0 <= _fd)  { FD_SET(_fd, &fds);  }
          if (0 <= _fd6) { FD_SET(_fd6, &fds); }
          FD_SET(_stopfds[0], &fds);
          maxfd = std::max({_fd, _fd6, _stopfds[0]}) + 1;
        };
        
        while (_run) {
          reset_fds();
          if (select(maxfd, &fds, nullptr, nullptr, nullptr) > 0) {
            if (FD_ISSET(_stopfds[0], &fds)) {
              break;
            }
            if ((0 <= _fd) && FD_ISSET(_fd, &fds)) {
              char  buf[1500];
              socklen_t  fromAddrLen = sizeof(fromAddr);
              ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                         (sockaddr *)&fromAddr,              
                                         &fromAddrLen);
              Ipv4Address  fromIP(fromAddr.sin_addr.s_addr);
              if ((recvrc > 0) && (_acceptLocal || (fromIP != _config.mcast.intfAddr))) {
                UdpEndpoint  endPoint(fromAddr);
                MCLOG(Severity::debug, "Received {} bytes from {}",
                      recvrc, endPoint);
                _sources.ProcessPacket(endPoint, buf, recvrc);
              }
            }
            if ((0 <= _fd6) && FD_ISSET(_fd6, &fds)) {
              char  buf[1500];
              socklen_t  fromAddrLen = sizeof(fromAddr6);
              ssize_t  recvrc = recvfrom(_fd6, buf, sizeof(buf), 0,
                                         (sockaddr *)&fromAddr6,              
                                         &fromAddrLen);
              Ipv6Address  fromIP(fromAddr6.sin6_addr);
              if ((recvrc > 0) && (_acceptLocal || (fromIP != _config.mcast.intfAddr6))) {
                UdpEndpoint  endPoint(fromAddr6);
                MCLOG(Severity::debug, "Received {} bytes from {}",
                      recvrc, endPoint);
                _sources.ProcessPacket(endPoint, buf, recvrc);
              }
            }
          }
        }
      }
      _run = false;
      MCLOG(Severity::info, "MulticastReceiver thread done");
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
