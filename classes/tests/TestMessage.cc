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

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
struct MessageData 
{
  const char            *host;
  const char            *appname;
  Dwm::Mclog::Facility   facility;
  Dwm::Mclog::Severity   severity;
  std::string            msg;
};

static const std::vector<MessageData>  g_msgData = {
  { "spark.rfdm.com",  "app1", Dwm::Mclog::Facility::daemon,
    Dwm::Mclog::Severity::debug, "app1 daemon debug from spark" },
  { "foo.rfdm.com",    "app1", Dwm::Mclog::Facility::daemon,
    Dwm::Mclog::Severity::debug, "app1 daemon debug from foo" },
  { "bar.rfdm.com",    "app2", Dwm::Mclog::Facility::user,
    Dwm::Mclog::Severity::info, "app2 user info from bar" }
};

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static Dwm::Mclog::Message MakeMessage(const MessageData & msgData)
{
  Dwm::Mclog::MessageOrigin  origin(msgData.host, msgData.appname, getpid());
  Dwm::Mclog::MessageHeader  header(msgData.facility, msgData.severity, origin);
  return Dwm::Mclog::Message(header, msgData.msg);
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestStreamOperators()
{
  vector<Dwm::Mclog::Message>  msgvec1;
  for (const auto & data : g_msgData) {
    msgvec1.push_back(MakeMessage(data));
  }
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
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestStreamIO()
{
  vector<Dwm::Mclog::Message>  msgvec1;
  for (const auto & data : g_msgData) {
    msgvec1.push_back(MakeMessage(data));
  }
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
  return;
}

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
static void TestBZ2IO()
{
  vector<Dwm::Mclog::Message>  msgvec1;
  for (const auto & data : g_msgData) {
    msgvec1.push_back(MakeMessage(data));
  }
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
