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
//!  @file DwmMclogLogFiles.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGLOGFILES_HH_
#define _DWMMCLOGLOGFILES_HH_

#include <map>
#include <mutex>
#include <tuple>

#include "DwmMclogFilterDriver.hh"
#include "DwmMclogLogFile.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class LogFiles
    {
    public:
      using PathKey = std::tuple<std::string,std::string,Facility>;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      LogFiles()
          : _logDir(), _pathPattern("%H/%I"), _permissions(0644),
            _rollPeriod(RollPeriod::days_1), _keep(7),
            _paths(), _logFiles(), _mtx()
      {}

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      LogFiles(LogFiles && logFiles);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      ~LogFiles();
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & LogDirectory() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & LogDirectory(const std::string & logDir);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & PathPattern() const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & PathPattern(const std::string & pathPattern);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      mode_t Permissions() const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      mode_t Permissions(mode_t permissions);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      RollPeriod Period() const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      RollPeriod Period(const RollPeriod & period);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint32_t Keep() const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint32_t Keep(uint32_t keep);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool Log(const Message & msg);

      // bool Log(const std::string & fmtstr, const Message & msg);

      PathKey GetPathKey(const Message & msg) const;

      std::string LogPath(const Message & msg);
      
    private:
      std::string                    _logDir;
      std::string                    _pathPattern;
      mode_t                         _permissions;
      RollPeriod                     _rollPeriod;
      uint32_t                       _keep;
      std::map<PathKey,std::string>  _paths;
      std::map<std::string,LogFile>  _logFiles;
      mutable std::mutex             _mtx;
      std::vector<std::pair<std::unique_ptr<FilterDriver>,LogFileConfig>>  _filters;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGLOGFILES_HH_
