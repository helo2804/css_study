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

#ifndef _Auth_h_
#define _Auth_h_

// --------------------------------------------------------------------
// KEY1, KEY2 and buskey encryption for the authentication handshake 
//   between host (application) and drive (LU - logical unit).
// --------------------------------------------------------------------

#include "Array.hpp"
#include "Domain.hpp"
#include "Error.h"
#include <stdint.h>

namespace Auth
{
  typedef Array<uint8_t,5>  B5 ;

  typedef Domain<unsigned,0,31> Variant ;

  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Auth:" + s) {}
  } ;

  // (variant,nonce1) -> KEY1
  B5 makeKey1(Variant variant,uint8_t const(&nonce)[10]) ;

  // (variant,nonce2) -> KEY2
  B5 makeKey2(Variant variant,uint8_t const(&nonce)[10]) ;

  // (variant,KEY1,KEY2) -> buskey
  B5 makeBusKey(Variant variant,uint8_t const(&key1)[5],uint8_t const(&key2)[5]) ;

  // (nonce1,KEY1) -> variant
  Variant getVariant(uint8_t const(&nonce)[10],uint8_t const(&key1)[5]) ;
}

#endif // _Auth_h_ 
