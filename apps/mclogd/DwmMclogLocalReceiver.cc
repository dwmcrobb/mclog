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
//!  @file DwmMclogLocalReceiver.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
  #include <unistd.h>
}

#include "DwmIpv4Address.hh"
#include "DwmMclogLocalReceiver.hh"
#include "DwmMclogMessage.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool LocalReceiver::Start(Thread::Queue<Message> *msgQueue)
    {
      _run = true;
      _msgQueue = msgQueue;
      if (0 == pipe(_stopfds)) {
        _thread = std::thread(&LocalReceiver::Run, this);
        return true;
      }
      return false;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void LocalReceiver::Stop()
    {
      _run = false;
      char  stop = 's';
      ::write(_stopfds[1], &stop, sizeof(stop));
      if (_thread.joinable()) {
        _thread.join();
        ::close(_stopfds[1]);  _stopfds[1] = -1;
        ::close(_stopfds[0]);  _stopfds[0] = -1;
      }
      return;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void LocalReceiver::Run()
    {
      _ifd = socket(PF_INET, SOCK_DGRAM, 0);
      if (0 <= _ifd) {
        struct sockaddr_in  sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sin_family = PF_INET;
        sockAddr.sin_addr.s_addr = Ipv4Address("127.0.0.1").Raw();
        sockAddr.sin_port = htons(3455);
        if (bind(_ifd, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == 0) {
          timeval  tv = { 0, 100000 };
          fd_set   fds;
          sockaddr_in  fromAddr;
          auto  reset_tv  = [&] () -> void
          { tv.tv_sec = 10; tv.tv_usec = 0; };
          auto  reset_fds = [&] () -> void
          { FD_ZERO(&fds); FD_SET(_ifd, &fds); FD_SET(_stopfds[0], &fds); };
          while (_run) {
            reset_tv();
            reset_fds();
            int selectrc = select(std::max(_ifd, _stopfds[0]) + 1, &fds,
                                  nullptr, nullptr, &tv);
            if (selectrc > 0) {
              char  buf[1500];
              socklen_t    fromAddrLen = sizeof(fromAddr);
              ssize_t  recvrc = recvfrom(_ifd, buf, sizeof(buf), 0,
                                         (struct sockaddr *)&fromAddr,
                                         &fromAddrLen);
              if (recvrc > 0) {
                std::spanstream  sps{std::span{buf, (size_t)recvrc}};
                Message  msg;
                while (msg.Read(sps)) {
                  // std::cerr << msg;
                  _msgQueue->PushBack(msg);
                }
              }
            }
          }
        }
        close(_ifd);
        _ifd = -1;
      }
      _run = false;
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
