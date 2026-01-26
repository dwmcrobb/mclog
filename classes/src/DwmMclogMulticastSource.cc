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
//!  @file DwmMclogMulticastSource.cc
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#include "DwmMclogMessagePacket.hh"
#include "DwmMclogMulticastSource.hh"
#include "DwmMclogKeyRequester.hh"

namespace Dwm {

  namespace Mclog {

    using namespace std;
    
    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::BacklogEntry()
        : _data(nullptr), _datalen(0)
    {}
    
    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::BacklogEntry(const char *data,
                                                size_t datalen)
        : _data(nullptr), _datalen(0)
    {
      _receiveTime = chrono::system_clock::now();
      
      if ((nullptr != data) && (0 < datalen)) {
        _data = (char *)malloc(datalen);
        if (nullptr != _data) {
          _datalen = datalen;
          memcpy((void *)_data, data, _datalen);
        }
        else {
          _datalen = 0;
        }
      }
    }
    
    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::BacklogEntry(const MulticastSource::BacklogEntry & ble)
    {
      if (0 < ble._datalen) {
        _data = (char *)malloc(ble._datalen);
        if (nullptr != _data) {
          _datalen = ble._datalen;
          memcpy((void *)_data, ble._data, _datalen);
        }
        else {
          _datalen = 0;
        }
      }
      else {
        _data = nullptr;
        _datalen = 0;
      }

      _receiveTime = ble._receiveTime;
    }
      
    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::BacklogEntry(MulticastSource::BacklogEntry && ble)
    {
      _data = ble._data;
      _datalen = ble._datalen;
      ble._data = nullptr;
      ble._datalen = 0;

      _receiveTime = ble._receiveTime;
    }
    
    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry &
    MulticastSource::BacklogEntry::operator = (MulticastSource::BacklogEntry && ble)
    {
      if (this != &ble) {
        if ((nullptr != _data) && (0 < _datalen)) {
          free((void *)_data); _data = nullptr; _datalen = 0;
        }
        _data = ble._data;
        _datalen = ble._datalen;
        ble._data = nullptr;
        ble._datalen = 0;

        _receiveTime = ble._receiveTime;
      }
      
      return *this;
    }

    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry &
    MulticastSource::BacklogEntry::operator = (const BacklogEntry & ble)
    {
      if (this != &ble) {
        if ((nullptr != _data) && (0 < _datalen)) {
          free((void *)_data); _data = nullptr; _datalen = 0;
        }
        if ((nullptr != ble._data) && (0 < ble._datalen)) {
          _data = (char *)malloc(ble._datalen);
          if (nullptr != _data) {
            memcpy((void *)_data, ble._data, ble._datalen);
            _datalen = ble._datalen;
          }
          else {
            _datalen = 0;
          }
        }
        _receiveTime = ble._receiveTime;
      }
      return *this;
    }

    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::~BacklogEntry()
    {
      if ((nullptr != _data) && (0 < _datalen)) {
        free((void *)_data); _data = nullptr; _datalen = 0;
      }
    }
    
    //------------------------------------------------------------------------
    chrono::system_clock::time_point
    MulticastSource::BacklogEntry::ReceiveTime() const
    {
      return _receiveTime;
    }

    //========================================================================
    //========================================================================

    //------------------------------------------------------------------------
    MulticastSource::MulticastSource()
        : _endpoint(), _key(), _backlog(), _keyDir(nullptr), _sinks(nullptr),
          _queryDone(true), _queryThread(), _lastReceiveTime()
    {
      _backlog.MaxLength(100);   // limit backlog to 100 entries (packets)
    }

    //------------------------------------------------------------------------
    MulticastSource::~MulticastSource()
    {
      while (! _queryDone) {
      }
      if (_queryThread.joinable()) {
        _queryThread.join();
      }
    }
    
    //------------------------------------------------------------------------
    MulticastSource::MulticastSource(const UdpEndpoint & srcEndpoint,
                                     const std::string *keyDir,
                                     vector<MessageSink *> *sinks)
        : _endpoint(srcEndpoint), _key(), _backlog(), _keyDir(keyDir),
          _sinks(sinks), _queryDone(true), _queryThread(), _lastReceiveTime()
    {
      _backlog.MaxLength(100);   // limit backlog to 100 entries (packets)
    }

    //------------------------------------------------------------------------
    MulticastSource::MulticastSource(const MulticastSource & src)
        : _endpoint(src._endpoint), _key(src._key), _keyDir(src._keyDir),
          _sinks(src._sinks), _queryDone(true), _queryThread(),
          _lastReceiveTime(src._lastReceiveTime)
    {
      src._backlog.Copy(_backlog);
      _backlog.MaxLength(100);   // limit backlog to 100 entries (packets)
    }
    
    //------------------------------------------------------------------------
    MulticastSource::MulticastSource(MulticastSource && src)
        : _endpoint(std::move(src._endpoint)), _key(src._key),
          _keyDir(src._keyDir), _sinks(src._sinks), _queryDone(true),
          _queryThread(), _lastReceiveTime(src._lastReceiveTime)
    {
      _backlog.Swap(src._backlog);
      _backlog.MaxLength(100);   // limit backlog to 100 entries (packets)      
    }
    
