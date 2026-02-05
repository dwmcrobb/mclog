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
//!  @file DwmMclogMulticastSourceKey.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MulticastSourceKey implementation
//---------------------------------------------------------------------------

#include "DwmMclogMulticastSourceKey.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    MulticastSourceKey::MulticastSourceKey()
        : _mtx(), _value(), _lastRequested(), _lastQueried(), _lastUpdated()
    {}
    
    //------------------------------------------------------------------------
    MulticastSourceKey::MulticastSourceKey(std::string value)
        : _mtx(), _value(value)
    {}

    //------------------------------------------------------------------------
    MulticastSourceKey::MulticastSourceKey(const MulticastSourceKey & key)
        : _mtx(), _value(key._value)
    {}

    //------------------------------------------------------------------------
    MulticastSourceKey &
    MulticastSourceKey::operator = (const MulticastSourceKey & key)
    {
      if (&key != this) {
        _value = key._value;
        _lastRequested = key._lastRequested;
        _lastQueried = key._lastQueried;
        _lastUpdated = key._lastUpdated;
      }
      return *this;
    }
    
    //------------------------------------------------------------------------
    std::string MulticastSourceKey::Value() const
    {
      std::lock_guard  lck(_mtx);
      return _value;
    }
    
    //------------------------------------------------------------------------
    void MulticastSourceKey::Value(const std::string & value)
    {
      std::lock_guard  lck(_mtx);
      _value = value;
      return;
    }

    //------------------------------------------------------------------------
    MulticastSourceKey::Clock::time_point
    MulticastSourceKey::LastRequested() const
    {
      std::lock_guard  lck(_mtx);
      return _lastRequested;
    }
    
    //------------------------------------------------------------------------
    void MulticastSourceKey::LastRequested(Clock::time_point lastRequested)
    {
      std::lock_guard  lck(_mtx);
      _lastRequested = lastRequested;
      return;
    }

    //------------------------------------------------------------------------
    MulticastSourceKey::Clock::time_point
    MulticastSourceKey::LastQueried() const
    {
      std::lock_guard  lck(_mtx);
      return _lastQueried;
    }
    
    //------------------------------------------------------------------------
    void MulticastSourceKey::LastQueried(Clock::time_point lastQueried)
    {
      std::lock_guard  lck(_mtx);
      _lastQueried = lastQueried;
      return;
    }

    //------------------------------------------------------------------------
    MulticastSourceKey::Clock::time_point
    MulticastSourceKey::LastUpdated() const
    {
      std::lock_guard  lck(_mtx);
      return _lastUpdated;
    }
    
    //------------------------------------------------------------------------
    void MulticastSourceKey::LastUpdated(Clock::time_point lastUpdated)
    {
      std::lock_guard  lck(_mtx);
      _lastUpdated = lastUpdated;
      return;
    }
    
  }  // namespace Mclog

}  // namespace Dwm
