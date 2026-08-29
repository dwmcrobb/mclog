//===========================================================================
//  Copyright (c) Daniel W. McRobb 2025
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
//!  @file TestLogger.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

extern "C" {
  #include <unistd.h>
}

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "DwmMclogLogger.hh"
#include "DwmSysLogger.hh"
#include "DwmUnitAssert.hh"

using namespace std;
using namespace Dwm;

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void Usage(const char *argv0)
{
  cerr << "usage: " << argv0 << " [-v]\n";
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  using Dwm::Mclog::logger;

  int  optChar;
  bool verbose = false;
  
  while ((optChar = getopt(argc, argv, "v")) != -1) {
    switch (optChar) {
      case 'v':
        verbose = true;
        break;
      default:
        Usage(argv[0]);
        exit(1);
        break;
    }
  }
  
  assert(logger.Open(Dwm::Mclog::Facility::user));
  logger.LogLocations(true);

  if (verbose) {
    auto  cerrSink = new Dwm::Mclog::OstreamSink(cerr);
    logger.AddSinks({cerrSink});
  }

  ofstream  os("TestLogger.log");
  if (UnitAssert(os)) {
    auto  ofsSink = new Dwm::Mclog::OstreamSink(os);
    if (UnitAssert(ofsSink)) {
      logger.AddSinks({ofsSink});

      uint64_t  i = 0;
      for (; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
          MCLOG(LOG_INFO, "{} hello there info.", i*10 + j);
          MCLOG(Dwm::Mclog::Severity::debug, "{} hello there debug.", i*10 + j);
        }
        usleep(100000);
      }
    }
    os.close();
    auto  st = std::filesystem::file_size("TestLogger.log");
    UnitAssert(st > 2000);
    std::remove("TestLogger.log");
  }

  int  rc = 1;
  if (Assertions::Total().Failed()) {
    Assertions::Print(cerr, true);
  }
  else {
    cout << Assertions::Total() << " passed" << endl;
    rc = 0;
  }
  return rc;
}
