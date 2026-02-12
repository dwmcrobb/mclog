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

#include "DwmMclogFilterDriver.hh"
#include "DwmMclogLogFile.hh"
#include "DwmMclogMessageSink.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class LogFiles
      : public MessageSink
    {
    public:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      LogFiles();

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
      void Configure(const Config & config);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool Process(const Message & msg) override;

      void Close();
      
    private:
      using FilteredLogConfig =
        std::pair<std::unique_ptr<FilterDriver>,LogFileConfig>;

      class LogPathCacheKey
      {
      public:
        LogPathCacheKey(const Message & msg, const std::string & pathPattern)
            : _facilitySeverity((uint32_t)msg.Header().facility()
                                | (uint32_t)msg.Header().severity()),
              _hostname(msg.Header().origin().hostname()),
              _appname(msg.Header().origin().appname()),
              _pathPattern(pathPattern)
        {}
        
        bool operator < (const LogPathCacheKey & lpk) const
        {
          if (_facilitySeverity < lpk._facilitySeverity) {
            return true;
          }
          else if (_facilitySeverity == lpk._facilitySeverity) {
            if (_hostname < lpk._hostname) {
              return true;
            }
            else if (_hostname == lpk._hostname) {
              if (_appname < lpk._appname) {
                return true;
              }
              else if (_appname == lpk._appname) {
                return (_pathPattern < lpk._pathPattern);
              }
            }
          }
          return false;
        }
        
      private:
        uint32_t     _facilitySeverity;
        std::string  _hostname;
        std::string  _appname;
        std::string  _pathPattern;
      };
      
      mutable std::mutex                     _mtx;
      std::map<std::string,MessageSelector>  _selectors;
      FilesConfig                            _filesConfig;
      std::vector<FilteredLogConfig>         _filteredLogConfigs;
      std::map<std::string,LogFile>          _logFiles;
      std::map<LogPathCacheKey,std::string>  _logPathCache;

      std::string LogPathFromCache(const Message & msg,
                                   const LogFileConfig & logFileConfig);
      std::string LogPath(const Message & msg,
                          const LogFileConfig & logFileConfig);
      bool LogPathConfigs(const Message & msg,
                          std::map<std::string,LogFileConfig> & logPaths);
      bool LogPathConfigs(const Message & msg,
                          std::vector<std::pair<std::string,LogFileConfig &>> & logPaths);
      
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGLOGFILES_HH_
