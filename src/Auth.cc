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

#include "Auth.h"
#include "ByteGen.h"
#include "CArray.hpp"
#include "Format.h"
#include "Logging.h"

using namespace Auth ;

typedef Array<uint8_t,10> B10 ;

static void mix1(ByteGen *g,uint8_t v2,uint8_t(*text)[5])
{
  static uint8_t const xlat1[] = {
    0xc4,0xcd,0xce,0xcb,0xc8,0xc9,0xca,0xcf,0xcc,0xc5,0xc6,0xc3,0xc0,0xc1,0xc2,0xc7,
    0x14,0x1d,0x1e,0x1b,0x18,0x19,0x1a,0x1f,0x1c,0x15,0x16,0x13,0x10,0x11,0x12,0x17,
    0x24,0x2d,0x2e,0x2b,0x28,0x29,0x2a,0x2f,0x2c,0x25,0x26,0x23,0x20,0x21,0x22,0x27,
    0x34,0x3d,0x3e,0x3b,0x38,0x39,0x3a,0x3f,0x3c,0x35,0x36,0x33,0x30,0x31,0x32,0x37,
    0x04,0x0d,0x0e,0x0b,0x08,0x09,0x0a,0x0f,0x0c,0x05,0x06,0x03,0x00,0x01,0x02,0x07,
    0xd4,0xdd,0xde,0xdb,0xd8,0xd9,0xda,0xdf,0xdc,0xd5,0xd6,0xd3,0xd0,0xd1,0xd2,0xd7,
    0xe4,0xed,0xee,0xeb,0xe8,0xe9,0xea,0xef,0xec,0xe5,0xe6,0xe3,0xe0,0xe1,0xe2,0xe7,
    0xf4,0xfd,0xfe,0xfb,0xf8,0xf9,0xfa,0xff,0xfc,0xf5,0xf6,0xf3,0xf0,0xf1,0xf2,0xf7,
    0x44,0x4d,0x4e,0x4b,0x48,0x49,0x4a,0x4f,0x4c,0x45,0x46,0x43,0x40,0x41,0x42,0x47,
    0x94,0x9d,0x9e,0x9b,0x98,0x99,0x9a,0x9f,0x9c,0x95,0x96,0x93,0x90,0x91,0x92,0x97,
    0xa4,0xad,0xae,0xab,0xa8,0xa9,0xaa,0xaf,0xac,0xa5,0xa6,0xa3,0xa0,0xa1,0xa2,0xa7,
    0xb4,0xbd,0xbe,0xbb,0xb8,0xb9,0xba,0xbf,0xbc,0xb5,0xb6,0xb3,0xb0,0xb1,0xb2,0xb7,
    0x84,0x8d,0x8e,0x8b,0x88,0x89,0x8a,0x8f,0x8c,0x85,0x86,0x83,0x80,0x81,0x82,0x87,
    0x54,0x5d,0x5e,0x5b,0x58,0x59,0x5a,0x5f,0x5c,0x55,0x56,0x53,0x50,0x51,0x52,0x57,
    0x64,0x6d,0x6e,0x6b,0x68,0x69,0x6a,0x6f,0x6c,0x65,0x66,0x63,0x60,0x61,0x62,0x67,
    0x74,0x7d,0x7e,0x7b,0x78,0x79,0x7a,0x7f,0x7c,0x75,0x76,0x73,0x70,0x71,0x72,0x77
  } ;
  BOOST_STATIC_ASSERT(sizeof(xlat1) == 0x100) ;

  static uint8_t const xlat2[] = {
    0xc4,0x24,0x14,0x34,0xce,0x2e,0x1e,0x3e,0xcd,0x2d,0x1d,0x3d,0xcb,0x2b,0x1b,0x3b,
    0x44,0xa4,0x94,0xb4,0x4e,0xae,0x9e,0xbe,0x4d,0xad,0x9d,0xbd,0x4b,0xab,0x9b,0xbb,
    0x04,0xe4,0xd4,0xf4,0x0e,0xee,0xde,0xfe,0x0d,0xed,0xdd,0xfd,0x0b,0xeb,0xdb,0xfb,
    0x84,0x64,0x54,0x74,0x8e,0x6e,0x5e,0x7e,0x8d,0x6d,0x5d,0x7d,0x8b,0x6b,0x5b,0x7b,
    0xcc,0x2c,0x1c,0x3c,0xc6,0x26,0x16,0x36,0xc5,0x25,0x15,0x35,0xc3,0x23,0x13,0x33,
    0x4c,0xac,0x9c,0xbc,0x46,0xa6,0x96,0xb6,0x45,0xa5,0x95,0xb5,0x43,0xa3,0x93,0xb3,
    0x0c,0xec,0xdc,0xfc,0x06,0xe6,0xd6,0xf6,0x05,0xe5,0xd5,0xf5,0x03,0xe3,0xd3,0xf3,
    0x8c,0x6c,0x5c,0x7c,0x86,0x66,0x56,0x76,0x85,0x65,0x55,0x75,0x83,0x63,0x53,0x73,
    0xc8,0x28,0x18,0x38,0xca,0x2a,0x1a,0x3a,0xc9,0x29,0x19,0x39,0xcf,0x2f,0x1f,0x3f,
    0x48,0xa8,0x98,0xb8,0x4a,0xaa,0x9a,0xba,0x49,0xa9,0x99,0xb9,0x4f,0xaf,0x9f,0xbf,
    0x08,0xe8,0xd8,0xf8,0x0a,0xea,0xda,0xfa,0x09,0xe9,0xd9,0xf9,0x0f,0xef,0xdf,0xff,
    0x88,0x68,0x58,0x78,0x8a,0x6a,0x5a,0x7a,0x89,0x69,0x59,0x79,0x8f,0x6f,0x5f,0x7f,
    0xc0,0x20,0x10,0x30,0xc2,0x22,0x12,0x32,0xc1,0x21,0x11,0x31,0xc7,0x27,0x17,0x37,
    0x40,0xa0,0x90,0xb0,0x42,0xa2,0x92,0xb2,0x41,0xa1,0x91,0xb1,0x47,0xa7,0x97,0xb7,
    0x00,0xe0,0xd0,0xf0,0x02,0xe2,0xd2,0xf2,0x01,0xe1,0xd1,0xf1,0x07,0xe7,0xd7,0xf7,
    0x80,0x60,0x50,0x70,0x82,0x62,0x52,0x72,0x81,0x61,0x51,0x71,0x87,0x67,0x57,0x77
  } ;
  BOOST_STATIC_ASSERT(sizeof(xlat2) == 0x100) ;

  uint8_t last = 0 ;
  for (int i=0 ; i<5 ; ++i) {
    uint8_t code ;
    code  = (*text)[i] ;
    code ^= g->shift() ;
    code  = xlat1[code] ;
    code ^= v2 ;
    code  = xlat2[code] ;
    code ^= last ; 
    last = (*text)[i] ;
    (*text)[i] = code ;
  }
}

