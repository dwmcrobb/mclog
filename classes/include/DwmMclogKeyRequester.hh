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
//!  @file DwmMclogKeyRequester.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGKEYREQUESTER_HH_
#define _DWMMCLOGKEYREQUESTER_HH_

#include <cassert>
#include <cstdint>
#include <map>
#include <vector>

#include "DwmMclogUdpEndpoint.hh"
#include "DwmMclogKeyRequesterState.hh"
#include "DwmMclogMulticastSourceKey.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class KeyRequester
    {
    public:
      KeyRequester(const UdpEndpoint servEndpoint,
                   const std::string & keyDir)
          : _servEndpoint(servEndpoint), _keyDir(keyDir), _fd(-1),
            _state(_servEndpoint.Port(), keyDir)
      {
      }

      MulticastSourceKey GetKey();
      
    private:
      UdpEndpoint         _servEndpoint;
      std::string         _keyDir;
      int                 _fd;
      int                 _fd6;
      KeyRequesterState   _state;

      MulticastSourceKey GetKey4();
      MulticastSourceKey GetKey6();
    };

  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGKEYREQUESTER_HH_
