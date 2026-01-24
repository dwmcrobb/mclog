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
//!  @file DwmMclogMessagePacket.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sodium.h>
}

#include <cassert>
#include <cstring>

#include "DwmFormatters.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    bool MessagePacket::Encrypt(const std::string & secretKey)
    {
      constexpr auto  xcc20p1305enc =
        crypto_aead_xchacha20poly1305_ietf_encrypt;
      randombytes_buf(_buf, k_nonceLen);
      unsigned long long  cbuflen = _payloadLength + k_macLen;
      if (xcc20p1305enc((uint8_t *)_buf + k_nonceLen, &cbuflen,
                        (const uint8_t *)_buf + k_nonceLen, _payloadLength,
                        nullptr, 0,
                        nullptr, (const uint8_t *)_buf,  // nonce
                        (const uint8_t *)secretKey.data()) == 0) {
        return ((_payloadLength + k_macLen) == cbuflen);
      }
      return false;
    }

    //------------------------------------------------------------------------
    void MessagePacket::Reset()
    {
      _payload.seekp(0);
      _payloadLength = 0;
      return;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    ssize_t MessagePacket::SendTo(int fd, const std::string & secretKey,
                                  const UdpEndpoint & dst)
    {
      ssize_t        rc = -1;
      if (Encrypt(secretKey)) {
        size_t  len = k_nonceLen + k_macLen + _payloadLength;
        if (dst.Addr().Family() == PF_INET) {
          sockaddr_in  dstSock = dst;
          rc = sendto(fd, _buf, len, 0, (const sockaddr *)&dstSock,
                      sizeof(dstSock));
        }
        else {
          sockaddr_in6  dstSock = dst;
          rc = sendto(fd, _buf, len, 0, (const sockaddr *)&dstSock,
                      sizeof(dstSock));
        }
        if (rc != len) {
          FSyslog(LOG_ERR, "sendto() failed: {} ({})",
                  strerror(errno), errno);
        }
      }
      else {
        Syslog(LOG_ERR, "Encryption failed!!!");
      }
      return rc;
    }

    //------------------------------------------------------------------------
    ssize_t MessagePacket::SendTo(int fd, const UdpEndpoint & dst)
    {
      ssize_t        rc = -1;
      size_t  len = k_nonceLen + k_macLen + _payloadLength;
      if (dst.Addr().Family() == PF_INET) {
        sockaddr_in  dstSock = dst;
        rc = sendto(fd, _buf, len, 0, (const sockaddr *)&dstSock,
                    sizeof(dstSock));
      }
      else {
        sockaddr_in6  dstSock = dst;
        rc = sendto(fd, _buf, len, 0, (const sockaddr *)&dstSock,
                    sizeof(dstSock));
      }
      if (rc != len) {
        FSyslog(LOG_ERR, "sendto() failed: {} ({})",
                strerror(errno), errno);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    ssize_t MessagePacket::Decrypt(size_t recvlen,
                                   const std::string & secretKey)
    {
      ssize_t  rc = -1;
      if (recvlen > (k_nonceLen + k_macLen)) {
        constexpr auto  xcc20p1305dec =
          crypto_aead_xchacha20poly1305_ietf_decrypt;
        unsigned long long plainLen = recvlen - (k_nonceLen + k_macLen);
        if (xcc20p1305dec((uint8_t *)_buf + k_nonceLen,
                          &plainLen, nullptr,
                          (const uint8_t *)_buf + k_nonceLen,
                          recvlen - k_nonceLen,
                          nullptr, 0,
                          (const uint8_t *)_buf,
                          (const uint8_t *)secretKey.data()) == 0) {
          rc = recvlen;
          _payloadLength = plainLen;
          _payload = std::spanstream{std::span{_buf + k_nonceLen,              
                                     _payloadLength}};
        }
        else {
          _payload = std::spanstream{std::span{_buf + k_nonceLen,0}};
          _payloadLength = 0;
        }
      }
      else {
        _payload = std::spanstream{std::span{_buf + k_nonceLen,0}};
        _payloadLength = 0;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    ssize_t MessagePacket::RecvFrom(int fd, const std::string & secretKey,
                                    sockaddr_in *src)
    {
      ssize_t  rc = -1;
      _payloadLength = 0;
      socklen_t  srclen = sizeof(*src);
      ssize_t  recvrc = recvfrom(fd, _buf, _buflen, 0,
                                 (sockaddr *)src, &srclen);
      if (recvrc > 0) {
        rc = 0;
        if (recvrc > k_minPacketLen) {
          rc = Decrypt(recvrc, secretKey);
        }
        else {
          _payload = std::spanstream{std::span{_buf,0}};
        }
      }
      else {
        _payload = std::spanstream{std::span{_buf,0}};
      }
      return rc;
    }

    //------------------------------------------------------------------------
    ssize_t MessagePacket::RecvFrom(int fd, const std::string & secretKey,
                                    sockaddr_in6 *src)
    {
      ssize_t  rc = -1;
      _payloadLength = 0;
      socklen_t  srclen = sizeof(*src);
      ssize_t  recvrc = recvfrom(fd, _buf, _buflen, 0,
                                 (sockaddr *)src, &srclen);
      if (recvrc > 0) {
        rc = 0;
        if (recvrc > k_minPacketLen) {
          rc = Decrypt(recvrc, secretKey);
        }
        else {
          _payload = std::spanstream{std::span{_buf,0}};
        }
      }
      else {
        _payload = std::spanstream{std::span{_buf,0}};
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    ssize_t MessagePacket::RecvFrom(int fd, sockaddr_in *src)
    {
      ssize_t  rc = -1;
      _payloadLength = 0;
      socklen_t  srclen = sizeof(*src);
      ssize_t  recvrc = recvfrom(fd, _buf, _buflen, 0,
                                 (sockaddr *)src, &srclen);
      if (recvrc > 0) {
        rc = 0;
        if (recvrc > k_minPacketLen) {
          unsigned long long plainLen = recvrc - (k_nonceLen + k_macLen);
          rc = recvrc;
          _payloadLength = plainLen;
          _payload = std::spanstream{std::span{_buf + k_nonceLen,              
                                     _payloadLength}};
        }
        else {
          _payload = std::spanstream{std::span{_buf,0}};
        }
      }
      else {
        _payload = std::spanstream{std::span{_buf,0}};
      }
      return rc;
    }

    //------------------------------------------------------------------------
    ssize_t MessagePacket::RecvFrom(int fd, sockaddr_in6 *src)
    {
      ssize_t  rc = -1;
      _payloadLength = 0;
      socklen_t  srclen = sizeof(*src);
      ssize_t  recvrc = recvfrom(fd, _buf, _buflen, 0,
                                 (sockaddr *)src, &srclen);
      if (recvrc > 0) {
        rc = 0;
        if (recvrc > k_minPacketLen) {
          unsigned long long plainLen = recvrc - (k_nonceLen + k_macLen);
          rc = recvrc;
          _payloadLength = plainLen;
          _payload = std::spanstream{std::span{_buf + k_nonceLen,              
                                     _payloadLength}};
        }
        else {
          _payload = std::spanstream{std::span{_buf,0}};
        }
      }
      else {
        _payload = std::spanstream{std::span{_buf,0}};
      }
      return rc;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
