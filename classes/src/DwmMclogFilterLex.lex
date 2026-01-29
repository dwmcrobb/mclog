%{
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

[^\|\&!\(\) \t\n]+     {
  auto  it = drv.cfg.selectors.find(YYText());
  if (it != drv.cfg.selectors.cend()) {
    auto  tok = FilterParser::make_SELECTOR(&(it->second), loc);
    drv.tokens.push_back(tok);
    return tok;
  }
  else {
    // auto  tok = FilterParser::make_SELECTOR(nullptr, loc);
    auto tok = FilterParser::make_YYerror(loc);
    drv.tokens.push_back(tok);
    std::cerr << drv.location << ": '" << YYText()
              << "' is not a valid selector\n";
    return tok;
  }                                                                            
}

"||"                   { auto tok = FilterParser::make_OR(loc);
                         drv.tokens.push_back(tok);
                         return tok;
                       }
"&&"                   { auto tok = FilterParser::make_AND(loc);
                         drv.tokens.push_back(tok);
                         return tok;
                       }
"!"                    { auto tok = FilterParser::make_NOT(loc);
                         drv.tokens.push_back(tok);
                         return tok;
                       }
"("                    { auto tok = FilterParser::make_LPAREN(loc);
                         drv.tokens.push_back(tok);
                         return tok;
                       }
")"                    { auto tok = FilterParser::make_RPAREN(loc);
                         drv.tokens.push_back(tok);
                         return tok;
                       }
[ \t\r]+               { loc.step(); }
\n+                    { loc.lines(yyleng); loc.step(); }
<<EOF>>                { auto tok = FilterParser::make_YYEOF(loc);
                         drv.tokens.push_back(tok);
                         return tok;
                       }

%%

namespace Dwm {

  namespace Mclog {

#if 0
    void FilterDriver::scan_begin()
    {
      // yy_flex_debug = true;
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
#endif
    
  }  // namespace Mclog

}  // namespace Dwm
