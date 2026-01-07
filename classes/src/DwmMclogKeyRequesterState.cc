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
//!  @file DwmMclogKeyRequesterState.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <vector>
#include "spanstream"

#include "DwmIpv4Address.hh"
#include "DwmCredenceKnownKeys.hh"
#include "DwmCredenceXChaCha20Poly1305Istream.hh"
#include "DwmCredenceXChaCha20Poly1305Ostream.hh"
#include "DwmCredenceSigner.hh"
#include "DwmCredenceUtils.hh"
#include "DwmMclogKeyRequesterState.hh"
#include "DwmMclogMessagePacket.hh"

namespace Dwm {

  namespace Mclog {

    Credence::KeyStash  KeyRequesterState::_keyStash;

    //------------------------------------------------------------------------
    KeyRequesterState::KeyRequesterState(uint16_t port)
        : _port(port), _currentState(&KeyRequesterState::Initial),
          _lastStateChangeTime(time((time_t *)0)), _kxKeyPair(), _sharedKey()
    {}
    
    //------------------------------------------------------------------------
    bool KeyRequesterState::Initial(int fd, const sockaddr_in & src,
                                   char *buf, size_t buflen)
    { return false; }
    
    //------------------------------------------------------------------------
    bool KeyRequesterState::SendIdAndSig(int fd, const sockaddr_in & dst)
    {
      bool  rc = false;
      Credence::Ed25519KeyPair  myKeys;
      if (_keyStash.Get(myKeys)) {
        char  sendbuf[1500];
        MessagePacket  pkt(sendbuf, sizeof(sendbuf));
        pkt.Add(myKeys.PublicKey().Id());
        std::string  signedMsg;
        if (Credence::Signer::Sign(_theirKX.Value(),
                                   myKeys.SecretKey().Key(),
                                   signedMsg)) {
          pkt.Add(signedMsg);
          if (pkt.SendTo(fd, _sharedKey, (sockaddr *)&dst, sizeof(dst)) > 0) {
            rc = true;
          }
          else {
            Syslog(LOG_INFO, "SendTo(%d,%s:%hu,%zu) failed: %m",
                   fd,
                   ((std::string)(Ipv4Address(dst.sin_addr.s_addr))).c_str(),
                   ntohs(dst.sin_port), sizeof(dst));
          }
        }
        else {
          Syslog(LOG_ERR, "Failed to sign message");
        }
      }
      else {
        Syslog(LOG_ERR, "Failed to get my keys from key stash");
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool KeyRequesterState::KXKeySent(int fd, const sockaddr_in & src,
                                      char *buf, size_t buflen)
    {
      bool  rc = false;
      std::spanstream  sps{std::span{buf,buflen}};
      if (StreamIO::Read(sps, _theirKX)) {
        _sharedKey = _kxKeyPair.SharedKey(_theirKX.Value());
        struct sockaddr_in  dst;                                          
        memcpy(&dst, &src, sizeof(dst));
        dst.sin_port = htons(_port);
        if (SendIdAndSig(fd, dst)) {
          rc = true;
          ChangeState(&KeyRequesterState::IDSent);
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool KeyRequesterState::IsValidUser(const std::string & id,
                                        const std::string & signedMsg)
    {
      bool  rc = false;
      Credence::KnownKeys  knownKeys;
      std::string  key = knownKeys.Find(id);
      if (! key.empty()) {
        std::string  origMsg;
        if (Credence::Signer::Open(signedMsg, key, origMsg)) {
          if (origMsg.size() == (_kxKeyPair.PublicKey().Value().size()
                                 + crypto_kx_SESSIONKEYBYTES)) {
            if (origMsg.substr(0,_kxKeyPair.PublicKey().Value().size())
                == _kxKeyPair.PublicKey()) {
              //  xxx - Not ideal....
              _mcastKey = origMsg.substr(_kxKeyPair.PublicKey().Value().size());
              rc = true;
              Syslog(LOG_INFO, "%s authenticated", id.c_str());
            }
            else {
              Syslog(LOG_ERR, "Signed message content mismatch from id '%s'",
                     id.c_str());
            }
          }
          else {
              Syslog(LOG_ERR, "Signed message length mismatch from id '%s':"
                     " %llu should be %llu", id.c_str(), origMsg.size(),
                     _kxKeyPair.PublicKey().Value().size()
                     + crypto_kx_SESSIONKEYBYTES);
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
    bool KeyRequesterState::IDSent(int fd, const sockaddr_in & src,
                                  char *buf, size_t buflen)
    {
      bool  rc = false;
      MessagePacket  pkt(buf, buflen);
      ssize_t  decrc = pkt.Decrypt(buflen, _sharedKey);
      if (decrc > 0) {
        if (StreamIO::Read(pkt.Payload(), _theirId)) {
          std::string  signedMsg;
          if (StreamIO::Read(pkt.Payload(), signedMsg)) {
            if (IsValidUser(_theirId, signedMsg)) {
              rc = true;
              ChangeState(&KeyRequesterState::Success);
            }
            else {
              ChangeState(&KeyRequesterState::Failure);
              Syslog(LOG_ERR, "Invalid id '%s'", _theirId.c_str());
            }
          }
          else {
            ChangeState(&KeyRequesterState::Failure);
            Syslog(LOG_ERR, "Failed to read signed message from id '%s'",
                   _theirId.c_str());
          }
        }
        else {
          ChangeState(&KeyRequesterState::Failure);
          Syslog(LOG_ERR, "Failed to read their ID");
        }
      }
      else {
        ChangeState(&KeyRequesterState::Failure);
        Syslog(LOG_ERR, "Failed to decrypt packet");
      }
      return rc;
    }
    
    //------------------------------------------------------------------------
    bool KeyRequesterState::Success(int fd,
                                   const sockaddr_in & src,
                                   char *buf, size_t buflen)
    {
      return true;
    }
    
    //------------------------------------------------------------------------
    bool KeyRequesterState::Failure(int fd,
                                   const sockaddr_in & src,
                                   char *buf, size_t buflen)
    {
      return false;
    }
    
    //------------------------------------------------------------------------
    const std::string & KeyRequesterState::StateName() const
    {
      static const std::vector<std::pair<State,std::string>>  stateNames = {
        { &KeyRequesterState::Initial,    "Initial" },
        { &KeyRequesterState::KXKeySent,  "KXKeySent" },
        { &KeyRequesterState::IDSent,     "IDSent" },
        { &KeyRequesterState::Success,    "Success" },
        { &KeyRequesterState::Failure,    "Failure" },
      };
      auto  it = std::find_if(stateNames.begin(), stateNames.end(),
                              [this] (auto && s)
                              { return (s.first == this->_currentState); });
      assert(it != stateNames.end());
      return it->second;
    }
    
    //------------------------------------------------------------------------
    void KeyRequesterState::ChangeState(State state)
    {
      std::cerr << "State change from " << StateName();
      _currentState = state;
      std::cerr << " to " << StateName() << '\n';
    }
    
  }  // namespace Mclog
  
}  // namespace Dwm
