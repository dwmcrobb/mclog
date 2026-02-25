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
  //!  @file DwmMclogConfigParse.y
  //!  @author Daniel W. McRobb
  //!  @brief mclogd configuration parser
  //-------------------------------------------------------------------------
%}

%code requires
{
  #include <string>
  #include <map>
  #include <vector>

  #include "DwmIpv4Address.hh"
  #include "DwmIpv6Prefix.hh"

  using std::map, std::pair, std::string, std::vector, Dwm::Ipv4Address,
        Dwm::Ipv6Address;
}

%{
  #include <cstdio>

  extern "C" {
    #include <ifaddrs.h>
    #include <netdb.h>

    extern void mclogcfgerror(const char *arg, ...);
    extern FILE *mclogcfgin;
  }

  #include <map>
  #include <string>
  #include <vector>
  #include <boost/regex.hpp>

  #include "DwmIpv4Prefix.hh"
  #include "DwmIpv6Prefix.hh"
  #include "DwmLocalInterfaces.hh"
  #include "DwmSysLogger.hh"
  #include "DwmMclogConfig.hh"

  using namespace std;
  
  string                 g_configPath;
  Dwm::Mclog::Config    *g_config = nullptr;

  //-------------------------------------------------------------------------
  static bool IsValidIpv4MulticastAddr(const Dwm::Ipv4Address & addr)
  {
    if (addr != Dwm::Ipv4Address()) {
      Dwm::Ipv4Prefix  pfx("224.0.0.0/4");
      return pfx.Contains(addr);
    }
    return false;
  }

  //-------------------------------------------------------------------------
  static bool IsValidIpv4IntfAddr(const Dwm::Ipv4Address & addr)
  {
    std::map<std::string,Dwm::LocalInterface>  intfs;
    Dwm::GetLocalInterfaces(intfs);
    auto  it = std::find_if(intfs.begin(), intfs.end(),
                            [&] (const auto & intf)
                            { if (intf.second.Addr() == addr) {
                                return true;
                              }
                              else {
                                auto  alit = std::find_if(intf.second.Aliases().begin(),
                                                          intf.second.Aliases().end(),
                                                          [&] (const auto & alias) 
                                                          { return (alias.Addr() == addr); });
                                return (alit != intf.second.Aliases().end());
                              }
                            }
                            );
    return (it != intfs.end());
  }
  
  //-------------------------------------------------------------------------
  static bool IsValidIpv6MulticastAddr(const Dwm::Ipv6Address & addr)
  {
    if (addr != Dwm::Ipv6Address()) {
      Dwm::Ipv6Prefix  pfx("ff00::/8");
      return pfx.Contains(addr);
    }
    return false;
  }

  //-------------------------------------------------------------------------
  static bool IsValidIpv6IntfAddr(const Dwm::Ipv6Address & addr)
  {
    bool  rc = false;
    struct ifaddrs  *ifAddrs;
    if (getifaddrs(&ifAddrs) == 0) {
      struct ifaddrs  *ifAddr = ifAddrs;
      while (ifAddr) {
        if (ifAddr->ifa_addr) {
          if (ifAddr->ifa_addr->sa_family == AF_INET6) {
            const sockaddr_in6  *sa6 =
                (const sockaddr_in6 *)ifAddr->ifa_addr;
            if (Dwm::Ipv6Address(sa6->sin6_addr) == addr) {
              rc = true;
              break;
            }
          }
        }
        ifAddr = ifAddr->ifa_next;
      }
      freeifaddrs(ifAddrs);
    }
    return rc;
  }

  //--------------------------------------------------------------------------
  static std::string RetainEscapes(std::string value)
  {
    return boost::regex_replace(value, boost::regex(std::string("\\\\")),
                                std::string("\\\\\\\\")); // ,
  }

  //--------------------------------------------------------------------------
  static
  std::string ExpandFilterExpr(std::map<std::string,std::string> & vars,
                               const std::string & value)
  {
    std::string  rc = value;
    for (const auto & var : vars) {
      boost::regex  rgx(std::string("\\$") + var.first);
      rc = boost::regex_replace(rc, rgx, RetainEscapes(var.second));
    }
    return rc;
  }

%}

