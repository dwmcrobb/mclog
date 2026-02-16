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
//!  @file mclog.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
  #include <unistd.h>
}

#include <cstdio>
#include <fstream>
#include <regex>

#include "DwmMclogLoopbackSender.hh"

#if 0
//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
class MySink
  : public Dwm::Mclog::MessageSink
{
public:
  MySink()
  {
  }
  
  bool Process(const Dwm::Mclog::Message & msg) override
  {
    std::cout << msg << std::flush;
    return true;
  }

private:
  //  Dwm::Mclog::MessageSelector  _selector;
};
#endif

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static int OpenSocket()
{
  bool  rc = false;
  int fd = socket(PF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    std::cerr << "socket(PF_INET, SOCK_DGRAM, 0) failed: "
              << strerror(errno);
  }
  return fd;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static Dwm::Mclog::Message MakeMessage(const char *appname,
                                       Dwm::Mclog::Facility facility,
                                       Dwm::Mclog::Severity severity,
                                       const std::string & msg)
{
  char  hn[255];
  memset(hn, 0, sizeof(hn));
  gethostname(hn, sizeof(hn));
  Dwm::Mclog::MessageOrigin  origin(hn, appname, getppid());
  Dwm::Mclog::MessageHeader  header(facility, severity, origin);
  return Dwm::Mclog::Message(header, msg);
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void Usage(const char *argv0)
{
  std::cerr << "usage: " << argv0 << " [-p priority] ident message\n";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
std::pair<Dwm::Mclog::Facility,Dwm::Mclog::Severity>
GetPriorityInfo(const std::string & priority)
{
  std::regex  prgx("(kernel|user|mail|daemon|auth|syslog|lpr|news|uucp|cron"
                   "|authpriv|ftp|local[0-7])\\.(emerg|alert|crit|err|warning"
                   "|notice|info|debug)",std::regex::optimize);
  std::smatch  sm;
  if (regex_match(priority, sm, prgx)) {
    if (sm.size() == 3) {
      std::cerr << sm[1].str() << '.' << sm[2].str() << '\n';
      return std::make_pair(Dwm::Mclog::FacilityValue(sm[1].str()),
                            Dwm::Mclog::SeverityValue(sm[2].str()));
    }
  }
  std::cerr << "Invalid priority '" << priority << "'\n";
  exit(1);
  return std::make_pair(Dwm::Mclog::Facility::user,
                        Dwm::Mclog::Severity::info);
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  using Dwm::Mclog::Facility, Dwm::Mclog::Severity;
  std::pair<Facility,Severity>  priority =
    std::make_pair(Facility::user, Severity::info);
  
  int  optChar;
  while ((optChar = getopt(argc, argv, "p:")) != -1) {
    switch (optChar) {
      case 'p':
        priority = GetPriorityInfo(optarg);
        break;
      default:
        Usage(argv[0]);
        exit(1);
        break;
    }
  }

  if (optind <= (argc - 2)) {
    int  fd = OpenSocket();
    if (0 <= fd) {
      auto  msg = MakeMessage(argv[argc-2], priority.first,
                              priority.second, argv[argc-1]);
      char  pktbuf[1024];
      Dwm::Mclog::MessagePacket  pkt(pktbuf, sizeof(pktbuf));
      pkt.Add(msg);
      Dwm::Mclog::UdpEndpoint  dstAddr4(Dwm::Ipv4Address("127.0.0.1"), 3737);
      pkt.SendTo(fd, dstAddr4);
      ::close(fd);
      exit(0);
    }
    else {
      exit(1);
    }
  }
  else {
    Usage(argv[0]);
    exit(1);
  }
}
