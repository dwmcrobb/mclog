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
//!  @file DwmMclogMessage.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMESSAGE_HH_
#define _DWMMCLOGMESSAGE_HH_

extern "C" {
  #include <sys/time.h>
}

#include <vector>

#include "DwmMclogFacility.hh"
#include "DwmMclogMessageOrigin.hh"
#include "DwmMclogSeverity.hh"
#include "DwmMclogVersion.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class Timestamp
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Timestamp();

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint64_t StreamedLength() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint64_t Secs() const
      { return _usecs / 1000000ull; }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint64_t Usecs() const
      { return _usecs % 1000000ull; }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const Timestamp & ts);
      
    private:
      uint64_t  _usecs;
    };

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class MessageHeader
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      MessageHeader() = default;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      MessageHeader(const MessageHeader & hdr);

      MessageHeader & operator = (const MessageHeader & hdr);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      MessageHeader(Facility facility, Severity severity,
                    const MessageOrigin & origin, std::string msgId = "")
          : _timestamp(), _facility(facility), _severity(severity),
            _origin(origin), _msgid(msgId)
      {}
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const Timestamp & timestamp() const
      { return _timestamp; }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Facility facility() const
      { return _facility; }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Severity severity() const
      { return _severity; }
        
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & msgid() const
      { return _msgid.Value(); }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const MessageOrigin & origin() const
      { return _origin; }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint64_t StreamedLength() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const MessageHeader & hdr);
      
    private:
      Timestamp                   _timestamp;
      Facility                    _facility;
      Severity                    _severity;
      MessageOrigin               _origin;
      Credence::ShortString<255>  _msgid;
    };

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class MessageRoute
    {
    public:
      MessageRoute() = default;
      MessageRoute(const MessageRoute &) = default;
      MessageRoute(MessageRoute &&) = default;
      MessageRoute & operator = (const MessageRoute &) = default;
      MessageRoute & operator = (MessageRoute &&) = default;
      
      std::istream & Read(std::istream & is);
      std::ostream & Write(std::ostream & os) const;
      uint64_t StreamedLength() const;
      
    private:
      std::vector<std::string>  _relays;
    };
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class Message
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Message();

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Message(const MessageHeader & header, std::string && message);

      Message(const MessageHeader & header, const std::string & message);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const MessageHeader & Header() const
      { return _header; }
        
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint64_t StreamedLength() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const Message & msg);
      
    private:
      MessageHeader                _header;
      MessageRoute                 _route;
      Credence::ShortString<1500>  _message;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMESSAGE_HH_
