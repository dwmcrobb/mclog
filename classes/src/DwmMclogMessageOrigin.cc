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
//!  @file DwmMclogMessageOrigin.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmIOUtils.hh"
#include "DwmStreamIO.hh"
#include "DwmMclogMessageOrigin.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MessageOrigin::MessageOrigin(const char *hostname, const char *appname,
                                 pid_t pid)
        : _hostname(hostname), _appname(appname), _procid(pid)
    {}

    //------------------------------------------------------------------------
    MessageOrigin::MessageOrigin(const MessageOrigin & origin)
        : _mtx(), _hostname(origin.hostname()), _appname(origin.appname()),
          _procid(origin.processid())
    {}

    //------------------------------------------------------------------------
    MessageOrigin & MessageOrigin::operator = (const MessageOrigin & origin)
    {
      if (&origin != this) {
        std::lock_guard  lck(_mtx);
        _hostname = origin.hostname();
        _appname = origin.appname();
        _procid = origin.processid();
      }
      return *this;
    }

    //------------------------------------------------------------------------
    std::istream & MessageOrigin::Read(std::istream & is)
    {
      std::lock_guard  lck(_mtx);
      if (StreamIO::Read(is, _hostname)) {
        if (StreamIO::Read(is, _appname)) {
          StreamIO::Read(is, _procid);
        }
      }
      return is;
    }
            
    //------------------------------------------------------------------------
    std::ostream & MessageOrigin::Write(std::ostream & os) const
    {
      std::lock_guard  lck(_mtx);
      if (StreamIO::Write(os, _hostname)) {
        if (StreamIO::Write(os, _appname)) {
          StreamIO::Write(os, _procid);
        }
      }
      return os;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    uint64_t MessageOrigin::StreamedLength() const
    {
      std::lock_guard  lck(_mtx);
      return (IOUtils::StreamedLength(_hostname)
              + IOUtils::StreamedLength(_appname)
              + IOUtils::StreamedLength(_procid));
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os,
                                const MessageOrigin & origin)
    {
      std::lock_guard  lck(origin._mtx);
      os << origin._hostname << ' ' << origin._appname << '['
         << origin._procid << ']';
      return os;
    }

  }  // namespace Mclog

}  // namespace Dwm
