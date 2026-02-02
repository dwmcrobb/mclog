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
      return ((0 <= _fd) || (0 <= _fd6));
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
    bool KeyRequestListener::Start(int fd, int fd6, const std::string *keyDir,
                                   const std::string *mcastKey)
    {
      assert((0 <= fd) || (0 <= fd6));
      assert(mcastKey->size() == crypto_aead_xchacha20poly1305_ietf_KEYBYTES);
      
      if (! _run) {
        _keyDir = keyDir;
        _mcastKey = mcastKey;
        if (0 == pipe(_stopfds)) {
          _fd = fd;
          _fd6 = fd6;
          _run = true;
          _thread = std::thread(&KeyRequestListener::Run, this);
#if (defined(__FreeBSD__) || defined (__linux__))
          pthread_setname_np(_thread.native_handle(), "KeyReqListener");
#endif
          return true;
        }
      }
      return false;
    }
    
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
    void KeyRequestListener::Run()
    {
      Syslog(LOG_INFO, "KeyRequestListener thread started");
#if (__APPLE__)
      pthread_setname_np("KeyRequestListener");
#endif 
      if (Listen()) {
        while (_run) {
          fd_set  fds;
          FD_ZERO(&fds);
          if (0 <= _fd) { FD_SET(_fd, &fds); }
          if (0 <= _fd6)  { FD_SET(_fd6, &fds); }
          FD_SET(_stopfds[0], &fds);
          int  maxfd = std::max({_fd, _fd6, _stopfds[0]}) + 1;
          int  selectrc = select(maxfd, &fds, nullptr, nullptr, nullptr);
          if (selectrc > 0) {
            if (FD_ISSET(_stopfds[0], &fds)) {
              break;
            }
            if ((0 <= _fd) && FD_ISSET(_fd, &fds)) {
              struct sockaddr_in  clientAddr;
              socklen_t           clientAddrLen = sizeof(clientAddr);
              char  buf[1500];
              ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                         (struct sockaddr *)&clientAddr,
                                         &clientAddrLen);
              if (recvrc) {
                std::string  s(buf, recvrc);
                UdpEndpoint krcAddr(clientAddr);
                auto [clientit, dontCare] =
                  _clients.insert({krcAddr,KeyRequestClientState(_keyDir, _mcastKey)});
                if (clientit->second.ProcessPacket(_fd, krcAddr, buf, recvrc)) {
                  if (clientit->second.Success()) {
                    _clientsDone.push_back(*clientit);
                    _clients.erase(clientit);
                    FSyslog(LOG_DEBUG, "_clientsDone.size(): {}",
                            _clientsDone.size());
                  }
                }
              }
              else {
                FSyslog(LOG_ERR, "recvfrom({}) failed: {}", _fd, strerror(errno));
              }
            }
            if ((0 <= _fd6) && FD_ISSET(_fd6, &fds)) {
              struct sockaddr_in6  clientAddr;
              socklen_t            clientAddrLen = sizeof(clientAddr);
              char  buf[1500];
              ssize_t  recvrc = recvfrom(_fd6, buf, sizeof(buf), 0,
                                         (struct sockaddr *)&clientAddr,
                                         &clientAddrLen);
              if (recvrc > 0) {
                std::string  s(buf, recvrc);
                UdpEndpoint  krcAddr(clientAddr);
                auto [clientit, dontCare] =
                  _clients.insert({krcAddr,KeyRequestClientState(_keyDir, _mcastKey)});
                if (clientit->second.ProcessPacket(_fd6, krcAddr, buf, recvrc)) {
                  if (clientit->second.Success()) {
                    _clientsDone.push_back(*clientit);
                    _clients.erase(clientit);
                    FSyslog(LOG_DEBUG, "_clientsDone.size(): {}",
                            _clientsDone.size());
                  }
                }
              }
              else {
                FSyslog(LOG_ERR, "recvfrom({}) failed: {}",
                        _fd6, strerror(errno));
              }
            }
            
          }
          ClearExpired();
        }
      }
      Syslog(LOG_INFO, "KeyRequestListener thread done");
      return;
    }
    
  }  // namespace Mclog
    
}  // namespace Dwm
