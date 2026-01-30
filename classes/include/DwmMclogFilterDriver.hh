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
//!  @file DwmMclogFilterDriver.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGFILTERDRIVER_HH_
#define _DWMMCLOGFILTERDRIVER_HH_

#include <string>
#include <vector>

#include "DwmMclogConfig.hh"
#include "DwmMclogFilterParse.hh"
#include "DwmMclogFilterScanner.hh"

// Define the prototype of yylex we want ...
#undef YY_DECL
# define YY_DECL                                                   \
  Dwm::Mclog::FilterParser::symbol_type                            \
  Dwm::Mclog::FilterScanner::scan(Dwm::Mclog::FilterDriver & drv)

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class FilterDriver
    {
    public:
      FilterDriver(const Config & cfg, const std::string & expr);

      const Config                                           &cfg;
      std::string                                             expr;
      Dwm::Mclog::location                                    location;
      FilterScanner                                           scanner;
      std::vector<FilterParser::symbol_type>                  tokens;
      std::vector<FilterParser::symbol_type>::const_iterator  tokenIter;
      bool                                                    tokenized;
      
      FilterParser::symbol_type next_token();
      
      bool parse(const Message *msg, bool & result);
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGFILTERDRIVER_HH_
