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
//!  @file DwmMclogUdp4Endpoint.hh
//!  @author Daniel W. McRobb
//!  @brief NOT YET DOCUMENTED
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGUDP4ENDPOINT_HH_
#define _DWMMCLOGUDP4ENDPOINT_HH_

#if __has_include(<format>)
#  include <format>
#else
#  if __has_include(<fmt/format.h>)
#    include <fmt/format.h>
#  endif
#endif

#include "DwmIpv4Address.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  
    //------------------------------------------------------------------------
    class Udp4Endpoint
    {
    public:
      Udp4Endpoint() = default;
      Udp4Endpoint(const Udp4Endpoint &) = default;
      Udp4Endpoint & operator = (const Udp4Endpoint &) = default;
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Udp4Endpoint(const Ipv4Address & addr, uint16_t port)
          : _addr(addr), _port(port)
      {}

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      Udp4Endpoint(const sockaddr_in & sockAddr)
          : _addr(sockAddr.sin_addr.s_addr), _port(ntohs(sockAddr.sin_port))
      {}

      operator sockaddr_in () const;
        
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      const Ipv4Address & Addr() const
      { return _addr; }

      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      uint16_t Port() const
      { return _port; }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool operator < (const Udp4Endpoint & ep) const
      {
        if (_addr < ep._addr) {
          return true;
        }
        else if (_addr == ep._addr) {
          return (_port < ep._port);
        }
        return false;
      }
      
      //----------------------------------------------------------------------
      //!  
      //----------------------------------------------------------------------
      bool operator == (const Udp4Endpoint &) const = default;

      operator std::string() const;
      
      friend std::ostream &
      operator << (std::ostream & os, const Udp4Endpoint & endPoint);
      
    private:
      Ipv4Address  _addr;
      uint16_t     _port;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#if __has_include(<format>)

namespace std {
  //--------------------------------------------------------------------------
  //!  
  //--------------------------------------------------------------------------
  template <>
  struct formatter<Dwm::Mclog::Udp4Endpoint> 
  {
    constexpr auto parse(format_parse_context & ctx) 
    { return ctx.begin(); }
    
    auto format(const Dwm::Mclog::Udp4Endpoint & ep,
                format_context & ctx) const
    {
      return format_to(ctx.out(), "{}:{}", (string)(ep.Addr()), ep.Port());
    }
  };
}

#else
#  if __has_include(<fmt/format.h>)

//----------------------------------------------------------------------------
//!  
//----------------------------------------------------------------------------
template <>
struct fmt::formatter<Dwm::Mclog::Udp4Endpoint> 
{
  constexpr auto parse(fmt::format_parse_context & ctx) 
  { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const Dwm::Mclog::Udp4Endpoint & ep, FormatContext & ctx) 
  {
    return fmt::format_to(ctx.out(), "{}:{}",
                          (std::string)(ep.Addr()), ep.Port());
  }
};

#  endif
#endif
  
    
#endif  // _DWMMCLOGUDP4ENDPOINT_HH_
