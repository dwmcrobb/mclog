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
//!  @file DwmMclogFacility.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Facility declaration and related functions
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGFACILITY_HH_
#define _DWMMCLOGFACILITY_HH_

#include <cstdint>
#include <iostream>
#include <set>
#include <string>

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a log facility.  These mirror syslog facilities.  In
    //!  today's world, they're kind of antiquated (for example, 'uucp' and
    //!  'news' are almost dodo birds, and the original 'ftp' should never
    //!  be seen on a host whose security matters).
    //------------------------------------------------------------------------
    enum class Facility : uint8_t {
      kernel   = (0<<3),
      user     = (1<<3),
      mail     = (2<<3),
      daemon   = (3<<3),
      auth     = (4<<3),
      syslog   = (5<<3),
      lpr      = (6<<3),
      news     = (7<<3),
      uucp     = (8<<3),
      cron     = (9<<3),
      authpriv = (10<<3),
      ftp      = (11<<3),
      local0   = (16<<3),
      local1   = (17<<3),
      local2   = (18<<3),
      local3   = (19<<3),
      local4   = (20<<3),
      local5   = (21<<3),
      local6   = (22<<3),
      local7   = (23<<3)
    };

    //------------------------------------------------------------------------
    //!  Prints the name of the given @c facility to the given ostream @c os.
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Facility & facility);

    //------------------------------------------------------------------------
    //!  Returns a string representation of the given @c facility.
    //------------------------------------------------------------------------
    std::string FacilityName(Facility facility);

    //------------------------------------------------------------------------
    //!  Returns the Facility named @c facilityName.  If none exists, returns
    //!  local7.
    //------------------------------------------------------------------------
    Facility FacilityValue(const std::string & facilityName);
    
    //------------------------------------------------------------------------
    //!  Populates @c facilities with all facilities whose name matches
    //!  @c rgxstr (an ECMAScript regular expression).
    //------------------------------------------------------------------------
    void Facilities(const std::string & rgxstr,
                    std::set<Facility> & facilities);
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGFACILITY_HH_
