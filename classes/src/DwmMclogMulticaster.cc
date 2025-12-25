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
//!  @file DwmMclogMulticaster.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/socket.h>
}

#include <fstream>
#include <spanstream>
#include <sstream>

#include "DwmCredenceKXKeyPair.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogMulticaster.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    Multicaster::Multicaster()
        : _fd(-1), _run(false), _thread(), _outQueue()
    {
      Credence::KXKeyPair  key1;
      Credence::KXKeyPair  key2;
      _key = key2.SharedKey(key1.PublicKey().Value());
      std::ofstream  ofs("/tmp/mclogd.key");
      StreamIO::Write(ofs, _key);
      ofs.close();
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Multicaster::Open(const Ipv4Address & intfAddr,
                           const Ipv4Address & groupAddr, uint16_t port)
    {
      bool  rc = false;
      if (0 > _fd) {
        _groupAddr = groupAddr;
        _port = port;

        _fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (0 <= _fd) {
          in_addr  inAddr;
          inAddr.s_addr = intfAddr.Raw();
          if (setsockopt(_fd, IPPROTO_IP, IP_MULTICAST_IF,
                         &inAddr, sizeof(inAddr)) == 0) {
            rc = true;
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    size_t Multicaster::EncryptMessage(const Message & msg,
                                       char *buf, size_t buflen)
    {
      size_t  rc = 0;
      
      std::ostringstream  os;
      msg.Write(os);
      std::spanstream  sps{std::span{buf,buflen}};
      Credence::Nonce  nonce;
      if (nonce.Write(sps)) {
        std::string  cipherText;
        Credence::XChaCha20Poly1305::Encrypt(cipherText, os.str(), nonce, _key);
        if (sps.write((caddr_t)cipherText.data(), cipherText.size())) {
          rc = sps.tellp();
        }
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool Multicaster::Send(const Message & msg)
    {
      bool  rc = false;

      sockaddr_in  dst;
      memset(&dst, 0, sizeof(dst));
      dst.sin_addr.s_addr = _groupAddr.Raw();
      dst.sin_port = htons(_port);

      char  buf[1500];
      size_t  enclen = EncryptMessage(msg, buf, sizeof(buf));
      if (enclen) {
        std::cerr << "Multicasting " << enclen << " bytes for "
                  << msg.StreamedLength() << " byte Message\n";
        ssize_t  sendrc = sendto(_fd, buf, enclen, 0,
                                 (sockaddr *)&dst, sizeof(dst));
        if (sendrc > 0) {
          rc = true;
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void Multicaster::Close()
    {
      _run = false;
      if (_thread.joinable()) {
        _thread.join();
      }
      if (0 <= _fd) {
        ::close(_fd);
        _fd = -1;
      }
      return;
    }

    bool Multicaster::EncryptPacket(const char *plain, size_t plainLen,
                                    
    void Multicaster::Run()
    {
      while (_run) {
        _outQueue.WaitForNotEmpty();
        char  buf[1500];
        std::spanstream  sps{std::span{buf,sizeof(buf)}};
        while (! _outQueue.empty()) {
          Message  msg;
          outQueue.PopFront(msg);
          if (sps.tellp() + msg.StreamedLength() + 40 > sizeof(buf)) {
            
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
