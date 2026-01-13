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
//!  @file DwmMcLogKeyRequestListener.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sodium.h>
}

#include <cassert>
#include <iostream>

#include "DwmSysLogger.hh"
#include "DwmMclogKeyRequestListener.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    KeyRequestListener::~KeyRequestListener()
    {
      Stop();
    }
    
    //------------------------------------------------------------------------
    bool KeyRequestListener::Listen()
    {
      bool  rc = false;
      
      int  fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (fd >= 0) {
        sockaddr_in  sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
#ifndef __linux__
        sockAddr.sin_len = sizeof(sockAddr);
#endif
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_addr.s_addr = _addr.Raw();
        sockAddr.sin_port = htons(_port);
        socklen_t  addrLen = sizeof(sockAddr);
        if (::bind(fd, (const struct sockaddr *)&sockAddr, addrLen) == 0) {
          _fd = fd;
          int  ttl = 1;
          setsockopt(_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
          rc = true;
          Syslog(LOG_INFO, "KeyRequestListener bound to %s:%hu",
                 inet_ntoa(sockAddr.sin_addr), _port);
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    void KeyRequestListener::ClearExpired()
    {
      time_t  expTime = time((time_t *)0) - 5;
      auto  expired =
        [&] (auto && c) { return (c.second.LastStateChangeTime() < expTime); };
      
      if (! _clientsDone.empty()) { std::erase_if(_clientsDone, expired); }
      if (! _clients.empty())     { std::erase_if(_clients, expired); }
      
      return;
    }
    
    //------------------------------------------------------------------------
    void KeyRequestListener::ProcessPackets()
    {
      fd_set  fds;
      FD_ZERO(&fds);
      FD_SET(_fd, &fds);
      struct timeval  timeout = { 0, 100000 };
      int  selectrc = select(_fd + 1, &fds,
                             nullptr, nullptr, &timeout);
      if (selectrc > 0) {
        struct sockaddr_in  clientAddr;
        socklen_t           clientAddrLen = sizeof(clientAddr);
        if (FD_ISSET(_fd, &fds)) {
          char  buf[1500];
          ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                     (struct sockaddr *)&clientAddr,
                                     &clientAddrLen);
          if (recvrc) {
            std::string  s(buf, recvrc);
            KeyRequestClientAddr
              krcAddr(Ipv4Address(clientAddr.sin_addr.s_addr),
                      ntohs(clientAddr.sin_port));
            auto [clientit, dontCare] =
              _clients.insert({krcAddr,KeyRequestClientState(_keyDir, _mcastKey)});
            if (clientit->second.ProcessPacket(_fd, clientAddr, buf, recvrc)) {
              if (clientit->second.Success()) {
                _clientsDone.push_back(*clientit);
                _clients.erase(clientit);
                std::cerr << "_clientsDone.size(): " << _clientsDone.size()
                          << '\n';
              }
            }
          }
        }
      }
      ClearExpired();
      
      return;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool KeyRequestListener::Start(const Ipv4Address & addr, uint16_t port,
                                   const std::string *keyDir,
                                   const std::string *mcastKey)
    {
      assert(nullptr != mcastKey);
      assert(mcastKey->size()
             == crypto_aead_xchacha20poly1305_ietf_KEYBYTES);

      if (! _run) {
        _addr = addr;
        _port = port;
        _keyDir = keyDir;
        _mcastKey = mcastKey;
        if (0 == pipe(_stopfds)) {
          _run = true;
          _thread = std::thread(&KeyRequestListener::Run, this);
          return true;
        }
      }
      return false;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool KeyRequestListener::Stop()
    {
      if (_run) {
        _run = false;
        char  stop = 's';
        ::write(_stopfds[1], &stop, sizeof(stop));
        if (_thread.joinable()) {
          _thread.join();
        }
        ::close(_stopfds[1]);  _stopfds[1] = -1;
        ::close(_stopfds[0]);  _stopfds[0] = -1;
        return true;
      }
      return false;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void KeyRequestListener::Run()
    {
      if (Listen()) {
        while (_run) {
          fd_set  fds;
          FD_ZERO(&fds);
          FD_SET(_fd, &fds);
          FD_SET(_stopfds[0], &fds);
          struct timeval  timeout = { 10, 0 };
          int  selectrc = select(std::max(_fd, _stopfds[0]) + 1, &fds,
                                 nullptr, nullptr, &timeout);
          if (selectrc > 0) {
            if (FD_ISSET(_stopfds[0], &fds)) {
              break;
            }
            struct sockaddr_in  clientAddr;
            socklen_t           clientAddrLen = sizeof(clientAddr);
            if (FD_ISSET(_fd, &fds)) {
              char  buf[1500];
              ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                         (struct sockaddr *)&clientAddr,
                                         &clientAddrLen);
              if (recvrc) {
                std::string  s(buf, recvrc);
                KeyRequestClientAddr
                  krcAddr(Ipv4Address(clientAddr.sin_addr.s_addr),
                          ntohs(clientAddr.sin_port));
                auto [clientit, dontCare] =
                  _clients.insert({krcAddr,KeyRequestClientState(_keyDir, _mcastKey)});
                if (clientit->second.ProcessPacket(_fd, clientAddr, buf, recvrc)) {
                  if (clientit->second.Success()) {
                    _clientsDone.push_back(*clientit);
                    _clients.erase(clientit);
                    std::cerr << "_clientsDone.size(): " << _clientsDone.size()
                              << '\n';
                  }
                }
              }
            }
          }
          ClearExpired();
          //          ProcessPackets();
        }
        ::close(_fd);
        _fd = -1;
      }
      return;
    }
    
  }  // namespace Mclog
    
}  // namespace Dwm