static void mix2(uint8_t(*text)[5])
{
  static uint8_t const xlat3[] = {
    0x00,0x81,0x03,0x82,0x06,0x87,0x05,0x84,0x0c,0x8d,0x0f,0x8e,0x0a,0x8b,0x09,0x88,
    0x18,0x99,0x1b,0x9a,0x1e,0x9f,0x1d,0x9c,0x14,0x95,0x17,0x96,0x12,0x93,0x11,0x90,
    0x30,0xb1,0x33,0xb2,0x36,0xb7,0x35,0xb4,0x3c,0xbd,0x3f,0xbe,0x3a,0xbb,0x39,0xb8,
    0x28,0xa9,0x2b,0xaa,0x2e,0xaf,0x2d,0xac,0x24,0xa5,0x27,0xa6,0x22,0xa3,0x21,0xa0,
    0x60,0xe1,0x63,0xe2,0x66,0xe7,0x65,0xe4,0x6c,0xed,0x6f,0xee,0x6a,0xeb,0x69,0xe8,
    0x78,0xf9,0x7b,0xfa,0x7e,0xff,0x7d,0xfc,0x74,0xf5,0x77,0xf6,0x72,0xf3,0x71,0xf0,
    0x50,0xd1,0x53,0xd2,0x56,0xd7,0x55,0xd4,0x5c,0xdd,0x5f,0xde,0x5a,0xdb,0x59,0xd8,
    0x48,0xc9,0x4b,0xca,0x4e,0xcf,0x4d,0xcc,0x44,0xc5,0x47,0xc6,0x42,0xc3,0x41,0xc0,
    0xc0,0x41,0xc3,0x42,0xc6,0x47,0xc5,0x44,0xcc,0x4d,0xcf,0x4e,0xca,0x4b,0xc9,0x48,
    0xd8,0x59,0xdb,0x5a,0xde,0x5f,0xdd,0x5c,0xd4,0x55,0xd7,0x56,0xd2,0x53,0xd1,0x50,
    0xf0,0x71,0xf3,0x72,0xf6,0x77,0xf5,0x74,0xfc,0x7d,0xff,0x7e,0xfa,0x7b,0xf9,0x78,
    0xe8,0x69,0xeb,0x6a,0xee,0x6f,0xed,0x6c,0xe4,0x65,0xe7,0x66,0xe2,0x63,0xe1,0x60,
    0xa0,0x21,0xa3,0x22,0xa6,0x27,0xa5,0x24,0xac,0x2d,0xaf,0x2e,0xaa,0x2b,0xa9,0x28,
    0xb8,0x39,0xbb,0x3a,0xbe,0x3f,0xbd,0x3c,0xb4,0x35,0xb7,0x36,0xb2,0x33,0xb1,0x30,
    0x90,0x11,0x93,0x12,0x96,0x17,0x95,0x14,0x9c,0x1d,0x9f,0x1e,0x9a,0x1b,0x99,0x18,
    0x88,0x09,0x8b,0x0a,0x8e,0x0f,0x8d,0x0c,0x84,0x05,0x87,0x06,0x82,0x03,0x81,0x00
  } ;
  BOOST_STATIC_ASSERT(sizeof(xlat3) == 0x100) ;

  for (int i=0 ; i<5 ; ++i) {
    (*text)[i] = xlat3[(*text)[i]] ; 
  }
}

