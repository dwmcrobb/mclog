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
//!  @file DwmMclogMessage.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
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
    Timestamp::Timestamp()
    {
      timeval  tv;
      gettimeofday(&tv, nullptr);
      _usecs = tv.tv_sec;
      _usecs *= 1000000ull;
      _usecs += tv.tv_usec;
    }

    //------------------------------------------------------------------------
    std::istream & Timestamp::Read(std::istream & is)
    {
      _usecs = 0;
      EncodedU64  eu64;
      if (eu64.Read(is)) {
        _usecs = eu64;
      }
      return is;
    }
    
    //------------------------------------------------------------------------
    std::ostream & Timestamp::Write(std::ostream & os) const
    {
      EncodedU64  eu64 = _usecs;
      return eu64.Write(os);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    uint64_t Timestamp::StreamedLength() const
    {
      return EncodedU64(_usecs).StreamedLength();
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Timestamp & ts)
    {
      time_t  t = ts.Secs();
      tm      ltm;
      localtime_r(&t, &ltm);
      char  buf[32];
      memset(buf, 0, sizeof(buf));
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ltm);
      os << buf << "." << std::setfill('0') << std::setw(6) << ts.Usecs();
      return os;
    }
    
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Facility & facility)
    {
      using namespace std;
      static constexpr array<pair<Facility,const char *>,20> facilities = {
        make_pair(Facility::kernel,    "kernel"),
        make_pair(Facility::user,      "user"),
        make_pair(Facility::mail,      "mail"),
        make_pair(Facility::daemon,    "daemon"),
        make_pair(Facility::auth,      "auth"),
        make_pair(Facility::syslog,    "syslog"),
        make_pair(Facility::lpr,       "lpr"),
        make_pair(Facility::news,      "news"),
        make_pair(Facility::uucp,      "uucp"),
        make_pair(Facility::cron,      "cron"),
        make_pair(Facility::authpriv,  "authpriv"),
        make_pair(Facility::ftp,       "ftp"),
        make_pair(Facility::local0,    "local0"),
        make_pair(Facility::local1,    "local1"),
        make_pair(Facility::local2,    "local2"),
        make_pair(Facility::local3,    "local3"),
        make_pair(Facility::local4,    "local4"),
        make_pair(Facility::local5,    "local5"),
        make_pair(Facility::local6,    "local6"),
        make_pair(Facility::local7,    "local7")
      };
      if (auto it = ranges::find(facilities, facility,
                                 &pair<Facility,const char *>::first);
          it != facilities.end()) {
        os << it->second;
      }
      else {
        os << "unknown";
      }
      return os;
    }
    
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Severity & severity)
    {
      using namespace std;
      static constexpr array<pair<Severity,const char *>,8>
        severities = {
        make_pair(Severity::emerg,   "[M]"),
        make_pair(Severity::alert,   "[A]"),
        make_pair(Severity::crit,    "[C]"),
        make_pair(Severity::err,     "[E]"),
        make_pair(Severity::warning, "[W]"),
        make_pair(Severity::notice,  "[N]"),
        make_pair(Severity::info,    "[I]"),
        make_pair(Severity::debug,   "[D]")
      };
      if (auto it = ranges::find(severities, severity,
                                 &pair<Severity,const char *>::first);
          it != severities.end()) {
        os << it->second;
      }
      else {
        os << "[U]";
      }
      return os;
    }

    //------------------------------------------------------------------------
    MessageHeader::MessageHeader(const MessageHeader & hdr)
        : _timestamp(hdr._timestamp), _facility(hdr._facility),
          _severity(hdr._severity), _origin(hdr._origin), _msgid(hdr._msgid)
    {}

    //------------------------------------------------------------------------
    MessageHeader & MessageHeader::operator = (const MessageHeader & hdr)
    {
      if (&hdr != this) {
        _timestamp = hdr._timestamp;
        _facility = hdr._facility;
        _severity = hdr._severity;
        _origin = hdr._origin;
        _msgid = hdr._msgid;
      }
      return *this;
    }
    
    //------------------------------------------------------------------------
    std::istream & MessageHeader::Read(std::istream & is)
    {
      if (_timestamp.Read(is)) {
        if (StreamIO::Read(is, _facility)) {
          if (_facility <= Facility::local7) {
            if (StreamIO::Read(is, _severity)) {
              if (_severity <= Severity::debug) {
                if (_origin.Read(is)) {
                  StreamIO::Read(is, _msgid);
                }
              }
              else {
                is.setstate(std::ios_base::failbit);
              }
            }
          }
          else {
            is.setstate(std::ios_base::failbit);
          }
        }
      }
      return is;
    }
    
    //------------------------------------------------------------------------
    std::ostream & MessageHeader::Write(std::ostream & os) const
    {
      if (_timestamp.Write(os)) {
        if (StreamIO::Write(os, _facility)) {
          if (StreamIO::Write(os, _severity)) {
            if (_origin.Write(os)) {
              StreamIO::Write(os, _msgid);
            }
          }
        }
      }
      return os;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    uint64_t MessageHeader::StreamedLength() const
    {
      return (_timestamp.StreamedLength()
              + IOUtils::StreamedLength(_facility)
              + IOUtils::StreamedLength(_severity)
              + _origin.StreamedLength()
              + IOUtils::StreamedLength(_msgid));
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream &                                                     
    operator << (std::ostream & os, const MessageHeader & hdr)
    {
      os << hdr._timestamp << ' ' << hdr._origin << ": " << hdr._severity;
      return os;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::istream & MessageRoute::Read(std::istream & is)
    {
      return StreamIO::Read(is, _relays);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & MessageRoute::Write(std::ostream & os) const
    {
      return StreamIO::Write(os, _relays);
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    uint64_t MessageRoute::StreamedLength() const
    {
      return IOUtils::StreamedLength(_relays);
    }
    
    //------------------------------------------------------------------------
    Message::Message()
        : _header(), _route(), _message()
    {}

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Message::Message(const MessageHeader & header, std::string && message)
        : _header(header), _message(message)
    {}

    //------------------------------------------------------------------------
    //!  
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
    //!  
    //------------------------------------------------------------------------
    std::ostream & Message::Write(std::ostream & os) const
    {
      if (_header.Write(os)) {
        //        if (_route.Write(os)) {
          StreamIO::Write(os, _message);
          //        }
      }
      return os;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    uint64_t Message::StreamedLength() const
    {
      return (_header.StreamedLength()
              // + _route.StreamedLength()
              + IOUtils::StreamedLength(_message));
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Message & msg)
    {
      os << msg._header << ' ' << msg._message << '\n';
      return os;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
