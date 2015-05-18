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

#include "CArray.hpp"
#include "Css.h"
#include "Pack.h"
#include "Predict.h"
#include "Recover.h"
#include "Revert.h"

typedef Recover::B5 B5 ;

// ----[ player-key ]--------------------------------------------------

// s4 x c x p -> (s0,...,s4)
//   s: keystream
//   c: cipher-text
//   p: plain-text
static boost::optional<B5> key(uint8_t s4,uint8_t const (&c)[5],uint8_t const (&p)[5])
{
  B5 s,t ; s[4] = s4 ; uint8_t p0 ;
  t[4] = s[4] ^ Css::table[c[4]] ^ c[3] ; // (eqn 1)
  t[3] = s[4] ^ Css::table[t[4]] ^ p[4] ; // (eqn 6)
  s[3] = t[3] ^ Css::table[c[3]] ^ c[2] ; // (eqn 2)
  t[2] = s[3] ^ Css::table[t[3]] ^ p[3] ; // (eqn 7)
  s[2] = t[2] ^ Css::table[c[2]] ^ c[1] ; // (eqn 3)
  t[1] = s[2] ^ Css::table[t[2]] ^ p[2] ; // (eqn 8)
  s[1] = t[1] ^ Css::table[c[1]] ^ c[0] ; // (eqn 4)
  t[0] = s[1] ^ Css::table[t[1]] ^ p[1] ; // (eqn 9)
  s[0] = t[0] ^ Css::table[c[0]] ^ t[4] ; // (eqn 5)
  p0   = s[0] ^ Css::table[t[0]]        ; // (eqn 10)
  return (p0 == p[0]) ? s : boost::optional<B5>() ;
}

std::vector<B5> Recover::playerKey(uint8_t const (&discKey)[5],uint8_t const (&cipher)[5])
{
  std::vector<B5> result ;
  for (unsigned s4=0 ; s4<0x100 ; ++s4) {
    boost::optional<B5> o = key(s4,cipher,discKey) ;
    if (o) {
      result.push_back(*o) ;
    }
  }
  return result ;
}

std::vector<B5> Recover::playerKey(uint8_t const (&s)[5])
{
  std::vector<B5> result ;
  std::vector<Revert::Pair> v = Revert::headStream(s,false,false) ;
  for (size_t i=0 ; i<v.size() ; ++i) {
    if (v[i].n == 5) {
      ByteGen g0 = v[i].g ;
      g0.unshift(s) ;
      result.push_back(g0.k()) ;
    }
  }
  return result ;
}

// ----[ disc-key ]----------------------------------------------------

struct S014 
{
  // LFSR-B: (s0,s1,s4) <-> (k2,k3,k4)
  std::vector<uint32_t> lup ; 
  // 24 bits required for operation
  // +1 bit if check is on (assert)
  static inline size_t ofs(uint8_t const (&s)[5])
  {
    size_t ofs = s[4] ;
    ofs <<= 8 ;
    ofs += s[1] ;
    ofs <<= 8 ;
    ofs += s[0] ;
    return ofs ;
  }
  S014() : lup(0x1000000,0) 
  { 
    uint32_t k234 = 0 ;
    do {
      ByteGen::LfsrB lfsr = ByteGen::LfsrB::make(k234,k234>>8,k234>>16) ;
      B5 s ; lfsr.shift(&s) ;
      uint32_t &r = this->lup[ofs(s)] ;
      // assert(r == 0) ;
      r = 0x1000000 + k234 ;
    }
    while (++k234 != 0x1000000) ;
  }
  inline void complete(uint8_t const (&s)[5],uint8_t (*k)[5]) const
  {
    uint32_t const &k234 = this->lup[ofs(s)] ;
    // assert(k234 != 0) ;
    (*k)[2] = (k234      ) ;
    (*k)[3] = (k234 >>  8) ;
    (*k)[4] = (k234 >> 16) ;
  } 
} ;

