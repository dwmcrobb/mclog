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
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMESSAGEPACKET_HH_
#define _DWMMCLOGMESSAGEPACKET_HH_

extern "C" {
  #include <sys/socket.h>
}

#include <span>
#include <spanstream>

#include "DwmStreamIO.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class MessagePacket
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      MessagePacket(char *buf, size_t buflen)
          : _buf(buf), _buflen(buflen), _payload{std::span{buf+24,buflen-24}}
      { assert(_buf && (_buflen > 40)); }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      template <typename T>
      bool Add(const T & t)
      {
        auto  prevPos = _payload.tellp();
        if (StreamIO::Write(_payload, t)) {
          std::cerr << "Added message to packet\n";
          return true;
        }
        std::cerr << "Failed to add message to packet\n";
        _payload.clear();
        _payload.seekp(prevPos);
        return false;
      }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      ssize_t SendTo(int fd, const std::string & secretKey,
                     struct sockaddr *dst, socklen_t dstlen);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool HasPayload()
      { return _payload.tellp() != 0; }
        
    private:
      char             *_buf;
      size_t            _buflen;
      std::spanstream   _payload;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMESSAGEPACKET_HH_
