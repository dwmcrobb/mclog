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

#include "DwmMclogMulticastSource.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::BacklogEntry()
        : _data(nullptr), _datalen(0)
    {}
    
    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::BacklogEntry(const char *data,
                                                size_t datalen)
    {
      if ((nullptr != _data) && (0 < _datalen)) {
        free((void *)_data); _data = nullptr; _datalen = 0;
      }
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
    }
      
    //------------------------------------------------------------------------
    MulticastSource::BacklogEntry::BacklogEntry(MulticastSource::BacklogEntry && ble)
    {
      _data = ble._data;
      _datalen = ble._datalen;
      ble._data = nullptr;
      ble._datalen = 0;
    }
    
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
    std::chrono::system_clock::time_point
    MulticastSource::BacklogEntry::ReceiveTime() const
    {
      return _receiveTime;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
