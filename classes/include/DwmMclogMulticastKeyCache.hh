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
//!  @file DwmMclogMulticastKeyCache.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMULTICASTKEYCACHE_HH_
#define _DWMMCLOGMULTICASTKEYCACHE_HH_

#include <map>
#include <string>
#include <thread>

#include "DwmMclogUdp4Endpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class MulticastKeyCache
    {
    public:
      using CacheKey = Udp4Endpoint;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      class CacheValue
      {
      public:
        using Clock = std::chrono::system_clock;

        CacheValue(std::string mcastKey);
        CacheValue(const CacheValue & cacheValue);
        CacheValue & operator = (const CacheValue & cacheValue);
        
        std::string McastKey() const;
        void McastKey(const std::string & mcastKey);
        Clock::time_point LastRequested() const;
        void LastRequested(Clock::time_point lastRequested);
        Clock::time_point LastQueried() const;
        void LastQueried(Clock::time_point lastQueried);
        Clock::time_point LastUpdated() const;
        void LastUpdated(Clock::time_point lastUpdated);
        
      private:
        mutable std::mutex  _mtx;
        std::string         _mcastKey;
        Clock::time_point   _lastRequested;
        Clock::time_point   _lastQueried;
        Clock::time_point   _lastUpdated;
      };
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      class QueryThread
      {
      public:
        QueryThread(const Udp4Endpoint & src, const std::string & keyDir);
        bool Running() const;
        bool Done() const;
        std::string Result() const;
        
      private:
        std::atomic<bool>   _running;
        std::atomic<bool>   _done;
        std::jthread        _jthread;
        Udp4Endpoint        _mcastSource;
        std::string         _keyDir;
        mutable std::mutex  _resultMtx;
        std::string         _result;
        
        void Result(std::string result);
        void Run();
      };

      MulticastKeyCache();

      std::string McastKey(const CacheKey & cacheKey,
                           const std::string & keyDir);
      
    private:
      mutable std::mutex                        _cacheMtx;
      mutable std::map<CacheKey,CacheValue>     _cache;
      mutable std::mutex                        _queryThreadsMtx;
      mutable std::map<CacheKey,QueryThread *>  _queryThreads;

      void ClearEmptyDone();
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMULTICASTKEYCACHE_HH_
