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
//!  @file DwmMclogRollInterval.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::RollInterval implementation
//---------------------------------------------------------------------------

#include <algorithm>
#include <cstring>
#include <vector>

#include "DwmMclogRollInterval.hh"

namespace Dwm {

  namespace Mclog {

    using namespace std;

    //------------------------------------------------------------------------
    struct IncrementInfo
    {
      RollPeriod  rp;
      int tm::*   field;
      int         amount;
      time_t      secs;
    };

    //------------------------------------------------------------------------
    static const vector<IncrementInfo>  g_incrementInfo = {
      { RollPeriod::minutes_5,  &tm::tm_min,  5,  300    },
      { RollPeriod::minutes_15, &tm::tm_min,  15, 900    },
      { RollPeriod::minutes_30, &tm::tm_min,  30, 1800   },
      { RollPeriod::hours_1,    &tm::tm_hour, 1,  3600   },
      { RollPeriod::hours_2,    &tm::tm_hour, 2,  7200   },
      { RollPeriod::hours_4,    &tm::tm_hour, 4,  14400  },
      { RollPeriod::hours_6,    &tm::tm_hour, 6,  21600  },
      { RollPeriod::hours_8,    &tm::tm_hour, 8,  28800  },
      { RollPeriod::hours_12,   &tm::tm_hour, 12, 43200  },
      { RollPeriod::days_1,     &tm::tm_mday, 1,  86400  },
      { RollPeriod::days_7,     &tm::tm_mday, 7,  604800 }
    };
      
    //------------------------------------------------------------------------
    static int tm::* FieldToIncrement(RollPeriod rp)
    {
      auto it = std::find_if(g_incrementInfo.cbegin(), g_incrementInfo.cend(),
                             [&] (const auto & incInfo)
                             { return (incInfo.rp == rp); });
      if (it != g_incrementInfo.cend()) {
        return it->field;
      }
      return &tm::tm_mday;
    }

    //------------------------------------------------------------------------
    static int IncrementAmount(RollPeriod rp)
    {
      auto it = std::find_if(g_incrementInfo.cbegin(), g_incrementInfo.cend(),
                             [&] (const auto & incInfo)
                             { return (incInfo.rp == rp); });
      if (it != g_incrementInfo.cend()) {
        return it->amount;
      }
      return 1;
    }

    //------------------------------------------------------------------------
    static time_t IncrementSeconds(RollPeriod rp)
    {
      auto it = std::find_if(g_incrementInfo.cbegin(), g_incrementInfo.cend(),
                             [&] (const auto & incInfo)
                             { return (incInfo.rp == rp); });
      if (it != g_incrementInfo.cend()) {
        return it->secs;
      }
      return 1;
    }
    
    //------------------------------------------------------------------------
    static time_t NextMark(RollPeriod rp)
    {
      int tm::*  field = FieldToIncrement(rp);
      int        amount = IncrementAmount(rp);
      time_t     t = time((time_t *)0);
      tm         tms;

      localtime_r(&t, &tms);
      int  mark = 0;
      while (mark <= tms.*field) {
        mark += amount;
      }
      tms.*field = mark;
      
      if (&tm::tm_mday == field) {
        tms.tm_hour = 0;
        tms.tm_min = 0;
        tms.tm_sec = 0;
      }
      else if (&tm::tm_hour == field) {
        tms.tm_min = 0;
        tms.tm_sec = 0;
      }
      else if (&tm::tm_min == field) {
        tms.tm_sec = 0;
      }
      return mktime(&tms);
    }
    
    //------------------------------------------------------------------------
    RollInterval::RollInterval()
        : _fieldToIncrement(&tm::tm_mday), _incrementAmount(1)
    {
      _endTime = NextMark(RollPeriod::days_1);
    }

    //------------------------------------------------------------------------
    RollInterval::RollInterval(RollPeriod rp)
        : _rp(rp)
    {
      _fieldToIncrement = FieldToIncrement(_rp);
      _incrementAmount = IncrementAmount(_rp);
      _endTime = NextMark(_rp);
    }

    //------------------------------------------------------------------------
    void RollInterval::SetToCurrent()
    {
      _endTime = NextMark(_rp);
      return;
    }

    //------------------------------------------------------------------------
    time_t RollInterval::StartTime() const
    {
      return _endTime - IncrementSeconds(_rp);
    }
    
    //------------------------------------------------------------------------
    time_t RollInterval::EndTime() const
    {
      return _endTime;
    }
    
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const RollInterval & ri)
    {
      tm  tms;
      time_t  t = ri.StartTime();
      localtime_r(&t, &tms);
      char  buf[24];
      memset(buf, 0, sizeof(buf));
      strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tms);
      os << buf << " - ";
      t = ri._endTime;
      localtime_r(&t, &tms);
      memset(buf, 0, sizeof(buf));
      strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tms);
      os << buf;
      return os;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
