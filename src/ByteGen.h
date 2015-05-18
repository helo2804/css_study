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

#ifndef _ByteGen_h_
#define _ByteGen_h_

#include "Array.hpp"
#include "Range.hpp"
#include <stdexcept>
#include <stdint.h>

struct ByteGen 
{
  static inline uint8_t reverse(uint8_t b) 
  {
    b = (b & 0xf0) >> 4 | (b & 0x0f) << 4 ;
    b = (b & 0xcc) >> 2 | (b & 0x33) << 2 ;
    b = (b & 0xaa) >> 1 | (b & 0x55) << 1 ;
    return b ;
    // note, a bit faster is:
    // (((0x80200802ul * b) & 0x0884422110ul) * 0x0101010101ul) >> 32 
    // see http://graphics.stanford.edu/~seander/bithacks.html
  }

  struct LfsrA
  {
    static LfsrA make(uint32_t r) 
    { 
      if (r > 0x1ffffu)
	throw std::domain_error("LfsrA:register out of range") ;
      return LfsrA(r) ; 
    }

    uint32_t value() const { return 0x1ffffu & this->r ; }

    bool operator== (LfsrA const &lfsr) const { return this->value() == lfsr.value() ; }

    bool operator!= (LfsrA const &lfsr) const { return ! ((*this) == lfsr) ; }

    static LfsrA make(uint8_t k0,uint8_t k1) 
    { 
      uint32_t r = k1 ;
      r <<= 9 ;
      r |= 0x100 | k0 ;
      return LfsrA(r) ;
    }

    uint8_t k0() const { return this->r      ; }
    uint8_t k1() const { return this->r >> 9 ; }

    bool valid() const { return (this->r & 0x100) == 0x100 ; }

    uint8_t shift() 
    {
      uint32_t s = this->r >> 9 ;
      s ^= this->r << 5 ;
      s ^= (s >> 3) & 0x1c ;
      s ^= (s >> 3) & 0x03 ;
      this->r <<= 8 ;
      this->r |= 0xff & s ;
      return reverse(this->r) ;
    }

    void shift(Range<uint8_t> s)
    {
      for (uint8_t *p=s.begin() ; p!=s.end() ; ++p)
	(*p) = this->shift() ;
    }

    uint8_t unshift()
    {
      uint8_t b = reverse(this->r) ;
      uint32_t s = (this->r << 9) ^ (this->r << 6) ;
      this->r >>= 8 ;
      this->r &=   0x1ff ;
      this->r |= 0x1fe00 & s ;
      return b ;
    }

  private:

    LfsrA(uint32_t r) : r(r) {}

    uint32_t r ; 
  } ;

  struct LfsrB
  {
    static LfsrB make(uint32_t r) 
    { 
      if (r > 0x1ffffffu)
	throw std::domain_error("LfsrB:register out of range") ;
      return LfsrB(r) ; 
    }

    uint32_t value() const { return 0x1ffffffu & this->r ; }

    bool operator== (LfsrB const &lfsr) const { return this->value() == lfsr.value() ; }

    bool operator!= (LfsrB const &lfsr) const { return ! ((*this) == lfsr) ; }

    static LfsrB make(uint8_t k2,uint8_t k3,uint8_t k4) 
    {
      uint32_t r = k4 ;
      r <<= 8 ;
      r |= k3 ;
      r <<= 8 ;
      r |= k2 ;
      r <<= 1 ;
      r += 8 - ( k2 & 7 ) ;
      return LfsrB(r) ;
    }

    uint8_t k2() const { return ((this->r >>  1) & ~0x7) | (this->r & 0x7) ; }
    uint8_t k3() const { return   this->r >>  9                            ; }
    uint8_t k4() const { return   this->r >> 17                            ; }

    bool valid() const { return (this->r & 0x8) == 0x8 ; }

    uint8_t shift()
    {
      uint32_t s ;
      s  = this->r >> 17 ;
      s ^= this->r >> 14 ;
      s ^= this->r >> 13 ;
      s ^= this->r >>  5 ;
      this->r <<= 8 ;
      this->r |= 0xff & s ;
      return reverse(this->r) ;
    }

    void shift(Range<uint8_t> s)
    {
      for (uint8_t *p=s.begin() ; p!=s.end() ; ++p)
	(*p) = this->shift() ;
    }

    uint8_t unshift()
    {
      uint8_t b = reverse(this->r) ;
      uint32_t s ;
      s  = (this->r >> 22) & 0x07 ; // 22:0000 0aaa
      s ^= (this->r >> 21) & 0x0f ; // 21:0000 bbbb
      s ^= (this->r >> 13)        ; // 13:cccc cccc
      s ^=  this->r               ; //  0:dddd dddd
      s ^= (s << 3) & 0x38 ;        // 22:00aa a...
      s ^= (s << 4) & 0x70 ;        // 21:0bbb ....
      s ^= (s << 3) & 0xc0 ;        // 22:aa.. ....
      s ^= (s << 4) & 0x80 ;        // 21:b... ....
      this->r >>= 8 ;
      this->r &= ~0x1fe0000 ;
      this->r |=  0x1fe0000 & (s << 17) ;
      return b ;
    }

