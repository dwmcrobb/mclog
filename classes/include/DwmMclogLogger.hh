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
//!  @file DwmMclogLogger.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Logger class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGLOGGER_HH_
#define _DWMMCLOGLOGGER_HH_

#include <memory>
#include <mutex>
#include <source_location>

#if __has_include(<format>)
#  include <format>
#  define DWM_MCLOG_HAVE_STD_FORMAT 1
#endif

#ifndef DWM_MCLOG_HAVE_STD_FORMAT
#  if __has_include(<fmt/format.h>)
#    include <fmt/format.h>
#    define DWM_MCLOG_HAVE_LIBFMT 1
#  endif
#endif

#include "DwmIpv4Address.hh"
#include "DwmThreadQueue.hh"
#include "DwmMclogMessage.hh"
#include "DwmMclogMessagePacket.hh"
#include "DwmMclogUdpEndpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class Logger
    {
    public:
      static const int  logStderr = 0x20;
      static const int  logSyslog = 0x40;

      using Clock = std::chrono::system_clock;
      
      static bool Open(const char *ident, int logopt, Facility facility);

      static bool LogLocations()
      { return _logLocations; }
      
      static bool LogLocations(bool logLocations)
      { return _logLocations = logLocations; }
      
      static bool Close();

#if DWM_MCLOG_HAVE_STD_FORMAT
      template <typename ...Args>
      static bool Log(std::source_location loc, Severity severity,
                      std::format_string<Args...> fm, Args &&...args)
      {
        return Log(severity, Format(fm, std::forward<Args>(args)...), loc);
      }
#else
#  if DWM_MCLOG_HAVE_LIBFMT
      template <typename ...Args>
      static bool Log(std::source_location loc, Severity severity,
                      fmt::format_string<Args...> fm, Args &&...args)
      {
        return Log(severity, Format(fm, std::forward<Args>(args)...), loc);
      }
#  endif
#endif
      
      static bool
      Log(Severity severity, std::string && msg,
          std::source_location loc = std::source_location::current());
      
    private:
      static MessageOrigin           _origin;
      static Facility                _facility;
      static std::atomic<bool>       _logLocations;
      static int                     _options;
      static int                     _ofd;
      static UdpEndpoint             _dstAddr;
      static std::mutex              _ofdmtx;
      static std::mutex              _cerrmtx;
      static Thread::Queue<Message>  _msgs;
      static std::thread             _thread;
      static std::atomic<bool>       _run;
      static Clock::time_point       _nextSendTime;
      
      static bool OpenSocket();
      static bool SendMessage(const Message & msg);
      static bool SendPacket(MessagePacket & pkt);
      static void Run();
      
#if DWM_MCLOG_HAVE_STD_FORMAT
      template <typename ...Args>
      static std::string Format(std::format_string<Args...> fm, Args &&...args)
      {
        return std::format(fm, std::forward<Args>(args)...);
      }
#else
#  if DWM_MCLOG_HAVE_LIBFMT
      template <typename ...Args>
      static std::string Format(fmt::format_string<Args...> fm, Args &&...args)
      {
        return fmt::format(fm, std::forward<Args>(args)...);
      }
#  endif
#endif
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#define MCLOG(...) \
  Dwm::Mclog::Logger::Log(std::source_location::current(),__VA_ARGS__)

#endif  // _DWMMCLOGLOGGER_HH_
