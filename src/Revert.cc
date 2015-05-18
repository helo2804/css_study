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

#include "Revert.h"

// (LFSR-A,keystream,invertA,B,carry) -> (LFSR-A,LFSR-B,carry)
static uint32_t assemble(ByteGen::LfsrA *lfsrA,uint8_t const (&s)[3],bool invertA_,bool invertB_,bool *carry)
{
  uint8_t invertA = invertA_ ? ~0 : 0 ;
  uint8_t invertB = invertB_ ? ~0 : 0 ;
  uint32_t registerB = 0 ;
  for (int i=0 ; i<3 ; ++i) {
    uint32_t diff = s[i] ;
    diff -= invertA ^ lfsrA->shift() ; 
    diff -= (*carry) ;
    (*carry) = 1 & (diff >> 8) ;
    registerB <<= 8 ;
    registerB |= invertB ^ ByteGen::reverse(diff) ; 
  }
  return registerB ;
}

std::vector<ByteGen> Revert::snippet(uint8_t const (&s)[3],bool invertA,bool invertB)
{
  std::vector<ByteGen> v ; v.reserve(1<<19) ;
  for (int carry0=0 ; carry0<=1 ; ++carry0) {
    for (uint32_t registerA0=0 ; registerA0<(1<<17) ; ++registerA0) {
      ByteGen::LfsrA lfsrA3 = ByteGen::LfsrA::make(registerA0) ;
      bool carry3 = carry0 ;
      uint32_t registerB3 = assemble(&lfsrA3,s,invertA,invertB,&carry3) ;
      v.push_back(ByteGen::make(lfsrA3,ByteGen::LfsrB::make(          registerB3),invertA,invertB,carry3)) ;
      v.push_back(ByteGen::make(lfsrA3,ByteGen::LfsrB::make(0x1000000|registerB3),invertA,invertB,carry3)) ;
    }
  }
  return v ;
}

std::vector<ByteGen> Revert::head(uint8_t const (&s)[3],bool invertA,bool invertB)
{
  std::vector<ByteGen> v ; v.reserve(1<<17) ; 
  // note: actually only 2^16 records are used since half 
  //   of the LFSR-B values have an invalid start state
  for (int k1=0 ; k1<0x100 ; ++k1) {
    for (int k0=0 ; k0<0x100 ; ++k0) {
      ByteGen::LfsrA lfsrA0 = ByteGen::LfsrA::make(k0,k1) ;
      ByteGen::LfsrA lfsrA3 = lfsrA0 ;
      bool carry3 = 0 ;
      uint32_t registerB = assemble(&lfsrA3,s,invertA,invertB,&carry3) ;
      ByteGen::LfsrB lfsrB3 = ByteGen::LfsrB::make(registerB) ;
      ByteGen::LfsrB lfsrB0 = lfsrB3 ;
      lfsrB0.unshift() ;
      lfsrB0.unshift() ;
      lfsrB0.unshift() ;
      if (lfsrB0.valid())
	v.push_back(ByteGen::make(lfsrA3,lfsrB3,invertA,invertB,carry3)) ;
      lfsrB3 = ByteGen::LfsrB::make(0x1000000|registerB) ;
      lfsrB0 = lfsrB3 ;
      lfsrB0.unshift() ;
      lfsrB0.unshift() ;
      lfsrB0.unshift() ;
      if (lfsrB0.valid())
	v.push_back(ByteGen::make(lfsrA3,lfsrB3,invertA,invertB,carry3)) ;
    }
  }
  return v ;
}

std::vector<size_t> Revert::match(Range<ByteGen> g,Range<uint8_t const> s)
{
  std::vector<size_t> v ; v.reserve(g.size()) ; 
  for (ByteGen *p=g.begin() ; p!=g.end() ; ++p) {
    size_t n = p->match(s) ;
    v.push_back(n) ;
  }
  return v ;
}

static std::vector<Revert::Pair> match(bool isHead,Range<uint8_t const> s,bool invertA,bool invertB)
{
  if (s.size() < 3)
    throw Error("not enough bytes") ;
  std::vector<ByteGen> g = isHead
    ? Revert::   head(s.sub<3>(0),invertA,invertB)
    : Revert::snippet(s.sub<3>(0),invertA,invertB) ;
  std::vector<size_t> n = Revert::match(&g,s.sub(3,s.size())) ;
  std::vector<Revert::Pair> r ; r.reserve(g.size()) ;
  for (size_t i=0 ; i<g.size() ; ++i) 
    r.push_back(Revert::Pair(g[i],n[i]+3)) ;
  return r ;
}

std::vector<Revert::Pair> Revert::snippetStream(Range<uint8_t const> s,bool invertA,bool invertB)
{
  return match(false,s,invertA,invertB) ;
}

std::vector<Revert::Pair> Revert::headStream(Range<uint8_t const> s,bool invertA,bool invertB)
{
  return match(true,s,invertA,invertB) ;
}

