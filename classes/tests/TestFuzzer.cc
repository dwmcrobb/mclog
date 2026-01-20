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
//!  @file FuzzTest.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/select.h>
}

#include <cstdlib>
#include <iostream>

#include "DwmIpv4Address.hh"

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void RandomFillBuffer(uint8_t *buf, size_t len)
{
  arc4random_buf(buf, len);
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool SendTo(int fd, const uint8_t *buf, size_t buflen, sockaddr_in *dst)
{
  auto  sendrc = sendto(fd, buf, buflen, 0, (sockaddr *)dst, sizeof(*dst));
  return (sendrc == buflen);
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool SendToMcastGroup(int fd, const uint8_t *buf, size_t buflen,
                             const Dwm::Ipv4Address & dst)
{
  sockaddr_in  dstAddr;
  memset(&dstAddr, 0, sizeof(dstAddr));
  dstAddr.sin_family = PF_INET;
  dstAddr.sin_addr.s_addr = dst.Raw();
  dstAddr.sin_port = htons(3456);
#ifndef __linux__
  dstAddr.sin_len = sizeof(dstAddr);
#endif

  return SendTo(fd, buf, buflen, &dstAddr);
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool SendToLocal(int fd, const uint8_t *buf, size_t buflen)
{
  sockaddr_in  dstAddr;
  memset(&dstAddr, 0, sizeof(dstAddr));
  dstAddr.sin_family = PF_INET;
  dstAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  dstAddr.sin_port = htons(3456);
#ifndef __linux__
  dstAddr.sin_len = sizeof(dstAddr);
#endif

  return SendTo(fd, buf, buflen, &dstAddr);
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool SendToMcastSource(int fd, const uint8_t *buf, size_t buflen,
                              const Dwm::Ipv4Address & intfAddr,
                              uint16_t port)
{
  sockaddr_in  dst;
  memset(&dst, 0, sizeof(dst));
  dst.sin_family = PF_INET;
  dst.sin_addr.s_addr = intfAddr.Raw();
  dst.sin_port = htons(port);
#ifndef __linux__
  dst.sin_len = sizeof(dst);
#endif

  return SendTo(fd, buf, buflen, &dst);
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  Dwm::Ipv4Address  intfAddr(argv[1]);
  Dwm::Ipv4Address  groupAddr(argv[2]);
  uint16_t          port = std::stoul(argv[3]);
  
  int  fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (0 <= fd) {
    in_addr  intfInAddr;
    intfInAddr.s_addr = intfAddr.Raw();
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF,
               &intfInAddr, sizeof(intfInAddr));
    int  ttl = 1;
    setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    uint8_t  buf[1400];
    for (size_t i = 1; i < sizeof(buf); i += 14) {
      RandomFillBuffer(buf, i);
      SendToMcastGroup(fd, buf, i, groupAddr);
      SendToLocal(fd, buf, i);
      SendToMcastSource(fd, buf, i, intfAddr, port);
      std::cerr << "Sent " << i << " bytes\n";
    }
  }
}
