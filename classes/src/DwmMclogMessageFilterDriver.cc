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
//!  @file DwmMclogMessageFilterDriver.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MessageFilterDriver implementation
//---------------------------------------------------------------------------

#include <cassert>
#include <sstream>

#include "DwmMclogMessageFilterDriver.hh"
#include "DwmMclogMessageFilterParse.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MessageFilterDriver::MessageFilterDriver(const std::string & expr)
        : tokens(), tokenIter(tokens.begin()), tokenized(false),
          expr(expr), parsemtx()
    { }
    
    //------------------------------------------------------------------------
    MessageFilterParser::symbol_type MessageFilterDriver::next_token()
    {
      if (tokenIter == tokens.end()) {
        return MessageFilterParser::make_YYEOF(location);
      }
      return *tokenIter++;
    }
    
    //------------------------------------------------------------------------
    bool MessageFilterDriver::parse(const Message *msg, bool & result)
    {
      std::lock_guard  lck(parsemtx);
      
      location.initialize(nullptr);
      if (! tokenized) {
        std::istringstream  is(expr);
        tokens.clear();
        scanner.switch_streams(is, std::cerr);
        while (scanner.scan(*this).kind()
               != MessageFilterParser::symbol_kind_type::S_YYEOF) {
        }
        tokenized = true;
      }
      result = false;
      tokenIter = tokens.begin();
      MessageFilterParser  parser(this, msg, result);
      bool  rc = (0 == parser());
      if (! rc) {
        parser.error(location, std::string("invalid filter '") + expr + "'");
      }
      return rc;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
