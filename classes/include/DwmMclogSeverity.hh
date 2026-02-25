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
//!  @file DwmMclogSeverity.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Severity and related utility declarations
//---------------------------------------------------------------------------

extern "C" {
  #include <syslog.h>
}

#ifndef _DWMMCLOGSEVERITY_HH_
#define _DWMMCLOGSEVERITY_HH_

#include <cstdint>
#include <iostream>
#include <string>

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Valid log message severities.
    //------------------------------------------------------------------------
    enum class Severity : uint8_t {
      emerg   = LOG_EMERG,
      alert   = LOG_ALERT,
      crit    = LOG_CRIT,
      err     = LOG_ERR,
      warning = LOG_WARNING,
      notice  = LOG_NOTICE,
      info    = LOG_INFO,
      debug   = LOG_DEBUG
    };

    //------------------------------------------------------------------------
    //!  Emit a Severity to an ostream in human-readable form.
    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os, const Severity & severity);

    //------------------------------------------------------------------------
    //!  Read a Severity from an istream in human-readable form (e.g. "[I]").
    //------------------------------------------------------------------------
    std::istream & operator >> (std::istream & is, Severity & severity);
    
    //------------------------------------------------------------------------
    //!  Given a @c severityName, returns the corresponding Severity.  Valid
    //!  @c severityyName values: "debug", "info", "notice", "warning", "err",
    //!  "crit", "alert" and "emerg".  If @c severityyName is not a valid
    //!  severity name, returns @c emerg.
    //------------------------------------------------------------------------
    Severity SeverityValue(const std::string & severityName);

    //------------------------------------------------------------------------
    //!  Returns the string representation of the given @c severity.
    //------------------------------------------------------------------------
    std::string SeverityName(Severity severity);
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGSEVERITY_HH_
