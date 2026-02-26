
%language "c++"
%skeleton "lalr1.cc"
%require "3.8.2"
%header
%define api.token.raw
%define api.namespace {Dwm::Mclog}
%define api.parser.class {MessageFilterParser}
%define api.location.file none
%define api.value.type variant
%define parse.assert
%define api.token.constructor

%code requires
{
  #include <iostream>
  #include <string>
  #include <boost/regex.hpp>

  #include "DwmMclogMessage.hh"

  namespace Dwm {
    namespace Mclog {
      class MessageFilterDriver;
      class MessageFilterParser;
      class MessageFilterScanner;
    }
  }
}

%param {Dwm::Mclog::MessageFilterDriver * drv}
%param {const Dwm::Mclog::Message *msg}
%param {bool & result}
%locations
%define parse.lac full

%code
{
    #include "DwmMclogMessage.hh"
    #include "DwmMclogMessageFilterDriver.hh"
    
    Dwm::Mclog::MessageFilterParser::symbol_type
    yylex(Dwm::Mclog::MessageFilterDriver *drv,
          const Dwm::Mclog::Message *msg, bool & result)
    {
      return drv->next_token();
    }
}

%token AND EQUAL GREATER GREATEROREQ LESS LESSOREQ LPAREN NOT NOTEQ OR QUOTE
%token RPAREN SLASH
%token FACILITY HOST IDENT MSG SEVERITY
%token <Dwm::Mclog::Facility> FACVALUE
%token <Dwm::Mclog::Severity> SEVVALUE
%token DEBUG INFO NOTICE WARNING ERR CRIT ALERT EMERG
%token KERNEL USER MAIL DAEMON AUTH SYSLOG LPR NEWS UUCP CRON AUTHPRIV FTP
%token LOCAL0 LOCAL1 LOCAL2 LOCAL3 LOCAL4 LOCAL5 LOCAL6 LOCAL7
%token <std::string> STRING
%token <boost::regex> REGEX
%type <bool> Expression
%type <std::string> QuotedString
%type <boost::regex> Regex

%left OR
%left AND
%left EQUAL NOTEQ
%left LESS GREATER LESSOREQ GREATEROREQ
%left NOT
%left LPAREN RPAREN

%%

Result: Expression { result = $1; }
| YYerror {
  return 1;
};

Expression: SEVERITY EQUAL SEVVALUE { $$ = (msg->Header().severity() == $3); }
| SEVERITY NOTEQ SEVVALUE { $$ = (msg->Header().severity() != $3); }
| SEVERITY LESS SEVVALUE { $$ = (msg->Header().severity() > $3); }
| SEVERITY LESSOREQ SEVVALUE { $$ = (msg->Header().severity() >= $3); }
| SEVERITY GREATER SEVVALUE { $$ = (msg->Header().severity() < $3); }
| SEVERITY GREATEROREQ SEVVALUE { $$ = (msg->Header().severity() <= $3); }
| FACILITY EQUAL FACVALUE { $$ = (msg->Header().facility() == $3); }
| FACILITY NOTEQ FACVALUE { $$ = (msg->Header().facility() != $3); }
| FACILITY LESS FACVALUE { $$ = (msg->Header().facility() < $3); }
| FACILITY LESSOREQ FACVALUE { $$ = (msg->Header().facility() <= $3); }
| FACILITY GREATER FACVALUE { $$ = (msg->Header().facility() > $3); }
| FACILITY GREATEROREQ FACVALUE { $$ = (msg->Header().facility() >= $3); }
| FACILITY NOTEQ FACVALUE { $$ = (msg->Header().facility() != $3); }
| HOST EQUAL QuotedString { $$ = (msg->Header().origin().hostname() == $3); }
| HOST NOTEQ QuotedString { $$ = (msg->Header().origin().hostname() != $3); }
| HOST EQUAL Regex  {
    boost::smatch  sm;
    $$ = boost::regex_match(msg->Header().origin().hostname(), sm, $3);
}
| HOST NOTEQ Regex  {
    boost::smatch  sm;
    $$ = ! boost::regex_match(msg->Header().origin().hostname(), sm, $3);
}
| IDENT EQUAL QuotedString { $$ = (msg->Header().origin().appname() == $3); }
| IDENT NOTEQ QuotedString { $$ = (msg->Header().origin().appname() != $3); }
| IDENT EQUAL Regex {
  boost::smatch  sm;
  $$ = boost::regex_match(msg->Header().origin().appname(), sm, $3);
}
| IDENT NOTEQ Regex {
  boost::smatch  sm;
  $$ = ! boost::regex_match(msg->Header().origin().appname(), sm, $3);
}
| MSG EQUAL QuotedString { $$ = (msg->Data() == $3); }
| MSG NOTEQ QuotedString { $$ = (msg->Data() != $3); }
| MSG EQUAL Regex {
  boost::smatch  sm;
  $$ = boost::regex_match(msg->Data(), sm, $3);
}
| MSG NOTEQ Regex {
  boost::smatch  sm;
  $$ = ! boost::regex_match(msg->Data(), sm, $3);
}
| NOT Expression { $$ = (! ($2)); }
| Expression OR Expression { $$ = (($1) || ($3)); }
| Expression AND Expression { $$ = (($1) && ($3)); }
| LPAREN Expression RPAREN { $$ = $2; }
;

QuotedString: QUOTE STRING QUOTE  { $$ = $2; };

Regex: SLASH REGEX SLASH { $$ = $2; };

%%

namespace Dwm {

  namespace Mclog {

    //---------------------------------------------------------------------
    void MessageFilterParser::error(const location_type & l,
                                    const std::string & msg)
    {
        std::cerr << l.begin.line << ':' << l.begin.column
                  << '-' << l.end.column << " : " << msg << '\n';
      return;
    }

  }  // namespace Mclog

}  // namespace Dwm
