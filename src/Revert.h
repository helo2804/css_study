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

#ifndef _Revert_h_
#define _Revert_h_

#include "ByteGen.h"
#include "Error.h"

namespace Revert
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Revert:" + s) {}
  } ;

  // get all the generators for the given 3-byte keystream snippet
  //   this return 2^19 generator states (at offset s+3)
  std::vector<ByteGen> snippet(uint8_t const (&s)[3],bool invertA,bool invertB) ;

  // get all the generators for the given 3-byte keystream head
  //   this return 2^16 generator states (at offset s+3)
  std::vector<ByteGen> head(uint8_t const (&s)[3],bool invertA,bool invertB) ;
  // this is about three times faster than reverting from an arbitrary snippet

  // for each generator return the size in bytes that matches the keystream
  //   the generators are shifted by the number of matching bytes
  std::vector<size_t> match(Range<ByteGen> g,Range<uint8_t const> s) ;

  struct Pair {
    ByteGen g ;
    size_t n ;
    Pair(ByteGen const &g, size_t n) : g(g),n(n) {}
  } ;

  // for the keystream (there must be at least 3 bytes) get all generators 
  //   and the number of matching bytes. the number of matching bytes is 
  //   less or equal to the keystream size
  std::vector<Pair>    headStream(Range<uint8_t const> s,bool invertA,bool invertB) ;
  std::vector<Pair> snippetStream(Range<uint8_t const> s,bool invertA,bool invertB) ;
}

#endif // _Revert_h_
