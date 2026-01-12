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
//!  @file DwmMclogLogFile.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include <regex>

#include "DwmMclogLogFile.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    LogFile::LogFile(const std::string & path)
        : _path(path), _ofs(), _nextRollTime(NextMidnight())
    {}

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool LogFile::Open()
    {
      namespace fs = std::filesystem;
      
      bool  rc = false;
      if (_ofs.is_open()) {
        rc = true;
      }
      else {
        _ofs.open(_path);
        if (static_cast<bool>(_ofs)) {
          rc = true;
        }
        else if (! fs::exists(_path.parent_path())) {
          if (fs::create_directories(_path.parent_path())) {
            _ofs.open(_path);
            rc = static_cast<bool>(_ofs);
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void LogFile::Close()
    {
      if (_ofs.is_open()) {
        _ofs.close();
      }
      return;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool LogFile::Log(const Message & msg)
    {
      bool  rc = false;
      if (_ofs.is_open()) {
        if (_ofs << msg << std::flush) {
          if (RollCriteriaMet()) {
            Close();
            Roll();
            rc = Open();
          }
          else {
            rc = true;
          }
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    std::chrono::system_clock::time_point LogFile::NextMidnight() const
    {
      time_t  now = time((time_t *)0);
      return std::chrono::system_clock::from_time_t(now + 10);  // xxx - debug!!!!
      
      tm  tms;
      localtime_r(&now, &tms);
      ++tms.tm_mday;
      tms.tm_sec = 0;
      tms.tm_min = 0;
      tms.tm_hour = 0;
      return std::chrono::system_clock::from_time_t(mktime(&tms));
    }

    //------------------------------------------------------------------------
    bool LogFile::RollCriteriaMet() const
    {
      return (Clock::now() >= _nextRollTime);
    }

    //------------------------------------------------------------------------
    void LogFile::RollArchives(size_t numToKeep) const
    {
      namespace fs = std::filesystem;
      
      std::vector<LogFile::Archive>  archives;
      GetArchives(archives);
      for (const auto & archive : archives) {
        if ((archive.Num() + 1) >= numToKeep) {
          fs::remove(archive.String());
        }
        else {
          fs::rename(archive.String(), archive.Next().String());
        }
      }
      return;
    }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void LogFile::RollCurrent()
    {
      namespace fs = std::filesystem;
      
      std::string  dst(_path.string() + ".0");
      fs::rename(_path, dst);
      std::string  cmd("bzip2 " + dst + " > /dev/null 2>&1 &");
      system(cmd.c_str());
      return;
    }

    //------------------------------------------------------------------------
    void LogFile::Roll()
    {
      RollArchives(7);
      RollCurrent();
      _nextRollTime = NextMidnight();
      return;
    }

    //------------------------------------------------------------------------
    size_t LogFile::GetArchives(std::vector<LogFile::Archive> & archives) const
    {
      namespace fs = std::filesystem;
      
      archives.clear();
      auto  dir = _path.parent_path();
      auto  f = _path.filename();
      std::string  s(f.string() + "\\.([0-9]+)\\.bz2");
      std::regex   rgx(s, std::regex::ECMAScript|std::regex::optimize);
      std::smatch  sm;
      for (auto const & dirEntry : fs::directory_iterator(dir)) {
        if (dirEntry.is_regular_file()) {
          std::string  fname = dirEntry.path().filename().string();
          if (regex_match(fname, sm, rgx)) {
            if (sm.size() == 2) {
              LogFile::Archive  archive(_path, stoull(sm[1].str()));
              archives.push_back(archive);
            }
          }
        }
      }
      if (! archives.empty()) {
        sort(archives.begin(), archives.end(), std::greater<LogFile::Archive>());
      }
    }

    //------------------------------------------------------------------------
    LogFile::Archive::Archive(const std::filesystem::path & base, size_t num)
        : _num(num), _base(base)
    { }

    //------------------------------------------------------------------------
    std::string LogFile::Archive::String() const
    { return (_base.string() + "." + std::to_string(_num) + ".bz2"); }

    //------------------------------------------------------------------------
    size_t LogFile::Archive::Num() const
    { return _num; }

    //------------------------------------------------------------------------
    std::filesystem::path LogFile::Archive::Base() const
    { return _base; }
        
    //------------------------------------------------------------------------
    LogFile::Archive LogFile::Archive::Next() const
    { return LogFile::Archive(_base, _num + 1); }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    bool LogFile::Archive::operator < (const LogFile::Archive & la) const
    { return _num < la._num; }
        
    //------------------------------------------------------------------------
    bool LogFile::Archive::operator > (const LogFile::Archive & la) const
    { return _num > la._num; }
    
  }  // namespace Mclog

}  // namespace Dwm
