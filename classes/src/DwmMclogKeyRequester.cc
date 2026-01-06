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
#include "spanstream"

#include "DwmCredenceKeyStash.hh"
#include "DwmCredenceKXKeyPair.hh"
#include "DwmMclogKeyRequester.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    std::string KeyRequester::GetKey()
    {
      std::string  rc;
      if (_port != 0) {
        _fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (0 <= _fd) {
          struct sockaddr_in  sockAddr;
          memset(&sockAddr, 0, sizeof(sockAddr));
#ifndef __linux__
          sockAddr.sin_len = sizeof(sockAddr);
#endif
          sockAddr.sin_family = AF_INET;
          sockAddr.sin_addr.s_addr = _servAddr.Raw();
          socklen_t  addrLen = sizeof(sockAddr);
          char  buf[1500] = {0};
          std::spanstream  ss{buf,sizeof(buf)};
          _state.KX().PublicKey().Write(ss);
          sockAddr.sin_port = htons(_port);
          ssize_t  sendrc = sendto(_fd, buf, ss.tellp(), 0,
                                   (const struct sockaddr *)&sockAddr,
                                   addrLen);
          if (sendrc == ss.tellp()) {
            _state.ChangeState(&KeyRequesterState::KXKeySent);
            while ((_state.CurrentState() != &KeyRequesterState::Success)
                   && (_state.CurrentState() != &KeyRequesterState::Failure)) {
              fd_set  fds;
              FD_ZERO(&fds);
              FD_SET(_fd, &fds);
              timeval  timeout = { 5, 0 };
              if (select(_fd+1, &fds, nullptr, nullptr, &timeout) > 0) {
                struct sockaddr_in  srcAddr;
                socklen_t           srcAddrLen = sizeof(srcAddr);
                ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                           (struct sockaddr *)&srcAddr,
                                           &srcAddrLen);
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
              rc = _state.McastKey();
            }
          }
          
          close(_fd);
          _fd = -1;
        }
      }
      return rc;
    }

  }  // namespace Mclog

}  // namespace Dwm
