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
//!  @file DwmMclogMulticastKeyCache.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmMclogMulticastKeyCache.hh"
#include "DwmMclogKeyRequester.hh"

namespace Dwm {

  namespace Mclog {

    using CacheValue = MulticastKeyCache::CacheValue;

    //------------------------------------------------------------------------
    CacheValue::CacheValue(std::string mcastKey)
        : _mtx(), _mcastKey(mcastKey)
    {}

    //------------------------------------------------------------------------
    CacheValue::CacheValue(const CacheValue & cacheValue)
        : _mtx(), _mcastKey(cacheValue._mcastKey)
    {}

    //------------------------------------------------------------------------
    CacheValue & CacheValue::operator = (const CacheValue & cacheValue)
    {
      if (&cacheValue != this) {
        _mcastKey = cacheValue._mcastKey;
        _lastRequested = cacheValue._lastRequested;
        _lastQueried = cacheValue._lastQueried;
        _lastUpdated = cacheValue._lastUpdated;
      }
      return *this;
    }
    
    //------------------------------------------------------------------------
    std::string CacheValue::McastKey() const
    {
      std::lock_guard  lck(_mtx);
      return _mcastKey;
    }
    
    //------------------------------------------------------------------------
    void CacheValue::McastKey(const std::string & mcastKey)
    {
      std::lock_guard  lck(_mtx);
      _mcastKey = mcastKey;
      return;
    }

    //------------------------------------------------------------------------
    CacheValue::Clock::time_point CacheValue::LastRequested() const
    {
      std::lock_guard  lck(_mtx);
      return _lastRequested;
    }
    
    //------------------------------------------------------------------------
    void CacheValue::LastRequested(Clock::time_point lastRequested)
    {
      std::lock_guard  lck(_mtx);
      _lastRequested = lastRequested;
      return;
    }

    //------------------------------------------------------------------------
    CacheValue::Clock::time_point CacheValue::LastQueried() const
    {
      std::lock_guard  lck(_mtx);
      return _lastQueried;
    }
    
    //------------------------------------------------------------------------
    void CacheValue::LastQueried(Clock::time_point lastQueried)
    {
      std::lock_guard  lck(_mtx);
      _lastQueried = lastQueried;
      return;
    }

    //------------------------------------------------------------------------
    CacheValue::Clock::time_point CacheValue::LastUpdated() const
    {
      std::lock_guard  lck(_mtx);
      return _lastUpdated;
    }
    
    //------------------------------------------------------------------------
    void CacheValue::LastUpdated(Clock::time_point lastUpdated)
    {
      std::lock_guard  lck(_mtx);
      _lastUpdated = lastUpdated;
      return;
    }

    using QueryThread = MulticastKeyCache::QueryThread;

    //------------------------------------------------------------------------
    QueryThread::QueryThread(const Udp4Endpoint & src,
                             const std::string & keyDir)
        : _done(false), _mcastSource(src), _keyDir(keyDir), _resultMtx(),
          _result()
    {
      assert(src.Port() != 0);
      _running = true;
      _jthread = std::jthread(&QueryThread::Run, this);
    }

    //------------------------------------------------------------------------
    bool QueryThread::Running() const
    {
      return _running;
    }

    //------------------------------------------------------------------------
    bool QueryThread::Done() const
    {
      return _done.load();
    }

    //------------------------------------------------------------------------
    std::string QueryThread::Result() const
    {
      std::lock_guard  lck(_resultMtx);
      return _result;
    }

    //------------------------------------------------------------------------
    void QueryThread::Result(std::string result)
    {
      // std::lock_guard  lck(_resultMtx);
      _result = result;
      return;
    }
    
    //------------------------------------------------------------------------
    void QueryThread::Run()
    {
      Syslog(LOG_INFO, "MulticastKeyCache::QueryThread started");
      
      KeyRequester  keyRequester(_mcastSource.Addr(),
                                 _mcastSource.Port(), _keyDir);
      std::string  result = keyRequester.GetKey();
      Result(result);
      _running = false;
      _done.store(true);
      Syslog(LOG_INFO, "MulticastKeyCache::QueryThread done");
      return;
    }

    //------------------------------------------------------------------------
    MulticastKeyCache::MulticastKeyCache()
        : _cacheMtx(), _cache(), _queryThreadsMtx(), _queryThreads()
    {}

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    void MulticastKeyCache::ClearEmptyDone()
    {
      std::lock_guard  lck(_queryThreadsMtx);
      for (auto fit = _queryThreads.begin(), lit = _queryThreads.end();
           fit != lit; ) {
        if (fit->second->Done() && fit->second->Result().empty()) {
          Syslog(LOG_DEBUG,
                 "deleting MulticastKeyCache::QueryThread for %s:%hu",
                 ((std::string)fit->first.Addr()).c_str(), fit->first.Port());
          delete fit->second;
          fit->second = nullptr;
          fit = _queryThreads.erase(fit);
        }
        else {
          ++fit;
        }
      }
      return;
    }
    
    //------------------------------------------------------------------------
    std::string MulticastKeyCache::McastKey(const CacheKey & cacheKey,
                                            const std::string & keyDir)
    {
      {
        std::lock_guard  lck(_cacheMtx);
        auto  it = _cache.find(cacheKey);
        if (it != _cache.end()) {
          return it->second.McastKey();
        }
      }
      {
        std::lock_guard  lck(_queryThreadsMtx);
        Syslog(LOG_DEBUG, "_queryThreads.size(): %llu", _queryThreads.size());
        auto  it = _queryThreads.find(cacheKey);
        if (it != _queryThreads.end()) {
          if (it->second->Done()) {
            std::string  result = it->second->Result();
            if (! result.empty()) {
              std::lock_guard  cachelck(_cacheMtx);
              _cache.insert({cacheKey, CacheValue(result)});
            }
            delete it->second;
            it->second = nullptr;
            _queryThreads.erase(it);
            return result;
          }
        }
        else {
          _queryThreads.insert({cacheKey, new QueryThread(cacheKey, keyDir)});
        }
      }
      ClearEmptyDone();
      return std::string();
    }
    
  }  // namespace Mclog

}  // namespace Dwm
