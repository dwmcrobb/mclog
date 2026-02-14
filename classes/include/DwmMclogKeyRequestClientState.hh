//===========================================================================
//  Copyright (c) Daniel W. McRobb 2025, 2026
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
//!  @file DwmMcLogKeyRequestClientState.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::KeyRequestClient class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGKEYREQUESTCLIENTSTATE_HH_
#define _DWMMCLOGKEYREQUESTCLIENTSTATE_HH_

extern "C" {
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/select.h>
  #include <sys/time.h>
}

#include <vector>

#include "DwmCredenceKXKeyPair.hh"
#include "DwmCredenceKeyStash.hh"
#include "DwmMclogUdpEndpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates the state of a key request client (a client requesting
    //!  the multicast decryption key).  Only used by mclogd.
    //------------------------------------------------------------------------
    class KeyRequestClientState
    {
    public:
      using State = bool (KeyRequestClientState::*)(int,
                                                    const UdpEndpoint &,
                                                    char *, size_t);
      
      //----------------------------------------------------------------------
      //!  Construct with a pointer to the path of our Credence key storage
      //!  directory and a pointer to the multicast decryption key.
      //----------------------------------------------------------------------
      KeyRequestClientState(const std::string *keyDir,
                            const std::string *mcastKey);

      //----------------------------------------------------------------------
      //!  Destructor.
      //----------------------------------------------------------------------
      ~KeyRequestClientState();
      
      //----------------------------------------------------------------------
      //!  Process the @c buflen bytes of @c buf from @c src received on
      //!  descriptor @c fd.  This just calls the current state.  Returns
      //!  true on success, false on failure.
      //----------------------------------------------------------------------
      bool ProcessPacket(int fd, const UdpEndpoint & src,
                         char *buf, size_t buflen)
      { return (this->*_state)(fd, src, buf, buflen); }
      
      //----------------------------------------------------------------------
      //!  The initial state.
      //----------------------------------------------------------------------
      bool Initial(int fd, const UdpEndpoint & src,
                   char *buf, size_t buflen);
      
      //----------------------------------------------------------------------
      //!  The state after we have sent our session key information.
      //----------------------------------------------------------------------
      bool KXKeySent(int fd, const UdpEndpoint & src,
                     char *buf, size_t buflen);
      
      //----------------------------------------------------------------------
      //!  The state after we have sent our signed authentication and the
      //!  multicast encryption key.
      //----------------------------------------------------------------------
      bool IDSent(int fd, const UdpEndpoint & src,
                  char *buf, size_t buflen);
      
      //----------------------------------------------------------------------
      //!  The failure state.
      //----------------------------------------------------------------------
      bool Failure(int fd, const UdpEndpoint & src,
                   char *buf, size_t buflen);
      
      //----------------------------------------------------------------------
      //!  Returns the current state.
      //----------------------------------------------------------------------
      const State CurrentState() const
      { return _state; }
      
      //----------------------------------------------------------------------
      //!  Returns a string representation of the current state.
      //----------------------------------------------------------------------
      const std::string & StateName() const;
      
      //----------------------------------------------------------------------
      //!  Returns the last state change time.
      //----------------------------------------------------------------------
      time_t LastStateChangeTime() const
      { return _lastStateChangeTime; }

      //----------------------------------------------------------------------
      //!  The success state.
      //----------------------------------------------------------------------
      bool Success()
      { return (_state == &KeyRequestClientState::IDSent); }
      
    private:
      uint16_t                    _port;
      State                       _state;
      time_t                      _lastStateChangeTime;
      Credence::KXKeyPair         _keyPair;
      Credence::ShortString<255>  _theirKX;
      std::string                 _sharedKey;
      std::string                 _theirId;
      const std::string          *_keyDir;
      const std::string          *_mcastKey;
      
      void ChangeState(State newState, const UdpEndpoint & src);
      bool SendIdAndSig(int fd, const UdpEndpoint & dst);
      bool IsValidUser(const std::string & id, const std::string & signedMsg);
    };

  }  // namespace Mclog
  
}  // namespace Dwm

#endif  // _DWMMCLOGKEYREQUESTCLIENTSTATE_HH_
