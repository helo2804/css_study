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

#ifndef _Bitset_hpp_
#define _Bitset_hpp_

#include <bitset>
#include <stdexcept>
#include <stdint.h>
#include <boost/static_assert.hpp>

template <size_t N> struct Bitset
{
  Bitset(unsigned long i=0) : v(i) 
  {
    if (this->v.to_ulong() != i) 
      throw std::range_error("Bitset:c'tor exceeds number of bits") ;
    // note that std::bitset ignores leading digits
  }

  // from byte-array; lsb first
  Bitset(uint8_t const(&p)[(N+7)/8])
  {
    for (size_t i=0 ; i<N ; ++i) {
      this->set(i,1&(p[i/8]>>(i%8))) ;
    }
  }

  bool operator== (Bitset const &x) const { return this->v == x.v ; }
  bool operator!= (Bitset const &x) const { return this->v != x.v ; }

  // shift operations are reversed (compared with std::bitset)
  bool shr(bool in) { bool out = this->v[N-1] ; this->v <<= 1 ; this->v[  0] = in ; return out ; }
  bool shl(bool in) { bool out = this->v[  0] ; this->v >>= 1 ; this->v[N-1] = in ; return out ; }

  Bitset reverse() const
  {
    Bitset y(this->v) ;
    for (size_t i=0 ; i<N/2 ; ++i) {
      bool b     = y.v[i] ;
      y.v[i]     = y.v[N-1-i] ;
      y.v[N-1-i] = b ;
    }
    return y ;
  }

  template <size_t M> Bitset<N+M> operator+ (Bitset<M> const &x) const
  {
    Bitset<N+M> y ;
    for (size_t i=0 ; i<N ; ++i)
      y.set(i,(*this)[i]) ;
    for (size_t i=0 ; i<M ; ++i)
      y.set(N+i,x[i]) ;
    return y ;
  }

  // return [POS1,POS2) 
  template <size_t POS1,size_t POS2> Bitset<POS2-POS1> sub() const
  {
    BOOST_STATIC_ASSERT(POS1 <= POS2) ;
    BOOST_STATIC_ASSERT(POS2 <=    N) ;
    Bitset<POS2-POS1> y ;
    for (size_t i=0 ; i<POS2-POS1 ; ++i)
      y.set(i,(*this)[POS1+i]) ;
    return y ;
  }

  bool operator[] (size_t pos) const { return this->v[pos] ; }

  Bitset& set(size_t pos,bool b) { this->v.set(pos,b) ; return (*this) ; }

  // throws if unsigned long cannot cover the value
  unsigned long to_ulong() const { return this->v.to_ulong() ; }

  // to byte-array; lsb first
  void to(uint8_t(*p)[(N+7)/8]) const
  {
    for (size_t i=0 ; i<(N+7)/8 ; ++i) {
      (*p)[i] = 0 ;
    }
    for (size_t i=0 ; i<N ; ++i) {
      (*p)[i/8] |= (*this)[i]<<(i%8) ;
    }
  }

  friend std::ostream& operator<< (std::ostream &os,Bitset const &x) 
  { 
    // sequence of bits, lsb first
    return os << x.reverse().v ; 
    // note that std::bitset prints a binary number, msb first
  }

private:

  Bitset(std::bitset<N> const &v) : v(v) {}

  std::bitset<N> v ;
} ;

#endif // _Bitset_hpp_
