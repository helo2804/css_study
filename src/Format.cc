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

#include "Domain.hpp"
#include "Format.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <iomanip> // setfill,setw
#include <limits> // std::numeric_limits
#include <sstream>

#include <boost/optional.hpp>

typedef Domain<unsigned long,0,15> D16 ;

// ('0',0) ('1',1) .. ('9',9) ('a',10) .. ('f',15)
static boost::optional<D16> asciiTo16(char c)
{
  D16::Type i ;
  if (('0' <= c) && (c <= '9')) {
    i = c - '0' ;
  }
  else {
    if ((c < 'a') || ('f' < c)) {
      return boost::optional<D16>() ;
    }
    i = c - 'a' + 10 ;
  }
  return D16(i) ;
}

// --------------------------------------------------------------------

std::vector<uint8_t> Format::byteV(std::string const &hex) 
{
  std::vector<uint8_t> v ;
  size_t n = convert(hex.c_str(),&v) ;
  v.resize(n) ;
  convert(hex.c_str(),&v) ;
  return v ;
}

size_t Format::convert(char const hex[],Range<uint8_t> a)
{
  char const *p = hex ;
  size_t i = 0 ;
  goto Enter ;
  while (p[0] == ':') {
    ++p ;
  Enter:
    if (!p[0] || !p[1]) 
      throw Error("hex dyad expected") ;
    char s[3] = { p[0],p[1],0 } ;
    if (i < a.size())
      a[i] = to_ulong(s,16) ;
    ++i ;
    p += 2 ;
  }
  if (p[0] != 0)
    throw Error("hex dyads must be separated by colon (:)") ;
  return i ;
}

std::string Format::escape(std::string const &s)
{
  std::ostringstream os ;
  for (std::string::const_iterator i=s.begin() ; i!=s.end() ; ++i) {
    if ((*i) == '\\') {
      os << "\\\\" ;
    }
    else {
      int c = static_cast<unsigned char>(*i) ;
      if (0 != isprint(c)) {
	os << (*i) ;
      }
      else {
	os << "\\x" << std::setfill('0') << std::setw(2) << std::hex << c ;
      }
    }
  }
  return os.str() ;
}

std::string Format::hex(Range<uint8_t const> r)
{
  std::ostringstream os ;
  size_t i = 0 ;
  if (i < r.size()) {
    os << std::hex << std::setfill('0') << std::setw(2) << int(r[i]) ;
    while (++i < r.size()) {
      os << ":" << std::setfill('0') << std::setw(2) << int(r[i]) ;
    }
  }
  return os.str() ;
}

std::string Format::sprintf(char const format[],...) 
{
  va_list args ;
  va_start(args,format) ;
  int size = vsnprintf(NULL,0,format,args) ;
  char *buffer = static_cast<char*>(malloc(size+1)) ;
  assert(buffer != NULL) ;
  va_start(args,format) ;
  int result = vsnprintf(buffer,size+1,format,args) ;
  assert(result == size) ;
  std::string s(buffer,size) ;
  free(buffer) ;
  return s ;
}

unsigned long Format::to_ulong(char const s[],size_t base)
{
  D16(base-1) ; // range check
  unsigned long v = 0 ;
  for (char const *p=s ; (*p)!=0 ; ++p) {
    boost::optional<D16> o = asciiTo16(*p) ;
    if (!o)
      throw Error("digit out of range") ;
    D16::Type d = (*o) ;
    if (d >= base)
      throw Error("digit out of range") ;
    if (std::numeric_limits<unsigned long>::max() / base < v)
      throw Error("overflow") ;
    v *= base ; 
    if (std::numeric_limits<unsigned long>::max() - d < v) 
      throw std::range_error("overflow") ;
    v += d ;
  }
  return v ;
}

unsigned long Format::to_ulong(char const s[])
{
  if (s[0] == '0') {
    if (s[1] == 'x') return to_ulong(s+2,16) ;
    if (s[1] == 'b') return to_ulong(s+2, 2) ;
  }
  return to_ulong(s,10) ;
}
