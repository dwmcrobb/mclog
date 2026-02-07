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
//!  @file DwmMclogFileLogger.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::FileLogger class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGFILELOGGER_HH_
#define _DWMMCLOGFILELOGGER_HH_

#include <memory>
#include <thread>

#include "DwmThreadQueue.hh"
#include "DwmMclogConfig.hh"
#include "DwmMclogFilterDriver.hh"
#include "DwmMclogLogFiles.hh"
#include "DwmMclogMessageSink.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates logging to files.
    //------------------------------------------------------------------------
    class FileLogger
      : public MessageSink
    {
    public:
      using FilteredLog = std::pair<std::unique_ptr<FilterDriver>,LogFiles>;
      
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      FileLogger();
      
      //----------------------------------------------------------------------
      //!  Starts the FileLogger using the given @c config.  Returns true on
      //!  success, false on failure.
      //----------------------------------------------------------------------
      bool Start(const Config & config);
      
      //----------------------------------------------------------------------
      //!  Restarts the FileLogger using the given @c config.  Returns true
      //!  on success, false on failure.
      //----------------------------------------------------------------------
      bool Restart(const Config & config);
      
      //----------------------------------------------------------------------
      //!  Stops the FileLogger.  Returns true on success, false on failure.
      //----------------------------------------------------------------------
      bool Stop();

      //----------------------------------------------------------------------
      //!  Enqueues the given Message @c msg.  Returns true on success, false
      //!  on failure.
      //----------------------------------------------------------------------
      bool PushBack(const Message & msg) override;
      
    private:
      std::thread               _thread;
      Thread::Queue<Message>    _inQueue;
      std::atomic<bool>         _run;
      std::vector<FilteredLog>  _logs;
      
      void Run();
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGFILELOGGER_HH_
