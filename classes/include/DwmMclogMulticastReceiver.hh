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
//!  @file DwmMclogMulticastReceiver.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MulticastReceiver class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMULTICASTRECEIVER_HH_
#define _DWMMCLOGMULTICASTRECEIVER_HH_

#include <mutex>
#include <thread>
#include <vector>

#include "DwmIpv4Address.hh"
#include "DwmThreadQueue.hh"
#include "DwmMclogConfig.hh"
#include "DwmMclogMessageSink.hh"
#include "DwmMclogMulticastSources.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a multicast receiver of log messages.  Runs in its own
    //!  thread, sending each message received via multicast to each of the
    //!  contained sinks (which are configured via AddSink(), RemoveSink()
    //!  and ClearSinks()).
    //------------------------------------------------------------------------
    class MulticastReceiver
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      MulticastReceiver();
      
      //----------------------------------------------------------------------
      //!  Destructor
      //----------------------------------------------------------------------
      ~MulticastReceiver();
      
      //----------------------------------------------------------------------
      //!  Open the multicast receiver using the given configuration @c cfg.
      //!  Accept multicast messages from the local host if @c acceptLocal is
      //!  @c true, else ignore multicast messages from the local host.  A
      //!  client normally wants @c acceptLocal to be @c true.  mclogd is
      //!  the only application expected to set @c acceptLocal to @c false,
      //!  in order to not process its own multicast output as multicast
      //!  input.
      //!  Returns true on success, false on failure.
      //----------------------------------------------------------------------
      bool Open(const Config & cfg, bool acceptLocal = true);
      
      //----------------------------------------------------------------------
      //!  Restarts the multicast receiver using the given configuration
      //!  @c cfg.  Returns true on success, false on failure.
      //----------------------------------------------------------------------
      bool Restart(const Config & cfg);
      
      //----------------------------------------------------------------------
      //!  Closes the multicast receiver.
      //----------------------------------------------------------------------
      void Close();
      
      //----------------------------------------------------------------------
      //!  Adds a sink to the multicast receiver.
      //----------------------------------------------------------------------
      bool AddSink(MessageSink *sink);
      
      //----------------------------------------------------------------------
      //!  Removes a sink from the multicast receiver.
      //----------------------------------------------------------------------
      bool RemoveSink(MessageSink *sink);
      
      //----------------------------------------------------------------------
      //!  Removes all sinks from the multicast receiver.
      //----------------------------------------------------------------------
      void ClearSinks();
      
    private:
      Config                      _config;
      int                         _fd;
      int                         _fd6;
      bool                        _acceptLocal;
      std::mutex                  _sinksMutex;
      std::vector<MessageSink *>  _sinks;
      std::thread                 _thread;
      int                         _stopfds[2];
      std::atomic<bool>           _run;
      MulticastSources            _sources;
      
      bool BindSocket();
      bool BindSocket6();
      bool JoinGroup();
      bool JoinGroup6();
      void Run();
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMULTICASTRECEIVER_HH_
