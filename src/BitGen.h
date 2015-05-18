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

#ifndef _BitGen_h_
#define _BitGen_h_

#include "Bitset.hpp"
#include <boost/dynamic_bitset.hpp>

struct BitGen
{
  struct LfsrA
  {
    typedef Bitset<17> Register ;

    typedef Bitset<16> Seed ;

    static LfsrA make(Seed k)
    {
      return LfsrA(k.sub<0,8>() + Bitset<1>(1) + k.sub<8,16>()) ;
    }

    Seed k() const
    {
      return this->r.sub<0,8>() + this->r.sub<9,17>() ;
    }

    bool valid() const { return this->r[8] ; }

    bool shift()
    {
      bool s = this->r[2] ^ this->r[16] ;
      this->r.shr(s) ;
      return s ;
    }

    bool unshift()
    {
      bool s = this->r[0] ;
      this->r.shl(s^this->r[3]) ;
      return s ;
    }

    LfsrA(Register const &r) : r(r) {}

    Register r ;
  } ;

  struct LfsrB
  {
    typedef Bitset<25> Register ;

    typedef Bitset<24> Seed ;

    static LfsrB make(Seed k)
    {
      return LfsrB(k.sub<0,3>() + Bitset<1>(1) + k.sub<3,24>()) ;
    }

    Seed k() const
    {
      return this->r.sub<0,3>() + this->r.sub<4,25>() ;
    }

    bool valid() const { return this->r[3] ; }

    bool shift()
    {
      bool s = this->r[12] ^ this->r[20] ^ this->r[21] ^ this->r[24] ;
      this->r.shr(s) ;
      return s ;
    }

    bool unshift()
    {
      bool s = this->r[0] ;
      this->r.shl(s^this->r[13]^this->r[21]^this->r[22]) ;
      return s ;
    }

    LfsrB(Register const &r) : r(r) {}

    Register r ;
  } ;

  typedef Bitset<40> Seed ;

  static BitGen make(Seed        k,
		     bool  invertA,
		     bool  invertB,
		     bool    carry) 
  {
    LfsrA lfsrA = LfsrA::make(k.sub< 0,16>()) ;
    LfsrB lfsrB = LfsrB::make(k.sub<16,40>()) ;
    return BitGen(lfsrA,lfsrB,invertA,invertB,carry) ;
  }

  bool valid() const
  {
    return 
      this->carry == false && 
      this->lfsrA.valid()  && 
      this->lfsrB.valid() ;
  }

  Seed k() const { return this->lfsrA.k() + this->lfsrB.k() ; }

  bool shift()
  {
    unsigned sum = this->carry ;
    sum += this->lfsrA.shift() ^ this->invertA ; 
    sum += this->lfsrB.shift() ^ this->invertB ; 
    this->carry = static_cast<bool>(sum & 2) ;
    return sum & 1 ;
  }

  void unshift(bool s)
  {
    int sum = this->carry ;
    sum <<= 1 ;
    sum += s ;
    sum -= this->lfsrA.unshift() ^ this->invertA ;
    sum -= this->lfsrB.unshift() ^ this->invertB ;
    this->carry = 1 & sum ;
  }

  boost::dynamic_bitset<> shift(size_t n)
  {
    boost::dynamic_bitset<> s(n) ;
    for (size_t i=0 ; i<n ; ++i) {
      s[i] = this->shift() ;
    }
    return s ;
  }

  void unshift(boost::dynamic_bitset<> const &s)
  {
    size_t i = s.size() ;
    while (i > 0)
      this->unshift(s[--i]) ;
  }
  
  BitGen(LfsrA  lfsrA,    
	 LfsrB  lfsrB,    
	 bool invertA, 
	 bool invertB, 
	 bool   carry) 
  : lfsrA(lfsrA),lfsrB(lfsrB),invertA(invertA),invertB(invertB),carry(carry) {}
  
  LfsrA lfsrA ;
  LfsrB lfsrB ;

  bool invertA ; 
  bool invertB ; 

  bool carry ;

} ;

#endif // _BitGen_h_