    //------------------------------------------------------------------------
    MulticastSource & MulticastSource::operator = (const MulticastSource & src)
    {
      if (this != &src) {
        _endpoint = src._endpoint;
        _key = src._key;
        _keyDir = src._keyDir;
        _sinks = src._sinks;
        src._backlog.Copy(_backlog);
        while (! _queryDone) {
        }
        _queryDone.store(true);  // Don't copy thread, and our thread is done
        _lastReceiveTime = src._lastReceiveTime;
      }
      return *this;
    }
    
    //------------------------------------------------------------------------
    MulticastSource & MulticastSource::operator = (MulticastSource && src)
    {
      if (this != &src) {
        while (! src._queryDone) {
        }
        while (! _queryDone) {
        }
        _queryDone.store(true);
        _endpoint = std::move(src._endpoint);
        _key = src._key;
        _keyDir = src._keyDir;
        _sinks = src._sinks;
        _backlog.Clear();
        src._backlog.Swap(_backlog);
        _lastReceiveTime = src._lastReceiveTime;
      }
      return *this;
    }
    
    //------------------------------------------------------------------------
    MulticastSourceKey MulticastSource::Key() const
    {
      return _key;
    }
    
    //------------------------------------------------------------------------
    void MulticastSource::Key(const MulticastSourceKey & key)
    {
      _key = key;
      return;
    }

    //------------------------------------------------------------------------
    bool MulticastSource::ProcessBacklog()
    {
      string  mcastKey = Key().Value();
      
      if (! mcastKey.empty()) {
        while (! _backlog.Empty()) {
          BacklogEntry  ble;
          if (_backlog.PopFront(ble)) {
            MessagePacket  pkt(ble.Data(), ble.Datalen());
            ssize_t  decrc = pkt.Decrypt(ble.Datalen(), mcastKey);
            if (decrc > 0) {
              Message  msg;
              while (msg.Read(pkt.Payload())) {
                if (nullptr != _sinks) {
                  for (auto sink : *_sinks) {
                    sink->PushBack(msg);
                  }
                }
              }
            }
          }
        }
      }
      return _backlog.Empty();
    }

    //------------------------------------------------------------------------
    void MulticastSource::ClearOldBacklog()
    {
      auto  now = chrono::system_clock::now();
      
      BacklogEntry  ble;
      while (_backlog.PopFront(ble)) {
        if (now < ble.ReceiveTime() + chrono::seconds(5)) {
          _backlog.PushFront(ble);
          break;
        }
        else {
          FSyslog(LOG_DEBUG, "Dropped backlog entry of {} bytes from {}",
                 ble.Datalen(), _endpoint);
        }
      }
      return;
    }

    //------------------------------------------------------------------------
    bool MulticastSource::ProcessPacket(char *data, size_t datalen)
    {
      bool  rc = false;

      _lastReceiveTime = std::chrono::system_clock::now();
      
      string  mcastKey = Key().Value();
      if (! mcastKey.empty()) {
        ProcessBacklog();

        MessagePacket  pkt(data, datalen);
        ssize_t  decrc = pkt.Decrypt(datalen, mcastKey);
        if (decrc > 0) {
          Message  msg;
          while (msg.Read(pkt.Payload())) {
            rc = true;
            if (nullptr != _sinks) {
              for (auto sink : *_sinks) {
                sink->PushBack(msg);
              }
            }
          }
        }
        else {
          Key(MulticastSourceKey(""));
          _backlog.PushBack(BacklogEntry(data, datalen));
          auto  expireTime = (std::chrono::system_clock::now()
                              - std::chrono::seconds(5));
          if (Key().LastUpdated() < expireTime) {
            StartQuery();
          }
        }
      }
      else {
        _backlog.PushBack(BacklogEntry(data, datalen));
        StartQuery();
        //  xxx - what else?
        rc = true;
      }
      ClearOldBacklog();
      return rc;
    }

    //------------------------------------------------------------------------
    MulticastSource::Clock::time_point
    MulticastSource::LastReceiveTime() const
    {
      return _lastReceiveTime;
    }
    
    //------------------------------------------------------------------------
    void MulticastSource::StartQuery()
    {
      if (_queryDone.load()) {
        if (_queryThread.joinable()) {
          _queryThread.join();
        }
        _queryDone.store(false);
        _queryThread = std::thread(&MulticastSource::QueryForKey, this);
      }
      return;
    }
    
    //------------------------------------------------------------------------
    void MulticastSource::QueryForKey()
    {
      Syslog(LOG_INFO, "MulticastSource::QueryForKey started");
      
      KeyRequester  keyRequester(_endpoint, *_keyDir);
      auto  result = keyRequester.GetKey();
      
      Key(result);
      _queryDone.store(true);
      Syslog(LOG_INFO, "MulticastSource::QueryForKey done");
      return;
    }
    

  }  // namespace Mclog

}  // namespace Dwm
