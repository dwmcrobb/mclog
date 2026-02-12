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
//!  @file DwmMclogFileLogger.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::FileLogger implementation
//---------------------------------------------------------------------------

#include "DwmMclogFileLogger.hh"
#include "DwmMclogLogger.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    FileLogger::FileLogger()
        : _thread(), _inQueue(), _run(false), _logFiles()
    {
      _inQueue.MaxLength(1000);
    }
    
    //------------------------------------------------------------------------
    bool FileLogger::Start(const Config & cfg)
    {
      using namespace std;

      if (cfg.files.logs.empty()) {
        return true;
      }

      bool rc = true;
      _logFiles.Configure(cfg);
      _run = true;
      _thread = std::thread(&FileLogger::Run, this);
#if (defined(__FreeBSD__) || defined(__linux__))
      pthread_setname_np(_thread.native_handle(), "FileLogger");
#endif
      return rc;
    }

    //------------------------------------------------------------------------
    bool FileLogger::Restart(const Config & config)
    {
      bool  rc = false;
      Stop();
      return Start(config);
    }

    //------------------------------------------------------------------------
    bool FileLogger::Stop()
    {
      bool  rc = false;
      _run.store(false);
      MCLOG(Severity::info, "Stopping FileLogger");
      _inQueue.ConditionSignal();
      if (_thread.joinable()) {
        _thread.join();
        rc = true;
        MCLOG(Severity::info, "FileLogger stopped");
      }
      _logFiles.Close();
      return rc;
    }

    //------------------------------------------------------------------------
    bool FileLogger::Process(const Message & msg)
    {
      return _inQueue.PushBack(msg);
    }
    
    //------------------------------------------------------------------------
    void FileLogger::Run()
    {
      MCLOG(Severity::info, "FileLogger thread started");
#if (__APPLE__)
      pthread_setname_np("FileLogger");
#endif
      std::deque<Message>  msgs;
      while (_run.load()) {
        _inQueue.ConditionWait();
        _inQueue.Swap(msgs);
        for (const auto & msg : msgs) {
          _logFiles.Process(msg);
        }
        msgs.clear();
      }
      MCLOG(Severity::info, "FileLogger thread done");
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
