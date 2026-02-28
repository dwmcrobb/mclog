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
//!  @file DwmMclogLoopbackSender.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::LoopbackSender class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGLOOPBACKSENDER_HH_
#define _DWMMCLOGLOOPBACKSENDER_HH_

#include <thread>

#include "DwmThreadQueue.hh"
#include "DwmMclogMessagePacket.hh"
#include "DwmMclogMessageSink.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a threaded sink that will send processed messages to
    //!  the loopback address, under the assumption that mclogd is listening
    //!  on the loopback.
    //------------------------------------------------------------------------
    class LoopbackSender
      : public MessageSink
    {
    public:
      using Clock = std::chrono::system_clock;

      //----------------------------------------------------------------------
      //!  Constructor
      //----------------------------------------------------------------------
      LoopbackSender();
      
      //----------------------------------------------------------------------
      //!  Destructor
      //----------------------------------------------------------------------
      ~LoopbackSender();
      
      //----------------------------------------------------------------------
      //!  Start the sender.
      //----------------------------------------------------------------------
      bool Start();
      
      //----------------------------------------------------------------------
      //!  Stop the sender.
      //----------------------------------------------------------------------
      bool Stop();
      
      //----------------------------------------------------------------------
      //!  Ask the sender to send the given @c msg.  Returns true on success,
      //!  false on failure.
      //----------------------------------------------------------------------
      bool Process(const Message & msg) override;

    private:
      std::atomic<bool>       _run;
      int                     _ofd;
      Thread::Queue<Message>  _msgs;
      std::thread             _thread;
      Clock::time_point       _nextSendTime;
      std::atomic<bool>       _running;
      
      void Run();
      bool OpenSocket();
      bool SendPacket(MessagePacket & pkt);
      void SetSndBuf(int fd);
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGLOOPBACKSENDER_HH_
