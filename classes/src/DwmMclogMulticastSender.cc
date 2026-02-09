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
//!  @file DwmMclogMulticastSender.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MulticastSender implementation
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
  #include <net/if.h>
}

#include <sstream>

#include "DwmFormatters.hh"
#include "DwmCredenceKXKeyPair.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogMulticastSender.hh"
#include "DwmMclogMessagePacket.hh"
#include "DwmMclogLogger.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MulticastSender::MulticastSender()
        : _fd(-1), _fd6(-1), _run(false), _thread(), _outQueue(), _config(),
          _dstEndpoint(), _dstEndpoint6(), _key(), _keyRequestListener(),
          _filterDriver(nullptr)
    {
      Credence::KXKeyPair  key1;
      Credence::KXKeyPair  key2;
      _key = key2.SharedKey(key1.PublicKey().Value());
      _nextSendTime = Clock::now() + std::chrono::milliseconds(1000);

      _outQueue.MaxLength(1000);
    }

    //------------------------------------------------------------------------
    MulticastSender::~MulticastSender()
    {
      Close();
      _filterDriver = nullptr;
    }

    //------------------------------------------------------------------------
    bool MulticastSender::OpenSocket()
    {
      bool  rc = false;
      _fd = socket(PF_INET, SOCK_DGRAM, 0);
      if (0 <= _fd) {
        sockaddr_in  bindAddr;
        memset(&bindAddr, 0, sizeof(bindAddr));
        bindAddr.sin_family = PF_INET;
        bindAddr.sin_addr.s_addr = _config.mcast.intfAddr.Raw();
        bindAddr.sin_port = htons(_config.mcast.dstPort);
#ifndef __linux__
        bindAddr.sin_len = sizeof(bindAddr);
#endif
        if (setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_IF, &bindAddr.sin_addr,
                       sizeof(bindAddr.sin_addr)) == 0) {
          MCLOG(Severity::info, "MulticastSender socket fd {} interface"
                " set to {}", _fd, bindAddr.sin_addr);
          if (0 == bind(_fd, (sockaddr *)&bindAddr, sizeof(bindAddr))) {
            rc = true;
            socklen_t  socklen = sizeof(bindAddr);
            getsockname(_fd, (sockaddr *)&bindAddr, &socklen);
            MCLOG(Severity::info, "MulticastSender socket fd {} bound to {}",
                    _fd, bindAddr);
          }
          else {
            MCLOG(Severity::err, "bind({},{},{}) failed: {}",
                  _fd, bindAddr, sizeof(bindAddr), strerror(errno));
            ::close(_fd);  _fd = -1;
          }
        }
        else {
          MCLOG(Severity::err,
                "setsockopt({},IPPROTO_IP,IP_MULTICAST_IF,{},{}) failed: {}",
                _fd, bindAddr.sin_addr, sizeof(bindAddr.sin_addr),
                strerror(errno));
          ::close(_fd);  _fd = -1;
        }
      }
      else {
        MCLOG(Severity::err, "socket(PF_INET,SOCK_DGRAM,0) failed: {}",
              strerror(errno));
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool MulticastSender::OpenSocket6()
    {
      bool  rc = false;
      if (_config.mcast.intfName.empty()) {
        MCLOG(Severity::err, "intfName is empty in configuration but"
              " required for IPv6");
        return false;
      }
      
      _fd6 = socket(PF_INET6, SOCK_DGRAM, 0);
      if (0 <= _fd6) {
        unsigned int  intfIndex =
          if_nametoindex(_config.mcast.intfName.c_str());
        if (0 < intfIndex) {
          if (setsockopt(_fd6, IPPROTO_IPV6, IPV6_MULTICAST_IF, &intfIndex,
                       sizeof(intfIndex)) == 0) {
            MCLOG(Severity::info, "MulticastSender socket fd {} interface set"
                  " to {} ({})", _fd6, intfIndex, _config.mcast.intfName);
            sockaddr_in6  bindAddr;
            memset(&bindAddr, 0, sizeof(bindAddr));
            bindAddr.sin6_family = PF_INET6;
            bindAddr.sin6_addr = _config.mcast.intfAddr6;
            bindAddr.sin6_port = htons(_config.mcast.dstPort);
#ifndef __linux__
            bindAddr.sin6_len = sizeof(bindAddr);
#endif
            if (0 == bind(_fd6, (sockaddr *)&bindAddr, sizeof(bindAddr))) {
              rc = true;
              socklen_t  socklen = sizeof(bindAddr);
              getsockname(_fd6, (sockaddr *)&bindAddr, &socklen);
              MCLOG(Severity::info, "MulticastSender socket fd {} bound to {}",
                    _fd6, bindAddr);
            }
            else {
              MCLOG(Severity::err, "bind({},{},{}) failed: {}",
                    _fd6, bindAddr, sizeof(bindAddr), strerror(errno));
              ::close(_fd6);  _fd6 = -1;
            }
          }
          else {
            MCLOG(Severity::err, "setsockopt({},IPPROTO_IPV6,IPV6_MULTICAST_IF,"
                  "{},{}) failed: {}", _fd6, intfIndex, sizeof(intfIndex),
                  strerror(errno));
            ::close(_fd6);  _fd6 = -1;
          }
        }
        else {
          MCLOG(Severity::err,
                "interface index not found for interface '{}': {}",
                _config.mcast.intfName, strerror(errno));
          ::close(_fd6);  _fd6 = -1;
        }
      }
      else {
        MCLOG(Severity::err, "socket(PF_INET6,SOCK_DGRAM,0) failed: {}",
              strerror(errno));
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool MulticastSender::DesiredSocketsOpen() const
    {
      bool  v4ok =
        (_config.mcast.ShouldSendIpv4() ? (0 <= _fd) : (0 > _fd));
      bool  v6ok =
        (_config.mcast.ShouldSendIpv6() ? (0 <= _fd6) : (0 > _fd6));

      return (v4ok && v6ok);
    }
    
    //------------------------------------------------------------------------
    bool MulticastSender::Open(const Config & config)
    {
      bool  rc = false;
      _config = config;
      _dstEndpoint = UdpEndpoint(config.mcast.groupAddr, config.mcast.dstPort);
      _dstEndpoint6 = UdpEndpoint(config.mcast.groupAddr6, config.mcast.dstPort);
      if (! config.mcast.outFilter.empty()) {
        _filterDriver = std::make_unique<FilterDriver>(_config, config.mcast.outFilter);
      }
      else {
        _filterDriver = nullptr;
      }

      if (! _config.mcast.ShouldRunSender()) {
        return true;
      }
      
      if (_config.mcast.ShouldSendIpv4()) {
        if (0 > _fd) {
          if (! OpenSocket()) {
            MCLOG(Severity::err, "Failed to open IPv4 socket");
          }
        }
        else {
          MCLOG(Severity::err, "Ipv4 socket already open (fd {})", _fd);
        }
      }
      if (_config.mcast.ShouldSendIpv6()) {
        if (0 > _fd6) {
          if (! OpenSocket6()) {
            MCLOG(Severity::err, "Failed to open IPv4 socket");
          }
        }
        else {
          MCLOG(Severity::err, "Ipv6 socket already open (fd {})", _fd6);
        }
      }
      if (DesiredSocketsOpen()) {
        if (_keyRequestListener.Start(_fd, _fd6,
                                      &_config.service.keyDirectory,
                                      &_key)) {
          _run = true;
          _thread = std::thread(&MulticastSender::Run, this);
#if (defined(__FreeBSD__) || defined(__linux__))
          pthread_setname_np(_thread.native_handle(), "MulticastSender");
#endif
          rc = true;
        }
        else {
          MCLOG(Severity::err, "Failed to start KeyRequestListener");
        }
      }
      else {
        MCLOG(Severity::err,
               "Desired sockets not open, KEyRequestListener not started!");
      }
      
      return rc;
    }

    //------------------------------------------------------------------------
    bool MulticastSender::Restart(const Config & config)
    {
      Close();
      return Open(config);
    }
    
    //------------------------------------------------------------------------
    void MulticastSender::Close()
    {
      _keyRequestListener.Stop();
      _run = false;
      _outQueue.ConditionSignal();
      if (_thread.joinable()) { _thread.join();   }
      if (0 <= _fd)           { ::close(_fd);   _fd = -1;  }
      if (0 <= _fd6)          { ::close(_fd6);  _fd6 = -1; }
      return;
    }

    //------------------------------------------------------------------------
    bool MulticastSender::PassesFilter(const Message & msg)
    {
      if (nullptr == _filterDriver) {
        return true;
      }
      else {
        bool  filterPassed = false;
        if (_filterDriver->parse(&msg, filterPassed)) {
          return filterPassed;
        }
      }
      return false;
    }
    
    //------------------------------------------------------------------------
    //!  This function runs in the context of the caller; be aware of
    //!  the consequences of calling something inside here that is not
    //!  threadsafe.
    //------------------------------------------------------------------------
    bool MulticastSender::Process(const Message & msg)
    {
      if (PassesFilter(msg)) {
        return _outQueue.PushBack(msg);
      }
      return false;
    }
    
    //------------------------------------------------------------------------
    bool MulticastSender::SendPacket(MessagePacket & pkt)
    {
      ssize_t  ip4rc = -1, ip6rc = -1;
      if (pkt.Encrypt(_key)) {
        if (0 <= _fd)  { ip4rc = pkt.SendTo(_fd, _dstEndpoint);   }
        if (0 <= _fd6) { ip6rc = pkt.SendTo(_fd6, _dstEndpoint6); }
      }
      pkt.Reset();

      bool  rc = ((0 <= _fd) ? (ip4rc > 0) : true);
      rc &= ((0 <= _fd6) ? (ip6rc > 0) : true);
      return rc;
    }
    
    //------------------------------------------------------------------------
    void MulticastSender::Run()
    {
      MCLOG(Severity::info, "MulticastSender thread started");
#if (__APPLE__)
      pthread_setname_np("MulticastSender");
#endif
      char  buf[1200];
      MessagePacket  pkt(buf, sizeof(buf));
      Message  msg;
      while (_run) {
        if (_outQueue.ConditionTimedWait(std::chrono::seconds(1))) {
          auto  now = Clock::now();
          while (_outQueue.PopFront(msg)) {
            if (! pkt.Add(msg)) {
              if (! SendPacket(pkt)) {
                MCLOG(Severity::err, "SendPacket() failed");
              }
              _nextSendTime = now + std::chrono::milliseconds(1000);
              pkt.Add(msg);
            }
          }
        }
        else {
          auto  now = Clock::now();
          if (pkt.HasPayload()) {
            if (! SendPacket(pkt)) {
              MCLOG(Severity::err, "SendPacket() failed");
            }
            _nextSendTime = now + std::chrono::milliseconds(1000);
          }
        }
      }
      MCLOG(Severity::info, "MulticastSender thread done");
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