static B5 crypt(ByteGen *g,uint8_t v2,uint8_t const(&plain)[5])
{
  B5 cipher(plain) ;
  mix1(g,v2,&cipher.carray()) ;                          cipher[0] ^= cipher[4] ;
  mix1(g,v2,&cipher.carray()) ;                          cipher[0] ^= cipher[4] ;
  mix1(g,v2,&cipher.carray()) ; mix2(&cipher.carray()) ; cipher[0] ^= cipher[4] ;
  mix1(g,v2,&cipher.carray()) ; mix2(&cipher.carray()) ; cipher[0] ^= cipher[4] ;
  mix1(g,v2,&cipher.carray()) ;                          cipher[0] ^= cipher[4] ;
  mix1(g,v2,&cipher.carray()) ;
  return cipher ;
}

static B5 eor(uint8_t const (&a)[5],uint8_t const (&b)[5])
{
  B5 y ; 
  for (size_t i=0 ; i<5 ; ++i) 
    y[i] = a[i] ^ b[i] ; 
  return y ; 
}

static B5 makeKey(uint8_t const (&nonce)[10],uint8_t v2)
{
  uint8_t const (&head)[5] = CArray::sub<0, 5>(nonce) ;
  uint8_t const (&tail)[5] = CArray::sub<5,10>(nonce) ;
  static uint8_t const salt[] = {0xf4,0x10,0x45,0xa3,0xe2} ;
  B5 seed = eor(head,salt) ;
  ByteGen g = ByteGen::make(seed,true,true) ;
  B5 cipher = crypt(&g,v2,tail) ;
  return cipher ;
}

static B10 permutation(uint8_t const(&data)[10],uint8_t const(&sequence)[10])
{
  B10 y ;
  for (int i=0 ; i<10 ; ++i) {
    y[i] = data[sequence[i]] ;
  }
  return y ;
}

