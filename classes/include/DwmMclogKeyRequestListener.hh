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
//!  @file DwmMclogKeyRequestListener.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGKEYREQUESTLISTENER_HH_
#define _DWMMCLOGKEYREQUESTLISTENER_HH_

#include <cstdint>
#include <deque>
#include <map>
#include <thread>

#include "DwmMclogKeyRequestClientState.hh"
#include "DwmMclogKeyRequestClientAddr.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class KeyRequestListener
    {
    public:
      KeyRequestListener()
          : _port(0), _keyDir(nullptr), _mcastKey(nullptr), _fd(-1),
            _thread(), _run(false), _clients(), _clientsDone()
      {
        _stopfds[0] = -1;
        _stopfds[1] = -1;
      }

      ~KeyRequestListener();

      bool Start(const Ipv4Address & addr, uint16_t port,
                 const std::string *keyDir, const std::string *mcastKey);
      bool Stop();
      
    private:
      Ipv4Address         _addr;
      uint16_t            _port;
      const std::string  *_keyDir;
      const std::string  *_mcastKey;
      int                 _fd;
      int                 _stopfds[2];
      std::thread         _thread;
      std::atomic<bool>   _run;
      
      std::map<KeyRequestClientAddr,KeyRequestClientState>               _clients;
      std::deque<std::pair<KeyRequestClientAddr,KeyRequestClientState>>  _clientsDone;
      
      void ClearExpired();
      void Run();
      bool Listen();
    };

  }  // namespace Mclog
    
}  // namespace Dwm

#endif  // _DWMMCLOGKEYREQUESTLISTENER_HH_
