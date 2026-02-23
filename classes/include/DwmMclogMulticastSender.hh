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
//!  @file DwmMclogMulticastSender.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MulticastSender class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMULTICASTSENDER_HH_
#define _DWMMCLOGMULTICASTSENDER_HH_

#include <chrono>
#include <memory>
#include <span>

#include "DwmIpv4Address.hh"
#include "DwmThreadQueue.hh"
#include "DwmCredenceKeyStash.hh"
#include "DwmCredenceKnownKeys.hh"
#include "DwmMclogConfig.hh"
#include "DwmMclogMessageFilterDriver.hh"
#include "DwmMclogMessageSink.hh"
#include "DwmMclogMessagePacket.hh"
#include "DwmMclogKeyRequestListener.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a thread to transmit log messages via multicast,
    //!  encrypted.
    //------------------------------------------------------------------------
    class MulticastSender
      : public MessageSink
    {
    public:
      using Clock = std::chrono::system_clock;
      
      //----------------------------------------------------------------------
      //!  Default constructor.
      //----------------------------------------------------------------------
      MulticastSender();

      //----------------------------------------------------------------------
      //!  Destructor.
      //----------------------------------------------------------------------
      ~MulticastSender();
      
      //----------------------------------------------------------------------
      //!  Open the multicast sender using the given @c config.  Returns true
      //!  on success, false on failure.
      //----------------------------------------------------------------------
      bool Open(const Config & config);

      //----------------------------------------------------------------------
      //!  Restarts the multicast sender using the given @c config.  Returns
      //!  true on success, false on failure.
      //----------------------------------------------------------------------
      bool Restart(const Config & config);
      
      //----------------------------------------------------------------------
      //!  Close the multicast sender.
      //----------------------------------------------------------------------
      void Close();

      //----------------------------------------------------------------------
      //!  Returns a pointer to the message queue (do I need this?).
      //----------------------------------------------------------------------
      Thread::Queue<Message> *OutputQueue()
      { return &_outQueue; }

      //----------------------------------------------------------------------
      //!  Processes the given @c msg.  Returns true on success, false on
      //!  failure.
      //----------------------------------------------------------------------
      bool Process(const Message & msg) override;
      
      //----------------------------------------------------------------------
      //!  Returns the multicast encryption key.
      //----------------------------------------------------------------------
      std::string Key() const
      { return _key; }
        
    private:
      int                            _fd;
      int                            _fd6;
      std::atomic<bool>              _run;
      std::thread                    _thread;
      Thread::Queue<Message>         _outQueue;
      Config                         _config;
      UdpEndpoint                    _dstEndpoint;
      UdpEndpoint                    _dstEndpoint6;
      std::string                    _key;
      Clock::time_point              _nextSendTime;
      KeyRequestListener             _keyRequestListener;
      std::unique_ptr<MessageFilterDriver>  _filterDriver;
      
      bool DesiredSocketsOpen() const;
      bool OpenSocket();
      bool OpenSocket6();
      bool SendPacket(MessagePacket & pkt);
      bool PassesFilter(const Message & msg);
      void Run();
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMULTICASTSENDER_HH_
