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
//!  @file DwmMcLogKeyRequestClientState.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::KeyRequestClient state implementation
//---------------------------------------------------------------------------

#include <cstring>
#include <iostream>
#include <map>

#include "DwmFormatters.hh"
#include "DwmCredenceXChaCha20Poly1305Istream.hh"
#include "DwmCredenceXChaCha20Poly1305Ostream.hh"
#include "DwmCredenceSigner.hh"
#include "DwmCredenceKnownKeys.hh"
#include "DwmCredenceUtils.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogKeyRequestClientState.hh"
#include "DwmMclogLogger.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    const std::string & KeyRequestClientState::StateName() const
    {
      static const std::vector<std::pair<State,std::string>>  stateNames = {
        { &KeyRequestClientState::Initial,               "Initial" },
        { &KeyRequestClientState::KXKeySent,             "KXKeySent" },
        { &KeyRequestClientState::IDSent,                "IDSent" },
        { &KeyRequestClientState::Failure,               "Failure" }
      };
      auto  it = std::find_if(stateNames.begin(), stateNames.end(),
                              [this] (auto && s)
                              { return (s.first == this->_state); });
      assert(it != stateNames.end());
      return it->second;
    }
    
    //------------------------------------------------------------------------
    KeyRequestClientState::KeyRequestClientState(const std::string *keyDir,
                                                 const std::string *mcastKey)
        : _state(&KeyRequestClientState::Initial),
          _lastStateChangeTime(time((time_t *)0)), _keyPair(), _sharedKey(),
          _theirId(), _keyDir(keyDir), _mcastKey(mcastKey)
    {}
    
    //------------------------------------------------------------------------
    KeyRequestClientState::~KeyRequestClientState()
    {
      memset(_sharedKey.data(), 0, _sharedKey.size());
      memset(_theirId.data(), 0, _theirId.size());
    }
    
    //------------------------------------------------------------------------
    bool KeyRequestClientState::Initial(int fd,
                                        const struct UdpEndpoint & src,
                                        char *buf, size_t buflen)
    {
      bool  rc = false;
      std::spanstream  sps{std::span{buf,buflen}};
      if (StreamIO::Read(sps, _theirKX)) {
        if (_theirKX.Value().size() == 32) {
          _sharedKey = _keyPair.SharedKey(_theirKX.Value());
          sps.seekp(0);
          _keyPair.PublicKey().Write(sps);
          ssize_t  sendrc = -1;
          if (src.Addr().Family() == AF_INET) {
            sockaddr_in  dstAddr = src;
            sendrc = sendto(fd, buf, sps.tellp(), 0,
                            (const sockaddr *)&dstAddr, sizeof(dstAddr));
          }
          else {
            sockaddr_in6  dstAddr = src;
            sendrc = sendto(fd, buf, sps.tellp(), 0,
                            (const sockaddr *)&dstAddr, sizeof(dstAddr));
          }
          if (sendrc == sps.tellp()) {
            rc = true;
            ChangeState(&KeyRequestClientState::KXKeySent, src);
          }
          else {
            MCLOG(Severity::err, "sendto({},{},...) failed: {}",
                  fd, src, strerror(errno));
            ChangeState(&KeyRequestClientState::Failure, src);
          }
        }
        else {
          MCLOG(Severity::err, "Incorrect key length from {}", src);
          ChangeState(&KeyRequestClientState::Failure, src);
        }
      }
      else {
        MCLOG(Severity::err, "Failed to read client public key");
        ChangeState(&KeyRequestClientState::Failure, src);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool KeyRequestClientState::SendIdAndSig(int fd, const UdpEndpoint & dst)
    {
      using namespace Dwm::Credence;
      
      bool  rc = false;
      
      char  sendbuf[1500];
      std::spanstream  sps{std::span{sendbuf,sizeof(sendbuf)}};
      Credence::Ed25519KeyPair  myKeys;
      Credence::KeyStash  keyStash(*_keyDir);
      if (keyStash.Get(myKeys)) {
        MessagePacket  pkt(sendbuf, sizeof(sendbuf));
        pkt.Add(myKeys.PublicKey().Id());
        std::string  signedMsg;
        if (Signer::Sign(_theirKX.Value() + *_mcastKey,
                         myKeys.SecretKey().Key(), signedMsg)) {
          pkt.Add(signedMsg);
          if (pkt.SendTo(fd, _sharedKey, dst) > 0) {
            rc = true;
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool KeyRequestClientState::IsValidUser(const std::string & id,
                                            const std::string & signedMsg)
    {
      bool  rc = false;
      Credence::KnownKeys  knownKeys(*_keyDir);
      std::string  key = knownKeys.Find(id);
      if (! key.empty()) {
        std::string  origMsg;
        if (Credence::Signer::Open(signedMsg, key, origMsg)) {
          if (origMsg == _keyPair.PublicKey()) {
            rc = true;
            MCLOG(Severity::info, "KeyRequestClient {} authenticated", id);
          }
          else {
            MCLOG(Severity::err,
                  "Signed message content mismatch from id '{}'", id);
          }
        }
        else {
          MCLOG(Severity::err, "Failed to open signed message from id '{}'",
                id);
        }
      }
      else {
        MCLOG(Severity::warning, "Unknown KeyRequestClient id '{}'", id);
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool KeyRequestClientState::KXKeySent(int fd, const UdpEndpoint & src,
                                          char *buf, size_t buflen)
    {
      if (buflen < MessagePacket::k_minPacketLen) {
        ChangeState(&KeyRequestClientState::Failure, src);
        MCLOG(Severity::err, "Short packet from {}", src);
        return false;
      }
      
      bool  rc = false;
      MessagePacket  pkt(buf, buflen);
      ssize_t  decrc = pkt.Decrypt(buflen, _sharedKey);
      if (decrc > 0) {
        if (StreamIO::Read(pkt.Payload(), _theirId)) {
          std::string  signedMsg;
          if (StreamIO::Read(pkt.Payload(), signedMsg)) {
            if (IsValidUser(_theirId, signedMsg)) {
              if (SendIdAndSig(fd, src)) {
                rc = true;
                ChangeState(&KeyRequestClientState::IDSent, src);
              }
            }
            else {
              MCLOG(Severity::err, "Invalid id '{}'", _theirId);
              ChangeState(&KeyRequestClientState::Failure, src);
            }
          }
          else {
            MCLOG(Severity::err, "Failed to read signed message from id '{}'",
                    _theirId);
          }
        }
        else {
          MCLOG(Severity::err, "Failed to read _theirId");
        }
      }
      else {
        MCLOG(Severity::err, "Failed to decrypt packet");
      }
      return rc;
    }
  
    //------------------------------------------------------------------------
    bool KeyRequestClientState::IDSent(int fd, const UdpEndpoint & src,
                                       char *buf, size_t buflen)
    {
      bool  rc = false;
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool KeyRequestClientState::Failure(int fd, const UdpEndpoint & src,
                                       char *buf, size_t buflen)
    {
      return false;
    }
  
    //------------------------------------------------------------------------
    void KeyRequestClientState::ChangeState(State newState,
                                            const UdpEndpoint & src)
    {
      std::string  prevStateName = StateName();
      _state = newState;
      _lastStateChangeTime = time((time_t *)0);
      MCLOG(Severity::debug, "KeyRequestClient {} state {} -> {}",
              src, prevStateName, StateName());
      return;
    }

  }  // namespace Mclog
    
}  // namespace Dwm
