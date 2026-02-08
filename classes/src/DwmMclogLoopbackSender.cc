//===========================================================================
//  Copyright (c) Daniel W. McRobb 2026
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
//!  @file DwmMclogLoopbackSender.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
}

#include <cstring>

#include "DwmMclogLoopbackSender.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    LoopbackSender::LoopbackSender()
        : _run(false), _ofd(-1), _msgs(), _thread(), _nextSendTime()
    {
      Start();
    }

    //------------------------------------------------------------------------
    LoopbackSender::~LoopbackSender()
    {
      Stop();
    }

    //------------------------------------------------------------------------
    bool LoopbackSender::Process(const Message & msg)
    {
      return _msgs.PushBack(msg);
    }
    
    //------------------------------------------------------------------------
    bool LoopbackSender::OpenSocket()
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
    void LoopbackSender::SetSndBuf(int fd)
    {
      if (0 <= fd) {
        std::vector<int>  trySizes{131072, 98304, 65536, 32768};
        int  defaultsz;
        int  foundsz = 0;
        socklen_t  len = sizeof(defaultsz);
        if (0 == getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &defaultsz, &len)) {
          for (int trysz : trySizes) {
            if (0 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                                &trysz, sizeof(trysz))) {
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
    bool LoopbackSender::Start()
    {
      bool  rc = false;
      if (! _run.load()) {
        _run.store(true);
        _thread = std::thread(&LoopbackSender::Run, this);
#if (defined(__FreeBSD__) || defined(__linux__))
          pthread_setname_np(_thread.native_handle(), "LoopbackSender");
#endif
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool LoopbackSender::Stop()
    {
      if (_run.load()) {
        _run.store(false);
        _msgs.ConditionSignal();
        if (_thread.joinable())  { _thread.join(); }
        if (0 <= _ofd)           { ::close(_ofd);  _ofd = -1;  }
        return true;
      }
      return false;
    }

    //------------------------------------------------------------------------
    bool LoopbackSender::SendPacket(MessagePacket & pkt)
    {
      static const  UdpEndpoint  dstAddr4(Ipv4Address("127.0.0.1"), 3737);
      ssize_t  sendrc = pkt.SendTo(_ofd, dstAddr4);
      pkt.Reset();
      return (0 < sendrc);
    }
    
    //------------------------------------------------------------------------
    void LoopbackSender::Run()
    {
#if (__APPLE__)
      pthread_setname_np("LoopbackSender");
#endif
      if (OpenSocket()) {
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
      }
      
      return;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