%define api.prefix {mclogcfg}
%header

%union {
  int                                        intVal;
  uint16_t                                   uint16Val;
  string                                    *stringVal;
  Ipv4Address                               *ipv4AddrVal;
  Ipv6Address                               *ipv6AddrVal;
  Dwm::Mclog::ServiceConfig                 *serviceConfigVal;
  Dwm::Mclog::LoopbackConfig                *loopbackConfigVal;
  Dwm::Mclog::MulticastConfig               *mcastConfigVal;
  Dwm::Mclog::FilesConfig                   *filesConfigVal;
  map<string,string>                        *mapStringStringVal;
  pair<string,string>                       *stringPairVal;
  Dwm::Mclog::LogFileConfig                 *logFileVal;
  vector<Dwm::Mclog::LogFileConfig>         *logFilesVal;
  Dwm::Mclog::RollPeriod                     rollPeriodVal;
  Dwm::Mclog::FileFormat                     fileFormatVal;
  bool                                       boolVal;
}

%code provides
{
  // Tell Flex the expected prototype of yylex.
  #define YY_DECL                             \
    int mclogcfglex ()

  // Declare the scanner.
  YY_DECL;
}

%token BINARY COMPRESS FACILITY FILES FILTER FILTERS FORMAT GROUP GROUPADDR
%token GROUPADDR6 HOST IDENT INTFADDR INTFADDR6 INTFNAME KEEP KEYDIRECTORY
%token LISTENV4 LISTENV6 LOGICALOR LOGICALAND LOOPBACK LOGDIRECTORY LOGS
%token MINIMUMSEVERITY MULTICAST NOT OUTFILTER PATH PERIOD PERMS PORT
%token SERVICE TEXT USER

%token<stringVal>  STRING
%token<intVal>     INTEGER

%type<uint16Val>          UDP4Port Port
%type<stringVal>          Filter IntfName KeyDirectory LogDirectory
%type<intVal>             Keep Permissions
%type<rollPeriodVal>      RollPeriod
%type<fileFormatVal>      Format
%type<stringVal>          Compress Group OutFilter Path User
%type<serviceConfigVal>   ServiceSettings
%type<loopbackConfigVal>  LoopbackSettings
%type<filesConfigVal>     FilesSettings
%type<mcastConfigVal>     Multicast MulticastSettings
%type<ipv4AddrVal>        GroupAddr IntfAddr
%type<ipv6AddrVal>        GroupAddr6 IntfAddr6
%type<boolVal>            ListenV4 ListenV6
%type<mapStringStringVal> Filters FilterList
%type<stringPairVal>      FilterExpr
%type<logFilesVal>        Logs LogList
%type<logFileVal>         Log LogSettings

%%

Config: TopStanza | Config TopStanza;

TopStanza: Service | Loopback | Multicast | Files | Filters;

Service: SERVICE '{' ServiceSettings '}' ';'
{
  if (g_config) {
    g_config->service = (*$3);
  }
  delete $3;
};

ServiceSettings: KeyDirectory
{
  $$ = new Dwm::Mclog::ServiceConfig();
  $$->keyDirectory = (*$1);
  delete $1;
};

KeyDirectory : KEYDIRECTORY '=' STRING ';'
{
  $$ = $3;
};

Loopback: LOOPBACK '{' LoopbackSettings '}' ';'
{
  if (g_config) {
    g_config->loopback = *($3);
  }
  delete $3;
}

LoopbackSettings: ListenV4
{
  $$ = new Dwm::Mclog::LoopbackConfig();
  $$->listenIpv4 = $1;
}
| ListenV6
{
  $$ = new Dwm::Mclog::LoopbackConfig();
  $$->listenIpv6 = $1;
}
| Port
{
  $$ = new Dwm::Mclog::LoopbackConfig();
  $$->port = $1;
}
| LoopbackSettings ListenV4
{
  $$->listenIpv4 = $2;
}
| LoopbackSettings ListenV6
{
  $$->listenIpv6 = $2;
}
| LoopbackSettings Port
{
  $$->port = $2;
};

