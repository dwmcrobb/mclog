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
//!  @file TestConfig.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmMclogConfig.hh"

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  Dwm::Mclog::Config  cfg;
  
  assert(cfg.Parse("inputs/TestConfig1.cfg"));
  assert(cfg.service.keyDirectory == "/usr/local/etc/mclogd");
  assert(cfg.mcast.groupAddr == Dwm::Ipv4Address("239.108.111.103"));
  assert(cfg.mcast.groupAddr6 == Dwm::Ipv6Address("ff02::006d:636c:6f67"));
  assert(cfg.mcast.intfAddr == Dwm::Ipv4Address("192.168.168.42"));
  assert(cfg.mcast.intfAddr6 == Dwm::Ipv6Address("fd60:3019:f4a:6aaf::39"));
  assert(cfg.mcast.intfName == "en0");
  assert(cfg.mcast.dstPort == 3456);
  assert(cfg.mcast.outFilter == "mydaemons || myapps");
  
  assert(cfg.files.logDirectory == "/usr/local/var/logs");
  assert(false == cfg.loopback.ListenIpv4());
  assert(true == cfg.loopback.ListenIpv6());
  assert(3737 == cfg.loopback.port);
  assert(5 == cfg.selectors.size());
  assert(cfg.selectors.begin()->first == "myapps");

  auto it = cfg.selectors.find("mydaemons");
  assert(it != cfg.selectors.end());
  assert(it->second.Facilities().first.size() == 8);

  assert(2 == cfg.files.logs.size());
  assert(cfg.files.logs[0].filter == "mydaemons");
  assert(cfg.files.logs[0].pathPattern == "%H/%I");
  assert(cfg.files.logs[1].filter == "myapps");
  assert(cfg.files.logs[1].pathPattern == "%H/myapps");
  
  return 0;
}
