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

#include "Css.h"
#include "Decrypt.h"
#include "ByteGen.h"

using namespace Decrypt ;

B5 Decrypt::key(uint8_t const (&s)[5],uint8_t const (&c)[5])
{
  B5 t ;
  t[4] = s[4] ^ Css::table[c[4]] ^ c[3] ; // (eqn 1)
  t[3] = s[3] ^ Css::table[c[3]] ^ c[2] ; // (eqn 2)
  t[2] = s[2] ^ Css::table[c[2]] ^ c[1] ; // (eqn 3)
  t[1] = s[1] ^ Css::table[c[1]] ^ c[0] ; // (eqn 4)
  t[0] = s[0] ^ Css::table[c[0]] ^ t[4] ; // (eqn 5)

  B5 p ;
  p[4] = s[4] ^ Css::table[t[4]] ^ t[3] ; // (eqn 6)
  p[3] = s[3] ^ Css::table[t[3]] ^ t[2] ; // (eqn 7)
  p[2] = s[2] ^ Css::table[t[2]] ^ t[1] ; // (eqn 8)
  p[1] = s[1] ^ Css::table[t[1]] ^ t[0] ; // (eqn 9)
  p[0] = s[0] ^ Css::table[t[0]]        ; // (eqn 10)

  return p ;
}

B5 Decrypt::discKey(uint8_t const (&playerKey)[5],uint8_t const (&c)[5])
{
  ByteGen g = ByteGen::make(playerKey,false,false) ;
  uint8_t s[5] ; g.shift(&s) ;
  return key(s,c) ;
}

boost::optional<B5> Decrypt::discKey(uint8_t const (&playerKey)[5],uint8_t const (&c)[5],uint8_t const (&hash)[5])
{
  B5 key = discKey(playerKey,c) ;
  B5 self = discKey(key,hash) ;
  return (key == self) ? key : boost::optional<B5>() ;
}

B5 Decrypt::titleKey(uint8_t const (&discKey)[5],uint8_t const (&c)[5])
{
  ByteGen g = ByteGen::make(discKey,false,true) ;
  uint8_t s[5] ; g.shift(&s) ;
  return key(s,c) ;
}

void Decrypt::feature(uint8_t const (&seed)[5],Range<uint8_t const> c,Range<uint8_t> p)
{
  if (c.size() != p.size())
    throw Error("mismatched cipher-/plain-text size") ;
  ByteGen g = ByteGen::make(seed,true,false) ;
  for (uint32_t i=0 ; i<c.size() ; ++i) {
    p[i] = Css::table[c[i]] ^ g.shift() ;
  }
}

