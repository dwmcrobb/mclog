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

#include "DwmDaemonUtils.hh"
#include "DwmSysLogger.hh"
#include "DwmMclogLoopbackReceiver.hh"
#include "DwmMclogMulticaster.hh"
#include "DwmMclogMulticastReceiver.hh"
#include "DwmMclogFileLogger.hh"

static Dwm::Mclog::LoopbackReceiver   g_loopbackReceiver;
static Dwm::Mclog::Multicaster        g_mcaster;
static Dwm::Mclog::MulticastReceiver  g_mcastReceiver;
static Dwm::Mclog::FileLogger         g_fileLogger;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static bool Restart(const std::string & configPath)
{
  bool  rc = false;
  g_loopbackReceiver.Stop();
  
  Dwm::Mclog::Config  config;
  if (config.Parse("/usr/local/etc/mclogd.cfg")) {
    if (g_fileLogger.Restart(config.files)) {
      if (g_mcaster.Restart(config)) {
        if (g_mcastReceiver.Restart(config)) {
          rc = g_loopbackReceiver.Restart();
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
  sigaddset(&blockSet, SIGINT);
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
  sigaddset(&sigSet, SIGINT);
  int  signum;
  sigwait(&sigSet, &signum);
  Syslog(LOG_INFO, "Got signal %d", signum);
  
  return signum;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static std::string  g_pidFile;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void SavePID(const std::string & pidFile)
{
#ifndef O_EXLOCK
  #define O_EXLOCK 0
#endif
  g_pidFile = pidFile;
  pid_t  pid = getpid();
  std::string  &&pidstr = std::to_string(pid) + "\n";
  int    fd = open(g_pidFile.c_str(), O_WRONLY|O_CREAT|O_TRUNC|O_EXLOCK, 0644);
  if (fd >= 0) {
    if (write(fd, pidstr.c_str(), pidstr.size()) != pidstr.size()) {
      FSyslog(LOG_ERR, "Failed to save PID in {}", g_pidFile);
    }
    close(fd);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void RemovePID()
{
  if (! g_pidFile.empty()) {
    std::remove(g_pidFile.c_str());
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void Usage(const char *argv0)
{
  std::cerr << "usage: " << argv0 << " [-c configFile] [-d] [-p pidfile]\n";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  bool         daemonize = true;
  int          syslogOpts = 0;
  std::string  pidFile("/var/run/mclogd.pid");
  std::string  configPath("/usr/local/etc/mclogd.cfg");
  
  int  optChar;
  while ((optChar = getopt(argc, argv, "c:dp:")) != -1) {
    switch (optChar) {
      case 'c':
        configPath = optarg;
        break;
      case 'd':
        syslogOpts |= LOG_PERROR;
        daemonize = false;
        break;
      case 'p':
        pidFile = optarg;
        break;
      default:
        Usage(argv[0]);
        exit(1);
        break;
    }
  }

  if (daemonize) {
    Dwm::DaemonUtils::Daemonize();
  }
  
  Dwm::SysLogger::Open("mclogd", syslogOpts, LOG_USER);
  
  Dwm::Mclog::Config  config;
  if (config.Parse(configPath)) {
    SavePID(pidFile);
    g_mcaster.Open(config);
    g_fileLogger.Start(config.files);
    g_loopbackReceiver.AddSink(g_mcaster.OutputQueue());
    g_loopbackReceiver.AddSink(g_fileLogger.InputQueue());
    g_loopbackReceiver.Start();
    g_mcastReceiver.AddSink(g_fileLogger.InputQueue());
    g_mcastReceiver.Open(config, false);
    for (;;) {
      BlockSigHupAndTerm();
      int  sig = WaitSigHupOrTerm();
      if (SIGHUP == sig) {
        Restart(configPath);
      }
      else if ((SIGTERM == sig) || (SIGINT == sig)) {
        Syslog(LOG_INFO, "Received exit signal");
        g_loopbackReceiver.Stop();
        g_mcaster.Close();
        g_fileLogger.Stop();
        g_mcastReceiver.Close();
        RemovePID();
        exit(0);
      }
    }
  }
  return 0;
}
