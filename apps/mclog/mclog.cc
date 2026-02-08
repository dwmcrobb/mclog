//===========================================================================
// @(#) $DwmPath$
//===========================================================================
//  Copyright (c) Daniel W. McRobb 2025
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

#include <fstream>

#include "DwmIpv4Address.hh"
#include "DwmStreamIO.hh"
#include "DwmSysLogger.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogConfig.hh"
#include "DwmMclogMessageSelector.hh"
#include "DwmMclogMessageSink.hh"
#include "DwmMclogKeyRequester.hh"
#include "DwmMclogMulticastReceiver.hh"

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

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void Usage(const char *argv0)
{
  std::cerr << "usage: " << argv0 << " [-d]\n";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  bool  debug = false;
  
  int  optChar;
  while ((optChar = getopt(argc, argv, "d")) != -1) {
    switch (optChar) {
      case 'd':
        debug = true;
        break;
      default:
        Usage(argv[0]);
        exit(1);
        break;
    }
  }

  if (debug) {
    Dwm::SysLogger::Open("mclog", LOG_PERROR, LOG_USER);
  }
  
  std::string  keyDir("~/.credence");
  Dwm::Mclog::MulticastReceiver  mcastRecv;
  Dwm::Mclog::Config  config;
  if (config.Parse("/usr/local/etc/mclogd.cfg")) {
    config.service.keyDirectory = keyDir;
    if (mcastRecv.Open(config)) {
      MySink  mysink;
      mcastRecv.AddSink(&mysink);
      for (;;) {
#if 1
        sleep(60);
#else
        msgQueue.WaitForNotEmpty();
        while (! msgQueue.Empty()) {
          Dwm::Mclog::Message  msg;
          msgQueue.PopFront(msg);
          std::cout << msg;
        }
#endif
      }
    }
  }
  
  
  return 0;
}

          
          
