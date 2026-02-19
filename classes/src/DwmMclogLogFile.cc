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
//!  @brief Dwm::Mclog::LogFile implementation
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/stat.h>
  #include <grp.h>
  #include <pwd.h>
  #include <unistd.h>
}

#include <regex>

#include "DwmMclogLogFile.hh"
#include "DwmMclogLogger.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    static const std::map<std::string,std::string>  sg_compressionExt = {
      { "bzip2", "bz2" },
      { "gzip",  "gz"  }
    };

    //------------------------------------------------------------------------
    static std::string CompressCommand(const std::string & compressConfig)
    {
      auto  it = sg_compressionExt.find(compressConfig);
      if (it != sg_compressionExt.end()) {
        return it->first;
      }
      return "bzip2";
    }

    //------------------------------------------------------------------------
    static std::string ArchiveExtension(const std::string & compressConfig)
    {
      auto  it = sg_compressionExt.find(compressConfig);
      if (it != sg_compressionExt.end()) {
        return it->second;
      }
      return "bz2";
    }
    
    //------------------------------------------------------------------------
    LogFile::LogFile(const std::string & path, mode_t permissions,
                     RollPeriod period, uint32_t keep)
        : _mtx(), _path(path), _permissions(permissions), _keep(keep),
          _ofs(), _rollInterval(period), _user(getuid()), _group(getgid()),
          _compress("bzip2")
    {}

    //------------------------------------------------------------------------
    LogFile::LogFile(LogFile && logFile)
        : _mtx()
    {
      std::lock_guard  lck(logFile._mtx);
      _path = std::move(logFile._path);
      _permissions = logFile._permissions;
      _keep = logFile._keep;
      _ofs = std::move(logFile._ofs);
      _rollInterval = logFile._rollInterval;
      _user = logFile._user;
      _group = logFile._group;
      _compress = logFile._compress;
    }

    //------------------------------------------------------------------------
    LogFile & LogFile::operator = (LogFile && logFile)
    {
      if (&logFile != this) {
        std::scoped_lock  lck(_mtx, logFile._mtx);
        _path = std::move(logFile._path);
        _permissions = logFile._permissions;
        _keep = logFile._keep;
        _ofs = std::move(logFile._ofs);
        _rollInterval = logFile._rollInterval;
        _user = logFile._user;
        _group = logFile._group;
        _compress = std::move(logFile._compress);
      }
      return *this;
    }

    //------------------------------------------------------------------------
    LogFile::~LogFile()
    {
      Close();
    }

    //------------------------------------------------------------------------
    uid_t LogFile::User() const
    { return _user; }

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    uid_t LogFile::User(const std::string & user)
    {
      struct passwd   pwd;
      char            buf[4096];
      struct passwd  *result = nullptr;
      if (0 == getpwnam_r(user.c_str(), &pwd, buf, sizeof(buf), &result)) {
        if (nullptr != result) {
          _user = pwd.pw_uid;
        }
      }
      return _user;
    }
      
    //------------------------------------------------------------------------
    gid_t LogFile::Group() const
    { return _group; }

    //------------------------------------------------------------------------
    gid_t LogFile::Group(const std::string & group)
    {
      struct group   grp;
      char           buf[4096];
      struct group  *result = nullptr;
      if (0 == getgrnam_r(group.c_str(), &grp, buf, sizeof(buf), &result)) {
        if (nullptr != result) {
          _group = grp.gr_gid;
        }
      }
      return _group;
    }

    //------------------------------------------------------------------------
    const std::string & LogFile::Compression() const
    { return _compress; }
      
    //------------------------------------------------------------------------
    const std::string & LogFile::Compression(const std::string & compress)
    {
      auto  it = sg_compressionExt.find(compress);
      if (it != sg_compressionExt.end()) {
        return (_compress = compress);
      }
      return (_compress = "bzip2");
    }

    //------------------------------------------------------------------------
    bool LogFile::NeedRollBeforeOpen() const
    {
      struct stat  statbuf;
      if (0 == stat(_path.string().c_str(), &statbuf)) {
        if (_rollInterval.StartTime() > statbuf.st_mtime) {
          return true;
        }
      }
      return false;
    }

    //------------------------------------------------------------------------
    bool LogFile::SetPermissions() const
    {
      if (0 == chmod(_path.string().c_str(), _permissions)) {
        MCLOG(Severity::info, "LogFile '{}' permissions set to {:#04o}",
              _path.string(), _permissions);
        return true;
      }
      MCLOG(Severity::warning,
            "Failed to set {:#04o} permissions on LogFile '{}': {}",
            _permissions, _path.string(), strerror(errno));
      return false;
    }

    //------------------------------------------------------------------------
    bool LogFile::SetOwnership() const
    {
      if (0 == chown(_path.string().c_str(), _user, _group)) {
        MCLOG(Severity::info, "LogFile '{}' ownership set to {}:{}",
              _path.string(), _user, _group);
        return true;
      }
      MCLOG(Severity::warning, "LogFile '{}' SetOwnership({}:{}) failed: {}",
            _path.string(), _user, _group, strerror(errno));
      return false;
    }
    
    //------------------------------------------------------------------------
    bool LogFile::EnsureParentDirectory() const
    {
      namespace fs = std::filesystem;

      if (fs::exists(_path.parent_path())) {
        if (fs::is_directory(_path.parent_path())) {
          return true;
        }
      }
      else {
        if (fs::create_directories(_path.parent_path())) {
          return true;
        }
      }
      return false;
    }

    //------------------------------------------------------------------------
    bool LogFile::Open()
    {
      std::lock_guard  lck(_mtx);
      return OpenNoLock();
    }
    
    //------------------------------------------------------------------------
    bool LogFile::OpenNoLock()
    {
      bool  rc = false;
      if (_ofs.is_open()) {
        MCLOG(Severity::info, "LogFile '{}' already open", _path.string());
        rc = true;
      }
      else {
        if (NeedRollBeforeOpen()) {
          Roll();
        }
        if (EnsureParentDirectory()) {
          _ofs.open(_path, std::ios::out|std::ios::app);
          if (static_cast<bool>(_ofs)) {
            rc = true;
            MCLOG(Severity::info, "LogFile '{}' opened", _path.string());
            SetPermissions();
            SetOwnership();
          }
          else {
            MCLOG(Severity::err, "LogFile '{}' open failed: {}",
                  _path.string(), strerror(errno));
          }
        }
        else {
          MCLOG(Severity::err, "LogFile directory '{}' invalid",
                _path.parent_path().string());
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    void LogFile::Close()
    {
      std::lock_guard  lck(_mtx);
      if (_ofs.is_open()) {
        _ofs.close();
      }
      return;
    }

    //------------------------------------------------------------------------
    bool LogFile::Process(const Message & msg)
    {
      bool  rc = false;
      std::lock_guard  lck(_mtx);
      if (_ofs.is_open()) {
        if (RollCriteriaMet(msg)) {
          _ofs.close();
          Roll();
          if (OpenNoLock()) {
            MCLOG(Severity::info, "LogFile {} rolled", _path.string());
          }
        }
        if (_ofs << msg << std::flush) {
          rc = true;
        }
      }
      return rc;
    }

    //------------------------------------------------------------------------
    bool LogFile::RollCriteriaMet(const Message & msg) const
    {
      if (msg.Header().timestamp().Secs() >= _rollInterval.EndTime()) {
        return (time((time_t *)0) >= _rollInterval.EndTime());
      }
      return false;
    }

    //------------------------------------------------------------------------
    void LogFile::RollArchives() const
    {
      namespace fs = std::filesystem;
      
      std::vector<LogFile::Archive>  archives;
      GetArchives(archives);
      for (const auto & archive : archives) {
        if ((archive.Num() + 1) >= _keep) {
          fs::remove(archive.String());
        }
        else {
          fs::rename(archive.String(), archive.Next().String());
        }
      }
      return;
    }

    //------------------------------------------------------------------------
    void LogFile::RollCurrent()
    {
      namespace fs = std::filesystem;
      
      std::string  dst(_path.string() + ".0");
      fs::rename(_path, dst);
      std::string  cmd(CompressCommand(_compress) + " " + dst
                       + " > /dev/null 2>&1 &");
      system(cmd.c_str());
      return;
    }

    //------------------------------------------------------------------------
    void LogFile::Roll()
    {
      RollArchives();
      RollCurrent();
      _rollInterval.SetToCurrent();
      MCLOG(Severity::info, "LogFile '{}' rolled", _path.string());
      return;
    }

    //------------------------------------------------------------------------
    size_t LogFile::GetArchives(std::vector<LogFile::Archive> & archives) const
    {
      namespace fs = std::filesystem;
      
      archives.clear();
      auto  dir = _path.parent_path();
      auto  f = _path.filename();
      std::string  s(f.string() + "\\.([0-9]+)\\."
                     + ArchiveExtension(_compress));
      std::regex   rgx(s, std::regex::ECMAScript|std::regex::optimize);
      std::smatch  sm;
      for (auto const & dirEntry : fs::directory_iterator{dir}) {
        if (dirEntry.is_regular_file()) {
          std::string  fname = dirEntry.path().filename().string();
          if (regex_match(fname, sm, rgx)) {
            if (sm.size() == 2) {
              LogFile::Archive  archive(_path, stoull(sm[1].str()), _compress);
              archives.push_back(archive);
            }
          }
        }
      }
      if (! archives.empty()) {
        sort(archives.begin(), archives.end(), std::greater<LogFile::Archive>());
      }
      return archives.size();
    }

    //------------------------------------------------------------------------
    LogFile::Archive::Archive(const std::filesystem::path & base, size_t num,
                              const std::string & compress)
        : _num(num), _base(base), _compress(compress)
    { }

    //------------------------------------------------------------------------
    std::string LogFile::Archive::String() const
    {
      return (_base.string() + "." + std::to_string(_num)
              + "." + ArchiveExtension(_compress));
    }

    //------------------------------------------------------------------------
    size_t LogFile::Archive::Num() const
    { return _num; }

    //------------------------------------------------------------------------
    std::filesystem::path LogFile::Archive::Base() const
    { return _base; }
        
    //------------------------------------------------------------------------
    LogFile::Archive LogFile::Archive::Next() const
    { return LogFile::Archive(_base, _num + 1, _compress); }

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
