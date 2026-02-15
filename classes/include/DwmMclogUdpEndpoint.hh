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
//!  @file DwmMclogUdpEndpoint.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::UdpEndpoint class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGUDPENDPOINT_HH_
#define _DWMMCLOGUDPENDPOINT_HH_

#if __has_include(<format>)
#  include <format>
#else
#  if __has_include(<fmt/format.h>)
#    include <fmt/format.h>
#  endif
#endif

#include "DwmIpAddress.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a UDP end point (IP address and port).
    //------------------------------------------------------------------------
    class UdpEndpoint
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor.
      //----------------------------------------------------------------------
      UdpEndpoint() = default;
      
      //----------------------------------------------------------------------
      //!  Copy constructor.
      //----------------------------------------------------------------------
      UdpEndpoint(const UdpEndpoint &) = default;
      
      //----------------------------------------------------------------------
      //!  Copy assignment.
      //----------------------------------------------------------------------
      UdpEndpoint & operator = (const UdpEndpoint &) = default;
      
      //----------------------------------------------------------------------
      //!  Construct from the given @c addr and @c port.
      //----------------------------------------------------------------------
      UdpEndpoint(const IpAddress & addr, uint16_t port)
          : _addr(addr), _port(port)
      {}

      //----------------------------------------------------------------------
      //!  Construct from a sockaddr_in
      //----------------------------------------------------------------------
      UdpEndpoint(const sockaddr_in & sockAddr)
          : _addr(Ipv4Address(sockAddr.sin_addr.s_addr)),
            _port(ntohs(sockAddr.sin_port))
      {}

      //----------------------------------------------------------------------
      //!  Construct from a sockaddr_in6
      //----------------------------------------------------------------------
      UdpEndpoint(const sockaddr_in6 & sockAddr)
          : _addr(Ipv6Address(sockAddr.sin6_addr)),
            _port(ntohs(sockAddr.sin6_port))
      {}

      //----------------------------------------------------------------------
      //!  Return the end point as a sockaddr_in.
      //----------------------------------------------------------------------
      operator sockaddr_in () const;

      //----------------------------------------------------------------------
      //!  Return the end point as a sockaddr_in6.
      //----------------------------------------------------------------------
      operator sockaddr_in6 () const;
      
      //----------------------------------------------------------------------
      //!  Returns the address.
      //----------------------------------------------------------------------
      const IpAddress & Addr() const
      { return _addr; }

      //----------------------------------------------------------------------
      //!  Returns the port.
      //----------------------------------------------------------------------
      uint16_t Port() const
      { return _port; }

      //----------------------------------------------------------------------
      //!  Sets and returns the port.
      //----------------------------------------------------------------------
      uint16_t Port(uint16_t port)
      { return (_port = port); }
      
      //----------------------------------------------------------------------
      //!  Less-than operator.
      //----------------------------------------------------------------------
      bool operator < (const UdpEndpoint & ep) const
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
      //!  operator ==
      //----------------------------------------------------------------------
      bool operator == (const UdpEndpoint &) const = default;

      //----------------------------------------------------------------------
      //!  Return the end point as a human-readable string.
      //----------------------------------------------------------------------
      operator std::string() const;
      
      //----------------------------------------------------------------------
      //!  Emits a UdpEndpoint to an ostream in human-readable form.
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const UdpEndpoint & endPoint);
      
    private:
      IpAddress  _addr;
      uint16_t   _port;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#if __has_include(<format>)

namespace std {
  //--------------------------------------------------------------------------
  //!  Formatter for Dwm::Mclog::UdpEndpoint
  //--------------------------------------------------------------------------
  template <>
  struct formatter<Dwm::Mclog::UdpEndpoint> 
  {
    constexpr auto parse(format_parse_context & ctx) 
    { return ctx.begin(); }
    
    auto format(const Dwm::Mclog::UdpEndpoint & ep,
                format_context & ctx) const
    {
      return format_to(ctx.out(), "{}:{}", (string)(ep.Addr()), ep.Port());
    }
  };
}

#else
#  if __has_include(<fmt/format.h>)

//----------------------------------------------------------------------------
//!  Formatter for Dwm::Mclog::UdpEndpoint
//----------------------------------------------------------------------------
template <>
struct fmt::formatter<Dwm::Mclog::UdpEndpoint> 
{
  constexpr auto parse(fmt::format_parse_context & ctx) 
  { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const Dwm::Mclog::UdpEndpoint & ep, FormatContext & ctx) 
  {
    return fmt::format_to(ctx.out(), "{}:{}",
                          (std::string)(ep.Addr()), ep.Port());
  }
};

#  endif
#endif
  
    
#endif  // _DWMMCLOGUDPENDPOINT_HH_
