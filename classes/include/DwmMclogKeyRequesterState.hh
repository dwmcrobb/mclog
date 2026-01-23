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
//!  @file DwmMclogKeyRequesterState.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGKEYREQUESTERSTATE_HH_
#define _DWMMCLOGKEYREQUESTERSTATE_HH_

extern "C" {
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <sys/select.h>
  #include <sys/time.h>
}

#include "DwmCredenceKXKeyPair.hh"
#include "DwmMclogUdpEndpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class KeyRequesterState
    {
    public:
      using State = bool (KeyRequesterState::*)(int, const UdpEndpoint &,
                                               char *, size_t);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      KeyRequesterState(uint16_t port, const std::string & keyDir);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool ProcessPacket(int fd, const UdpEndpoint & src, char *buf,
                         size_t buflen)
      {
        return (this->*_currentState)(fd, src, buf, buflen);
      }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool Initial(int fd, const UdpEndpoint & src, char *buf, size_t buflen);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool KXKeySent(int fd, const UdpEndpoint & src,
                     char *buf, size_t buflen);
      
      bool IDSent(int fd, const UdpEndpoint & src, char *buf, size_t buflen);
      bool Success(int fd, const UdpEndpoint & src, char *buf, size_t buflen);
      bool Failure(int fd, const UdpEndpoint & src, char *buf, size_t buflen);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & StateName() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      void ChangeState(State state, const UdpEndpoint & src);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      State CurrentState() const
      { return _currentState; }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const Credence::KXKeyPair KX() const
      { return _kxKeyPair; }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & McastKey() const
      { return _mcastKey; }
      
    private:
      std::string                _keyDir;
      uint16_t                   _port;
      State                      _currentState;
      time_t                     _lastStateChangeTime;
      Credence::KXKeyPair        _kxKeyPair;
      Credence::ShortString<255> _theirKX;
      std::string                _sharedKey;
      std::string                _theirId;
      std::string                _mcastKey;
      
      bool SendIdAndSig(int fd, const UdpEndpoint & dst);
      bool IsValidUser(const std::string & id, const std::string & signedMsg);
    };

  }  // namespace Mclog
  
}  // namespace Dwm

#endif  // _DWMMCLOGKEYREQUESTERSTATE_HH_
