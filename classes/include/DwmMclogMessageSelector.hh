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
//!  @file DwmMclogMessageSelector.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MessageSelector class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMESSAGESELECTOR_HH_
#define _DWMMCLOGMESSAGESELECTOR_HH_

#include <boost/regex.hpp>
#include <set>
#include <utility>

#include "DwmMclogMessage.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates message selection criteria.  Instances of this class
    //!  are used as operands in the message filtering language.  This class
    //!  is relatively simple, and a 'match' is determined using a logical
    //!  AND ('&&') of the matches of each of the fields against the field's
    //!  corresponding expression.  We have 2 fields which will be matched
    //!  using regular expressions (SourceHost and Ident), one which will
    //!  be compared for membership in a set (Facilities) and one which will
    //!  be compared arithmetically (MinimumSeverity).
    //------------------------------------------------------------------------
    class MessageSelector
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor.  A default constructed MessageSelector will
      //!  match all messages with a non-empty origin (source) host.
      //----------------------------------------------------------------------
      MessageSelector();

      //----------------------------------------------------------------------
      //!  Sets the source host (origin) expression to match
      //!  (@c match == @c true) or not match (@c match == @c false).
      //----------------------------------------------------------------------
      bool SourceHost(const std::string & srcHostExpr, bool match = true);
      
      //----------------------------------------------------------------------
      //!  Sets the @c facilities to match (@c match == @c true) or not match
      //!  (@c match == @c false).
      //----------------------------------------------------------------------
      void Facilities(const std::set<Facility> & facilities,
                      bool match = true);
      
      //----------------------------------------------------------------------
      //!  Returns a mutable reference to the facilities to match or not
      //!  match.  This is a convenience for inserting into the facilities
      //!  when building a MessageSelector from configuration.
      //----------------------------------------------------------------------
      const std::pair<std::set<Facility>,bool> & Facilities() const
      { return _facilities; }
        
      //----------------------------------------------------------------------
      //!  Sets the minimum severity we will match.
      //----------------------------------------------------------------------
      void MinimumSeverity(Severity minSeverity);
      
      //----------------------------------------------------------------------
      //!  Sets the application name ('ident') expression we will match
      //!  (@c match == @c true) or not match (@c match == @c false).
      //----------------------------------------------------------------------
      bool Ident(const std::string & identExpr, bool match = true);

      //----------------------------------------------------------------------
      //!  Returns true if the given @c msg matches.
      //----------------------------------------------------------------------
      bool Matches(const Message & msg) const;
      
      //----------------------------------------------------------------------
      //!  Clears the selector (sets to default-constructed state).
      //----------------------------------------------------------------------
      void Clear();
      
    private:
      std::pair<boost::regex,bool>        _sourceHost;
      std::pair<std::set<Facility>,bool>  _facilities;
      Severity                            _minimumSeverity;
      std::pair<boost::regex,bool>        _ident;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMESSAGESELECTOR_HH_
