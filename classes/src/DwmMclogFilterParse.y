 // %language "C++"
%skeleton "lalr1.cc"
%require "3.8.2"
%header
%define api.token.raw
%define api.namespace {Dwm::Mclog}
%define api.parser.class {FilterParser}
%define api.location.file "DwmMclogFilterParserLocation.hh"
%define api.location.include {"DwmMclogFilterParserLocation.hh"}
 // %code requires {#include "DwmMclogFilterParserLocation.hh"}
%define api.value.type variant
%define parse.assert
%define api.token.constructor

%code requires
{
  #include <iostream>
  #include <string>

  #include "DwmMclogMessageFilter.hh"
    
  namespace Dwm {
    namespace Mclog {
      class FilterDriver;
      class FilterParser;
    }
  }
}

%param {Dwm::Mclog::FilterDriver & drv}
%locations                                                                                             %define parse.lac full

%code
{
    #include "DwmMclogFilterDriver.hh"
}

%token AND INTEGER LPAREN NOT OR RPAREN
%token <std::string> STRING
%nterm <bool> Expression

%%

Result: Expression { drv.result = $1; };
     
Expression: STRING
{
    return (($1).size() == 5);
}
| NOT Expression
{
  return (! ($2));
}
| Expression OR Expression
{
  return (($1) || ($3));
}
| Expression AND Expression
{
  return (($1) && ($3));
}
| LPAREN Expression RPAREN
{
  return $2;
};

%%

namespace Dwm {

  namespace Mclog {

    void FilterParser::error(const location_type & l,
                             const std::string & msg)
    {
      std::cerr << l << ": " << msg << '\n';
      return;
    }

  }  // namespace Mclog

}  // namespace Dwm

