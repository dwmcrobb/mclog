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
//!  @file DwmMclogTimestamp.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Timestamp class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGTIMESTAMP_HH_
#define _DWMMCLOGTIMESTAMP_HH_

#include <cstdint>
#include <iostream>

#include "DwmBZ2IO.hh"
#include "DwmGZIO.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a timestamp, microseconds since the UNIX epoch.
    //------------------------------------------------------------------------
    class Timestamp
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      Timestamp();
      
      //----------------------------------------------------------------------
      //!  Copy constructor
      //----------------------------------------------------------------------
      Timestamp(const Timestamp &) = default;
      
      //----------------------------------------------------------------------
      //!  Move constructor
      //----------------------------------------------------------------------
      Timestamp(Timestamp &&) = default;
      
      //----------------------------------------------------------------------
      //!  Copy assignment
      //----------------------------------------------------------------------
      Timestamp & operator = (const Timestamp &) = default;
      
      //----------------------------------------------------------------------
      //!  Move assignment
      //----------------------------------------------------------------------
      Timestamp & operator = (Timestamp &&) = default;
      
      //----------------------------------------------------------------------
      //!  Reads the timestamp from the given istream @c is.  Returns @c is.
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);

      //----------------------------------------------------------------------
      //!  Writes the timestamp to the given ostream @c os.  Returns @c os.
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

      //----------------------------------------------------------------------
      //!  Reads the timestamp from the given BZFILE @c bzf.  Returns the
      //!  number of bytes read on success, -1 on failure.
      //----------------------------------------------------------------------
      int BZRead(BZFILE *bzf);

      //----------------------------------------------------------------------
      //!  Writes the timestamp to the given BZFILE @c bzf.  Returns the
      //!  number of bytes written on success, -1 on failure.
      //----------------------------------------------------------------------
      int BZWrite(BZFILE *bzf) const;
      
      //----------------------------------------------------------------------
      //!  Returns the number of bytes that would be written if we called the
      //!  Write() member.
      //----------------------------------------------------------------------
      uint64_t StreamedLength() const;
      
      //----------------------------------------------------------------------
      //!  Returns the timestamps seconds.
      //----------------------------------------------------------------------
      uint64_t Secs() const
      { return _usecs / 1000000ull; }

      //----------------------------------------------------------------------
      //!  Returns the timestamp residual microseconds.
      //----------------------------------------------------------------------
      uint64_t Usecs() const
      { return _usecs % 1000000ull; }

      //----------------------------------------------------------------------
      //!  Print a timestamp to an ostream in human-readable form.
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const Timestamp & ts);
      
    private:
      uint64_t  _usecs;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGTIMESTAMP_HH_
