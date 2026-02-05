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
//!  @file DwmMclogUdpEndpoint.cc
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::UdpEndpoint implementation
//---------------------------------------------------------------------------

#include "DwmMclogUdpEndpoint.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    UdpEndpoint::operator sockaddr_in () const
    {
      assert(_addr.Family() == AF_INET);
      
      sockaddr_in  sockAddr;
      memset(&sockAddr, 0, sizeof(sockAddr));
      sockAddr.sin_family = PF_INET;
      sockAddr.sin_addr.s_addr = _addr.Addr<Ipv4Address>()->Raw();
      sockAddr.sin_port = htons(_port);
#ifndef __linux__
      sockAddr.sin_len = sizeof(sockAddr);
#endif
      return sockAddr;
    }

    //------------------------------------------------------------------------
    UdpEndpoint::operator sockaddr_in6 () const
    {
      assert(_addr.Family() == AF_INET6);
      
      sockaddr_in6  sockAddr;
      memset(&sockAddr, 0, sizeof(sockAddr));
      sockAddr.sin6_family = PF_INET6;
      sockAddr.sin6_addr = *(_addr.Addr<Ipv6Address>());
      sockAddr.sin6_port = htons(_port);
#ifndef __linux__
      sockAddr.sin6_len = sizeof(sockAddr);
#endif
      return sockAddr;
    }
    
    //------------------------------------------------------------------------
    UdpEndpoint::operator std::string () const
    {
      std::string  rc((std::string)_addr + ':' + std::to_string(_port));
      return rc;
    }

    //------------------------------------------------------------------------
    std::ostream & operator << (std::ostream & os,
                                const UdpEndpoint & endPoint)
    {
      os << (std::string)endPoint;
      return os;
    }
    

  }  // namespace Mclog

}  // namespace Dwm
