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

#ifndef _Os_h_
#define _Os_h_

#include "Range.hpp"
#include "Error.h"

#include <errno.h>
#include <fstream>
#include <stdint.h>
#include <sys/ioctl.h>
#include <boost/shared_ptr.hpp>

namespace Os
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Os:" + s) {}
    static Error make(std::string const &s,int error) ;
  } ;

  struct Fd // file descriptor (make sure it get closed)
  {
    static Fd openRo(std::string const &fname) ;
    operator int() const { return this->p->no ; }
  private:
    struct Data 
    {
      int no ;
      Data(int no) : no(no) {} 
      ~Data() { close(this->no) ; }
    } ;
    typedef boost::shared_ptr<Data> Ptr ;
    Ptr p ;
    Fd(Ptr p) : p(p) {}
  } ;

  template<typename T> int ioctl(int fd,int request,T *v)
  {
    int result = ::ioctl(fd,request,v) ;
    if (result < 0)
      throw Error::make("ioctl() failed",errno) ;
    return result ;
  }

  void open(std::fstream &io,
	    std::string const &fname,
	    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) ;

  void open(std::ifstream &is,
	    std::string const &fname,
	    std::ios_base::openmode mode = std::ios_base::in) ;

  void open(std::ofstream &os,
	    std::string const &fname,
	    std::ios_base::openmode mode = std::ios_base::out) ;

  std::streamoff size(std::istream &is) ;

  std::streamsize read(std::istream &is,Range<uint8_t> buffer) ;

  void readAll(std::istream &is,Range<uint8_t> buffer) ;

  void writeAll(std::ostream &os,Range<uint8_t const> buffer) ;

}

#endif // _Os_h_
