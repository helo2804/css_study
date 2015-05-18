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

#ifndef _Decrypt_h_
#define _Decrypt_h_

#include "Range.hpp"
#include "Error.h"
#include "Array.hpp"
#include <stdint.h>
#include <boost/optional.hpp>

namespace Decrypt
{
  typedef Array<uint8_t,5> B5 ;

  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Decrypt:" + s) {}
  } ;

  B5 key(uint8_t const (&keyStream)[5],uint8_t const (&cipher)[5]) ;

  B5 discKey(uint8_t const (&playerKey)[5],uint8_t const (&cipher)[5]) ;

  boost::optional<B5> discKey(uint8_t const (&playerKey)[5],uint8_t const (&cipher)[5],uint8_t const (&hash)[5]) ;

  B5 titleKey(uint8_t const (&discKey)[5],uint8_t const (&cipher)[5]) ;

  void feature(uint8_t const (&seed)[5],Range<uint8_t const> cipher,Range<uint8_t> plain) ;
}

#endif // _Decrypt_h_ 
