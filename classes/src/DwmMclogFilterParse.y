%{
  //=========================================================================
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
  //=========================================================================
  
  //-------------------------------------------------------------------------
  //!  @file DwmMclogFilterParse.y
  //!  @author Daniel W. McRobb
  //!  @brief mclog filter parser
  //-------------------------------------------------------------------------
%}

%language "c++"
%skeleton "lalr1.cc"
%require "3.8.2"
%header
%define api.token.raw
%define api.namespace {Dwm::Mclog}
%define api.parser.class {FilterParser}
%define api.location.file none
%define api.value.type variant
%define parse.assert
%define api.token.constructor

%code requires
{
  #include <iostream>
  #include <string>

  #include "DwmMclogMessage.hh"
  #include "DwmMclogMessageSelector.hh"
    
  namespace Dwm {
    namespace Mclog {
      class FilterDriver;
      class FilterParser;
      class FilterScanner;
    }
  }
}

%param {Dwm::Mclog::FilterDriver * drv}
%param {const Dwm::Mclog::Message * msg}
%param {bool & result}
%locations
%define parse.lac full

%code
{
  #include "DwmMclogMessage.hh"
  #include "DwmMclogMessageSelector.hh"
  #include "DwmMclogFilterDriver.hh"

  Dwm::Mclog::FilterParser::symbol_type
  yylex(Dwm::Mclog::FilterDriver * drv,
        const Dwm::Mclog::Message * msg,
        bool & result)
  {
      return drv->next_token();
      //    return drv.scanner.scan(drv);
  }
}

%token AND INTEGER LPAREN NOT OR RPAREN
%token <const Dwm::Mclog::MessageSelector *> SELECTOR
%type <bool> Expression

%%

 // %start Result;

Result:
{
    result = true;
}
| Expression
{
  result = $1;
}
| YYerror
{
  return 1;
};

Expression: SELECTOR
{
  if (nullptr != $1) {
    $$ = ($1)->Matches(*msg);
  }
  else {
    error(drv->location, "invalid selector");
    $$ = false;
  }
}
| NOT Expression
{
  $$ = (! ($2));
}
| Expression OR Expression
{
  $$ = (($1) || ($3));
}
| Expression AND Expression
{
  $$ = (($1) && ($3));
}
| LPAREN Expression RPAREN
{
  $$ = $2;
};

%%

namespace Dwm {

  namespace Mclog {

    //---------------------------------------------------------------------
    void FilterParser::error(const location_type & l,
                             const std::string & msg)
    {
        std::cerr << l.begin.line << ':' << l.begin.column
                  << '-' << l.end.column << " : " << msg << '\n';
      return;
    }

  }  // namespace Mclog

}  // namespace Dwm

