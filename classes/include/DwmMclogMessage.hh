//===========================================================================
//  Copyright (c) Daniel W. McRobb 2025, 2026
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
//!  @file DwmMclogMessage.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::Message class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGMESSAGE_HH_
#define _DWMMCLOGMESSAGE_HH_

#include <vector>

#include "DwmMclogMessageHeader.hh"
#include "DwmMclogVersion.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulates a log message.
    //------------------------------------------------------------------------
    class Message
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor
      //----------------------------------------------------------------------
      Message();
      
      //----------------------------------------------------------------------
      //!  Copy constructor
      //----------------------------------------------------------------------
      Message(const Message &) = default;
      
      //----------------------------------------------------------------------
      //!  Copy assignment
      //----------------------------------------------------------------------
      Message & operator = (const Message &) = default;
      
      //----------------------------------------------------------------------
      //!  Move constructor
      //----------------------------------------------------------------------
      Message(Message &&) = default;
      
      //----------------------------------------------------------------------
      //!  Move assignment
      //----------------------------------------------------------------------
      Message & operator = (Message &&) = default;
      
      //----------------------------------------------------------------------
      //!  Construct from a header and move-from message string.
      //----------------------------------------------------------------------
      Message(const MessageHeader & header, std::string && message);

      //----------------------------------------------------------------------
      //!  Construct from a header and a message string.
      //----------------------------------------------------------------------
      Message(const MessageHeader & header, const std::string & message);

      //----------------------------------------------------------------------
      //!  Returns a const reference to the contained message header.
      //----------------------------------------------------------------------
      const MessageHeader & Header() const
      { return _header; }

      //----------------------------------------------------------------------
      //!  Returns a const reference to the message data.
      //----------------------------------------------------------------------
      const std::string & Data() const
      { return _message.Value(); }
        
      //----------------------------------------------------------------------
      //!  Reads the message from the given istream @c is.  Returns @c is.
      //----------------------------------------------------------------------
      std::istream & Read(std::istream & is);

      //----------------------------------------------------------------------
      //!  Writes the message to the given ostream @c os.  Returns @c os.
      //----------------------------------------------------------------------
      std::ostream & Write(std::ostream & os) const;

      //----------------------------------------------------------------------
      //!  Returns the number of bytes that would be written if we called
      //!  the Write() member.
      //----------------------------------------------------------------------
      uint64_t StreamedLength() const;
      
      //----------------------------------------------------------------------
      //!  Prints the given @c msg to the given ostream @c os in the form
      //!  suitable for human consumption.
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const Message & msg);
      
    private:
      MessageHeader                _header;
      Credence::ShortString<1500>  _message;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGMESSAGE_HH_
