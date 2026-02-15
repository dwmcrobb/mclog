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
//!  @file DwmMclogMulticastSourceKey.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::MulticastSourceKey class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMULTICASTSOURCEKEY_HH_
#define _DWMMCLOGMULTICASTSOURCEKEY_HH_

#include <chrono>
#include <mutex>
#include <string>

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates the multicast decryption key for a multicast log
    //!  source, including time information that allows us to make message
    //!  backlog decisions and drive new decryption key queries as needed.
    //------------------------------------------------------------------------
    class MulticastSourceKey
    {
    public:
      using Clock = std::chrono::system_clock;

      //----------------------------------------------------------------------
      //!  Default constructor.
      //----------------------------------------------------------------------
      MulticastSourceKey();

      //----------------------------------------------------------------------
      //!  Construct with the given decryption key @c mcastKey.
      //----------------------------------------------------------------------
      MulticastSourceKey(std::string mcastKey);

      //----------------------------------------------------------------------
      //!  Copy constructor.
      //----------------------------------------------------------------------
      MulticastSourceKey(const MulticastSourceKey & key);

      //----------------------------------------------------------------------
      //!  Copy assignment.
      //----------------------------------------------------------------------
      MulticastSourceKey & operator = (const MulticastSourceKey & key);
      
      //----------------------------------------------------------------------
      //!  Returns the decryption key value.  Empty if we have no decryption
      //!  key.
      //----------------------------------------------------------------------
      std::string Value() const;
      
      //----------------------------------------------------------------------
      //!  Sets the decryption key value.
      //----------------------------------------------------------------------
      void Value(const std::string & value);

      //----------------------------------------------------------------------
      //!  Returns the last time we were asked for the decryption key.
      //----------------------------------------------------------------------
      Clock::time_point LastRequested() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the last time we were asked for the decryption
      //!  key.
      //----------------------------------------------------------------------
      void LastRequested(Clock::time_point lastRequested);
      
      //----------------------------------------------------------------------
      //!  Returns the last time we queried the source for the decryption key.
      //----------------------------------------------------------------------
      Clock::time_point LastQueried() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the last time we queried the source for the
      //!  decryption key.
      //----------------------------------------------------------------------
      void LastQueried(Clock::time_point lastQueried);
      
      //----------------------------------------------------------------------
      //!  Returns the last time we successfully updated the decryption key.
      //----------------------------------------------------------------------
      Clock::time_point LastUpdated() const;
      
      //----------------------------------------------------------------------
      //!  Sets and returns the last time we successfully updated the
      //!  decryption key.
      //----------------------------------------------------------------------
      void LastUpdated(Clock::time_point lastUpdated);
      
    private:
      mutable std::mutex  _mtx;
      std::string         _value;
      Clock::time_point   _lastRequested;
      Clock::time_point   _lastQueried;
      Clock::time_point   _lastUpdated;
    };

  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMULTICASTSOURCEKEY_HH_
