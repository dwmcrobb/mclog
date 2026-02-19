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
//!  @file DwmMclogTimestamp.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/time.h>
  #include <time.h>
}

#include <cstring>
#include <iomanip>

#include "DwmEncodedUnsigned.hh"
#include "DwmMclogTimestamp.hh"

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
    int Timestamp::BZRead(BZFILE *bzf)
    {
      _usecs = 0;
      EncodedU64  eu64;
      int rc = eu64.BZRead(bzf);
      if (0 < rc) {
        _usecs = eu64;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    int Timestamp::BZWrite(BZFILE *bzf) const
    {
      EncodedU64  eu64 = _usecs;
      return eu64.BZWrite(bzf);
    }
    
    //------------------------------------------------------------------------
    uint64_t Timestamp::StreamedLength() const
    {
      return EncodedU64(_usecs).StreamedLength();
    }

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
    
  }  // namespace Mclog

}  // namespace Dwm
