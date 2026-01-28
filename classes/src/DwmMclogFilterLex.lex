%{
  #include <string>

  #include "DwmMclogFilterDriver.hh"
  #include "DwmMclogFilterParse.hh"
  
  using FilterParser = Dwm::Mclog::FilterParser;
  
  using token = FilterParser::token;

  #undef yyterminate
  #define yyterminate() return FilterParser::make_YYEOF(loc)

%}
  
%option nodefault
%option noyywrap

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

[^\|\&!\(\) \t\n]+       { return FilterParser::make_STRING(yytext, loc); }
[\|\|]                   { return FilterParser::make_OR(loc); }
[\&\&]                   { return FilterParser::make_AND(loc); }
[!]                      { return FilterParser::make_NOT(loc); }
[\(]                     { return FilterParser::make_LPAREN(loc); }
[\)]                     { return FilterParser::make_RPAREN(loc); }
[ \t\r]+                 { loc.step(); }
\n+                      { loc.lines(yyleng); loc.step(); }
<<EOF>>                  { return FilterParser::make_YYEOF(loc); }

%%

namespace Dwm {

  namespace Mclog {

    void FilterDriver::scan_begin()
    {
      yy_flex_debug = true;
      if (file.empty() || ("-" == file)) {
        yyin = stdin;
      }
      else if (!(yyin = fopen (file.c_str(), "r"))) {
        std::cerr << "cannot open " << file << ": " << strerror(errno) << '\n';
        exit(EXIT_FAILURE);
      }
    }

    void FilterDriver::scan_end()
    {
      fclose(yyin);
    }

    
  }  // namespace Mclog

}  // namespace Dwm
