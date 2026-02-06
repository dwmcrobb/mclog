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


#include "DwmFormatters.hh"
#include "DwmMclogLogger.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    UdpEndpoint    Logger::_dstAddr;
    
    //------------------------------------------------------------------------
    MessageOrigin      Logger::_origin("","",0);
    Facility           Logger::_facility = Facility::user;
    std::atomic<bool>  Logger::_logLocations{false};
    int                Logger::_options = 0;
    int                Logger::_ofd = -1;
    std::mutex         Logger::_ofdmtx;
    std::mutex         Logger::_cerrmtx;
    
    Thread::Queue<Message>     Logger::_msgs;
    std::thread                Logger::_thread;
    std::atomic<bool>          Logger::_run{false};
    Logger::Clock::time_point  Logger::_nextSendTime;

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void Logger::SetSndBuf(int fd)
    {
      if (0 <= fd) {
        std::vector<int>  trySizes{131072, 98304, 65536, 32768};
        int  defaultsz;
        int  foundsz = 0;
        socklen_t  len = sizeof(defaultsz);
        if (0 == getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &defaultsz, &len)) {
          for (int trysz : trySizes) {
            if (0 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &trysz, sizeof(trysz))) {
              foundsz = trysz;
              break;
            }
          }
        }
        if (foundsz > defaultsz) {
          FSyslog(LOG_INFO, "Logger fd {} sndbuf {}", fd, foundsz);
          return;
        }
        if (0 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &defaultsz,
                            sizeof(defaultsz))) {
          FSyslog(LOG_INFO, "Logger fd {} sndbuf {}", fd, defaultsz);
        }
        else {
          FSyslog(LOG_ERR, "Logger setsockopt({},SOL_SOCKET, SO_SNDBUF,"
                  " {}) failed: {}", fd, defaultsz, strerror(errno));
        }
      }
      else {
        FSyslog(LOG_ERR, "Logger::SetSndBuf() invalid fd {}", fd);
      }
      return;
    }
      
    //------------------------------------------------------------------------
    bool Logger::OpenSocket()
    {
      bool  rc = false;
      if (0 > _ofd) {
        _ofd = socket(PF_INET, SOCK_DGRAM, 0);
        if (0 <= _ofd) {
          SetSndBuf(_ofd);
          rc = true;
        }
        else {
          FSyslog(LOG_ERR, "socket(PF_INET, SOCK_DGRAM, 0) failed: {}",
                  strerror(errno));
        }
      }
      else {
        Syslog(LOG_ERR, "Logger socket alread open");
      }
      
      return rc;
    }

    //------------------------------------------------------------------------
    bool Logger::Open(const char *ident, int logopt, Facility facility)
    {
      bool  rc = false;

      _dstAddr = UdpEndpoint(Ipv4Address("127.0.0.1"), 3737);

      char  hn[255];
      memset(hn, 0, sizeof(hn));
      gethostname(hn, sizeof(hn));

      _origin.hostname(hn);
      _origin.appname(ident);
      _origin.processid(getpid());

      std::lock_guard  lck(_ofdmtx);
      _options = logopt;

      if (_options & logSyslog) {
        Dwm::SysLogger::Open(ident, logopt, (int)facility);
      }
      if (OpenSocket()) {
        _run = true;
        _thread = std::thread(&Logger::Run);
        return true;
      }
      return false;
    }

    //------------------------------------------------------------------------
    bool Logger::Close()
    {
      _run = false;
      _msgs.ConditionSignal();
      if (_thread.joinable()) {
        _thread.join();
      }
      
      _origin.hostname("");
      _origin.appname("");
      _origin.processid(0);

      std::lock_guard  lck(_ofdmtx);
      if (0 <= _ofd) {
        ::close(_ofd);
        _ofd = -1;
        if (_options & logSyslog) {
          Dwm::SysLogger::Close();
        }
        return true;
      }
      return false;
    }

    //------------------------------------------------------------------------
    bool Logger::SendPacket(MessagePacket & pkt)
    {
      ssize_t  sendrc = pkt.SendTo(_ofd, _dstAddr);
      pkt.Reset();
      return (0 < sendrc);
    }
    
    //------------------------------------------------------------------------
    bool Logger::Log(Severity severity, std::string && msg,
                     std::source_location loc)
    {
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
        if (_options & logStderr) {
          std::lock_guard  lck(_cerrmtx);
          std::cerr << logmsg;
        }
        if (_options & logSyslog) {
          Syslog((int)(severity)|(int)(_facility), msg.c_str());
        }
        rc = _msgs.PushBack(std::move(logmsg));
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    void Logger::Run()
    {
      char           buf[1200];
      MessagePacket  pkt(buf, sizeof(buf));
      Message        msg;
      
      while (_run) {
        if (_msgs.ConditionTimedWait(std::chrono::seconds(1))) {
          auto  now = Clock::now();
          while (_msgs.PopFront(msg)) {
            if (! pkt.Add(msg)) {
              if (! SendPacket(pkt)) {
                Syslog(LOG_ERR, "SendPacket() failed");
              }
              _nextSendTime = now + std::chrono::milliseconds(1000);
              pkt.Add(msg);
            }
          }
        }
        else {
          auto  now = Clock::now();
          if (pkt.HasPayload()) {
            if (! SendPacket(pkt)) {
              Syslog(LOG_ERR, "SendPacket() failed");
            }
            _nextSendTime = now + std::chrono::milliseconds(1000);
          }
        }
      }
      Syslog(LOG_INFO, "Logger thread done");
      return;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