static bool match(uint8_t const (&c)[5],uint8_t const (&p)[5],uint8_t const (&s)[5],uint8_t const (&t)[5])
{
  // note: this function is 2^25 times invoked to check for a matching 
  // disc-key. instead of verifying the disc-key p with the cipher c 
  // immediately, we solve the remaining equations first...
  uint8_t t3 = Css::table [ t[4] ] ^ s[4] ^ p[4] ; // (eqn 6)
  uint8_t s3 = Css::table [ c[3] ] ^ c[2] ^  t3  ; // (eqn 2)
  uint8_t t2 = Css::table [  t3  ] ^  s3  ^ p[3] ; // (eqn 7)
  uint8_t s2 = Css::table [ c[2] ] ^ c[1] ^  t2  ; // (eqn 3)
  uint8_t p2 = Css::table [  t2  ] ^ t[1] ^  s2  ; // (eqn 8)
  // ...the next (p2 == p[2]) check reduces the odds to have a matching
  // disc-keyby about 2^8 which saves some CPU time for the expensive
  // disc-key decryption.
  return (p2 == p[2]) && (CArray::equal(p,Decrypt::discKey(p,c).carray())) ;
  // note: you might think verification of the key might go quicker. 
  // Especially by using the already calculated key-stream sA. However,
  // actually, it doesn't make a difference. so we keep it simple: 
  // decrypt the key with the default operation and verify the result.
}

// may return zero, one or multiple disc-keys
// e.g. 
//   00:00:00:00:02 -> no disc key
//   10:00:00:00:00 -> 3a:e2:32:c5:f9 ad:15:67:91:17 2f:95:9b:94:1d
std::vector<B5> Recover::discKey(uint8_t const (&c)[5])
{
  std::vector<B5> result ;
  // iterate thru all (t0,t1,p0) variants
  uint8_t t[5] ; 
  t[0] = 0x00 ; do {
    uint8_t p[5] ; 
    p[0] = 0x00 ; do {
      uint8_t s[5] ; 
      s[0] = Css::table [ t[0] ] ^ p[0]        ; // (eqn 10)
      t[4] = Css::table [ c[0] ] ^ t[0] ^ s[0] ; // (eqn 5)
      s[4] = Css::table [ c[4] ] ^ t[4] ^ c[3] ; // (eqn 1) 
      t[1] = 0x00 ; do {
	s[1] = c[0] ^ Css::table [c[1]] ^ t[1] ; // (eqn 4)
	p[1] = s[1] ^ Css::table [t[1]] ^ t[0] ; // (eqn 9)
	// LFSR-A key-stream
	ByteGen::LfsrA lfsrA = ByteGen::LfsrA::make(p[0],p[1]) ;
	uint8_t sA[5] ; lfsrA.shift(&sA) ;
	// get values at index 0,1 & 4 of LFSR-B key-stream
	uint8_t sB[5] ; 
	sB[0] = s[0] - sA[0] ;
	sB[1] = s[1] - sA[1] ;
	if (s[0] < sA[0]) 
	  --sB[1] ;
	sB[4] = s[4] - sA[4] ;
	// check for a match...
	static S014 const s014 ;
	s014.complete(sB,&p) ;
	if (match(c,p,s,t)) {
	  result.push_back(p) ;
	}
	// ...we have to try a 2nd time (with carry-over from s3 to s4)
	--sB[4] ;
	s014.complete(sB,&p) ;
	if (match(c,p,s,t)) {
	  result.push_back(p) ;
	}
      }
      while (++t[1] != 0x00) ;
    }
    while (++p[0] != 0x00) ;
  }
  while (++t[0] != 0x00) ;
  return result ;
}

// ----[ title-key ]---------------------------------------------------

typedef Array<uint8_t,10> B10 ;

void Recover::Status::Counter::increment(Status const &status)
{
  switch (status.e) {
  case Status::notScrambled : ++this->notScrambled ; break ;
  case Status::noPattern    : ++this->noPattern    ; break ;
  case Status::tooSmall     : ++this->tooSmall     ; break ;
  case Status::noPrediction : ++this->noPrediction ; break ;
  case Status::notDecrypted : ++this->notDecrypted ; break ;
  } 
}

static B10 recoverKeyStream(uint8_t const (&data)[0x800],size_t wsize)
{
  Range<uint8_t const> word(data,0x80-wsize,0x80) ;
  Range<uint8_t const> cipher(data,0x80,0x80+10) ;
  B10 kstream ;
  for (unsigned i=0 ; i<10 ; ++i) {
    kstream[i] = Css::table[cipher[i]] ^ word[i%wsize] ;
  }
  return kstream ;
}

