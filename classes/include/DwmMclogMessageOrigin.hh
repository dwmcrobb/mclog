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
//!  @file DwmMclogMessageOrigin.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MessageOrigin class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMESSAGEORIGIN_HH_
#define _DWMMCLOGMESSAGEORIGIN_HH_

#include <cstdint>
#include <iostream>
#include <mutex>

#include "DwmCredenceShortString.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates message origin: hostname, appname (ident) and process
    //!  ID.
    //------------------------------------------------------------------------
    class MessageOrigin
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      MessageOrigin() = default;

      //----------------------------------------------------------------------
      //!  Copy constructor
      //----------------------------------------------------------------------
      MessageOrigin(const MessageOrigin & origin);

      //----------------------------------------------------------------------
      //!  Copy assignment
      //----------------------------------------------------------------------
      MessageOrigin & operator = (const MessageOrigin & origin);

      //----------------------------------------------------------------------
      //!  Move constructor
      //----------------------------------------------------------------------
      MessageOrigin(MessageOrigin && origin);

      //----------------------------------------------------------------------
      //!  Move assignment
      //----------------------------------------------------------------------
      MessageOrigin & operator = (MessageOrigin && origin);
      
      //----------------------------------------------------------------------
      //!  Construct from the given @c hostname, @c appname and @c pid.
      //----------------------------------------------------------------------
      MessageOrigin(const char *hostname, const char *appname, pid_t pid);

      //----------------------------------------------------------------------
      //!  Returns the origin hostname.
      //----------------------------------------------------------------------
      const std::string & hostname() const
      { std::lock_guard lck(_mtx); return _hostname.Value(); }

      //----------------------------------------------------------------------
      //!  Sets and returns the origin hostname.
      //----------------------------------------------------------------------
      const std::string & hostname(const char *hostname)
      {
        std::lock_guard lck(_mtx);
        _hostname = hostname;
        return _hostname.Value();
      }
        
      //----------------------------------------------------------------------
      //!  Returns the origin appname (ident).
      //----------------------------------------------------------------------
      const std::string & appname() const
      { std::lock_guard lck(_mtx); return _appname.Value(); }

      //----------------------------------------------------------------------
      //!  Sets and returns the origin appname (ident).
      //----------------------------------------------------------------------
      const std::string & appname(const char *appname)
      {
        std::lock_guard lck(_mtx);
        _appname = appname;
        return _appname.Value();
      }

      //----------------------------------------------------------------------
      //!  Returns the origin process ID.
      //----------------------------------------------------------------------
      const uint32_t processid() const
      { std::lock_guard lck(_mtx); return _procid; }

      //----------------------------------------------------------------------
      //!  Sets and returns the origin process ID.
      //----------------------------------------------------------------------
      const uint32_t processid(uint32_t procid)
      { std::lock_guard lck(_mtx); _procid = procid; return _procid; }

      //----------------------------------------------------------------------
      //!  Reads the origin from the given istream @c is.  Returns @c is.
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);
      
      //----------------------------------------------------------------------
      //!  Writes the origin to the given ostream @c os.  Returns @c os.
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

      //----------------------------------------------------------------------
      //!  Reads the origin from the given BZFILE @c bzf.  Returns the
      //!  number of bytes read on success, -1 on failure.
      //----------------------------------------------------------------------
      int BZRead(BZFILE *bzf);

      //----------------------------------------------------------------------
      //!  Writes the origin to the given BZFILE @c bzf.  Returns the
      //!  number of bytes written on success, -1 on failure.
      //----------------------------------------------------------------------
      int BZWrite(BZFILE *bzf) const;

      //----------------------------------------------------------------------
      //!  Reads the origin from the given gzFile @c gzf.  Returns the
      //!  number of bytes read on success, -1 on failure.
      //----------------------------------------------------------------------
      int Read(gzFile gzf);

      //----------------------------------------------------------------------
      //!  Writes the origin to the given gzFile @c gzf.  Returns the
      //!  number of bytes written on success, -1 on failure.
      //----------------------------------------------------------------------
      int Write(gzFile gzf) const;
      
      //----------------------------------------------------------------------
      //!  Returns the number of bytes that would be written if we called the
      //!  Write() member.
      //----------------------------------------------------------------------
      uint64_t StreamedLength() const;
      
      //----------------------------------------------------------------------
      //!  Prints the origin to an ostream in human-readable form.
      //----------------------------------------------------------------------
      friend std::ostream & operator << (std::ostream & os,
                                         const MessageOrigin & origin);

      //----------------------------------------------------------------------
      //!  Reads the origin from an istream in human-readable form.
      //----------------------------------------------------------------------
      friend std::istream & operator >> (std::istream & is,
                                         MessageOrigin & origin);
      
    private:
      mutable std::mutex          _mtx;
      Credence::ShortString<255>  _hostname;
      Credence::ShortString<255>  _appname;
      uint32_t                    _procid;
    };
        
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMESSAGEORIGIN_HH_
