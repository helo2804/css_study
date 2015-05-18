// Copyright (c) 2015, helo2804. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions 
// are met:
//
// * Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright 
//   notice, this list of conditions and the following disclaimer in 
//   the documentation and/or other materials provided with the 
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.

// This project is hosted at https://github.com/helo2804/css_study

#ifndef _Sg_h_
#define _Sg_h_

// --------------------------------------------------------------------
// SCSI access thru the Linux SG device driver (http://sg.danny.cz/sg)
// --------------------------------------------------------------------

#include "Error.h"
#include "Range.hpp"

#include <stdint.h>
#include <stdexcept>
#include <sys/types.h>
#include <scsi/sg.h>
#include <string>
#include <vector>

namespace Sg
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Sg:" + s) {}
  } ;

  // returns version number (x.y.z) as (x*10000 + y*100 + z) 
  int version(int fd) ;

  struct Status
  {
    uint16_t senseCode() const ;
    uint8_t senseKey() const ;
    bool success() const ;
    std::string toString() const ;
    ~Status() throw() {} // required by Mmc::Error which includes Status
  private:
    friend Status sgctl(int,Range<uint8_t const>,unsigned int,int,Range<uint8_t>) ;
    Status(sg_io_hdr const &io,Range<uint8_t const> sense) : io(io),sense(sense.begin(),sense.end()) {}
    sg_io_hdr const io ;
    std::vector<uint8_t> sense ;
  } ;

  // ioctl(SG_IO) wrapper
  Status sgctl(int                    fd,
	       Range<uint8_t const>  cmd,
	       unsigned int      timeout,   // struct sg_io_hdr.timeout
	       int             direction,   // struct sg_io_hdr.dxfer_direction
	       Range<uint8_t>       xfer) ; // space limited by interface

  // public SCSI methods:
  Status invoke    (int fd,Range<uint8_t const> cmd,unsigned int timeout /* no xfer data       */ ) ;
  Status invokeRecv(int fd,Range<uint8_t const> cmd,unsigned int timeout,Range<uint8_t>       xfer) ;
  Status invokeSend(int fd,Range<uint8_t const> cmd,unsigned int timeout,Range<uint8_t const> xfer) ;

}

#endif // _Sg_h_ 
