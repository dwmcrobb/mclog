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
//!  @file TestLogFile.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <sys/stat.h>
}

#include "DwmUnitAssert.hh"
#include "DwmMclogLogFile.hh"

using namespace std;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestPermissions()
{
  Dwm::Mclog::LogFile  logFile2("./TestLogFile2_log", 0600);
  if (UnitAssert(logFile2.Open())) {
    logFile2.Close();
    if (UnitAssert(std::filesystem::exists("./TestLogFile2_log"))) {
      struct stat  statbuf;
      stat("./TestLogFile2_log", &statbuf);
      UnitAssert(0600 == (statbuf.st_mode & 0777));
      std::remove("./TestLogFile2_log");
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestOpen()
{
  Dwm::Mclog::LogFile  logFile("./TestLogFile_log");
  if (UnitAssert(logFile.Open())) {
    logFile.Close();
    UnitAssert(std::filesystem::exists("./TestLogFile_log"));
    std::remove("./TestLogFile_log");
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  using Dwm::Assertions;
  int  rc = 1;
  
  TestOpen();
  TestPermissions();

  if (Assertions::Total().Failed()) {
    Assertions::Print(cerr, true);
  }
  else {
    cout << Assertions::Total() << " passed" << endl;
    rc = 0;
  }
  return rc;

}
