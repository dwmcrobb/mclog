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
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <cstring>
#include <iostream>
#include <map>

#include "DwmIpv4Address.hh"
#include "DwmCredenceXChaCha20Poly1305Istream.hh"
#include "DwmCredenceXChaCha20Poly1305Ostream.hh"
#include "DwmCredenceSigner.hh"
#include "DwmCredenceKnownKeys.hh"
#include "DwmCredenceUtils.hh"
#include "DwmCredenceXChaCha20Poly1305.hh"
#include "DwmMclogKeyRequestClientState.hh"
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
                                        const struct sockaddr_in & src,
                                        char *buf, size_t buflen)
    {
      bool  rc = false;
      std::spanstream  sps{std::span{buf,buflen}};
      if (StreamIO::Read(sps, _theirKX)) {
        if (_theirKX.Value().size() == 32) {
          _sharedKey = _keyPair.SharedKey(_theirKX.Value());
          sps.seekp(0);
          _keyPair.PublicKey().Write(sps);
          ssize_t sendrc = sendto(fd, buf, sps.tellp(), 0,
                                  (const struct sockaddr *)&src,
                                  sizeof(src));
          if (sendrc == sps.tellp()) {
            rc = true;
            ChangeState(&KeyRequestClientState::KXKeySent);
          }
          else {
            Syslog(LOG_ERR, "sendto(...,%s:%hu,...) failed: %m",
                   ((std::string)Ipv4Address(src.sin_addr.s_addr)).c_str(),
                   ntohs(src.sin_port));
            ChangeState(&KeyRequestClientState::Failure);
          }
        }
        else {
          Syslog(LOG_ERR, "Incorrect key length from %s:%hu",
                 ((std::string)Ipv4Address(src.sin_addr.s_addr)).c_str(),
                 ntohs(src.sin_port));
          ChangeState(&KeyRequestClientState::Failure);
        }
      }
      else {
        Syslog(LOG_ERR, "Failed to read client public key");
        ChangeState(&KeyRequestClientState::Failure);
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool KeyRequestClientState::SendIdAndSig(int fd, const sockaddr_in & dst)
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
          if (pkt.SendTo(fd, _sharedKey, (const sockaddr *)&dst,
                         sizeof(dst)) > 0) {
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
            Syslog(LOG_INFO, "%s authenticated", id.c_str());
          }
          else {
            Syslog(LOG_ERR, "Signed message content mismatch from id '%s'",
                   id.c_str());
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to open signed message from id '%s",
                 id.c_str());
        }
      }
      else {
        Syslog(LOG_ERR, "Unknown id '%s'", id.c_str());
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool KeyRequestClientState::KXKeySent(int fd,
                                          const struct sockaddr_in & src,
                                          char *buf, size_t buflen)
    {
      if (buflen < MessagePacket::k_minPacketLen) {
        ChangeState(&KeyRequestClientState::Failure);
        Syslog(LOG_ERR, "Short packet from %s:%hu",
               inet_ntoa(src.sin_addr), ntohs(src.sin_port));
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
                ChangeState(&KeyRequestClientState::IDSent);
              }
            }
            else {
              Syslog(LOG_ERR, "Invalid id '%s'", _theirId.c_str());
              ChangeState(&KeyRequestClientState::Failure);
            }
          }
          else {
            Syslog(LOG_ERR, "Failed to read signed message from id '%s'",
                   _theirId.c_str());
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to read _theirId");
        }
      }
      else {
        Syslog(LOG_ERR, "Failed to decrypt packet");
      }
      return rc;
    }
  
    //------------------------------------------------------------------------
    bool KeyRequestClientState::IDSent(int fd,
                                       const struct sockaddr_in & src,
                                       char *buf, size_t buflen)
    {
      bool  rc = false;
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool KeyRequestClientState::Failure(int fd,
                                       const struct sockaddr_in & src,
                                       char *buf, size_t buflen)
    {
      return false;
    }
  
    //------------------------------------------------------------------------
    void KeyRequestClientState::ChangeState(State newState)
    {
      std::string  prevStateName = StateName();
      _state = newState;
      _lastStateChangeTime = time((time_t *)0);
      Syslog(LOG_INFO, "State changed from %s to %s", prevStateName.c_str(),
             StateName().c_str());
      return;
    }

  }  // namespace Mclog
    
}  // namespace Dwm
