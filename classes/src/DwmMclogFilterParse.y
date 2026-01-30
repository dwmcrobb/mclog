%language "c++"
%skeleton "lalr1.cc"
%require "3.8.2"
%header
%define api.token.raw
%define api.namespace {Dwm::Mclog}
%define api.parser.class {FilterParser}
%define api.location.file none
 // %define api.location.file "DwmMclogFilterParserLocation.hh"
 // %define api.location.include {"DwmMclogFilterParserLocation.hh"}
 // %code requires {#include "DwmMclogFilterParserLocation.hh"}
%define api.value.type variant
%define parse.assert
%define api.token.constructor

%code requires
{
  #include <iostream>
  #include <string>

  #include "DwmMclogMessage.hh"
  #include "DwmMclogMessageSelector.hh"
  #include "DwmMclogMessageFilter.hh"
    
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

Result: Expression
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