B5 Auth::makeKey1(Variant variant,uint8_t const(&nonce)[10])
{
  static uint8_t const v[] = {   
    0x00,0x01,0x04,0x05,0x10,0x11,0x14,0x15,
    0x20,0x21,0x24,0x25,0x30,0x31,0x34,0x35,
    0x80,0x81,0x84,0x85,0x90,0x91,0x94,0x95,
    0xa0,0xa1,0xa4,0xa5,0xb0,0xb1,0xb4,0xb5 
  } ;
  BOOST_STATIC_ASSERT(sizeof(v)-1 == Variant::upper) ;
  uint8_t v2 = v[variant] ;
  static uint8_t const p[] = {1,5,3,0,7,4,2,9,6,8} ;
  B5 cipher = makeKey(permutation(nonce,p),v2) ;
  if (Logging::level() >= 2) {
    std::ostringstream os ;
    os << "variant:" << Format::hex(variant.value()) << ","
       << "nonce1:"  << Format::hex(nonce)   << ","
       << "key1:"    << Format::hex(cipher) ;
    Logging::out(__FILE__,__LINE__,os.str()) ;
  }
  return cipher ;
}

B5 Auth::makeKey2(Variant variant,uint8_t const(&nonce)[10])
{
  static uint8_t const v[] = { 
    0x24,0x20,0x34,0x30,0x25,0x21,0x35,0x31,
    0xa4,0xa0,0xb4,0xb0,0xa5,0xa1,0xb5,0xb1,
    0x04,0x00,0x14,0x10,0x05,0x01,0x15,0x11,
    0x84,0x80,0x94,0x90,0x85,0x81,0x95,0x91
  } ;
  BOOST_STATIC_ASSERT(sizeof(v)-1 == Variant::upper) ;
  uint8_t v2 = v[variant] ;
  static uint8_t const p[] = {7,9,5,2,4,1,6,0,8,3} ;
  B5 cipher = makeKey(permutation(nonce,p),v2) ;
  if (Logging::level() >= 2) {
    std::ostringstream os ;
    os << "variant:" << Format::hex(variant.value()) << "," 
       << "nonce2:"  << Format::hex(nonce)   << ","
       << "key2:"    << Format::hex(cipher) ;
    Logging::out(__FILE__,__LINE__,os.str()) ;
  }
  return cipher ;
}

B5 Auth::makeBusKey(Variant variant,uint8_t const(&key1)[5],uint8_t const(&key2)[5])
{
  static uint8_t const v[] = { 
    0x84,0xa4,0x94,0xb4,0x04,0x24,0x14,0x34,
    0x80,0xa0,0x90,0xb0,0x00,0x20,0x10,0x30,
    0x85,0xa5,0x95,0xb5,0x05,0x25,0x15,0x35,
    0x81,0xa1,0x91,0xb1,0x01,0x21,0x11,0x31
  } ;
  BOOST_STATIC_ASSERT(sizeof(v)-1 == Variant::upper) ;
  uint8_t v2 = v[variant] ;
  B10 nonce = cling(key1,key2) ;
  static uint8_t const p[] = {5,3,8,6,2,7,9,1,4,0} ;
  B5 cipher = makeKey(permutation(nonce,p),v2) ;
  if (Logging::level() >= 2) {
    std::ostringstream os ;
    os << "variant:" << Format::hex(variant.value()) << ","
       << "key1:"    << Format::hex(key1)    << ","
       << "key2:"    << Format::hex(key2)    << ","
       << "buskey:"  << Format::hex(cipher) ;
    Logging::out(__FILE__,__LINE__,os.str()) ;
  }
  return cipher ;
}

Variant Auth::getVariant(uint8_t const(&nonce)[10],uint8_t const(&key1)[5])
{
  for (typename Variant::Type variant=Variant::lower ; variant<=Variant::upper ; ++variant) {
    B5 cipher = makeKey1(variant,nonce) ;
    if (CArray::equal(key1,cipher.carray())) 
      return variant ;
  }
  std::ostringstream os ;
  os << "no variant for key1:" << Format::hex(key1) << ",nonce1:" << Format::hex(nonce) ;
  throw Error(os.str()) ;
}