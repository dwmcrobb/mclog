%require "3.2"
%language "C++"
%define api.namespace {Dwm::Mclog}
%define api.parser.class {FilterParser}
%parse-param {Dwm::Mclog::MessageFilter *msgfilter}
%define api.value.type variant
%define api.token.constructor

%code requires
{
  #include <iostream>
  #include <string>

  #include "DwmMclogMessageFilter.hh"
    
  namespace Dwm {
    namespace Mclog {
      class FilterParser;
    }
  }
}

%code
{
  namespace Dwm {

    namespace Mclog {
        
      FilterParser::symbol_type yylex();
        
    }  // namespace Mclog

  }  // namespace Dwm

}

%token STRING INTEGER OR AND
%type<std::string> STRING

%%

Expression: STRING
{}
| '!' Expression
{}
| Expression OR Expression
{}
| Expression AND Expression
{}
| '(' Expression ')'
{};

%%

namespace Dwm {

  namespace Mclog {

    void FilterParser::error(const std::string & msg)
    {
      std::cout << "syntax error : " << msg << '\n';
      return;
    }

  }  // namespace Mclog

}  // namespace Dwm