ListenV4: LISTENV4 '=' STRING ';'
{
  if ("false" == *($3)) {
    $$ = false;
  }
  else {
    $$ = true;
  }
  delete $3;
};

ListenV6: LISTENV6 '=' STRING ';'
{
  if ("true" == *($3)) {
    $$ = true;
  }
  else {
    $$ = false;
  }
  delete $3;
};


Files: FILES '{' FilesSettings '}' ';'
{
  if (g_config) {
    g_config->files = *($3);
  }
  delete $3;
};

FilesSettings: LogDirectory
{
  $$ = new Dwm::Mclog::FilesConfig();
  $$->logDirectory = *($1);
  delete $1;
}
| Logs
{
  $$ = new Dwm::Mclog::FilesConfig();
  $$->logs = *($1);
  delete $1;
}
| FilesSettings LogDirectory
{
  $$->logDirectory = *($2);
  delete $2;
}
| FilesSettings Logs
{
  $$->logs = *($2);
  delete $2;
};

LogDirectory: LOGDIRECTORY '=' STRING ';'
{
  $$ = $3;
};

Multicast: MULTICAST '{' MulticastSettings '}' ';'
{
  if (g_config) {
    g_config->mcast = *($3);
  }
  delete $3;
}

MulticastSettings: GroupAddr
{
  $$ = new Dwm::Mclog::MulticastConfig();
  $$->groupAddr = *($1);
  delete $1;
}
| GroupAddr6
{
  $$ = new Dwm::Mclog::MulticastConfig();
  $$->groupAddr6 = *($1);
  delete $1;
}
| IntfAddr
{
  $$ = new Dwm::Mclog::MulticastConfig();
  $$->intfAddr = *($1);
  delete $1;
}
| IntfAddr6
{
  $$ = new Dwm::Mclog::MulticastConfig();
  $$->intfAddr6 = *($1);
  delete $1;
}
| IntfName
{
  $$ = new Dwm::Mclog::MulticastConfig();
  $$->intfName = *($1);
  delete $1;
}
| Port
{
  $$ = new Dwm::Mclog::MulticastConfig();
  $$->dstPort = $1;
}
| OutFilter
{
    $$ = new Dwm::Mclog::MulticastConfig();
    $$->outFilter = *($1);
    delete $1;
}
| MulticastSettings GroupAddr
{
  $$->groupAddr = *($2);
  delete $2;
}
| MulticastSettings GroupAddr6
{
  $$->groupAddr6 = *($2);
  delete $2;
}
| MulticastSettings IntfAddr
{
  $$->intfAddr = *($2);
  delete $2;
}
| MulticastSettings Port
{
  $$->dstPort = $2;
}
| MulticastSettings OutFilter
{
    $$->outFilter = *($2);
    delete $2;
}
| MulticastSettings IntfAddr6
{
  $$->intfAddr6 = *($2);
  delete $2;
}
| MulticastSettings IntfName
{
  $$->intfName = *($2);
  delete $2;
}
;

GroupAddr: GROUPADDR '=' STRING ';'
{
  $$ = new Dwm::Ipv4Address(*$3);
  if (! IsValidIpv4MulticastAddr(*($$))) {
    mclogcfgerror("invalid ipv4 multicast group address %s",
                  $3->c_str());
    delete $3;
    return 1;
  }
  delete $3;
};

GroupAddr6: GROUPADDR6 '=' STRING ';'
{
  $$ = new Dwm::Ipv6Address(*($3));
  if (! IsValidIpv6MulticastAddr(*($$))) {
    mclogcfgerror("invalid ipv6 multicast group address %s",
                  $3->c_str());
    delete $3;
    return 1;
  }
  delete $3;
};

IntfAddr: INTFADDR '=' STRING ';'
{
  $$ = new Dwm::Ipv4Address(*$3);
  if (! IsValidIpv4IntfAddr(*$$)) {
    mclogcfgerror("Warning: invalid intfAddr %s for this host",
                  $3->c_str());
  }
  delete $3;
};

