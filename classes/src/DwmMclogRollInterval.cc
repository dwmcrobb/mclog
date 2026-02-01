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
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

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
    };

    //------------------------------------------------------------------------
    static const vector<IncrementInfo>  g_incrementInfo = {
      { RollPeriod::minutes_5,  &tm::tm_min,  5  },
      { RollPeriod::minutes_15, &tm::tm_min,  15 },
      { RollPeriod::minutes_30, &tm::tm_min,  30 },
      { RollPeriod::hours_1,    &tm::tm_hour, 1  },
      { RollPeriod::hours_2,    &tm::tm_hour, 2  },
      { RollPeriod::hours_4,    &tm::tm_hour, 4  },
      { RollPeriod::hours_6,    &tm::tm_hour, 6  },
      { RollPeriod::hours_8,    &tm::tm_hour, 8  },
      { RollPeriod::hours_12,   &tm::tm_hour, 12 },
      { RollPeriod::days_1,     &tm::tm_mday, 1  },
      { RollPeriod::days_7,     &tm::tm_mday, 7  }
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
    {
      _fieldToIncrement = FieldToIncrement(rp);
      _incrementAmount = IncrementAmount(rp);
      _endTime = NextMark(rp);
    }
    
  }  // namespace Mclog

}  // namespace Dwm
