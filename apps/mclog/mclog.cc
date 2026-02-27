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

#include <filesystem>
#include <fstream>

#include "DwmIpv4Address.hh"
#include "DwmStreamIO.hh"
#include "DwmSysLogger.hh"
#include "DwmTimeValue64.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogConfig.hh"
#include "DwmMclogLogger.hh"
#include "DwmMclogKeyRequester.hh"
#include "DwmMclogMulticastReceiver.hh"
#include "DwmMclogMessageFilterDriver.hh"

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
class MySink
  : public Dwm::Mclog::MessageSink
{
public:
  MySink()
      : _filter(nullptr)
  {
  }

  void SetFilter(const std::string & expr)
  {
    _filter = make_unique<Dwm::Mclog::MessageFilterDriver>(expr);
  }
  
  bool Process(const Dwm::Mclog::Message & msg) override
  {
    if (! _filter) {
      std::cout << msg << std::flush;
    }
    else {
      bool  result;
      if (_filter->parse(&msg, result) && result) {
        std::cout << msg << std::flush;
      }
    }
    return true;
  }

private:
  std::unique_ptr<Dwm::Mclog::MessageFilterDriver>  _filter;
};

#if 0
//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestIstreamOperatorPerformance()
{
  std::ifstream  is("/tmp/messages2");
  if (is) {
    Dwm::Mclog::Message  msg;
    size_t  count = 0;
    Dwm::TimeValue64  startTime(true);
    while (is >> msg) {
      ++count;
    }
    Dwm::TimeValue64  endTime(true);
    endTime -= startTime;
    std::cerr << count << " messages (" << count / (double)endTime
              << " messages/sec)\n";
    is.close();
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestBinaryReadPerformance()
{
  std::ifstream  is("/tmp/mclog_msgs/myapps");
  if (is) {
    Dwm::Mclog::Message  msg;
    size_t  count = 0;
    Dwm::TimeValue64  startTime(true);
    while (msg.Read(is)) {
      ++count;
    }
    Dwm::TimeValue64  endTime(true);
    endTime -= startTime;
    std::cerr << count << " messages (" << count / (double)endTime
              << " messages/sec)\n";
    is.close();
  }
  return;
}
#endif

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
void ProcessBZ2File(const char *filename,
                    std::shared_ptr<Dwm::Mclog::MessageFilterDriver> filter)
{
  BZFILE  *bzf = BZ2_bzopen(filename, "rb");
  if (bzf) {
    Dwm::Mclog::Message  msg;
    bool                 filterResult;
    while (msg.BZRead(bzf) > 0) {
      if ((! filter) || (filter->parse(&msg, filterResult) && filterResult)) {
        std::cout << msg;
      }
    }
    BZ2_bzclose(bzf);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
void ProcessGZFile(const char *filename,
                   std::shared_ptr<Dwm::Mclog::MessageFilterDriver> filter)
{
  gzFile  gzf = gzopen(filename, "rb");
  if (gzf) {
    Dwm::Mclog::Message  msg;
    bool                 filterResult;
    while (msg.Read(gzf) > 0) {
      if ((! filter) || (filter->parse(&msg, filterResult) && filterResult)) {
        std::cout << msg;
      }
    }
    gzclose(gzf);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
void ProcessBinaryFile(const char *filename,
                       std::shared_ptr<Dwm::Mclog::MessageFilterDriver> filter)
{
  std::ifstream  is(filename);
  if (is) {
    Dwm::Mclog::Message  msg;
    bool                 filterResult;
    while (msg.Read(is)) {
      if ((! filter) || (filter->parse(&msg, filterResult) && filterResult)) {
        std::cout << msg;
      }
    }
    is.close();
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
void ProcessFile(const std::filesystem::path & path,
                 std::shared_ptr<Dwm::Mclog::MessageFilterDriver> filter)
{
  auto  ext = path.extension();
  if (ext.string() == ".bz2") {
    ProcessBZ2File(path.string().c_str(), filter);
  }
  else if (ext.string() == ".gz") {
    ProcessGZFile(path.string().c_str(), filter);
  }
  else {
    ProcessBinaryFile(path.string().c_str(), filter);
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void Usage(const char *argv0)
{
  std::cerr << "usage: " << argv0
            << " [-d] [-F filterExpression] [files...]\n";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  bool         debug = false;
  std::string  filtexpr;
  int          optChar;
  while ((optChar = getopt(argc, argv, "dF:")) != -1) {
    switch (optChar) {
      case 'd':
        debug = true;
        break;
      case 'F':
        filtexpr = optarg;
        break;
      default:
        Usage(argv[0]);
        exit(1);
        break;
    }
  }

  std::shared_ptr<Dwm::Mclog::MessageFilterDriver>  filter{nullptr};
  
  if (! filtexpr.empty()) {
    filter = make_shared<Dwm::Mclog::MessageFilterDriver>(filtexpr);
  }

  if (optind < argc) {
    for (int i = optind; i < argc; ++i) {
      ProcessFile(argv[i], filter);
    }
    return 0;
  }

  Dwm::Mclog::OstreamSink  cerrSink(std::cerr);
  if (debug) {
    Dwm::SysLogger::Open("mclog", LOG_PERROR, LOG_USER);
    Dwm::Mclog::logger.Open(Dwm::Mclog::Facility::user, {&cerrSink}, "mclog");
  }
  
  std::string  keyDir("~/.credence");
  Dwm::Mclog::MulticastReceiver  mcastRecv;
  Dwm::Mclog::Config  config;
  if (config.Parse("/usr/local/etc/mclogd.cfg")) {
    config.service.keyDirectory = keyDir;
    if (mcastRecv.Open(config)) {
      MySink  mysink;
      if (! filtexpr.empty()) {
        mysink.SetFilter(filtexpr);
      }
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

          
          
