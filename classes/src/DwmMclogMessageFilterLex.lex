%{
  #include "DwmMclogMessageFilterDriver.hh"
  #include "DwmMclogMessageFilterParse.hh"

  using MFP = Dwm::Mclog::MessageFilterParser;

  #undef yyterminate
  #define yyterminate() return MFP::make_YYEOF(loc)
%}

%option c++
%option nodefault
%option noyywrap
%option prefix="msgfilt"
%option yyclass="Dwm::Mclog::MessageFilterScanner"

m_sevval (debug|info|notice|warning|err|crit|alert|emerg)
m_facval (kernel|user|mail|daemon|auth|syslog|lpr|news|uucp|cron|authpriv|ftp|local[0-7])

%x x_quoted
%x x_regex

%{
  #define YY_USER_ACTION  loc.columns(yyleng);
%}

%%

%{
  // A handy shortcut to the location held by the driver.
  auto &  loc = drv.location;
  // Code run each time yylex is called.
  loc.step();
%}

<INITIAL>severity  {
  auto tok = MFP::make_SEVERITY(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>facility  {
  auto tok = MFP::make_FACILITY(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>host      {
  auto tok = MFP::make_HOST(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>ident     {
  auto tok = MFP::make_IDENT(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>{m_sevval} {
  auto tok = MFP::make_SEVVALUE(Dwm::Mclog::SeverityValue(YYText()),loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>{m_facval} {
  auto tok = MFP::make_FACVALUE(Dwm::Mclog::FacilityValue(YYText()),loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"<="      {
  auto tok = MFP::make_LESSOREQ(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>">="      {
  auto tok = MFP::make_GREATEROREQ(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"!="      {
  auto tok = MFP::make_NOTEQ(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"||"      {
  auto tok = MFP::make_OR(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"&&"      {
  auto tok = MFP::make_AND(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"!"       {
  auto tok = MFP::make_NOT(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"("       {
  auto tok = MFP::make_LPAREN(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>")"       {
  auto tok = MFP::make_RPAREN(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"<"       {
  auto tok = MFP::make_LESS(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>">"       {
  auto tok = MFP::make_GREATER(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>"="       {
  auto tok = MFP::make_EQUAL(loc);
  drv.tokens.push_back(tok);
  return tok;
}
<INITIAL>["]       { BEGIN(x_quoted); }
<x_quoted>[^"]*    {
  auto tok = MFP::make_STRING(YYText(), loc);
  drv.tokens.push_back(tok);
  return tok;
}
<x_quoted>["]      { BEGIN(INITIAL); }
<INITIAL>[\/]      { BEGIN(x_regex); }
<x_regex>[^\/]*    {
  try {
    auto tok = MFP::make_REGEX(boost::regex(YYText()), loc);
    drv.tokens.push_back(tok);
    return tok;
  }
  catch (boost::bad_expression & bex) {
    std::cerr << "Exception creating regex from '" << YYText() << "': "
              << bex.what() << '\n';
    auto tok = MessageFilterParser::make_YYerror(loc);
    drv.tokens.push_back(tok);
    return tok;
  }
}
<x_regex>[\/]      { BEGIN(INITIAL); }
  
<INITIAL>[ \t\r]+  { loc.step(); }
<INITIAL>\n+       { loc.lines(yyleng); loc.step(); }
<INITIAL>.         {
  auto tok = MessageFilterParser::make_YYerror(loc);
  drv.tokens.push_back(tok);
  return tok;
}

%%
