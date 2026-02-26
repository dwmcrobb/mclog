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
//!  @file TestMessage.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmUnitAssert.hh"
#include "DwmMclogMessage.hh"

using namespace std;

static const std::vector<const char *>  g_msgHosts = {
  "foo.rfdm.com",  "foo.mcplex.net",  "bar.rfdm.com",  "bar.mcplex.net"
};

static const std::vector<const char *>  g_msgApps = {
  "app1",  "daemon1",  "app2",  "daemon2"
};

static const std::vector<Dwm::Mclog::Facility>  g_msgFacilities = {
  Dwm::Mclog::Facility::user,    Dwm::Mclog::Facility::daemon,
  Dwm::Mclog::Facility::local0,  Dwm::Mclog::Facility::local7
};

static const std::vector<Dwm::Mclog::Severity>  g_msgSeverities = {
  Dwm::Mclog::Severity::emerg,    Dwm::Mclog::Severity::alert,
  Dwm::Mclog::Severity::crit,     Dwm::Mclog::Severity::err,
  Dwm::Mclog::Severity::warning,  Dwm::Mclog::Severity::notice,
  Dwm::Mclog::Severity::info,     Dwm::Mclog::Severity::debug
};

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static size_t MakeMessages(std::vector<Dwm::Mclog::Message> & messages)
{
  messages.clear();
  for (const auto & host : g_msgHosts) {
    for (const auto & app : g_msgApps) {
      for (const auto & facility : g_msgFacilities) {
        for (const auto & severity : g_msgSeverities) {
          Dwm::Mclog::MessageOrigin  origin(host, app, getpid());
          Dwm::Mclog::MessageHeader  header(facility, severity, origin);
          std::string  msgdata(std::string(host) + " " + std::string(app) + " ");
          msgdata += Dwm::Mclog::FacilityName(facility) + " ";
          msgdata += Dwm::Mclog::SeverityName(severity);
          messages.push_back(Dwm::Mclog::Message(header, msgdata));
        }
      }
    }
  }
  return messages.size();
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestStreamOperators()
{
  vector<Dwm::Mclog::Message>  msgvec1;
  if (UnitAssert(MakeMessages(msgvec1) > 0)) {
    std::stringstream  ss;
    for (const auto & msg : msgvec1) {
      ss << msg;
    }
    vector<Dwm::Mclog::Message>  msgvec2;
    Dwm::Mclog::Message  msg;
    while (ss >> msg) {
      msgvec2.push_back(msg);
    }
    if (UnitAssert(msgvec2.size() == msgvec1.size())) {
      for (size_t i = 0; i < msgvec1.size(); ++i) {
        UnitAssert(msgvec2[i] == msgvec1[i]);
      }
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestStreamIO()
{
  vector<Dwm::Mclog::Message>  msgvec1;
  if (UnitAssert(MakeMessages(msgvec1) > 0)) {
    std::stringstream  ss;
    for (const auto & msg : msgvec1) {
      msg.Write(ss);
    }
    vector<Dwm::Mclog::Message>  msgvec2;
    Dwm::Mclog::Message  msg;
    while (msg.Read(ss)) {
      msgvec2.push_back(msg);
    }
    if (UnitAssert(msgvec2.size() == msgvec1.size())) {
      for (size_t i = 0; i < msgvec1.size(); ++i) {
        UnitAssert(msgvec2[i] == msgvec1[i]);
      }
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestBZ2IO()
{
  vector<Dwm::Mclog::Message>  msgvec1;
  if (UnitAssert(MakeMessages(msgvec1) > 0)) {
    BZFILE *bzf = BZ2_bzopen("./TestMessage.bz2", "wb");
    if (UnitAssert(bzf)) {
      for (const auto & msg : msgvec1) {
        msg.BZWrite(bzf);
      }
      BZ2_bzclose(bzf);
      bzf = BZ2_bzopen("./TestMessage.bz2", "rb");
      if (bzf) {
        vector<Dwm::Mclog::Message>  msgvec2;
        Dwm::Mclog::Message  msg;
        while (msg.BZRead(bzf) > 0) {
          msgvec2.push_back(msg);
        }
        BZ2_bzclose(bzf);
        if (UnitAssert(msgvec2.size() == msgvec1.size())) {
          for (size_t i = 0; i < msgvec1.size(); ++i) {
            UnitAssert(msgvec2[i] == msgvec1[i]);
          }
        }
      }
      std::remove("./TestMessage.bz2");
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestGZIO()
{
  vector<Dwm::Mclog::Message>  msgvec1;
  if (UnitAssert(MakeMessages(msgvec1) > 0)) {
    gzFile gzf = gzopen("./TestMessage.gz", "wb");
    if (UnitAssert(gzf)) {
      for (const auto & msg : msgvec1) {
        msg.Write(gzf);
      }
      gzclose(gzf);
      gzf = gzopen("./TestMessage.gz", "rb");
      if (gzf) {
        vector<Dwm::Mclog::Message>  msgvec2;
        Dwm::Mclog::Message  msg;
        while (msg.Read(gzf) > 0) {
          msgvec2.push_back(msg);
        }
        gzclose(gzf);
        if (UnitAssert(msgvec2.size() == msgvec1.size())) {
          for (size_t i = 0; i < msgvec1.size(); ++i) {
            UnitAssert(msgvec2[i] == msgvec1[i]);
          }
        }
      }
      std::remove("./TestMessage.gz");
    }
  }
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  using Dwm::Assertions;

  TestStreamOperators();
  TestStreamIO();
  TestBZ2IO();
  TestGZIO();
  
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
