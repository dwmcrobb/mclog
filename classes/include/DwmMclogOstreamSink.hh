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
//!  @file DwmMclogOstreamSink.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::OstreamSink class
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGOSTREAMSINK_HH_
#define _DWMMCLOGOSTREAMSINK_HH_

#include <iostream>
#include <mutex>

#include "DwmMclogMessageSink.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  A MessageSink wrapper around a std::ostream.
    //------------------------------------------------------------------------
    class OstreamSink
      : public MessageSink 
    {
    public:
      //----------------------------------------------------------------------
      //!  No default construction; we need an ostream reference.
      //----------------------------------------------------------------------
      OstreamSink() = delete;
      
      //----------------------------------------------------------------------
      //!  Construct from a reference to an ostream.  Note we keep the
      //!  ostream as a reference (they can't be copied), so the caller
      //!  must ensure that the given ostream @c os outlives their use
      //!  of the @c OstreamSink.
      //----------------------------------------------------------------------
      OstreamSink(std::ostream & os)
          : _mtx(), _os(os)
      {}

      //----------------------------------------------------------------------
      //!  Process the given message @c msg.  This will cause the @c msg to
      //!  be written to the ostream in human-readable form.  Returns true
      //!  on success, false on failure.
      //----------------------------------------------------------------------
      bool Process(const Message & msg)
      {
        std::lock_guard  lck(_mtx);
        _os << msg << std::flush;
        return (! _os.fail());
      }
      
    private:
      std::mutex     _mtx;
      std::ostream & _os;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGOSTREAMSINK_HH_
