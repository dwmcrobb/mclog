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
//!  @file DwmMclogConfig.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Config class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGCONFIG_HH_
#define _DWMMCLOGCONFIG_HH_

#include "DwmIpv4Address.hh"
#include "DwmIpv6Address.hh"
#include "DwmMclogRollPeriod.hh"
#include "DwmMclogMessageSelector.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulate multicast configuration ('multicast' in config file)
    //------------------------------------------------------------------------
    class MulticastConfig
    {
    public:
      MulticastConfig();
      MulticastConfig(const MulticastConfig &) = default;
      MulticastConfig & operator = (const MulticastConfig &) = default;
      bool ShouldSendIpv4() const;
      bool ShouldSendIpv6() const;
      bool ShouldRunSender() const;
      void Init();
      
      Ipv4Address  groupAddr;   // ipv4 group address
      Ipv6Address  groupAddr6;  // ipv6 group address
      std::string  intfName;    // interface name (e.g. "eth0")
      Ipv4Address  intfAddr;    // interface ipv4 address
      Ipv6Address  intfAddr6;   // interface ipv6 address
      uint16_t     dstPort;     // destination port
      std::string  outFilter;   // output filter expression
    };

    //------------------------------------------------------------------------
    //!  Encapsulate loopback configuration ('loopback' in config file)
    //------------------------------------------------------------------------
    class LoopbackConfig
    {
    public:
      LoopbackConfig()  { Init(); }
      LoopbackConfig(const LoopbackConfig &) = default;
      LoopbackConfig & operator = (const LoopbackConfig &) = default;
      bool ListenIpv4() const  { return (listenIpv4 && (port != 0)); }
      bool ListenIpv6() const  { return (listenIpv6 && (port != 0)); }
      void Init() { listenIpv4 = false; listenIpv6 = false; port = 3737; }

      bool         listenIpv4;  // listen on 127.0.0.1 ?
      bool         listenIpv6;  // listen on ::1 ?
      uint16_t     port;        // listen port
    };

    //------------------------------------------------------------------------
    //!  Encapsulates service configuration ('service' in config file)
    //------------------------------------------------------------------------
    class ServiceConfig
    {
    public:
      ServiceConfig()  { Init(); }
      ServiceConfig(const ServiceConfig &) = default;
      ServiceConfig & operator = (const ServiceConfig &) = default;
      void Init();
      
      std::string  keyDirectory;  // directory where Credence keys are stored
    };

    //------------------------------------------------------------------------
    //!  Log file configuration (each entry in 'logs' in config file)
    //------------------------------------------------------------------------
    class LogFileConfig
    {
    public:
      LogFileConfig();
      LogFileConfig(const LogFileConfig &) = default;
      LogFileConfig & operator = (const LogFileConfig &) = default;
      void Init();
      
      std::string  filter;        // filter expression
      std::string  pathPattern;   // path pattern (can contain %H, %I, %F)
      mode_t       permissions;   // file permissions (octal)
      RollPeriod   period;        // time between rollovers
      uint32_t     keep;          // number of log files to keep when rolling
    };
    
    //------------------------------------------------------------------------
    //!  Log files configuration ('files' in config file)
    //------------------------------------------------------------------------
    class FilesConfig
    {
    public:
      FilesConfig()  { Init(); }
      FilesConfig(const FilesConfig &) = default;
      FilesConfig & operator = (const FilesConfig &) = default;
      void Init();
      
      std::string                 logDirectory;
      std::vector<LogFileConfig>  logs;
    };
    
    //------------------------------------------------------------------------
    //!  Encapsulates mclogd configuration.
    //------------------------------------------------------------------------
    class Config
    {
    public:
      Config() { Init(); }
      Config(const Config &) = default;
      Config & operator = (const Config &) = default;
      bool Parse(const std::string & path);
      bool ShouldSendIpv4() const;
      bool ShouldSendIpv6() const;
      void Init();

      LoopbackConfig                         loopback;
      MulticastConfig                        mcast;
      ServiceConfig                          service;
      std::map<std::string,MessageSelector>  selectors;
      FilesConfig                            files;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGCONFIG_HH_
