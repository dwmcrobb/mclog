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
//!  @brief NOT YET DOCUMENTED
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


#include "DwmIpv4Address.hh"
#include "DwmMclogLogger.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    sockaddr_in    Logger::_dstAddr;
    
    //------------------------------------------------------------------------
    MessageOrigin  Logger::_origin("","",0);
    Facility       Logger::_facility = Facility::user;
    int            Logger::_options = 0;
    int            Logger::_ofd = -1;
    std::mutex     Logger::_ofdmtx;
    
    //------------------------------------------------------------------------
    bool Logger::OpenSocket()
    {
      bool  rc = false;
      if (0 > _ofd) {
        _ofd = socket(PF_INET, SOCK_DGRAM, 0);
        if (0 <= _ofd) {
          rc = true;
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool Logger::Open(const char *ident, int logopt, Facility facility)
    {
      bool  rc = false;

      memset(&_dstAddr, 0, sizeof(_dstAddr));
      _dstAddr.sin_family = PF_INET;
      _dstAddr.sin_addr.s_addr = Ipv4Address("127.0.0.1").Raw();
      _dstAddr.sin_port = htons(3455);
#ifndef __linux__
      _dstAddr.sin_len = sizeof(_dstAddr);
#endif

      char  hn[255];
      memset(hn, 0, sizeof(hn));
      gethostname(hn, sizeof(hn));

      _origin.hostname(hn);
      _origin.appname(ident);
      _origin.processid(getpid());

      std::lock_guard  lck(_ofdmtx);
      _options = logopt;

      return OpenSocket();
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Logger::Close()
    {
      _origin.hostname("");
      _origin.appname("");
      _origin.processid(0);

      std::lock_guard  lck(_ofdmtx);
      if (0 <= _ofd) {
        ::close(_ofd);
        _ofd = -1;
        return true;
      }
      return false;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Logger::SendMessage(const Message & msg)
    {
      bool  rc = false;
      if (0 <= _ofd) {
        char  buf[1500];
        std::spanstream  sps{std::span{buf,sizeof(buf)}};
        if (msg.Write(sps)) {
          ssize_t  sendrc = ::sendto(_ofd, buf, sps.tellp(), 0,
                                     (sockaddr *)&_dstAddr, sizeof(_dstAddr));
          if (sendrc == sps.tellp()) {
            rc = true;
          }
          else {
            Ipv4Address  dst(_dstAddr.sin_addr.s_addr);
            Syslog(LOG_ERR, "sendto(%d,%s:%hu) failed: %s",
                   _ofd, ((std::string)dst).c_str(), ntohs(_dstAddr.sin_port),
                   strerror(errno));
          }
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Logger::Log(Severity severity, std::string_view msg,
                     std::source_location loc)
    {
      namespace fs = std::filesystem;
      bool  rc = false;
      
      if (_origin.processid()) {
        MessageHeader  hdr(_facility, severity, _origin);
        fs::path  locFile(loc.file_name());
        Message  logmsg(hdr, std::string(msg.data(), msg.size()) + " {"
                        + locFile.filename().string() + ':'
                        + std::to_string(loc.line()) + '}');
        if (_options & logStderr) {
          std::cerr << logmsg;
        }
        std::lock_guard  lck(_ofdmtx);
        rc = SendMessage(logmsg);
      }
      return rc;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
