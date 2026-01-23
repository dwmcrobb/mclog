//===========================================================================
//  Copyright (c) Daniel W. McRobb 2026
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
//!  @file DwmMclogKeyRequester.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/select.h>
  #include <sys/time.h>
}

#include <iostream>

#include "DwmFormatters.hh"
#include "DwmCredenceKeyStash.hh"
#include "DwmCredenceKXKeyPair.hh"
#include "DwmMclogKeyRequester.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MulticastSourceKey KeyRequester::GetKey4()
    {
      MulticastSourceKey  rc;
      rc.LastRequested(std::chrono::system_clock::now());
      _fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (0 <= _fd) {
        struct sockaddr_in  dstAddr = _servEndpoint;
        socklen_t  addrLen = sizeof(dstAddr);
        char  buf[1500] = {0};
        std::spanstream  ss{std::span{buf,sizeof(buf)}};
        _state.KX().PublicKey().Write(ss);
        ssize_t  sendrc = sendto(_fd, buf, ss.tellp(), 0,
                                 (const sockaddr *)&dstAddr, addrLen);
        if (sendrc == ss.tellp()) {
          _state.ChangeState(&KeyRequesterState::KXKeySent, _servEndpoint);
          while ((_state.CurrentState() != &KeyRequesterState::Success)
                 && (_state.CurrentState() != &KeyRequesterState::Failure)) {
            fd_set  fds;
            FD_ZERO(&fds);
            FD_SET(_fd, &fds);
            timeval  timeout = { 1, 0 };
            if (select(_fd+1, &fds, nullptr, nullptr, &timeout) > 0) {
              struct sockaddr_in  srcAddr;
              socklen_t           srcAddrLen = sizeof(srcAddr);
              ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                         (sockaddr *)&srcAddr, &srcAddrLen);
              if (recvrc > 0) {
                if (! _state.ProcessPacket(_fd, srcAddr, buf, recvrc)) {
                  break;
                }
              }
            }
            else {
              break;
            }
          }
          if (_state.CurrentState() == &KeyRequesterState::Success) {
            rc.LastUpdated(std::chrono::system_clock::now());
            rc.Value(_state.McastKey());
          }
        }
        else {
          FSyslog(LOG_ERR, "sendto({},{}) failed", _fd, dstAddr);
        }
        close(_fd);
        _fd = -1;
      }
      else {
        Syslog(LOG_ERR, "Failed to open socket");
      }

      return rc;
    }

    //------------------------------------------------------------------------
    MulticastSourceKey KeyRequester::GetKey6()
    {
      MulticastSourceKey  rc;
      rc.LastRequested(std::chrono::system_clock::now());
      _fd6 = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
      if (0 <= _fd6) {
        struct sockaddr_in6  dstAddr = _servEndpoint;
        socklen_t  addrLen = sizeof(dstAddr);
        char  buf[1500] = {0};
        std::spanstream  ss{std::span{buf,sizeof(buf)}};
        _state.KX().PublicKey().Write(ss);
        ssize_t  sendrc = sendto(_fd6, buf, ss.tellp(), 0,
                                 (const sockaddr *)&dstAddr, addrLen);
        if (sendrc == ss.tellp()) {
          _state.ChangeState(&KeyRequesterState::KXKeySent, _servEndpoint);
          while ((_state.CurrentState() != &KeyRequesterState::Success)
                 && (_state.CurrentState() != &KeyRequesterState::Failure)) {
            fd_set  fds;
            FD_ZERO(&fds);
            FD_SET(_fd6, &fds);
            timeval  timeout = { 1, 0 };
            if (select(_fd6+1, &fds, nullptr, nullptr, &timeout) > 0) {
              struct sockaddr_in6  srcAddr;
              socklen_t            srcAddrLen = sizeof(srcAddr);
              ssize_t  recvrc = recvfrom(_fd6, buf, sizeof(buf), 0,
                                         (sockaddr *)&srcAddr, &srcAddrLen);
              if (recvrc > 0) {
                if (! _state.ProcessPacket(_fd6, srcAddr, buf, recvrc)) {
                  break;
                }
              }
            }
            else {
              break;
            }
          }
          if (_state.CurrentState() == &KeyRequesterState::Success) {
            rc.LastUpdated(std::chrono::system_clock::now());
            rc.Value(_state.McastKey());
          }
        }
        else {
          Syslog(LOG_ERR, "sendto({},{}) failed", _fd6, dstAddr);
        }
        close(_fd6);
        _fd6 = -1;
      }
      else {
        Syslog(LOG_ERR, "Failed to open socket");
      }

      return rc;
    }
    
    //------------------------------------------------------------------------
    MulticastSourceKey KeyRequester::GetKey()
    {
      if (_servEndpoint.Addr().Family() == PF_INET) {
        return GetKey4();
      }
      else {
        return GetKey6();
      }
    }

  }  // namespace Mclog

}  // namespace Dwm
