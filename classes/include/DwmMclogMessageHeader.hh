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
//!  @file DwmMclogMessageHeader.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MessageHeader class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMESSAGEHEADER_HH_
#define _DWMMCLOGMESSAGEHEADER_HH_

#include "DwmMclogFacility.hh"
#include "DwmMclogMessageOrigin.hh"
#include "DwmMclogSeverity.hh"
#include "DwmMclogTimestamp.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a message header
    //------------------------------------------------------------------------
    class MessageHeader
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      MessageHeader() = default;

      //----------------------------------------------------------------------
      //!  Copy constructor
      //----------------------------------------------------------------------
      MessageHeader(const MessageHeader & hdr);

      //----------------------------------------------------------------------
      //!  Copy assignment operator
      //----------------------------------------------------------------------
      MessageHeader & operator = (const MessageHeader & hdr);
      
      //----------------------------------------------------------------------
      //!  Construct from the given @c facility, @c severity and @c origin.
      //----------------------------------------------------------------------
      MessageHeader(Facility facility, Severity severity,
                    const MessageOrigin & origin)
          : _timestamp(), _facility(facility), _severity(severity),
            _origin(origin)
      {}

      //----------------------------------------------------------------------
      //!  operator ==
      //----------------------------------------------------------------------
      bool operator == (const MessageHeader & hdr) const = default;
      
      //----------------------------------------------------------------------
      //!  Returns the timestamp.
      //----------------------------------------------------------------------
      const Timestamp & timestamp() const
      { return _timestamp; }

      //----------------------------------------------------------------------
      //!  Returns the facility.
      //----------------------------------------------------------------------
      Facility facility() const
      { return _facility; }

      //----------------------------------------------------------------------
      //!  Returns the severity.
      //----------------------------------------------------------------------
      Severity severity() const
      { return _severity; }
        
      //----------------------------------------------------------------------
      //!  Returns the origin.
      //----------------------------------------------------------------------
      const MessageOrigin & origin() const
      { return _origin; }
      
      //----------------------------------------------------------------------
      //!  Reads the header from the given istream @c is.  Returns @c is
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);

      //----------------------------------------------------------------------
      //!  Writes the header to the given ostream @c os.  Returns @c os
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

      //----------------------------------------------------------------------
      //!  Reads the header from the given BZFILE @c bzf.  Returns the number
      //!  of bytes read on success, -1 on failure.
      //----------------------------------------------------------------------
      int BZRead(BZFILE *bzf);

      //----------------------------------------------------------------------
      //!  Writes the header to the given BZFILE @c bzf.  Returns the number
      //!  of bytes written on success, -1 on failure.
      //----------------------------------------------------------------------
      int BZWrite(BZFILE *bzf) const;

      //----------------------------------------------------------------------
      //!  Reads the header from the given gzFile @c gzf.  Returns the number
      //!  of bytes read on success, -1 on failure.
      //----------------------------------------------------------------------
      int Read(gzFile gzf);

      //----------------------------------------------------------------------
      //!  Writes the header to the given gzFile @c gzf.  Returns the number
      //!  of bytes written on success, -1 on failure.
      //----------------------------------------------------------------------
      int Write(gzFile gzf) const;
      
      //----------------------------------------------------------------------
      //!  Returns the number of bytes that would be written if we called the
      //!  Write() member.
      //----------------------------------------------------------------------
      uint64_t StreamedLength() const;
      
      //----------------------------------------------------------------------
      //!  Prints a MessageHeader to an ostream in the form we use for
      //!  human consumption.
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const MessageHeader & hdr);

      //----------------------------------------------------------------------
      //!  Reads a message header from an istream in the form we use for
      //!  human consumption.
      //----------------------------------------------------------------------
      friend std::istream &
      operator >> (std::istream & is, MessageHeader & hdr);
      
    private:
      Timestamp                   _timestamp;
      Facility                    _facility;
      Severity                    _severity;
      MessageOrigin               _origin;
    };

  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMESSAGEHEADER_HH_