IntfAddr6: INTFADDR6 '=' STRING ';'
{
  $$ = new Dwm::Ipv6Address(*($3));
  if (Dwm::Ipv6Address() == *($$)) {
    mclogcfgerror("invalid ipv6 intfAddr address %s",
                  $3->c_str());
    delete $3;
    return 1;
  }
  else {
    if (! IsValidIpv6IntfAddr(*($$))) {
      mclogcfgerror("Warning: invalid intfAddr6 address %s for this host",
                    $3->c_str());
    }
  }
  delete $3;
};

IntfName: INTFNAME '=' STRING ';'
{
  $$ = $3;
};

Port: PORT '=' UDP4Port ';'
{
  $$ = $3;
};

UDP4Port: INTEGER
{
  if (($1 > 0) && ($1 < 65536)) {
    $$ = $1;
  }
  else {
    mclogcfgerror("invalid UDP port number");
    return 1;
  }
}
| STRING
{
  auto  servEntry = getservbyname($1->c_str(), "udp");
  if (servEntry) {
    $$ = ntohs(servEntry->s_port);
  }
  else {
    mclogcfgerror("unknown UDP service");
    delete $1;
    return 1;
  }
  delete $1;
};

Filters: FILTERS '{' FilterList '}' ';'
{
  if (g_config) {
    g_config->filters = *($3);
  }
  delete $3;
};

FilterList: FilterExpr
{
  $$ = new std::map<std::string,std::string>();
  $$->insert({$1->first, ExpandFilterExpr(*($$), $1->second)});
  delete $1;
}
| FilterList FilterExpr
{
  $$->insert({$2->first, ExpandFilterExpr(*($$), $2->second)});
  delete $2;
};

FilterExpr: STRING '=' STRING ';'
{
  $$ = new std::pair<std::string,std::string>(*($1),*($3));
  delete $1;
  delete $3;
};

OutFilter: OUTFILTER '=' STRING ';'
{
    $$ = new std::string(ExpandFilterExpr(g_config->filters,*($3)));
    delete $3;
};

Logs: LOGS '{' LogList '}' ';'
{
  $$ = $3;
};

LogList: Log
{
  $$ = new std::vector<Dwm::Mclog::LogFileConfig>();
  $$->push_back(*($1));
  delete $1;
}
| LogList ',' Log
{
  $$->push_back(*($3));
  delete $3;
};

Log: '{' LogSettings '}'
{
  $$ = $2;
};

LogSettings: Filter 
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->filter = *($1);
  delete $1;
}
| Path
{
    $$ = new Dwm::Mclog::LogFileConfig();
  $$->pathPattern = *($1);
  delete $1;
}
| Permissions
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->permissions = 0644;
  if (0 == ($1 & 07133)) {
    $$->permissions = $1;
  }
  else {
    mclogcfgerror("invalid perms '%o', using 0644", $1);
  }
}
| RollPeriod
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->period = $1;
}
| Keep
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->keep = 7;
  if (0 <= $1) {
    $$->keep = $1;
  }
  else {
    mclogcfgerror("invalid keep '%d', using 7", $1);
  }
}
| User
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->user = *($1);
  delete $1;
}
| Group
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->group = *($1);
  delete $1;
}
| Compress
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->compress = *($1);
  delete $1;
}
| Format
{
  $$ = new Dwm::Mclog::LogFileConfig();
  $$->format = $1;
}
| LogSettings Filter
{
  $$->filter = *($2);
  delete $2;
}
| LogSettings Path
{
  $$->pathPattern = *($2);
  delete $2;
}
| LogSettings Permissions
{
  $$->permissions = 0644;
  if (0 == ($2 & 07133)) {
    $$->permissions = $2;
  }
  else {
    mclogcfgerror("invalid perms '%o', using 0644", $2);
  }
}
| LogSettings RollPeriod
{
  $$->period = $2;
}
| LogSettings Keep
{
  $$->keep = 7;
  if (0 <= $2) {
    $$->keep = $2;
  }
  else {
    mclogcfgerror("invalid keep '%d', using 7", $2);
  }
}
| LogSettings User
{
  $$->user = *($2);
  delete $2;
}
| LogSettings Group
{
  $$->group = *($2);
  delete $2;
}
| LogSettings Compress
{
  $$->compress = *($2);
  delete $2;
}
| LogSettings Format
{
  $$->format = $2;
};

