//===========================================================================
// @(#) $DwmPath$
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
//!  @file mclog.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
}

#include <fstream>
#include <spanstream>

#include "DwmIpv4Address.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmStreamIO.hh"
#include "DwmMclogMessage.hh"
#include "DwmMclogMulticastReceiver.hh"

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
#if 1
  Dwm::Mclog::MulticastReceiver  mcastRecv;
  if (mcastRecv.Open(Dwm::Ipv4Address("224.225.226.227"),
                     Dwm::Ipv4Address("192.168.168.57"), 3456)) {
    Dwm::Thread::Queue<Dwm::Mclog::Message> msgQueue;
    mcastRecv.AddInputQueue(&msgQueue);

    for (;;) {
      msgQueue.WaitForNotEmpty();
      while (! msgQueue.Empty()) {
        Dwm::Mclog::Message  msg;
        msgQueue.PopFront(msg);
        std::cout << msg;
      }
    }
  }
  
#else
  std::ifstream  is("/tmp/mclogd.key");
  std::string    key;
  Dwm::StreamIO::Read(is, key);
  is.close();
  
  int  fd = socket(PF_INET, SOCK_DGRAM, 0);
  if (0 <= fd) {
    sockaddr_in  locAddr;
    memset(&locAddr, 0, sizeof(locAddr));
    locAddr.sin_family = PF_INET;
    locAddr.sin_port = htons(3456);
    locAddr.sin_addr.s_addr = inet_addr("224.225.226.227");
    if (::bind(fd, (sockaddr *)&locAddr, sizeof(locAddr)) == 0) {
      struct ip_mreq group;

      group.imr_multiaddr.s_addr = inet_addr("224.225.226.227");
      group.imr_interface.s_addr = inet_addr("192.168.168.57");
      if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group,
                     sizeof(group)) == 0) {
        fd_set       fds;
        sockaddr_in  fromAddr;
        timeval      tv;
        auto  reset_tv  = [&] () -> void
        { tv.tv_sec = 0; tv.tv_usec = 100000; };
        auto  reset_fds = [&] () -> void
        { FD_ZERO(&fds); FD_SET(fd, &fds); };
        for (;;) {
          reset_tv();
          reset_fds();
          if (select(fd+1, &fds, nullptr, nullptr, &tv) > 0) {
            if (FD_ISSET(fd, &fds)) {
              char  buf[1500];
              socklen_t  fromAddrLen = sizeof(fromAddr);
              ssize_t  recvrc = recvfrom(fd, buf, sizeof(buf), 0,
                                         (sockaddr *)&fromAddr, &fromAddrLen);
              if (recvrc > 0) {
                std::cerr << "Received " << recvrc << " bytes\n";
                Dwm::Credence::Nonce  nonce;
                std::spanstream  sps{std::span{buf,(size_t)recvrc}};
                if (nonce.Read(sps)) {
                  std::string  ciphertext(&(buf[0]) + sps.tellg(),
                                          recvrc - sps.tellg());
                  std::string  plaintext;
                  if (Dwm::Credence::XChaCha20Poly1305::Decrypt(plaintext,
                                                                ciphertext,
                                                                nonce, key)) {
                    std::spanstream  mss{std::span{plaintext.data(), plaintext.size()}};
                    Dwm::Mclog::Message  msg;
                    while (msg.Read(mss)) {
                      std::cout << msg;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    else {
      std::cerr << "bind() failed\n";
    }
    
  }
#endif
  return 0;
}

          
          
