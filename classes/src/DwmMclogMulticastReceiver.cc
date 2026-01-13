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
//!  @file DwmMclogMulticastReceiver.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <algorithm>

#include "DwmSysLogger.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogKeyRequester.hh"
#include "DwmMclogMulticastReceiver.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MulticastReceiver::MulticastReceiver()
        : _fd(-1), _groupAddr(), _intfAddr(), _port(0), _acceptLocal(true),
          _queuesMutex(), _queues(), _thread(), _run(false)
    {
      _stopfds[0] = -1;
      _stopfds[1] = -1;
    }

    //------------------------------------------------------------------------
    MulticastReceiver::~MulticastReceiver()
    {
      Close();
    }
    
    //------------------------------------------------------------------------
    bool MulticastReceiver::BindSocket()
    {
      bool  rc = false;
      int   on = 1;
      if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == 0) {
        if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == 0) {
          sockaddr_in  locAddr;
          memset(&locAddr, 0, sizeof(locAddr));
          locAddr.sin_family = PF_INET;
          locAddr.sin_port = htons(_port);
          locAddr.sin_addr.s_addr = _groupAddr.Raw();
#ifndef __linux__
          locAddr.sin_len = sizeof(locAddr);
#endif
          if (::bind(_fd, (sockaddr *)&locAddr, sizeof(locAddr)) == 0) {
            rc = true;
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool MulticastReceiver::JoinGroup()
    {
      struct ip_mreq group;
      group.imr_multiaddr.s_addr = _groupAddr.Raw();
      group.imr_interface.s_addr = _intfAddr.Raw();
      return (setsockopt(_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                         (char *)&group, sizeof(group)) == 0);
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool MulticastReceiver::Open(const Ipv4Address & groupAddr,
                                 const Ipv4Address & intfAddr,
                                 uint16_t port, bool acceptLocal)
    {
      bool  rc = false;
      _groupAddr = groupAddr;
      _intfAddr = intfAddr;
      _port = port;
      _acceptLocal = acceptLocal;
      
      if (0 > _fd) {
        _fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (0 <= _fd) {
          if (BindSocket()) {
            if (JoinGroup()) {
              if (0 == pipe(_stopfds)) {
                _run = true;
                _thread = std::thread(&MulticastReceiver::Run, this);
                rc = true;
              }
              else {
                Close();
              }
            }
            else {
              Close();
            }
          }
          else {
            Close();
          }
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    void MulticastReceiver::Close()
    {
      _run = false;
      char  stop;
      write(_stopfds[1], &stop, sizeof(stop));
      if (_thread.joinable()) {
        _thread.join();
      }
      if (0 <= _fd) {
        ::close(_fd);
        _fd = -1;
      }
      ::close(_stopfds[1]);  _stopfds[1] = -1;
      ::close(_stopfds[0]);  _stopfds[0] = -1;
      return;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool MulticastReceiver::AddInputQueue(Thread::Queue<Message> *queue)
    {
      bool  rc = false;
      std::lock_guard  lck(_queuesMutex);
      auto  it = std::find_if(_queues.cbegin(), _queues.cend(),
                              [queue] (const auto & q)
                              { return (queue != q); });
      if (it == _queues.cend()) {
        _queues.push_back(queue);
        rc = true;
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::string MulticastReceiver::SenderKey(const sockaddr_in & sockAddr)
    {
      MulticastSource  src(sockAddr);
      auto  it = _senderKeys.find(src);
      if (it != _senderKeys.end()) {
        return it->second;
      }
      else {
        std::cerr << "src.Port(): " << src.Port() << '\n';
        KeyRequester  keyRequester(src.Addr(), _port + 1);
        std::string   key = keyRequester.GetKey();
        if (! key.empty()) {
          _senderKeys[src] = key;
          return key;
        }
      }
      return std::string();
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void MulticastReceiver::Run()
    {
      Syslog(LOG_INFO, "MulticastReceiver started");

      if (0 <= _fd) {
        fd_set       fds;
        sockaddr_in  fromAddr;
        timeval      tv;
        auto  reset_tv  = [&] () -> void
        { tv.tv_sec = 10; tv.tv_usec = 0; };
        auto  reset_fds = [&] () -> void
        { FD_ZERO(&fds); FD_SET(_fd, &fds); FD_SET(_stopfds[0], &fds); };
        while (_run) {
          reset_tv();
          reset_fds();
          if (select(std::max(_fd,_stopfds[0]) + 1, &fds, nullptr, nullptr, &tv) > 0) {
            if (FD_ISSET(_stopfds[0], &fds)) {
              break;
            }
            if (FD_ISSET(_fd, &fds)) {
              char  buf[1500];
              socklen_t  fromAddrLen = sizeof(fromAddr);
              ssize_t  recvrc = recvfrom(_fd, buf, sizeof(buf), 0,
                                         (sockaddr *)&fromAddr,              
                                         &fromAddrLen);
              Ipv4Address  fromIP(fromAddr.sin_addr.s_addr);
              if ((recvrc > 0) && (_acceptLocal || (fromIP != _intfAddr))) {
                std::string  senderKey = SenderKey(fromAddr);
                if (! senderKey.empty()) {
                  MessagePacket  pkt(buf, sizeof(buf));
                  ssize_t  decrc = pkt.Decrypt(recvrc, senderKey);
                  if (decrc > 0) {
                    Syslog(LOG_DEBUG, "Received %lld bytes from %s:%hu",
                           recvrc, ((std::string)fromIP).c_str(),
                           ntohs(fromAddr.sin_port));
                    Dwm::Mclog::Message  msg;
                    while (msg.Read(pkt.Payload())) {
                      for (auto q : _queues) {
                        q->PushBack(msg);
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      _run = false;
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
