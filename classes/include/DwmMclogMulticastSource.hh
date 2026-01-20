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
//!  @file DwmMclogMulticastSource.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMULTICASTSOURCE_HH_
#define _DWMMCLOGMULTICASTSOURCE_HH_

#include <chrono>
#include <span>
#include <thread>

#include "DwmThreadQueue.hh"
#include "DwmMclogMessage.hh"
#include "DwmMclogMulticastSourceKey.hh"
#include "DwmMclogUdp4Endpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  When I process a packet...
    //!  - If I don't yet have a muticast decryption key, put packet on
    //!    backlog and start query thread if one is not running.
    //!  - If I fail to decrypt with existing key... put packet on backlog,
    //!    start fetch of new decryption key.
    //!  - Process backlog.  For each entry...
    //!    - If I have a decryption key that is newer than backlog entry,
    //!      try decrypting with new key.  If this fails, do nothing
    //!      (we've already popped it from the queue to process it).
    //!    - If my last decryption key query is more than 5 seconds newer
    //!      than my last decryption key update, do nothing (we've already
    //!      popped it from the queue to process it).
    //!      .....
    //------------------------------------------------------------------------
    class MulticastSource
    {
    public:
      using Clock = std::chrono::system_clock;
      
      MulticastSource();
      ~MulticastSource();
      MulticastSource(const Udp4Endpoint & srcEndpoint,
                      const std::string *keyDir,
                      std::vector<Thread::Queue<Message> *> *sinks);
      MulticastSource(const MulticastSource & src);
      MulticastSource(MulticastSource && src);
      MulticastSource & operator = (const MulticastSource & src);
      MulticastSource & operator = (MulticastSource && src);
      
      MulticastSourceKey Key() const;
      void Key(const MulticastSourceKey & key);
      bool ProcessPacket(char *data, size_t datalen);
      Clock::time_point LastReceiveTime() const;
      
    private:
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      class BacklogEntry
      {
      public:
        BacklogEntry();
        BacklogEntry(const char *data, size_t datalen);
        BacklogEntry(const BacklogEntry & ble);
        BacklogEntry(BacklogEntry && ble);
        BacklogEntry & operator = (const BacklogEntry & ble);
        BacklogEntry & operator = (BacklogEntry && ble);
        ~BacklogEntry();
        std::chrono::system_clock::time_point ReceiveTime() const;
        char *Data() const        { return _data; }
        size_t Datalen() const    { return _datalen; }
        
      private:
        std::chrono::system_clock::time_point   _receiveTime;
        char                                   *_data;
        size_t                                  _datalen;
      };

      Udp4Endpoint                            _endpoint;
      MulticastSourceKey                      _key;
      Thread::Queue<BacklogEntry>             _backlog;
      const std::string                      *_keyDir;
      std::vector<Thread::Queue<Message> *>  *_sinks;
      std::atomic<bool>                       _queryDone;
      std::jthread                            _queryThread;
      Clock::time_point                       _lastReceiveTime;
      
      bool ProcessBacklog();
      void ClearOldBacklog();
      void StartQuery();
      void QueryForKey();
      
    };

    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMULTICASTSOURCE_HH_
