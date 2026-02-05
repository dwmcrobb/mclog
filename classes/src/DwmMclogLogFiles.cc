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
    LogFiles::LogFiles(LogFiles && logFiles)
        : _logDir(std::move(logFiles._logDir)),
          _pathPattern(std::move(logFiles._pathPattern)),
          _permissions(logFiles._permissions),
          _rollPeriod(logFiles._rollPeriod), _keep(logFiles._keep),
          _paths(std::move(logFiles._paths)),
          _logFiles(std::move(logFiles._logFiles)),
          _mtx()
    {}

    //------------------------------------------------------------------------
    LogFiles::~LogFiles()
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _paths.clear();
    }
    
    //------------------------------------------------------------------------
    const std::string & LogFiles::LogDirectory() const
    {
      std::lock_guard  lck(_mtx);
      return _logDir;
    }
    
    //------------------------------------------------------------------------
    const std::string & LogFiles::LogDirectory(const std::string & logDir)
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _paths.clear();
      return _logDir = logDir;
    }

    //------------------------------------------------------------------------
    const std::string & LogFiles::PathPattern() const
    {
      std::lock_guard  lck(_mtx);
      return _pathPattern;
    }
    
    //------------------------------------------------------------------------
    const std::string & LogFiles::PathPattern(const std::string & pathPattern)
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _paths.clear();
      if (pathPattern.empty()) {
        return _pathPattern = "%H/%I";   // default
      }
      else {
        return _pathPattern = pathPattern;
      }
    }

    //------------------------------------------------------------------------
    mode_t LogFiles::Permissions() const
    {
      std::lock_guard  lck(_mtx);
      return _permissions;
    }
      
    //------------------------------------------------------------------------
    mode_t LogFiles::Permissions(mode_t permissions)
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _paths.clear();
      
      return _permissions = permissions;
    }

    //------------------------------------------------------------------------
    RollPeriod LogFiles::Period() const
    {
      std::lock_guard  lck(_mtx);
      return _rollPeriod;
    }

    //------------------------------------------------------------------------
    RollPeriod LogFiles::Period(const RollPeriod & period)
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _paths.clear();
      return _rollPeriod = period;
    }
      
    //------------------------------------------------------------------------
    uint32_t LogFiles::Keep() const
    {
      std::lock_guard  lck(_mtx);
      return _keep;
    }

    //------------------------------------------------------------------------
    uint32_t LogFiles::Keep(uint32_t keep)
    {
      std::lock_guard  lck(_mtx);
      for (auto & logFile : _logFiles) {
        logFile.second.Close();
      }
      _logFiles.clear();
      _paths.clear();
      return _keep = keep;
    }
    
    //------------------------------------------------------------------------
    bool LogFiles::Log(const Message & msg)
    {
      bool  rc = false;
      std::lock_guard  lck(_mtx);

      std::string  key = LogPath(msg);
      
      auto  it = _logFiles.find(key);
      if (it != _logFiles.end()) {
        rc = it->second.Log(msg);
      }
      else {
        auto [nit, dontCare] =
          _logFiles.insert({key,LogFile(key,_permissions,_rollPeriod,_keep)});
        if (nit->second.Open()) {
          rc = nit->second.Log(msg);
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    std::tuple<std::string,std::string,Facility>
    LogFiles::GetPathKey(const Message & msg) const
    {
      return std::make_tuple(msg.Header().origin().hostname(),
                             msg.Header().origin().appname(),
                             msg.Header().facility());
    }
    
    //------------------------------------------------------------------------
    std::string LogFiles::LogPath(const Message & msg)
    {
      static const boost::regex  rgx("(\%H)|(\%I)|(\%F)");

      auto  pathKey = GetPathKey(msg);
      auto  it = _paths.find(pathKey);
      if (it != _paths.end()) {
        return it->second;
      }

      std::string  pathPattern = _pathPattern;
      const auto & hdr = msg.Header();
      const auto & origin = hdr.origin();
      std::string  rc;
      
      if (! pathPattern.empty()) {
        if (pathPattern.find_first_of('%') == std::string::npos) {
          if ('/' == pathPattern[0]) {
            rc = pathPattern;
          }
          else {
            rc = (_logDir + '/' + pathPattern);
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
            rc = _logDir + '/' + rc;
          }
        }
      }
      else {
        rc = "logs/" + origin.hostname() + '/' + origin.appname();
      }
      _paths[pathKey] = rc;
      return rc;
    }

  }  // namespace Mclog

}  // namespace Dwm
