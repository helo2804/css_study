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

#ifndef _Format_h_
#define _Format_h_

#include "Error.h"
#include "Range.hpp"

#include <string>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <boost/type_traits/is_unsigned.hpp>
#include <boost/utility/enable_if.hpp>

namespace Format
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Format:" + s) {}
  } ;

  // ---- hex/dec/bin ASCII string to scalar unsigned long ------------

  unsigned long to_ulong(char const s[],size_t base) ;

  unsigned long to_ulong(char const s[]) ;

  template<typename T> T number(std::string const &s) 
  { 
    unsigned long u1 = to_ulong(s.c_str()) ; 
    T u2 = static_cast<T>(u1) ;
    if (static_cast<unsigned long>(u1) != u2)
      throw Error("overflow") ;
    return u2 ;
  }

  // ---- scalar unsigned integer to hexadecimal ASCII ----------------

  template<typename T> typename boost::enable_if<boost::is_unsigned<T>,std::string>::type hex(T u)
  {
    std::ostringstream os ;
    do {
      T d = u % T(16) ;
      if (d < T(10)) os << static_cast<char>(T('0') + d) ;
      else           os << static_cast<char>(T('a') + d - T(10)) ;
      u = u / T(16) ;
    }
    while (u != T(0)) ;
    std::string s = os.str() ;
    std::reverse(s.begin(),s.end()) ;
    return s ;
  }

  // ---- byte array to/from hexadecimal ASCII ------------------------

  // byte array to hexadecimal ASCII string
  std::string hex(Range<uint8_t const> r) ;

  // hexadecimal ASCIIZ C-string to byte array, returns the number of
  //   bytes represented by <src> which maybe different from <dst> size
  size_t convert(char const src[],Range<uint8_t> dst) ;

  // hexadecimal ASCIIZ C-string to byte vector
  std::vector<uint8_t> byteV(std::string const &hex) ;

  // hexadecimal ASCIIZ C-string to byte array
  template<size_t N> Array<uint8_t,N> byteA(std::string const &hex) 
  {
    Array<uint8_t,N> a ;
    size_t n = convert(hex.c_str(),&a) ;
    if (n != N)
      throw Error("size does not match") ;
    return a ;
  }

  // ---- string formatting -------------------------------------------

  // based on vsnprintf which is C99 / C++11 and/or C-extension
  std::string sprintf(char const format[],...) ;

  // replace non-printable characters with "\x##" where ## is the hexa- 
  //   decimal value of the character (see isprint(3))
  std::string escape(std::string const &s) ;
} 

#endif // _Format_h_
