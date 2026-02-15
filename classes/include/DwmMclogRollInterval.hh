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
//!  @file DwmMclogRollInterval.hh
//!  @author Daniel W. McRobb
//!  @brief Dwm::Mclog::RollInterval class declaration
//---------------------------------------------------------------------------

#ifndef _DWMMCLOGROLLINTERVAL_HH_
#define _DWMMCLOGROLLINTERVAL_HH_

#include <ctime>
#include <iostream>

#include "DwmMclogRollPeriod.hh"

namespace Dwm {

  namespace Mclog {

    //------------------------------------------------------------------------
    //!  Encapsulate a logfile rollover interval.
    //------------------------------------------------------------------------
    class RollInterval
    {
    public:
      //----------------------------------------------------------------------
      //!  Default constructor.
      //----------------------------------------------------------------------
      RollInterval();
      
      //----------------------------------------------------------------------
      //!  Construct from the given RollPeriod @c rp.
      //----------------------------------------------------------------------
      RollInterval(RollPeriod rp);
      
      //----------------------------------------------------------------------
      //!  Copy constructor.
      //----------------------------------------------------------------------
      RollInterval(const RollInterval &) = default;
      
      //----------------------------------------------------------------------
      //!  Copy assignment.
      //----------------------------------------------------------------------
      RollInterval & operator = (const RollInterval &) = default;
      
      //----------------------------------------------------------------------
      //!  Move constructor.
      //----------------------------------------------------------------------
      RollInterval(RollInterval &&) = default;
      
      //----------------------------------------------------------------------
      //!  Move assignment.
      //----------------------------------------------------------------------
      RollInterval & operator = (RollInterval &&) = default;
      
      //----------------------------------------------------------------------
      //!  Returns the start time of the roll interval.
      //----------------------------------------------------------------------
      time_t StartTime() const;
      
      //----------------------------------------------------------------------
      //!  Returns the end time of the roll interval.
      //----------------------------------------------------------------------
      time_t EndTime() const;
      
      //----------------------------------------------------------------------
      //!  Set the roll interval to the interval that spans the current time.
      //----------------------------------------------------------------------
      void SetToCurrent();

      //----------------------------------------------------------------------
      //!  Emit a RollInterval to an ostream in human-readable form.
      //----------------------------------------------------------------------
      friend std::ostream &
      operator << (std::ostream & os, const RollInterval & ri);
      
    private:
      RollPeriod  _rp;
      int tm::*   _fieldToIncrement;
      int         _incrementAmount;
      time_t      _endTime;
    };
    
  }  // namespace Mclog

}  // namespace Dwm

#endif  // _DWMMCLOGROLLINTERVAL_HH_
