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

#ifndef _Handshake_h_
#define _Handshake_h_

#include "Auth.h"
#include "Error.h"
#include "Mmc.h"

#include <boost/optional.hpp>

namespace Handshake
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Handshake:" + s) {}
  } ;

  boost::optional<Mmc::DiscKey> unlock(int fd,uint8_t const (&nonce1)[10],unsigned int timeout) ;
  // throws various kinds of errors on problems
  // returns disc-key if CSS-protected disc was successfully unlocked
  // returns nothing if the disc is not CSS-protected

  boost::optional<Mmc::Title> title(int fd,uint8_t const (&nonce1)[10],unsigned int timeout,uint32_t lba) ;
  // throws various kinds of errors on problems
  // returns title-key for CSS-protected disc
  // returns 00:00:00:00:00 or nothing if the disc is not CSS-protected
} 

#endif // _Handshake_h_
