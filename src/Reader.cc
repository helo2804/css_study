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

#include "Format.h"
#include "Reader.h"
#include <sstream>

bool Reader::eof() const
{
  return this->ofs == this->data.size() ;
}

void Reader::expect(size_t n,uint32_t shall) 
{
  uint32_t got = this->peek(n) ;
  if (got != shall) {
    std::ostringstream os ;
    os << "unexpected data at " << this->ofs 
       << " shall: 0x" << Format::hex(shall) 
       <<   " got: 0x" << Format::hex(got) ;
    throw Error(os.str()) ;
  }
  this->ofs += n ;
}

uint32_t Reader::get(size_t n)
{
  uint32_t v = this->peek(n) ;
  this->ofs += n ;
  return v ;
}

uint32_t Reader::peek(size_t n)
{
  if (this->ofs + n > this->data.size()) {
    throw Error("unexpected eof") ; 
  }
  uint32_t v = 0 ;
  for (size_t i=0 ; i<n ; ++i) {
    v <<= 8 ;
    v += this->data[this->ofs+i] ;
  }
  return v ;
}

void Reader::pop(size_t n)
{
  if (n + this->ofs > this->data.size()) {
    throw Error("unexpected eof") ; 
  }
  this->ofs += n ;
}

size_t Reader::tell() const
{
  return this->ofs ;
}

