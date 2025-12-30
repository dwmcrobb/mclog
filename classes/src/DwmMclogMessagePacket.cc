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

#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    ssize_t MessagePacket::SendTo(int fd, const std::string & secretKey,
                                  struct sockaddr *dst, socklen_t dstlen)
    {
      constexpr auto  xcc20p1305enc =
        crypto_aead_xchacha20poly1305_ietf_encrypt;
      ssize_t        rc = -1;
      const size_t   nonceLen = crypto_secretbox_NONCEBYTES;
      randombytes_buf(_buf, nonceLen);
      const size_t   macLen = crypto_aead_xchacha20poly1305_ietf_ABYTES;
      unsigned long long  cbuflen = (size_t)_payload.tellp() + macLen;
      if (xcc20p1305enc((uint8_t *)_buf + nonceLen, &cbuflen,
                        (const uint8_t *)_buf + nonceLen,
                        (size_t)_payload.tellp(),
                        nullptr, 0,
                        nullptr, (const uint8_t *)_buf,  // nonce
                        (const uint8_t *)secretKey.data()) == 0) {
        rc = sendto(fd, _buf, nonceLen + cbuflen, 0, dst, dstlen);
      }
      else {
        std::cerr << "Encryption failed!!!\n";
      }
      _payload.seekp(0);
      return rc;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
