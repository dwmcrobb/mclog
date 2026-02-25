//===========================================================================
//  Copyright (c) Daniel W. McRobb 2025, 2026
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
//!  @file DwmMclogMessage.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Message implementation
//---------------------------------------------------------------------------

#include <algorithm>
#include <array>
#include <iomanip>
#include <iterator>

#include "DwmIOUtils.hh"
#include "DwmStreamIO.hh"
#include "DwmMclogMessage.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    Message::Message()
        : _header(), _message()
    {}

    //------------------------------------------------------------------------
    Message::Message(const MessageHeader & header, std::string && message)
        : _header(header), _message(message)
    {}

    //------------------------------------------------------------------------
    Message::Message(const MessageHeader & header, const std::string & message)
        : _header(header), _message(message)
    {}

    //------------------------------------------------------------------------
    std::istream & Message::Read(std::istream & is)
    {
      if (_header.Read(is)) {
        StreamIO::Read(is, _message);
      }
      return is;
    }
     
    //------------------------------------------------------------------------
    std::ostream & Message::Write(std::ostream & os) const
    {
      if (_header.Write(os)) {
        StreamIO::Write(os, _message);
      }
      return os;
    }

    //------------------------------------------------------------------------
    int Message::BZRead(BZFILE *bzf)
    { return BZ2IO::BZReadV(bzf, _header, _message); }
    
    //------------------------------------------------------------------------
    int Message::BZWrite(BZFILE *bzf) const
    { return BZ2IO::BZWriteV(bzf, _header, _message); }

    //------------------------------------------------------------------------
    int Message::Read(gzFile gzf)
    { return GZIO::ReadV(gzf, _header, _message); }

    //------------------------------------------------------------------------
    int Message::Write(gzFile gzf) const
    { return GZIO::WriteV(gzf, _header, _message); }
    
    //------------------------------------------------------------------------
    uint64_t Message::StreamedLength() const
    {
      return (_header.StreamedLength()
              + IOUtils::StreamedLength(_message));
    }
    
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Message & msg)
    {
      os << msg._header << ' ' << msg._message << '\n';
      return os;
    }

    //------------------------------------------------------------------------
    std::istream & operator >> (std::istream & is, Message & msg)
    {
      if (is >> msg._header) {
        is >> std::ws;
        std::string  s;
        std::getline(is, s);
        if (s.size() <= 1500) {
          msg._message = s;
        }
        else {
          is.setstate(std::ios_base::failbit);
        }
      }
      return is;
    }
      
  }  // namespace Mclog

}  // namespace Dwm
