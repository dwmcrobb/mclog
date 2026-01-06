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
//!  @file DwmMcLogKeyRequestClient.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGKEYREQUESTCLIENT_HH_
#define _DWMMCLOGKEYREQUESTCLIENT_HH_

#include <cstdint>
#include <ctime>
#include <iostream>  // for debugging
#include <vector>
#include "spanstream"

#include "DwmIpv4Address.hh"
#include "DwmSysLogger.hh"
#include "DwmCredenceKXKeyPair.hh"
#include "DwmCredenceUtils.hh"
#include "DwmMclogKeyRequestClientState.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class KeyRequestClient
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      KeyRequestClient(const std::string *mcastKey)
          : _lastReceive(time((std::time_t *)0)), _state(mcastKey)
      {}
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      KeyRequestClient(const KeyRequestClient & krc)
        : _state(krc._state)
      {
        _lastReceive = krc._lastReceive;
      }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      KeyRequestClient & operator = (const KeyRequestClient & krc)
      {
        if (&krc != this) {
          _state = krc._state;
          _lastReceive = krc._lastReceive;
        }
        return *this;
      }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool ProcessPacket(int fd, const sockaddr_in & src,
                         char *buf, size_t buflen)
      {
        Syslog(LOG_DEBUG, "Received %llu bytes from %s:%hu",
               buflen, ((std::string)Ipv4Address(src.sin_addr.s_addr)).c_str(),
               ntohs(src.sin_port));
        _lastReceive = time((time_t *)0);
        return _state.ProcessPacket(fd, src, buf, buflen);
      }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool Success() const
      {
        return (_state.CurrentState()
                == &KeyRequestClientState::IDSent);
      }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      time_t LastStateChangeTime() const
      { return _state.LastStateChangeTime(); }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      time_t LastReceive() const
      {
        return _lastReceive;
      }

  private:
    time_t                 _lastReceive;
    KeyRequestClientState  _state;
  };

  }  // namespace Mclog
  
}  // namespace Dwm

#endif  // _DWMMCLOGKEYREQUESTCLIENT_HH_
