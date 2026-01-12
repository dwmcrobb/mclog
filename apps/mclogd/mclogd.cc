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
  #include <unistd.h>
}

#include "DwmSysLogger.hh"
#include "DwmMclogLocalReceiver.hh"
#include "DwmMclogMulticaster.hh"
#include "DwmMclogMulticastReceiver.hh"
#include "DwmMclogLogFiles.hh"

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  Dwm::SysLogger::Open("mclogd", LOG_PERROR, LOG_USER);                                              
  Dwm::Mclog::LocalReceiver  localReceiver;
  Dwm::Mclog::Multicaster    mcaster;
  Dwm::Mclog::MulticastReceiver  mcastReceiver;
  
  Dwm::Thread::Queue<Dwm::Mclog::Message>  msgQueue;

  Dwm::Mclog::Config  config;
  if (config.Parse("/usr/local/etc/mclogd.cfg")) {
    mcaster.Open(config);
    localReceiver.Start(&msgQueue);
    mcastReceiver.Open(config.mcast.groupAddr, config.mcast.intfAddr,
                       config.mcast.dstPort, false);
    mcastReceiver.AddInputQueue(&msgQueue);
    Dwm::Mclog::LogFiles  logFiles("./logs");
    for (;;) {
      if (msgQueue.TimedWaitForNotEmpty(std::chrono::milliseconds(300))) {
        Dwm::Mclog::Message  msg;
        while (msgQueue.PopFront(msg)) {
          mcaster.Send(msg);
          logFiles.Log(msg);
        }
      }
    }
    localReceiver.Stop();
    mcaster.Close();
    mcastReceiver.Close();
  }
  return 0;
}
