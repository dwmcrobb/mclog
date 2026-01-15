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
//!  @file mclogd.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <signal.h>
  #include <unistd.h>
}

#include "DwmSysLogger.hh"
#include "DwmMclogLocalReceiver.hh"
#include "DwmMclogMulticaster.hh"
#include "DwmMclogMulticastReceiver.hh"
#include "DwmMclogFileLogger.hh"

static Dwm::Mclog::LocalReceiver      g_localReceiver;
static Dwm::Mclog::Multicaster        g_mcaster;
static Dwm::Mclog::MulticastReceiver  g_mcastReceiver;
static Dwm::Mclog::FileLogger         g_fileLogger;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool Restart(const std::string & configPath)
{
  bool  rc = false;
  g_localReceiver.Stop();
  
  Dwm::Mclog::Config  config;
  if (config.Parse("/usr/local/etc/mclogd.cfg")) {
    if (g_fileLogger.Restart(config.files)) {
      if (g_mcaster.Restart(config)) {
        if (g_mcastReceiver.Restart(config)) {
          if (g_localReceiver.Restart()) {
            rc = true;
          }
        }
      }
    }
  }
  return rc;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void BlockSigHupAndTerm()
{
  sigset_t  blockSet;
  sigemptyset(&blockSet);
  sigaddset(&blockSet, SIGHUP);
  sigaddset(&blockSet, SIGTERM);
  sigprocmask(SIG_BLOCK,&blockSet,NULL);
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static int WaitSigHupOrTerm()
{
  sigset_t  sigSet;
  sigemptyset(&sigSet);
  sigaddset(&sigSet, SIGHUP);
  sigaddset(&sigSet, SIGTERM);
  int  signum;
  sigwait(&sigSet, &signum);
  std::cerr << "Got signal " << signum << '\n';
  
  return signum;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  Dwm::SysLogger::Open("mclogd", LOG_PERROR, LOG_USER);                                              
  Dwm::Mclog::Config  config;
  if (config.Parse("/usr/local/etc/mclogd.cfg")) {
    g_mcaster.Open(config);
    g_fileLogger.Start(config.files);
    g_localReceiver.AddSink(g_mcaster.OutputQueue());
    g_localReceiver.AddSink(g_fileLogger.InputQueue());
    g_localReceiver.Start();
    g_mcastReceiver.AddSink(g_fileLogger.InputQueue());
    g_mcastReceiver.Open(config, false);
    for (;;) {
      BlockSigHupAndTerm();
      int  sig = WaitSigHupOrTerm();
      if (SIGHUP == sig) {
        Restart("/usr/local/etc/mclogd.cfg");
      }
      else if (SIGTERM == sig) {
        g_localReceiver.Stop();
        g_mcaster.Close();
        g_fileLogger.Stop();
        g_mcastReceiver.Close();
        exit(0);
      }
    }
  }
  return 0;
}
