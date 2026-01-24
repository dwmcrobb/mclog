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
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGLOGFILE_HH_
#define _DWMMCLOGLOGFILE_HH_

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include "DwmMclogMessage.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class LogFile
    {
    public:
      using Clock = std::chrono::system_clock;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      LogFile(const std::string & path);

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool Open();
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      void Close();

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool Log(const Message & msg);

    private:
      std::filesystem::path  _path;
      std::ofstream          _ofs;
      Clock::time_point      _nextRollTime;

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      class Archive
      {
      public:
        Archive(const std::filesystem::path & base, size_t num);
        std::string String() const;
        size_t Num() const;
        std::filesystem::path Base() const;
        Archive Next() const;
        bool operator < (const Archive & archive) const;
        bool operator > (const Archive & archive) const;
        
      private:
        size_t                 _num;
        std::filesystem::path  _base;
      };

      Clock::time_point NextMidnight() const;
      bool NeedRollBeforeOpen() const;
      bool RollCriteriaMet() const;
      void RollArchives(size_t numToKeep) const;
      void RollCurrent();
      void Roll();
      size_t GetArchives(std::vector<LogFile::Archive> & archives) const;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGLOGFILE_HH_
