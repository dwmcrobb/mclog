%{
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
  //!  @file DwmMclogConfigLex.lex
  //!  @author Daniel W. McRobb
  //!  @brief NOT YET DOCUMENTED
  //---------------------------------------------------------------------------
  
  #include <map>
  #include <regex>

  #include "DwmIpv4Address.hh"
  #include "DwmIpv6Address.hh"
  #include "DwmMclogConfig.hh"
  #include "DwmMclogConfigParse.hh"

  extern std::string  g_configPath;
  
  extern "C" {
    #include <stdarg.h>

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void mclogcfgerror(const char *arg, ...)
    {
      va_list  ap;
      va_start(ap, arg);
      vfprintf(stderr, arg, ap);
      va_end(ap);
      fprintf(stderr, " at line %d of %s\n",
              yylineno, g_configPath.c_str());
      return;
    }
  }

  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  static const std::map<std::string,int>  g_configKeywords = {
    { "facility",           FACILITY        },
    { "files",              FILES           },
    { "filter",             FILTER          }, 
    { "groupAddr",          GROUPADDR       },
    { "groupAddr6",         GROUPADDR6      },
    { "host",               HOST            },
    { "ident",              IDENT           },
    { "intfAddr",           INTFADDR        },
    { "intfAddr6",          INTFADDR6       },
    { "intfName",           INTFNAME        },
    { "keep",               KEEP            },
    { "keyDirectory",       KEYDIRECTORY    },
    { "listenV4",           LISTENV4        },
    { "listenV6",           LISTENV6        },
    { "logDirectory",       LOGDIRECTORY    },
    { "logs",               LOGS            },
    { "loopback",           LOOPBACK        },
    { "minimumSeverity",    MINIMUMSEVERITY },
    { "multicast",          MULTICAST       },
    { "outFilter",          OUTFILTER       },
    { "path",               PATH            },
    { "perms",              PERMS           },
    { "port",               PORT            },
    { "selectors",          SELECTORS       },
    { "service",            SERVICE         }
  };

  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  static bool IsKeyword(const std::string & s, int & token)
  {
    bool  rc = false;
    auto  it = g_configKeywords.find(s);
    if (g_configKeywords.end() != it) {
      token = it->second;
      rc = true;
    }
    return rc;
  }

  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  static bool IsNumber(const std::string & s)
  {
    using std::regex, std::smatch;
    static regex  rgx("[0-9]+", regex::ECMAScript|regex::optimize);
    smatch        sm;
    return std::regex_match(s, sm, rgx);
  }

%}

%option noyywrap
%option prefix="mclogcfg"
%option yylineno

%x x_quoted
        
%%

<INITIAL>#.*\n
<INITIAL>[^ \t\n\[\]{}=,;!\|\&"]+  {
                                     if (IsNumber(yytext)) {
                                       mclogcfglval.intVal =
                                         std::stol(yytext, nullptr, 0);
                                       return INTEGER;
                                     }
                                     else {
                                       int  token;
                                       if (IsKeyword(yytext, token)) {
                                         return token;
                                       }
                                       else {
                                         mclogcfglval.stringVal =
                                           new std::string(yytext);
                                         return STRING;
                                       }
                                     }
                                   }
<INITIAL>[\|]{2,2}                 { return LOGICALOR; }
<INITIAL>[\&]{2,2}                 { return LOGICALAND; }
<INITIAL>[!]                       { return NOT; }
<INITIAL>["]                       { BEGIN(x_quoted); }
<x_quoted>[^"]+                    { mclogcfglval.stringVal =
                                       new std::string(yytext);
                                     return STRING; }
<x_quoted>["]                      { BEGIN(INITIAL); }
<INITIAL>[=,;\[\]\{\}]             { return yytext[0]; }
<INITIAL>[ \t\n]

%%
