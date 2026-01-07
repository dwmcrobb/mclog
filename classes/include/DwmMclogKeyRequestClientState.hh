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
//!  @file DwmMcLogKeyRequestClientState.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
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

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class KeyRequestClientState
    {
    public:
      using State = bool (KeyRequestClientState::*)(int,
                                                    const struct sockaddr_in &,
                                                    char *, size_t);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      KeyRequestClientState(const std::string *mcastKey);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      ~KeyRequestClientState();
      
      bool ProcessPacket(int fd, const sockaddr_in & src,
                         char *buf, size_t buflen)
      { return (this->*_state)(fd, src, buf, buflen); }
      
      bool Initial(int fd, const sockaddr_in & src,
                   char *buf, size_t buflen);
      bool KXKeySent(int fd, const sockaddr_in & src,
                     char *buf, size_t buflen);
      bool IDSent(int fd, const sockaddr_in & src,
                  char *buf, size_t buflen);
      bool Failure(int fd, const sockaddr_in & src,
                   char *buf, size_t buflen);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const State CurrentState() const
      { return _state; }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & StateName() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      time_t LastStateChangeTime() const
      { return _lastStateChangeTime; }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool Success()
      { return (_state == &KeyRequestClientState::IDSent); }
      
    private:
      static Credence::KeyStash  _keyStash;
      
      uint16_t                    _port;
      State                       _state;
      time_t                      _lastStateChangeTime;
      Credence::KXKeyPair         _keyPair;
      Credence::ShortString<255>  _theirKX;
      std::string                 _sharedKey;
      std::string                 _theirId;
      const std::string          *_mcastKey;
      
      void ChangeState(State newState);
      bool SendIdAndSig(int fd, const sockaddr_in & dst);
      bool IsValidUser(const std::string & id, const std::string & signedMsg);
    };

  }  // namespace Mclog
  
}  // namespace Dwm

#endif  // _DWMMCLOGKEYREQUESTCLIENTSTATE_HH_
