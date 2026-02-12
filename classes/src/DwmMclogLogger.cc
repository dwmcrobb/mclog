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
//!  @file DwmMclogLogger.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Logger implementation
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
  #include <unistd.h>
}

#include <cassert>
#include <filesystem>
#include <span>
#include <version>

#if defined(__cpp_lib_spanstream)
#  if (__cpp_lib_spanstream >= 202106L)
#    if __has_include(<spanstream>)
#      include <spanstream>
#      define DWM_HAVE_STD_SPANSTREAM 1
#    endif
#  endif
#endif

#ifndef DWM_HAVE_STD_SPANSTREAM
#  include "spanstream.hh"
#endif

#if (defined(__FreeBSD__) || __APPLE__)
  #include <cstdlib>
  //--------------------------------------------------------------------------
  static const char *ProgramName()  { return getprogname(); }

#elif (defined(__linux__))
  #include <errno.h>
  //--------------------------------------------------------------------------
  static const char *ProgramName()  { return program_invocation_short_name; }
#endif

#include "DwmFormatters.hh"
#include "DwmMclogLogger.hh"
#include "DwmMclogUdpEndpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Logger::Logger()
        : _origin("","",0), _facility(Facility::user),
          _minimumSeverity(Severity::debug), _logLocations(false),
          _sinksMtx(), _sinks(), _loopbackSender(nullptr),
          _cerrSink(std::cerr), _syslogSink(nullptr)
    { }

    //------------------------------------------------------------------------
    bool Logger::Open(Facility facility,
                      const std::vector<MessageSink *> & sinks,
                      const char *ident)
    {
      bool  rc = false;
      char  hn[255];
      memset(hn, 0, sizeof(hn));
      gethostname(hn, sizeof(hn));

      _origin.hostname(hn);
      if (nullptr == ident) {
        _origin.appname(ProgramName());
      }
      else {
        _origin.appname(ident);
      }
      _origin.processid(getpid());
      _facility = facility;

      std::lock_guard  lck(_sinksMtx);
      _sinks.clear();
      if (sinks.empty()) {
        _loopbackSender = new LoopbackSender();
        {
          _sinks.push_back(_loopbackSender);
        }
      }
      else {
        for (auto sink : sinks) {
          _sinks.push_back(sink);
        }
      }
      return true;
    }

    //------------------------------------------------------------------------
    bool Logger::AddSinks(const std::vector<MessageSink *> & sinks)
    {
      auto  haveSink = [&] (MessageSink *ms)
      { return std::find_if(_sinks.begin(), _sinks.end(),
                            [&] (const auto entry)
                            { return (ms == entry); }) != _sinks.end(); 
      };
        
      bool  rc = false;
      std::lock_guard  lck(_sinksMtx);
      for (auto sink : sinks) {
        if (! haveSink(sink)) {
          _sinks.push_back(sink);
          rc = true;
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool Logger::RemoveSinks(const std::vector<MessageSink *> & sinks)
    {
      bool  rc = false;
      std::lock_guard  lck(_sinksMtx);
      for (auto sink : sinks) {
        auto it = std::find(_sinks.begin(), _sinks.end(), sink);
        if (it != _sinks.end()) {
          _sinks.erase(it);
          rc = true;
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    void Logger::SetSinks(const std::vector<MessageSink *> & sinks)
    {
      std::lock_guard  lck(_sinksMtx);
      
      _sinks.clear();
      if (nullptr != _loopbackSender) {
        delete _loopbackSender;
        _loopbackSender = nullptr;
      }
      for (auto sink : sinks) {
        if (nullptr != sink) {
          _sinks.push_back(sink);
        }
      }
      return;
    }
    
    //------------------------------------------------------------------------
    bool Logger::Close()
    {
      {
        std::lock_guard  lck(_sinksMtx);
        _sinks.clear();
        if (nullptr != _loopbackSender) {
          delete _loopbackSender;
          _loopbackSender = nullptr;
        }
      }
      
      _origin.hostname("");
      _origin.appname("");
      _origin.processid(0);

      return true;
    }

    //------------------------------------------------------------------------
    bool Logger::Log(Severity severity, std::string && msg,
                     std::source_location loc)
    {
      if (severity > _minimumSeverity) {
        return true;
      }
      
      //  NOTE: the performance of this function is likely abysmal; we
      //  have too many allocations.
      namespace fs = std::filesystem;
      bool  rc = false;
      
      if (_origin.processid()) {
        MessageHeader  hdr(_facility, severity, _origin);
        if (_logLocations) {
          fs::path       locFile(loc.file_name());
          msg += " {" + locFile.filename().string() + ':'
            + std::to_string(loc.line()) + '}';
        }
        Message  logmsg(hdr, msg);
        rc = true;
        {
          std::lock_guard  sinklock(_sinksMtx);
          for (auto sink : _sinks) {
            rc &= sink->Process(logmsg);
          }
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    Logger::~Logger()
    {
      Close();
    }
    
  }  // namespace Mclog

}  // namespace Dwm
