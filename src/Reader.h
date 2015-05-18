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

#ifndef _Reader_h_
#define _Reader_h_

#include "Range.hpp"
#include "Error.h"
#include <stdint.h>

struct Reader
{
  typedef Range<uint8_t const> CBR ;

  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Reader:" + s) {}
  } ;

  // eat <n> input bytes and throw Error if number <v> (MSB first) doesn't match
  void expect(size_t n,uint32_t v) ;

  // eat <n> input bytes and return integer value (MSB first)
  uint32_t get(size_t n) ;

  // don't eat <n> input bytes but return integer value (MSB first)
  uint32_t peek(size_t n) ;

  // return true if all input has been eaten up
  bool eof() const ;

  // eat <n> input bytes
  void pop(size_t n) ;

  // tell current offset
  size_t tell() const ;

  Reader(Range<uint8_t const> data) : data(data),ofs(0) {}

private:

  Range<uint8_t const> data ;
  size_t ofs ;

} ;

#endif // _Reader_h_
