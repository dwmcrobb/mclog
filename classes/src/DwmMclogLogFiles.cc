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
//!  @brief Dwm::Mclog::LogFiles implementation
//---------------------------------------------------------------------------

#include <boost/regex.hpp>

#include "DwmMclogLogFiles.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    LogFiles::LogFiles()
        : _mtx(), _filesConfig(), _filteredLogConfigs(), _logFiles()
    {
    }
    
    //------------------------------------------------------------------------
    LogFiles::LogFiles(LogFiles && logFiles)
        : _mtx(), _filesConfig(std::move(logFiles._filesConfig)),
          _filteredLogConfigs(std::move(logFiles._filteredLogConfigs)),
          _logFiles(std::move(logFiles._logFiles))
    {
    }

    //------------------------------------------------------------------------
    LogFiles::~LogFiles()
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _filteredLogConfigs.clear();
    }

    //------------------------------------------------------------------------
    void LogFiles::Configure(const Config & config)
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _filteredLogConfigs.clear();
      _logPathCache.clear();
      
      _filesConfig = config.files;
      for (const auto & logcfg : config.files.logs) {
        try {
          auto  filtLogCfg =
            FilteredLogConfig{std::make_unique<MessageFilterDriver>(logcfg.filter), logcfg};
          _filteredLogConfigs.push_back(std::move(filtLogCfg));
        }
        catch (std::invalid_argument & ex) {
          std::cerr << ex.what() << '\n';
        }
      }
      return;
    }
    
    //------------------------------------------------------------------------
    bool LogFiles::Process(const Message & msg)
    {
      bool  rc = true;
      std::vector<std::pair<std::string,LogFileConfig &>>  logPaths;
      std::lock_guard  lck(_mtx);
      if (LogPathConfigs(msg, logPaths)) {
        for (const auto & lp : logPaths) {
          auto  fit = _logFiles.find(lp.first);
          if (fit != _logFiles.end()) {
            rc &= fit->second.Process(msg);
          }
          else {
            LogFile  logFile(lp.first, lp.second.permissions,
                             lp.second.period, lp.second.keep,
                             lp.second.format);
            logFile.User(lp.second.user);
            logFile.Group(lp.second.group);
            logFile.Compression(lp.second.compress);
            auto [newit, dontCare] =
              _logFiles.insert({lp.first, std::move(logFile)});
            newit->second.Open();
            rc &= newit->second.Process(msg);
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void LogFiles::Close()
    {
      std::lock_guard  lck(_mtx);
      for (auto & lf : _logFiles) {
        lf.second.Close();
      }
      _logFiles.clear();
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::string LogFiles::LogPathFromCache(const Message & msg,
                                           const LogFileConfig & logFileConfig)
    {
      LogPathCacheKey  cacheKey(msg, logFileConfig.pathPattern);
      auto it = _logPathCache.find(cacheKey);
      if (it != _logPathCache.end()) {
        return it->second;
      }
      return "";
    }

    //------------------------------------------------------------------------
    std::string LogFiles::LogPath(const Message & msg,
                                  const LogFileConfig & logFileConfig)
    {
      std::string  rc = LogPathFromCache(msg, logFileConfig);
      if (rc.empty()) {
        static const boost::regex  rgx("(\%H)|(\%I)|(\%F)");
        std::string  pathPattern = logFileConfig.pathPattern;
        const auto & hdr = msg.Header();
        const auto & origin = hdr.origin();
        
        if (! pathPattern.empty()) {
          if (pathPattern.find_first_of('%') == std::string::npos) {
            if ('/' == pathPattern[0]) {
              rc = pathPattern;
            }
            else {
              rc = (_filesConfig.logDirectory + '/' + pathPattern);
            }
          }
          else {
            std::string  replacement("(?1" + origin.hostname()
                                     + ")(?2" + origin.appname()
                                     + ")(?3" + FacilityName(hdr.facility())
                                     + ")");
            rc = regex_replace(pathPattern, rgx, replacement,
                               boost::match_default | boost::format_all);
            if ('/' != rc[0]) {
              rc = _filesConfig.logDirectory + '/' + rc;
            }
          }
        }
        else {
          rc = "logs/" + origin.hostname() + '/' + origin.appname();
        }
        LogPathCacheKey  cacheKey(msg, logFileConfig.pathPattern);
        _logPathCache[cacheKey] = rc;
      }
      
      return rc;
    }
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool
    LogFiles::LogPathConfigs(const Message & msg,
                             std::map<std::string,LogFileConfig> & logPaths)
    {
      logPaths.clear();
      for (auto & logcfg : _filteredLogConfigs) {
        bool  accepted = false;
        bool  parserc = logcfg.first->parse(&msg, accepted);
        if (parserc && accepted) {
          auto  logPath = LogPath(msg, logcfg.second);
          if (logPaths.find(logPath) == logPaths.end()) {
            auto [lpit, dontCare] = logPaths.insert({logPath,logcfg.second});
          }
        }
      }
      return (! logPaths.empty());
    }

    bool
    LogFiles::LogPathConfigs(const Message & msg,
                             std::vector<std::pair<std::string,LogFileConfig &>> & logPaths)
    {
      logPaths.clear();
      auto  hasEntry = [&] (const auto & s) {
        return std::find_if(logPaths.cbegin(), logPaths.cend(),
                            [&] (const auto & e) { return (e.first == s); })
          != logPaths.cend();
      };
      
      for (auto & logcfg : _filteredLogConfigs) {
        bool  accepted = false;
        bool  parserc = logcfg.first->parse(&msg, accepted);
        if (parserc && accepted) {
          auto  logPath = LogPath(msg, logcfg.second);
          if (! hasEntry(logPath)) {
            logPaths.push_back({logPath,logcfg.second});
          }
        }
      }
      return (! logPaths.empty());
    }
    
  }  // namespace Mclog

}  // namespace Dwm
