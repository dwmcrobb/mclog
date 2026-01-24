%code requires
{
  #include <string>

  #include "DwmIpv4Address.hh"

  using std::string, Dwm::Ipv4Address, Dwm::Ipv6Address;
}

%{
  #include <cstdio>

  extern "C" {
    #include <netdb.h>

    extern void mclogcfgerror(const char *arg, ...);
    extern FILE *mclogcfgin;
  }
        
  #include <string>

  #include "DwmSysLogger.hh"
  #include "DwmMclogConfig.hh"

  using namespace std;
  
  string                 g_configPath;
  Dwm::Mclog::Config    *g_config = nullptr;

%}

%define api.prefix {mclogcfg}

%union {
  int                           intVal;
  uint16_t                      uint16Val;
  string                       *stringVal;
  Ipv4Address                  *ipv4AddrVal;
  Ipv6Address                  *ipv6AddrVal;
  Dwm::Mclog::ServiceConfig    *serviceConfigVal;
  Dwm::Mclog::MulticastConfig  *mcastConfigVal;
  Dwm::Mclog::FilesConfig      *filesConfigVal;
}

%code provides
{
  // Tell Flex the expected prototype of yylex.
  #define YY_DECL                             \
    int mclogcfglex ()

  // Declare the scanner.
  YY_DECL;
}

%token FILES GROUPADDR GROUPADDR6 INTFADDR INTFADDR6 INTFNAME KEYDIRECTORY
%token LOGDIRECTORY MULTICAST PORT SERVICE

%token<stringVal>  STRING
%token<intVal>     INTEGER

%type<uint16Val>          UDP4Port Port
%type<stringVal>          KeyDirectory LogDirectory IntfName
%type<serviceConfigVal>   ServiceSettings
%type<filesConfigVal>     FilesSettings
%type<mcastConfigVal>     Multicast MulticastSettings
%type<ipv4AddrVal>        GroupAddr IntfAddr
%type<ipv6AddrVal>        GroupAddr6 IntfAddr6

%%

Config: TopStanza | Config TopStanza;

TopStanza: Service | Multicast | Files;

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
  delete $3;
};

GroupAddr6: GROUPADDR6 '=' STRING ';'
{
  $$ = new Dwm::Ipv6Address(*($3));
  delete $3;
};

IntfAddr: INTFADDR '=' STRING ';'
{
  $$ = new Dwm::Ipv4Address(*$3);
  delete $3;
};

IntfAddr6: INTFADDR6 '=' STRING ';'
{
  $$ = new Dwm::Ipv6Address(*($3));
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