static boost::optional<ByteGen> decipherHead(uint8_t const (&keyStream)[10])
{
  std::vector<Revert::Pair> v = Revert::headStream(keyStream,true,false) ;
  size_t best = 0 ;
  for (size_t i=1 ; i<v.size() ; ++i) {
    if (v[i].n > v[best].n)
      best = i ;
  }
  if (v[best].n != 10) {
    return boost::optional<ByteGen>() ;
    // matches with 10 bytes are always unique and appear to be always 
    // positives. matches below 10 bytes are sometimes wrong positives, 
    // especially with only 5 or 6 matching bytes. an implementation to
    // sort out the wrong postives doesn't pay off at the moment.
  }
  ByteGen g0 = v[best].g ;
  g0.unshift(keyStream) ;
  return g0 ;
}

static boost::optional<ByteGen> decipherSnippet(Range<uint8_t const> keyStream)
{
  std::vector<Revert::Pair> v = Revert::snippetStream(keyStream,true,false) ;
  size_t best = 0 ;
  for (size_t i=1 ; i<v.size() ; ++i) {
    if (v[i].n > v[best].n)
      best = i ;
  }
  if (v[best].n != keyStream.size()) {
    return boost::optional<ByteGen>() ;
  }
  return v[best].g ;
}

static B5 recoverKey(uint8_t const (&data)[0x800],ByteGen const &g)
{
  B5 key = g.k() ;
  Range<uint8_t const> salt(data,0x54,0x54+5) ;
  for (unsigned i=0 ; i<5 ; ++i) {
    key[i] ^= salt[i] ;
  }
  return key ;
}

boost::variant<Recover::Status,B5> Recover::titleByCycle(uint8_t const (&data)[0x800],size_t minMatch)
{
  Pack pack = Pack::parse(data) ;
  if (!pack.pes1.scrambled()) {
    return Status(Status::notScrambled) ;
  }
  Pack::PES const *pes = pack.at0x80() ;
  if (pes == NULL) {
    return Status(Status::noPattern) ; // packet border or something like that
  }
  if (pes->payload+pes->size < 0x80 + 10) {
    return Status(Status::tooSmall) ;
  }
  Range<uint8_t const> plain(data,pes->payload,0x80) ;
  Predict::Cycle::Vector v = Predict::findCycle(plain,minMatch) ;
  if (v.empty()) {
    return Status(Status::noPrediction) ;
  }
  for (size_t i=0 ; i<v.size() ; ++i) {
    B10 keyStream = recoverKeyStream(data,v[i].wsize) ;
    boost::optional<ByteGen> g = decipherHead(keyStream) ;
    if (g) 
      return recoverKey(data,*g) ;
  }
  return Status(Status::notDecrypted) ;
}

boost::variant<Recover::Status,B5> Recover::titleByPadding(uint8_t const (&data)[0x800])
{
  Pack pack = Pack::parse(data) ;
  if (!pack.pes1.scrambled()) {
    return Status(Status::notScrambled) ;
  }
  uint16_t offset = pack.pes1.payload + pack.pes1.size ;
  if (offset == 0x800) {
    return Status(Status::noPattern) ;
  }
  std::vector<uint8_t> padding = Pack::PES::makePadding(offset) ;
  size_t skip,len ;
  if (offset <= 0x80) {
    len = 10 ;
    skip = 0x80 - offset ;
    // decipher head
  }
  else {
    len = 11 ;
    if (offset+len > 0x800) {
      return Status(Status::tooSmall) ;
    }
    skip = 0 ;
    // decipher snippet
  }
  Range<uint8_t const> plain(padding,skip,skip+len) ;
  Range<uint8_t const> cipher(data,offset+skip,offset+skip+len) ;
  std::vector<uint8_t> keyStream(plain.begin(),plain.end()) ;
  for (size_t i=0 ; i<keyStream.size() ; ++i) {
    keyStream[i] ^= Css::table[cipher[i]] ;
  }
  boost::optional<ByteGen> g = decipherSnippet(keyStream) ;
  if (!g) {
    return Status(Status::notDecrypted) ;
  }
  g->unshift(offset+skip-0x80+len) ;
  ByteGen gx = (*g) ;
  std::vector<uint8_t> x(offset+skip-0x80+len) ; gx.shift(&x) ;
  Range<uint8_t const> y(x,offset+skip,offset+skip+len) ;
  if (y != Range<uint8_t const>(keyStream)) {
    return Status(Status::notDecrypted) ;
    // note: less likely; todo: a scenario where this may happen
  }
  return recoverKey(data,*g) ;
}
