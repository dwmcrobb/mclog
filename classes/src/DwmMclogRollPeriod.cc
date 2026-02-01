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
//!  @file DwmMclogRollPeriod.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <algorithm>
#include <utility>
#include <vector>

#include "DwmMclogRollPeriod.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    RollPeriod GetRollPeriod(const std::string & str)
    {
      static const std::vector<std::pair<std::string,RollPeriod>>
      rollPeriods = {
        { "5m",  RollPeriod::minutes_5  },
        { "15m", RollPeriod::minutes_15 },
        { "30m", RollPeriod::minutes_30 },
        { "60m", RollPeriod::hours_1    },
        { "1h",  RollPeriod::hours_1    },
        { "2h",  RollPeriod::hours_2    },
        { "4h",  RollPeriod::hours_4    },
        { "6h",  RollPeriod::hours_6    },
        { "8h",  RollPeriod::hours_8    },
        { "12h", RollPeriod::hours_12   },
        { "24h", RollPeriod::days_1     },
        { "1d",  RollPeriod::days_1     },
        { "7d",  RollPeriod::days_7     }
      };
      auto it = std::find_if(rollPeriods.begin(), rollPeriods.end(),
                             [&] (const auto & rp)
                             { return (rp.first == str); });
      if (it != rollPeriods.end()) {
        return it->second;
      }
      return RollPeriod::days_1;
    }

    
  }  // namespace Mclog

}  // namespace Dwm
