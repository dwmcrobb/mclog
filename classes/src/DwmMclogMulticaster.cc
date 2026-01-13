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
//!  @file DwmMclogMulticaster.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
}

#include <sstream>

#include "DwmCredenceKXKeyPair.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogMulticaster.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    Multicaster::Multicaster()
        : _fd(-1), _run(false), _thread(), _outQueue(), _groupAddr(), _port(0),
          _key(), _keyRequestListener()
    {
      Credence::KXKeyPair  key1;
      Credence::KXKeyPair  key2;
      _key = key2.SharedKey(key1.PublicKey().Value());
      _nextSendTime = Clock::now() + std::chrono::milliseconds(1000);
    }

    //------------------------------------------------------------------------
    Multicaster::~Multicaster()
    {
      Close();
    }
    
    //------------------------------------------------------------------------
    bool Multicaster::Open(const Config & config)
    {
      bool  rc = false;
      if (0 > _fd) {
        _groupAddr = config.mcast.groupAddr;
        _port = config.mcast.dstPort;

        _fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (0 <= _fd) {
          in_addr  inAddr;
          inAddr.s_addr = config.mcast.intfAddr.Raw();
          if (setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_IF,
                         &inAddr, sizeof(inAddr)) == 0) {
            sockaddr_in  bindAddr;
            memset(&bindAddr, 0, sizeof(bindAddr));
            bindAddr.sin_family = PF_INET;
            bindAddr.sin_addr.s_addr = inAddr.s_addr;
            bindAddr.sin_port = 0;
            bind(_fd, (struct sockaddr *)&bindAddr, sizeof(bindAddr));
            if (_keyRequestListener.Start(config.mcast.intfAddr, _port + 1,
                                          &config.service.keyDirectory,
                                          &_key)) {
              _run = true;
              _thread = std::thread(&Multicaster::Run, this);
              rc = true;
            }
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool Multicaster::Send(const Message & msg)
    {
      return _outQueue.PushBack(msg);
    }

    //------------------------------------------------------------------------
    void Multicaster::Close()
    {
      _keyRequestListener.Stop();
      
      _run = false;
      _outQueue.ConditionSignal();
      if (_thread.joinable()) {
        _thread.join();
      }
      if (0 <= _fd) {
        ::close(_fd);
        _fd = -1;
      }
      return;
    }

    //------------------------------------------------------------------------
    bool Multicaster::SendPacket(MessagePacket & pkt)
    {
      sockaddr_in  dst;
      memset(&dst, 0, sizeof(dst));
      dst.sin_family = PF_INET;
      dst.sin_addr.s_addr = _groupAddr.Raw();
      dst.sin_port = htons(_port);
#ifndef __linux__
      dst.sin_len = sizeof(dst);
#endif
      ssize_t  sendrc = pkt.SendTo(_fd, _key, (sockaddr *)&dst, sizeof(dst));
      return (0 < sendrc);
    }
    
    //------------------------------------------------------------------------
    void Multicaster::Run()
    {
      char  buf[1400];
      MessagePacket  pkt(buf, sizeof(buf));
      Message  msg;
      while (_run) {
        _outQueue.ConditionWait();
        auto  now = Clock::now();
        while (_outQueue.PopFront(msg)) {
          if (! pkt.Add(msg)) {
            if (! SendPacket(pkt)) {
              Syslog(LOG_ERR, "SendPacket() failed");
            }
            _nextSendTime = now + std::chrono::milliseconds(1000);
            pkt.Add(msg);
          }
        }
        if (pkt.HasPayload() && (now > _nextSendTime)) {
          if (! SendPacket(pkt)) {
            Syslog(LOG_ERR, "SendPacket() failed");
          }
          _nextSendTime = now + std::chrono::milliseconds(1000);
        }
      }
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
