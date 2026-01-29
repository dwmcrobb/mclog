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
        : _sourceHost(boost::regex{".+"}, true),
          _facilities({}, true), _minimumSeverity(Severity::debug),
          _ident(boost::regex{".*"}, true)
    {}
    
    //------------------------------------------------------------------------
    bool MessageSelector::SourceHost(const string & srcHostExpr, bool match)
    {
      try {
        _sourceHost.first = boost::regex(srcHostExpr);
        _sourceHost.second = match;
        return true;
      }
      catch (...) {
        FSyslog(LOG_ERR, "Invalid sourceHost regular expression '{}'",
                srcHostExpr);
        return false;
      }
    }
    
    //------------------------------------------------------------------------
    void MessageSelector::Facilities(const set<Facility> & facilities,
                                     bool match)
    {
      _facilities.first = facilities;
      _facilities.second = match;
      return;
    }
    
    //------------------------------------------------------------------------
    void MessageSelector::MinimumSeverity(Severity minSeverity)
    {
      _minimumSeverity = minSeverity;
      return;
    }
    
    //------------------------------------------------------------------------
    bool MessageSelector::Ident(const string & identExpr, bool match)
    {
      try {
        _ident.first = boost::regex(identExpr);
        _ident.second = match;
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
      boost::smatch  hnsm;
      const MessageHeader  & hdr = msg.Header();
      if (boost::regex_match(hdr.origin().hostname(), hnsm, _sourceHost.first)
          == _sourceHost.second) {
        if (_facilities.first.empty()
            || (_facilities.second
                == (_facilities.first.find(hdr.facility())
                    != _facilities.first.end()))) {
          if (hdr.severity() <= _minimumSeverity) {
            boost::smatch  ansm;
            rc = (_ident.second
                  == boost::regex_match(hdr.origin().appname(), ansm, _ident.first));
          }
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    void MessageSelector::Clear()
    {
      _sourceHost.first = boost::regex(".+");
      _sourceHost.second = true;
      _facilities.first.clear();
      _facilities.second = true;
      _minimumSeverity = Severity::debug;
      _ident.first = boost::regex(".+");
      _ident.second = true;
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
