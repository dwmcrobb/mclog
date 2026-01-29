%code requires
{
  #include <string>
  #include <map>
    
  #include "DwmIpv4Address.hh"
  #include "DwmIpv6Prefix.hh"

  using std::map, std::pair, std::string, Dwm::Ipv4Address,
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
  
  #include "DwmIpv4Prefix.hh"
  #include "DwmIpv6Prefix.hh"
  #include "DwmLocalInterfaces.hh"
  #include "DwmSysLogger.hh"
  #include "DwmMclogConfig.hh"

  using namespace std;
  
  string                 g_configPath;
  Dwm::Mclog::Config    *g_config = nullptr;

  static bool IsValidIpv4MulticastAddr(const Dwm::Ipv4Address & addr)
  {
    if (addr != Dwm::Ipv4Address()) {
      Dwm::Ipv4Prefix  pfx("224.0.0.0/4");
      return pfx.Contains(addr);
    }
    return false;
  }

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
  
  static bool IsValidIpv6MulticastAddr(const Dwm::Ipv6Address & addr)
  {
    if (addr != Dwm::Ipv6Address()) {
      Dwm::Ipv6Prefix  pfx("ff00::/8");
      return pfx.Contains(addr);
    }
    return false;
  }

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
      
%}

%define api.prefix {mclogcfg}

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
  Dwm::Mclog::MessageSelector               *msgSelectorVal;
  pair<string,Dwm::Mclog::MessageSelector>  *namedSelectorVal;
  map<string,Dwm::Mclog::MessageSelector>   *selectorsVal;
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

%token FACILITY FILES GROUPADDR GROUPADDR6 HOST IDENT INTFADDR INTFADDR6
%token INTFNAME KEYDIRECTORY LISTENV4 LISTENV6 LOGICALOR LOGICALAND LOOPBACK
%token LOGDIRECTORY MINIMUMSEVERITY MULTICAST NOT PORT SELECTORS SERVICE

%token<stringVal>  STRING
%token<intVal>     INTEGER

%type<uint16Val>          UDP4Port Port
%type<stringVal>          KeyDirectory LogDirectory IntfName
%type<serviceConfigVal>   ServiceSettings
%type<loopbackConfigVal>  LoopbackSettings
%type<filesConfigVal>     FilesSettings
%type<mcastConfigVal>     Multicast MulticastSettings
%type<ipv4AddrVal>        GroupAddr IntfAddr
%type<ipv6AddrVal>        GroupAddr6 IntfAddr6
%type<boolVal>            ListenV4 ListenV6
%type<msgSelectorVal>     SelectorSettings
%type<namedSelectorVal>   Selector
%type<selectorsVal>       SelectorList

%%

Config: TopStanza | Config TopStanza;

TopStanza: Service | Loopback | Multicast | Files | Selectors;

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

Selectors: SELECTORS '{' SelectorList '}' ';'
{
  if (g_config) {
    g_config->selectors = *($3);
  }
  delete $3;
};

SelectorList: Selector
{
  $$ = new std::map<std::string,Dwm::Mclog::MessageSelector>();
  $$->insert(*$1);
  delete $1;
}
| SelectorList ',' Selector
{
  $$->insert(*$3);
  delete $3;
};

Selector: STRING '=' '{' SelectorSettings '}'
{
  $$ = new std::pair<std::string,Dwm::Mclog::MessageSelector>(*$1,*$4);
  delete $1;
  delete $4;
};
     
SelectorSettings: HOST '=' STRING ';'
{
  $$ = new Dwm::Mclog::MessageSelector();
  if (! $$->SourceHost(*$3)) {
    mclogcfgerror("bad host expression '%s'", $3->c_str());
    delete $3;
    delete $$;
    return 1;
  }
  delete $3;
}
| HOST NOT '=' STRING ';'
{
  $$ = new Dwm::Mclog::MessageSelector();
  if (! $$->SourceHost(*($4), false)) {
    mclogcfgerror("bad host expression '%s'", $4->c_str());
    delete $4;
    delete $$;
    return 1;
  }
  delete $4;
}

