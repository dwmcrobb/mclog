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
//!  @file DwmMclogMulticastSources.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmMclogMulticastSources.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    MulticastSources::MulticastSources()
        : _sources(), _sinks(nullptr)
    {}
    
    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    MulticastSources::MulticastSources(const std::string *keyDir,
                                       std::vector<Thread::Queue<Message> *> *sinks)
        : _keyDir(keyDir), _sources(), _sinks(sinks)
    {}

    //------------------------------------------------------------------------
    void MulticastSources::ProcessPacket(const Udp4Endpoint & srcEndpoint,
                                         char *data, size_t datalen)
    {
      auto  it = _sources.find(srcEndpoint);
      if (it != _sources.end()) {
        FSyslog(LOG_DEBUG, "Processing packet from {}", srcEndpoint);
        it->second.ProcessPacket(data, datalen);
      }
      else {
        auto [nit, dontCare] =
          _sources.insert({srcEndpoint,MulticastSource(srcEndpoint,_keyDir,_sinks)});
        nit->second.ProcessPacket(data, datalen);
        ClearOld();
        Syslog(LOG_INFO, "%llu active multicast sources", _sources.size());
      }
      return;
    }

    //------------------------------------------------------------------------
    void MulticastSources::ClearOld()
    {
      auto  expireTime = (std::chrono::system_clock::now()
                          - std::chrono::seconds(20));
      
      std::erase_if(_sources,
                    [&] (const auto & src)
                    {
                      if (src.second.LastReceiveTime() < expireTime) {
                        FSyslog(LOG_INFO, "MulticastSource {} expired",
                                src.first);
                        return true;
                      }
                      return false;
                    });
      return;
    }
    
    
  }  // namespace Mclog

}  // namespace Dwm
