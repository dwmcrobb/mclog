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
//!  @file DwmMclogUnixReceiver.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <unistd.h>
}

#include "DwmIpv4Address.hh"
#include "DwmMclogUnixReceiver.hh"
#include "DwmMclogMessage.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool UnixReceiver::Start()
    {
      _run = true;
      if (0 == pipe(_stopfds)) {
        _thread = std::thread(&UnixReceiver::Run, this);
        Syslog(LOG_INFO, "UnixReceiver started");
        return true;
      }
      else {
        Syslog(LOG_ERR, "UnixReceiver not started: pipe() failed (%m)");
      }
      return false;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool UnixReceiver::Restart()
    {
      Stop();
      return Start();
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void UnixReceiver::Stop()
    {
      _run = false;
      char  stop = 's';
      ::write(_stopfds[1], &stop, sizeof(stop));
      if (_thread.joinable()) {
        _thread.join();
        ::close(_stopfds[1]);  _stopfds[1] = -1;
        ::close(_stopfds[0]);  _stopfds[0] = -1;
        Syslog(LOG_INFO, "UnixReceiver stopped");
      }
      return;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool UnixReceiver::AddSink(Thread::Queue<Message> *sink)
    {
      bool  rc = false;
      std::lock_guard  lck(_sinksMutex);
      auto  it = std::find_if(_sinks.cbegin(), _sinks.cend(),
                              [sink] (const auto & q)
                              { return (sink == q); });
      if (it == _sinks.cend()) {
        _sinks.push_back(sink);
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void UnixReceiver::Run()
    {
      Syslog(LOG_INFO, "UnixReceiver thread started");
      _ifd = socket(PF_UNIX, SOCK_DGRAM, 0);
      if (0 <= _ifd) {
        int  on = 1;
        setsockopt(_ifd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        struct sockaddr_un  sockAddr;
        memset(&sockAddr, 0, sizeof(sockAddr));
        sockAddr.sun_family = PF_UNIX;
        strcpy(sockAddr.sun_path, "/usr/local/var/run/mclogd.sck");
        sockAddr.sun_len = SUN_LEN(&sockAddr);
        if (bind(_ifd, (struct sockaddr *)&sockAddr, SUN_LEN(&sockAddr)) == 0) {
          fd_set   fds;
          sockaddr_un  fromAddr;
          auto  reset_fds = [&] () -> void
          { FD_ZERO(&fds); FD_SET(_ifd, &fds); FD_SET(_stopfds[0], &fds); };
          char  buf[1500];
          Message  msg;
          while (_run) {
            reset_fds();
            int selectrc = select(std::max(_ifd, _stopfds[0]) + 1, &fds,
                                  nullptr, nullptr, nullptr);
            if (selectrc > 0) {
              if (FD_ISSET(_ifd, &fds)) {
#if 1
                ssize_t  recvrc = recvfrom(_ifd, buf, sizeof(buf), 0,
                                           nullptr, nullptr);
#else
                socklen_t    fromAddrLen = sizeof(fromAddr);
                ssize_t  recvrc = recvfrom(_ifd, buf, sizeof(buf), 0,
                                           (struct sockaddr *)&fromAddr,
                                           &fromAddrLen);
#endif
                if (recvrc > 0) {
                  std::spanstream  sps{std::span{buf, (size_t)recvrc}};
                  // Message  msg;
#if 1
                  if (msg.Read(sps)) {
#else
                  while (msg.Read(sps)) {
#endif
                    for (auto sink : _sinks) {
                      sink->PushBack(msg);
                    }
                  }
                }
              }
              else if (FD_ISSET(_stopfds[0], &fds)) {
                break;
              }
            }
          }
        }
        close(_ifd);
        _ifd = -1;
        std::remove("/usr/local/var/run/mclogd.sck");
      }
      _run = false;
      Syslog(LOG_INFO, "UnixReceiver thread done");
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
