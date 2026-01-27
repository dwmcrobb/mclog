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
//!  @file DwmMclogSeverity.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <algorithm>
#include <array>
#include <utility>

#include "DwmMclogSeverity.hh"

namespace Dwm {

  namespace Mclog {

    using namespace std;
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static constexpr array<pair<Severity,const char *>,8>
    g_severityNames = {
      make_pair(Severity::emerg,   "emerg"),
      make_pair(Severity::alert,   "alert"),
      make_pair(Severity::crit,    "crit"),
      make_pair(Severity::err,     "err"),
      make_pair(Severity::warning, "warning"),
      make_pair(Severity::notice,  "notice"),
      make_pair(Severity::info,    "info"),
      make_pair(Severity::debug,   "debug")
    };
    
    //------------------------------------------------------------------------
    std::string SeverityName(Severity severity)
    {
      if (auto it = ranges::find(g_severityNames, severity,
                                 &pair<Severity,const char *>::first);
          it != g_severityNames.end()) {
        return std::string(it->second);
      }
      return std::string();
    }

    //------------------------------------------------------------------------
    Severity SeverityValue(const std::string & name)
    {
      if (auto it = ranges::find(g_severityNames, name,
                                 &pair<Severity,const char *>::second);
          it != g_severityNames.end()) {
        return it->first;
      }
      return Severity::emerg;
    }
    
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Severity & severity)
    {
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
    
  }  // namespace Mclog

}  // namespace Dwm
