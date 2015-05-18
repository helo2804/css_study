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

#ifndef _Volume_hpp_
#define _Volume_hpp_

#include "Error.h"
#include "Format.h"
#include "Os.h"
#include <sstream>

template <typename AT,typename BST,BST BS> struct Volume
{
  // used as Volume<uint32_t,uint32_t,0x800>

  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Volume:" + s) {}
  } ;

  typedef AT AddressType ;

  typedef BST BlockSizeType ;

  static BlockSizeType const blockSize = BS ;

  AT count() { return Os::size(this->is) / blockSize ; }

  void read(AddressType lba,uint8_t(*block)[blockSize])
  {
    std::streampos p = size_t(blockSize) * lba ; // todo: watch overflows!
    this->is.seekg(p) ; 
    try {
      Os::readAll(this->is,block) ;
    }
    catch (Error const &e) {
      std::ostringstream os ;
      os << "at LBA 0x" << Format::hex(lba) << ":" << e.what() ; 
      throw Error(os.str()) ;
    }
  }

  Volume(std::string const &fname)
  {
    Os::open(this->is,fname) ;
  }

  std::ifstream is ;
} ;

#endif // _Volume_hpp_
