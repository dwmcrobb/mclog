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
//!  @file DwmMclogFacility.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Facility implementation
//---------------------------------------------------------------------------

#include <array>
#include <algorithm>
#include <regex>
#include <utility>

#include "DwmMclogFacility.hh"

namespace Dwm {

  namespace Mclog {

      using namespace std;
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    static constexpr array<pair<Facility,const string>,20> g_facilities = {
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
    
    //------------------------------------------------------------------------
    std::string FacilityName(Facility facility)
    {
      if (auto it = ranges::find(g_facilities, facility,
                                 &pair<Facility,const string>::first);
          it != g_facilities.end()) {
        return it->second;
      }
      return "unknown";
    }

    //------------------------------------------------------------------------
    Facility FacilityValue(const std::string & facilityName)
    {
      if (auto it = ranges::find(g_facilities, facilityName,
                                 &pair<Facility,const string>::second);
          it != g_facilities.end()) {
        return it->first;
      }
      return Facility::local7;
    }

    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Facility & facility)
    {
      os << FacilityName(facility);
      return os;
    }

    //------------------------------------------------------------------------
    std::istream & operator >> (std::istream & is, Facility & facility)
    {
      std::string  s;
      if (is >> s) {
        facility = FacilityValue(s);
      }
      return is;
    }
    
    //------------------------------------------------------------------------
    void Facilities(const std::string & rgxstr,
                    std::set<Facility> & facilities)
    {
      facilities.clear();
      std::regex  rgx(rgxstr, std::regex::ECMAScript|std::regex::optimize);
      smatch  sm;
      for (const auto & facility : g_facilities) {
        if (regex_match(facility.second, sm, rgx)) {
          facilities.insert(facility.first);
        }
      }
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
