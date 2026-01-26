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
//!  @file DwmMclogMessageSelector.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmMclogMessageSelector.hh"

namespace Dwm {

  namespace Mclog {

    using namespace std;
    
    //------------------------------------------------------------------------
    MessageSelector::MessageSelector()
        : _sourceHost(".+",regex::ECMAScript|regex::optimize),
          _facilities(), _minimumSeverity(Severity::debug),
          _ident(".*",regex::ECMAScript|regex::optimize)
    {}
    
    //------------------------------------------------------------------------
    bool MessageSelector::SourceHost(const string & srcHostExpr)
    {
      try {
        _sourceHost = regex(srcHostExpr, regex::ECMAScript|regex::optimize);
        return true;
      }
      catch (...) {
        FSyslog(LOG_ERR, "Invalid sourceHost regular expression '{}'",
                srcHostExpr);
        return false;
      }
    }
    
    //------------------------------------------------------------------------
    void MessageSelector::Facilities(const set<Facility> & facilities)
    {
      _facilities = facilities;
      return;
    }
    
    //------------------------------------------------------------------------
    void MessageSelector::MinimumSeverity(Severity minSeverity)
    {
      _minimumSeverity = minSeverity;
      return;
    }
    
    //------------------------------------------------------------------------
    bool MessageSelector::Ident(const string & identExpr)
    {
      try {
        _ident = regex(identExpr, regex::ECMAScript|regex::optimize);
        return true;
      }
      catch (...) {
        FSyslog(LOG_ERR, "Invalid ident regular expression '{}'",
                identExpr);
        return false;
      }
    }

    //------------------------------------------------------------------------
    bool MessageSelector::Matches(const Message & msg) const
    {
      bool  rc = false;
      smatch  hnsm;
      const MessageHeader  & hdr = msg.Header();
      if (regex_match(hdr.origin().hostname(), hnsm, _sourceHost)) {
        if (_facilities.empty()
            || (_facilities.find(hdr.facility()) != _facilities.end())) {
          if (hdr.severity() <= _minimumSeverity) {
            smatch  ansm;
            rc = regex_match(hdr.origin().appname(), ansm, _ident);
          }
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    void MessageSelector::Clear()
    {
      _sourceHost = regex(".+", regex::ECMAScript|regex::optimize);
      _facilities.clear();
      _minimumSeverity = Severity::debug;
      _ident = regex(".+", regex::ECMAScript|regex::optimize);
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