  private:

    LfsrB(uint32_t r) : r(r) {}

    uint32_t r ; 
  } ;

  static ByteGen make(LfsrA  lfsrA,    
		      LfsrB  lfsrB,    
		      bool invertA, 
		      bool invertB, 
		      bool   carry)
  {
    return ByteGen(lfsrA,lfsrB,invertA?~0:0,invertB?~0:0,carry) ;
  }

  static ByteGen make(uint8_t const (&k)[5],
		      bool          invertA,
		      bool          invertB)
  {
    LfsrA lfsrA = LfsrA::make(k[0],k[1]) ;
    LfsrB lfsrB = LfsrB::make(k[2],k[3],k[4]) ;
    return make(lfsrA,lfsrB,invertA,invertB,false) ;
  }

  bool  getInvertA() const { return this->invertA ; }
  bool  getInvertB() const { return this->invertB ; }
  bool  getCarry()   const { return this->carry   ; }

  bool operator== (ByteGen const &g) const
  {
    if (this->lfsrA != g.lfsrA) return false ;
    if (this->lfsrB != g.lfsrB) return false ;
    return this->carry == g.carry ;
  }

  bool operator!= (ByteGen const &g) const { return ! (this->operator==(g)) ; }

  void setCarry(bool carry=true) { this->carry = carry ; }

  void k(uint8_t (*v)[5]) const 
  {  
    (*v)[0] = this->lfsrA.k0() ;
    (*v)[1] = this->lfsrA.k1() ;
    (*v)[2] = this->lfsrB.k2() ;
    (*v)[3] = this->lfsrB.k3() ;
    (*v)[4] = this->lfsrB.k4() ;
  }

  Array<uint8_t,5> k() const { Array<uint8_t,5> a ; this->k(&a.carray()) ; return a ; } ;

  bool valid() const 
  {
    return 
      (this->carry == 0)  && 
      this->lfsrA.valid() && 
      this->lfsrB.valid() ;
  }

  uint8_t shift() 
  {
    this->carry += this->invertA ^ this->lfsrA.shift() ;
    this->carry += this->invertB ^ this->lfsrB.shift() ;
    uint8_t s = this->carry ;
    this->carry >>= 8 ;
    return s ;
  }

  void shift(Range<uint8_t> s)
  {
    for (uint8_t *p=s.begin() ; p!=s.end() ; ++p)
      (*p) = this->shift() ;
  }
  
  void unshift(uint8_t s)
  {
    this->carry <<= 8 ;
    this->carry += s ;
    this->carry -= this->invertA ^ this->lfsrA.unshift() ;
    this->carry -= this->invertB ^ this->lfsrB.unshift() ;
    this->carry &= 1 ;
  }

  void unshift(Range<uint8_t const> s)
  {
    for (uint8_t const *p=s.begin() ; p!=s.end() ; ++p)
      this->unshift(*p) ;
  }

  void unshift(size_t n=1)
  {
    for (size_t i=0 ; i<n ; ++i) {
      this->lfsrA.unshift() ;
      this->lfsrB.unshift() ;
    }
    this->carry = 0 ;
  }

  size_t match(Range<uint8_t const> s)
  {
    // shift the generator by the number of matching bytes which is returned
    size_t i = 0 ;
    while (true) {
      if (i >= s.size())
	break ;
      ByteGen gx = (*this) ;
      uint8_t b = gx.shift() ;
      if (s[i] != b)
	break ;
      ++i ;
      if (i >= s.size()) {
	(*this) = gx ;
	break ;
      }
      (*this) = gx ;
      b = this->shift() ;
      if (s[i] != b) {
	(*this) = gx ;
	break ;
      }
      ++i ;
    }
    return i ;
  }

  LfsrA lfsrA ;
  LfsrB lfsrB ;

private:

  uint32_t carry ; // 0 or 1, however at least 9 bits required for additions

  uint8_t invertA ; // 0xff or 0x00
  uint8_t invertB ; // 0xff or 0x00

  ByteGen(LfsrA     lfsrA,    
	  LfsrB     lfsrB,    
	  uint8_t invertA, 
	  uint8_t invertB, 
	  uint32_t  carry)
    : lfsrA(lfsrA),lfsrB(lfsrB),carry(carry),invertA(invertA),invertB(invertB) {}

} ;

#endif // _ByteGen_h_
