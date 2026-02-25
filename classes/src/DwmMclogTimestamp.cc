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
    static bool IsNormal(const tm & tm, uint32_t offhours, uint32_t offmins)
    {
      return ((tm.tm_mon >= 0) && (tm.tm_mon < 12)
              && (tm.tm_mday > 0) && (tm.tm_mday < 32)
              && (tm.tm_hour >= 0) && (tm.tm_hour < 24)
              && (tm.tm_min >= 0) && (tm.tm_min < 60)
              && (tm.tm_sec >= 0) && (tm.tm_sec < 60)
              && (offhours < 24) && (offmins < 60));
    }

    //------------------------------------------------------------------------
    static uint64_t ToMicroseconds(const char *s)
    {
      uint64_t  rc = 0;
      tm        tms;
      uint32_t  usecs, offhours, offmins;
      int32_t   offsecs;
      char      offsign;
      if (sscanf(s, "%04d-%02d-%02d %02d:%02d:%02d.%06u%c%02u%02u",
                 &(tms.tm_year), &(tms.tm_mon), &(tms.tm_mday),
                 &(tms.tm_hour), &(tms.tm_min), &(tms.tm_sec),
                 &usecs, &offsign, &offhours, &offmins) == 10) {
        --tms.tm_mon;
        if (! IsNormal(tms, offhours, offmins)) {
          return 0;
        }
        tms.tm_year -= 1900;
        tms.tm_isdst = -1;
        offsecs = offhours * 3600;
        offsecs += offmins * 60;
        if ('-' == offsign) {
          offsecs *= -1;
        }
        else if ('+' != offsign) {
          return 0;
        }
        rc = timegm(&tms) - offsecs;
        rc *= 1000000ULL;
        rc += usecs;
      }
      return rc;
    }
    
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
    int Timestamp::Read(gzFile gzf)
    {
      _usecs = 0;
      EncodedU64  eu64;
      int rc = eu64.Read(gzf);
      if (0 < rc) {
        _usecs = eu64;
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    int Timestamp::Write(gzFile gzf) const
    {
      EncodedU64  eu64 = _usecs;
      return eu64.Write(gzf);
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
      strftime(buf, sizeof(buf), "%z", &ltm);
      os << buf;
      return os;
    }

    //------------------------------------------------------------------------
    std::istream & operator >> (std::istream & is, Timestamp & ts)
    {
      ts._usecs = 0;
      char  buf[32];
      memset(buf, 0, sizeof(buf));
      if (is.read(buf, 31)) {
        ts._usecs = ToMicroseconds(buf);
        if (0 == ts._usecs) {
          is.setstate(std::ios_base::failbit);
        }
      }
      return is;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
