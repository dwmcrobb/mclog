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
//!  @file DwmMclogMulticastReceiver.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMULTICASTRECEIVER_HH_
#define _DWMMCLOGMULTICASTRECEIVER_HH_

#include <mutex>
#include <thread>
#include <vector>

#include "DwmIpv4Address.hh"
#include "DwmThreadQueue.hh"
#include "DwmMclogMessage.hh"
#include "DwmMclogMulticastSource.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class MulticastReceiver
    {
    public:
      MulticastReceiver();
      ~MulticastReceiver();
      bool Open(const Ipv4Address & groupAddr, const Ipv4Address & intfAddr,
                uint16_t port, const std::string & keyDir,
                bool acceptLocal = true);
      void Close();
      bool AddInputQueue(Thread::Queue<Message> *queue);
      
    private:
      int                                    _fd;
      Ipv4Address                            _groupAddr;
      Ipv4Address                            _intfAddr;
      uint16_t                               _port;
      std::string                            _keyDir;
      bool                                   _acceptLocal;
      std::mutex                             _queuesMutex;
      std::vector<Thread::Queue<Message> *>  _queues;
      std::thread                            _thread;
      int                                    _stopfds[2];
      std::atomic<bool>                      _run;
      std::map<MulticastSource,std::string>  _senderKeys;
      
      bool BindSocket();
      bool JoinGroup();
      std::string SenderKey(const sockaddr_in & sockAddr);
      void Run();
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMULTICASTRECEIVER_HH_
