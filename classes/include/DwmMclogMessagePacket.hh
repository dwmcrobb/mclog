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
//!  @file DwmMclogMessagePacket.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MessagePacket class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMESSAGEPACKET_HH_
#define _DWMMCLOGMESSAGEPACKET_HH_

extern "C" {
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sodium.h>
}

#include <span>
#include <version>

#if defined(__cpp_lib_spanstream)
#  if (__cpp_lib_spanstream >= 202106L)
#    if __has_include(<spanstream>)
#      include <spanstream>
#      define DWM_HAVE_STD_SPANSTREAM 1
#    endif
#  endif
#endif

#ifndef DWM_HAVE_STD_SPANSTREAM
#  include "spanstream.hh"
#endif

#include "DwmStreamIO.hh"
#include "DwmMclogUdpEndpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a buffer containing one or more messages, in the form
    //!  used for network transport (including bytes needed for encryption
    //!  and AEAD).  Note that MessagePacket does NOT own the buffer and
    //!  does not participate in the lifetime of the buffer.
    //------------------------------------------------------------------------
    class MessagePacket
    {
    public:
      static const size_t k_nonceLen = crypto_secretbox_NONCEBYTES;
      static const size_t k_macLen = crypto_aead_xchacha20poly1305_ietf_ABYTES;
      static const size_t k_minPacketLen = k_nonceLen + k_macLen;

      //----------------------------------------------------------------------
      //!  Construct from the buffer @c buf of length @c buflen.  Note that
      //!  MessagePacket does NOT participate in the lifetime of the buffer;
      //!  the caller is responsible for ensuring it remains valid as long
      //!  as it might be referenced by the MessagePacket.
      //----------------------------------------------------------------------
      MessagePacket(char *buf, size_t buflen)
          : _buf(buf), _buflen(buflen),
            _payload{std::span{buf + k_nonceLen,
                               buflen - (k_nonceLen + k_macLen)}},
            _payloadLength(0)
      { assert(_buf && (_buflen > k_minPacketLen)); }

      //----------------------------------------------------------------------
      //!  If it will fit, @c t is appended to the payload and @c true is
      //!  returned.  If it will not fit, @c t is not appended and @c false
      //!  is returned.
      //----------------------------------------------------------------------
      template <typename T>
      bool Add(const T & t)
      {
        bool  rc = false;
        auto  prevPos = _payload.tellp();
        if (StreamIO::Write(_payload, t)) {
          _payloadLength = _payload.tellp();
          rc = true;
        }
        else {
          _payload.clear();
          _payload.seekp(prevPos);
        }
        _payloadLength = _payload.tellp();
        return rc;
      }

      //----------------------------------------------------------------------
      //!  Send the packet to the given destination @c dst via the given
      //!  descriptor @c fd, using the given @c secretKey to encrypt the
      //!  packet.  Returns the number of bytes sent (positive on success).
      //----------------------------------------------------------------------
      ssize_t SendTo(int fd, const std::string & secretKey,
                     const UdpEndpoint & dst);

      //----------------------------------------------------------------------
      //!  Send the packet to the given destination @c dst via the given
      //!  descriptor @c fd, unencrypted.  This is used by local loggers
      //!  sending to the loopback address.  Returns the number of bytes sent
      //!  (positive on success).
      //----------------------------------------------------------------------
      ssize_t SendTo(int fd, const UdpEndpoint & dst);

      //----------------------------------------------------------------------
      //!  Receives the packet via the given descriptor @c fd, decrypts it
      //!  using the given @c secretKey and populates @c src with the source
      //!  of the packet.  Returns the number of bytes received (positive on
      //!  success).
      //----------------------------------------------------------------------
      ssize_t RecvFrom(int fd, const std::string & secretKey,
                       struct sockaddr_in *src);

      //----------------------------------------------------------------------
      //!  Receives the packet via the given descriptor @c fd, decrypts it
      //!  using the given @c secretKey and populates @c src with the source
      //!  of the packet.  Returns the number of bytes received (positive on
      //!  success).
      //----------------------------------------------------------------------
      ssize_t RecvFrom(int fd, const std::string & secretKey,
                       struct sockaddr_in6 *src);
      
      //----------------------------------------------------------------------
      //!  Receives the packet (unencrypted) via the given descriptor @c fd
      //!  and populates @c src with the source of the packet.  Returns the
      //!  number of bytes received (positive on success).
      //----------------------------------------------------------------------
      ssize_t RecvFrom(int fd, struct sockaddr_in *src);

      //----------------------------------------------------------------------
      //!  Receives the packet (unencrypted) via the given descriptor @c fd
      //!  and populates @c src with the source of the packet.  Returns the
      //!  number of bytes received (positive on success).
      //----------------------------------------------------------------------
      ssize_t RecvFrom(int fd, struct sockaddr_in6 *src);

      //----------------------------------------------------------------------
      //!  Encrypts the packet with the given @c secretKey.  Returns true
      //!  on success, false on failure.
      //----------------------------------------------------------------------
      bool Encrypt(const std::string & secretKey);

      //----------------------------------------------------------------------
      //!  Clears the payload of the packet.
      //----------------------------------------------------------------------
      void Reset();
      
      //----------------------------------------------------------------------
      //!  Decrypts the packet (in-place) of length @c recvlen using the
      //!  given @c secretKey.  Returns the number of bytes decrypted
      //!  (@c recvlen on success, -1 on failure).
      //----------------------------------------------------------------------
      ssize_t Decrypt(size_t recvlen, const std::string & secretKey);
      
      //----------------------------------------------------------------------
      //!  Returns true if the packet has a non-empty payload.
      //----------------------------------------------------------------------
      bool HasPayload()
      { return _payloadLength > 0; }

      //----------------------------------------------------------------------
      //!  Returns the payload.
      //----------------------------------------------------------------------
      std::spanstream & Payload()
      { return _payload; }
      
    private:
      char             *_buf;
      size_t            _buflen;
      std::spanstream   _payload;
      size_t            _payloadLength;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMESSAGEPACKET_HH_
