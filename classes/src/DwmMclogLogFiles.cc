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
//!  @file DwmMclogLogFiles.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmMclogLogFiles.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & LogFiles::LogDirectory() const
    {
      std::lock_guard  lck(_mtx);
      return _logDir;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    const std::string & LogFiles::LogDirectory(const std::string & logDir)
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      return _logDir = logDir;
    }
    
    //------------------------------------------------------------------------
    bool LogFiles::Log(const Message & msg)
    {
      bool  rc = false;
      std::lock_guard  lck(_mtx);
      
      std::string  key(_logDir
                       + '/' + msg.Header().origin().hostname()
                       + '/' + msg.Header().origin().appname());
      auto  it = _logFiles.find(key);
      if (it != _logFiles.end()) {
        rc = it->second.Log(msg);
      }
      else {
        auto [nit, dontCare] = _logFiles.insert({key,LogFile(key)});
        if (nit->second.Open()) {
          rc = nit->second.Log(msg);
        }
      }
      return rc;
    }

  }  // namespace Mclog

}  // namespace Dwm
