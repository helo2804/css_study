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

#include "Os.h"

#include <cassert>
#include <cerrno>
#include <cstring> // strerror
#include <fcntl.h>
#include <sstream>

Os::Error Os::Error::make(std::string const &s,int error) 
{
  std::ostringstream os ;
  os << s << " " << error << ":" <<  strerror(error) ;
  return Error(os.str()) ;
}

Os::Fd Os::Fd::openRo(std::string const &fname)
{
  int fd = ::open(fname.c_str(),O_RDONLY) ;
  if (fd < 0) 
    throw Error::make("open() failed",errno) ;
  return Ptr(new Data(fd)) ;
}

void Os::open(std::fstream &io,std::string const &fname,std::ios_base::openmode mode)
{
  io.open(fname.c_str(),mode) ;
  if (!io)
    throw Error::make(std::string("fstream(") + fname + ") failed",errno) ;
}

void Os::open(std::ifstream &is,std::string const &fname,std::ios_base::openmode mode)
{
  is.open(fname.c_str(),mode) ;
  if (!is)
    throw Error::make(std::string("ifstream(") + fname + ") failed",errno) ;
}

void Os::open(std::ofstream &os,std::string const &fname,std::ios_base::openmode mode)
{
  os.open(fname.c_str(),mode) ;
  if (!os)
    throw Error::make(std::string("ofstream(") + fname + ") failed",errno) ;
}

std::streamsize Os::read(std::istream &is,Range<uint8_t> buffer)
{
  std::streamsize n = buffer.size() ;
  assert(n >= 0) ; // (signed) streamsize and (unsigned) size_t 
  BOOST_STATIC_ASSERT(sizeof(char) == sizeof(uint8_t)) ;
  char *p = reinterpret_cast<char*>(&buffer[0]) ;
  is.read(p,n) ;
  if (is.bad())
    throw Error::make("istream.bad",errno) ; 
  if (is.good()) {
    if (is.gcount() != n)
      throw Error("istream good but not complete") ;
  }
  else {
    if (is.gcount() >= n)
      throw Error("istream complete bot not good") ;
  }
  return is.gcount() ;
}

void Os::readAll(std::istream &is,Range<uint8_t> buffer)
{
  std::streamsize got = read(is,buffer) ;
  if (got != static_cast<std::streamsize>(buffer.size())) {
    std::ostringstream os ;
    os << "istream:incomplete block:should=" << buffer.size() << " got=" << got ;
    throw Error(os.str()) ; 
  }
  // note: this is the same as read() plus an additional check.
  //   it is not suitable for non-blocking i/o
}

std::streamoff Os::size(std::istream &is) 
{
  std::streampos current = is.tellg() ;
  is.seekg(0,std::ios::beg) ;
  std::streampos begin = is.tellg() ;
  is.seekg(0,std::ios::end) ;
  std::streampos end = is.tellg() ;
  is.seekg(current,std::ios::beg) ;
  return (end - begin) ;
}

void Os::writeAll(std::ostream &os,Range<uint8_t const> buffer) 
{
  std::streamsize n = buffer.size() ;
  assert(n >= 0) ; // (signed) streamsize and (unsigned) size_t 
  BOOST_STATIC_ASSERT(sizeof(char const) == sizeof(uint8_t const)) ;
  char const *p = reinterpret_cast<char const*>(&buffer[0]) ;
  os.write(p,n) ;
  if (os.bad()) {
    throw Error::make("ostream.bad",errno) ; 
  }
  if (!os.good()) {
    throw Error("!ostream.good") ; 
  }
}