Filter: FILTER '=' STRING ';'
{
    $$ = new std::string(ExpandFilterExpr(g_config->filters, *($3)));
    delete $3;
};

Path: PATH '=' STRING ';'
{
  $$ = $3;
};

Permissions: PERMS '=' INTEGER ';'
{
  $$ = $3;
};

RollPeriod: PERIOD '=' STRING ';'
{
  $$ = Dwm::Mclog::GetRollPeriod(*($3));
  delete $3;
};

Keep: KEEP '=' INTEGER ';'
{
  $$ = $3;
};

User: USER '=' STRING ';'
{
  $$ = $3;
};

Group: GROUP '=' STRING ';'
{
  $$ = $3;
};

Compress: COMPRESS '=' STRING ';'
{
  $$ = $3;
};

Format: FORMAT '=' TEXT ';'
{
  $$ = Dwm::Mclog::FileFormat::text;
}
| FORMAT '=' BINARY ';'
{
  $$ = Dwm::Mclog::FileFormat::binary;
};

%%

namespace Dwm {

  namespace Mclog {

    //---------------------------------------------------------------------
    MulticastConfig::MulticastConfig()
    {
      Init();
    }
    
    //---------------------------------------------------------------------
    bool MulticastConfig::ShouldSendIpv4() const
    {
      return ((intfAddr != Ipv4Address())
              && (dstPort != 0));
    }
    
    //-----------------------------------------------------------------------
    bool MulticastConfig::ShouldSendIpv6() const
    {
      return ((! intfName.empty())
              && (intfAddr6 != Ipv6Address())
              && (dstPort != 0));
    }

    //-----------------------------------------------------------------------
    bool MulticastConfig::ShouldRunSender() const
    {
      return (ShouldSendIpv4() || ShouldSendIpv6());
    }
    
    //-----------------------------------------------------------------------
    void MulticastConfig::Init()
    {
      groupAddr = Ipv4Address("239.108.111.103");
      intfAddr = Ipv4Address();
      groupAddr6 = Ipv6Address("ff02::006d:636c:6f67");
      intfAddr6 = Ipv6Address();
      dstPort = 3737;
      intfName.clear();
      outFilter.clear();
    }
    
    //------------------------------------------------------------------------
    void ServiceConfig::Init()
    {
      keyDirectory = "/usr/local/etc/mclogd";
      return;
    }

    //------------------------------------------------------------------------
    LogFileConfig::LogFileConfig()
    { Init(); }

    //-----------------------------------------------------------------------
    void LogFileConfig::Init()
    {
      filter.clear();
      pathPattern = "%H/%I";
      permissions = 0644;
      period = RollPeriod::days_1;
      keep = 7;
      user.clear();
      group.clear();
      compress = "bzip2";
      format = FileFormat::text;
    }
    
    //-----------------------------------------------------------------------
    void FilesConfig::Init()
    {
        logDirectory = "/usr/local/var/mclog";
        logs.clear();
        return;
    }
    
    //------------------------------------------------------------------------
    void Config::Init()
    {
      loopback.Init();
      mcast.Init();
      service.Init();
      files.Init();
      
      return;
    }

    //-----------------------------------------------------------------------
    bool Config::ShouldSendIpv4() const
    { return mcast.ShouldSendIpv4(); }

    //-----------------------------------------------------------------------
    bool Config::ShouldSendIpv6() const
    { return mcast.ShouldSendIpv6(); }
    
    //------------------------------------------------------------------------
    bool Config::Parse(const string & path)
    {
      bool  rc = false;
      Init();
      
      mclogcfgin = fopen(path.c_str(), "r");
      if (mclogcfgin) {
        g_configPath = path;
        g_config = this;
        rc = (0 == mclogcfgparse());
        fclose(mclogcfgin);
      }
      else {
        FSyslog(LOG_ERR, "Failed to open config file {}", path);
      }
      return rc;
    }

  }  // namespace Mclog

}  // namespace Dwm