| FACILITY '=' STRING ';'
{
  $$ = new Dwm::Mclog::MessageSelector();
  std::set<Dwm::Mclog::Facility>  facilities;
  Dwm::Mclog::Facilities(*($3), facilities);
  $$->Facilities(facilities);
  delete $3;
}
| FACILITY NOT '=' STRING ';'
{
  $$ = new Dwm::Mclog::MessageSelector();
  std::set<Dwm::Mclog::Facility>  facilities;
  Dwm::Mclog::Facilities(*($4), facilities);
  $$->Facilities(facilities, false);
  delete $4;
}
| MINIMUMSEVERITY '=' STRING ';'
{
  $$ = new Dwm::Mclog::MessageSelector();
  $$->MinimumSeverity(Dwm::Mclog::SeverityValue(*($3)));
  delete $3;
}
| IDENT '=' STRING ';'
{
  $$ = new Dwm::Mclog::MessageSelector();
  if (! $$->Ident(*$3)) {
    mclogcfgerror("bad ident expression '%s'", $3->c_str());
    delete $3;
    return 1;
  }
  delete $3;
}
| IDENT NOT '=' STRING ';'
{
  $$ = new Dwm::Mclog::MessageSelector();
  if (! $$->Ident(*($4), false)) {
    mclogcfgerror("bad ident expression '%s'", $4->c_str());
    delete $4;
    delete $$;
    return 1;
  }
  delete $4;
}
| SelectorSettings HOST '=' STRING ';'
{
  if (! $$->SourceHost(*$4)) {
    mclogcfgerror("bad host expression '%s'", $4->c_str());
    delete $4;
    return 1;
  }
  delete $4;
}
| SelectorSettings HOST NOT '=' STRING ';'
{
  if (! $$->SourceHost(*$5)) {
    mclogcfgerror("bad host expression '%s'", $5->c_str());
    delete $5;
    return 1;
  }
  delete $5;
}
| SelectorSettings FACILITY '=' STRING ';'
{
  std::set<Dwm::Mclog::Facility>  facilities;
  Dwm::Mclog::Facilities(*($4), facilities);
  $$->Facilities(facilities);
  delete $4;
}
| SelectorSettings FACILITY NOT '=' STRING ';'
{
  std::set<Dwm::Mclog::Facility>  facilities;
  Dwm::Mclog::Facilities(*($5), facilities);
  $$->Facilities(facilities, false);
  delete $5;
}
| SelectorSettings MINIMUMSEVERITY '=' STRING ';'
{
  $$->MinimumSeverity(Dwm::Mclog::SeverityValue(*($4)));
  delete $4;
}
| SelectorSettings IDENT '=' STRING ';'
{
  if (! $$->Ident(*$4)) {
    mclogcfgerror("bad ident expression '%s'", $4->c_str());
    delete $4;
    return 1;
  }
  delete $4;
}
| SelectorSettings IDENT NOT '=' STRING ';'
{
  if (! $$->Ident(*($5), false)) {
    mclogcfgerror("bad ident expression '%s'", $5->c_str());
    delete $5;
    return 1;
  }
  delete $5;
};

FilterExpression: STRING
{}
| NOT FilterExpression
{}
| FilterExpression LOGICALOR FilterExpression
{}
| FilterExpression LOGICALAND FilterExpression
{}
| '(' FilterExpression ')'
{};

%%

namespace Dwm {

  namespace Mclog {

    //---------------------------------------------------------------------
    //!  
    //---------------------------------------------------------------------
    bool MulticastConfig::ShouldSendIpv4() const
    {
      return ((groupAddr != Ipv4Address())
              && (intfAddr != Ipv4Address())
              && (dstPort != 0));
    }
    
    //-----------------------------------------------------------------------
    bool MulticastConfig::ShouldSendIpv6() const
    {
      return ((! intfName.empty())
              && (groupAddr6 != Ipv6Address())
              && (intfAddr6 != Ipv6Address())
              && (dstPort != 0));
    }

    //------------------------------------------------------------------------
    void MulticastConfig::Clear()
    {
      groupAddr = Ipv4Address();
      intfAddr = Ipv4Address();
      dstPort = 0;
      return;
    }

    //------------------------------------------------------------------------
    void ServiceConfig::Clear()
    {
      keyDirectory.clear();
      return;
    }
    
    //------------------------------------------------------------------------
    void Config::Clear()
    {
      loopback.Clear();
      mcast.Clear();
      service.Clear();
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
      Clear();
      
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
      if (rc) {
        if ((ShouldSendIpv4() || ShouldSendIpv6())
            && (! service.keyDirectory.empty())) {
          FSyslog(LOG_INFO, "Config loaded from {}", path);
        }
        else {
          rc = false;
        }
      }
      else {
        FSyslog(LOG_ERR, "Failed to load config from {}", path);
      }
      return rc;
    }

  }  // namespace Mclog

}  // namespace Dwm
