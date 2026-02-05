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
  //!  @file DwmMclogFilterLex.lex
  //!  @author Daniel W. McRobb
  //!  @brief filter lexer implementation.  Note this just pushes tokens
  //!  into a deque because we don't want to rescan our filter input every
  //!  time we parse.  The parser uses the deque of tokens.
  //-------------------------------------------------------------------------

  #include <string>

  #include "DwmMclogFilterDriver.hh"
  #include "DwmMclogFilterParse.hh"
  
  using FilterParser = Dwm::Mclog::FilterParser;
  
  #undef yyterminate
  #define yyterminate() return FilterParser::make_YYEOF(loc)
%}
  
%option c++
%option nodefault
%option noyywrap
%option yyclass="Dwm::Mclog::FilterScanner"

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns(yyleng);
%}

%%

%{
  // A handy shortcut to the location held by the driver.
  auto &  loc = drv.location;
  // Code run each time yylex is called.
  loc.step();
%}

[^\|\&!\(\) \t\n]+  {
  auto  it = drv.cfg.selectors.find(YYText());
  if (it != drv.cfg.selectors.cend()) {
    auto  tok = FilterParser::make_SELECTOR(&(it->second), loc);
    drv.tokens.push_back(tok);
    return tok;
  }
  else {
    auto tok = FilterParser::make_YYerror(loc);
    drv.tokens.push_back(tok);
    std::cerr << drv.location << ": '" << YYText()
              << "' is not a valid selector\n";
    return tok;
  }                                                                            
}

"||" {
  auto tok = FilterParser::make_OR(loc);
  drv.tokens.push_back(tok);
  return tok;
}

"&&" {
  auto tok = FilterParser::make_AND(loc);
  drv.tokens.push_back(tok);
  return tok;
}

"!"  {
  auto tok = FilterParser::make_NOT(loc);
  drv.tokens.push_back(tok);
  return tok;
}

"("  {
  auto tok = FilterParser::make_LPAREN(loc);
  drv.tokens.push_back(tok);
  return tok;
}

")"  {
  auto tok = FilterParser::make_RPAREN(loc);
  drv.tokens.push_back(tok);
  return tok;
}

[ \t\r]+  { loc.step(); }

\n+  { loc.lines(yyleng); loc.step(); }

<<EOF>>  {
  auto tok = FilterParser::make_YYEOF(loc);
  drv.tokens.push_back(tok);
  return tok;
}

%%

