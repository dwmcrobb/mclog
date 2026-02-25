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
//!  @file DwmMclogLogFile.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::LogFile class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGLOGFILE_HH_
#define _DWMMCLOGLOGFILE_HH_

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "DwmMclogMessageSink.hh"
#include "DwmMclogFileFormat.hh"
#include "DwmMclogRollInterval.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a log file and its archives.  An instance of this
    //!  class may be used as a sink of the Logger.
    //------------------------------------------------------------------------
    class LogFile
      : public MessageSink
    {
    public:
      //----------------------------------------------------------------------
      //!  Delete the default constructor.
      //----------------------------------------------------------------------
      LogFile() = delete;
      
      //----------------------------------------------------------------------
      //!  Construct from the given @c path and optional permissions, roll
      //!  period and number of files to keep (active file + archives).
      //----------------------------------------------------------------------
      LogFile(const std::string & path, mode_t permissions = 0644,
              RollPeriod period = RollPeriod::days_1, uint32_t keep = 7,
              FileFormat format = FileFormat::text);

      //----------------------------------------------------------------------
      //!  Copy construction is invalid.
      //----------------------------------------------------------------------
      LogFile(const LogFile &) = delete;
      
      //----------------------------------------------------------------------
      //!  Move constructor.
      //----------------------------------------------------------------------
      LogFile(LogFile && logFile);

      //----------------------------------------------------------------------
      //!  Copy assignment is invalid.
      //----------------------------------------------------------------------
      LogFile & operator = (const LogFile &) = delete;

      //----------------------------------------------------------------------
      //!  Move assignment.
      //----------------------------------------------------------------------
      LogFile & operator = (LogFile && logFile);

      //----------------------------------------------------------------------
      //!  Destructor
      //----------------------------------------------------------------------
      ~LogFile();

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uid_t User() const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uid_t User(const std::string & user);
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      gid_t Group() const;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      gid_t Group(const std::string & group);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & Compression() const;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const std::string & Compression(const std::string & compress);
      
      //----------------------------------------------------------------------
      //!  Opens the log file.  Returns true on success, false on failure.
      //----------------------------------------------------------------------
      bool Open();
      
      //----------------------------------------------------------------------
      //!  Closes the log file.
      //----------------------------------------------------------------------
      void Close();

      //----------------------------------------------------------------------
      //!  Processes the given @c msg.  Returns true on success, false on
      //!  failure.
      //----------------------------------------------------------------------
      bool Process(const Message & msg) override;

    private:
      std::mutex             _mtx;
      std::filesystem::path  _path;
      mode_t                 _permissions;
      uint32_t               _keep;
      std::ofstream          _ofs;
      RollInterval           _rollInterval;
      uid_t                  _user;
      gid_t                  _group;
      std::string            _compress;
      FileFormat             _format;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      class Archive
      {
      public:
        Archive(const std::filesystem::path & base, size_t num,
                const std::string & compress);
        std::string String() const;
        size_t Num() const;
        std::filesystem::path Base() const;
        Archive Next() const;
        bool operator < (const Archive & archive) const;
        bool operator > (const Archive & archive) const;
        
      private:
        size_t                 _num;
        std::filesystem::path  _base;
        std::string            _compress;
      };

      bool NeedRollBeforeOpen() const;
      bool OpenNoLock();
      bool EnsureParentDirectory() const;
      bool SetPermissions() const;
      bool SetOwnership() const;
      bool RollCriteriaMet(const Message & msg) const;
      void RollArchives() const;
      void RollCurrent();
      void Roll();
      size_t GetArchives(std::vector<LogFile::Archive> & archives) const;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGLOGFILE_HH_
