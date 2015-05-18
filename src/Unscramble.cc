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

#include "Handshake.h"
#include "Os.h"
#include "Unscramble.h"
#include <deque>
#include <set>

using namespace Unscramble ;

std::vector<B5> Unscramble::readKeys(std::string const &fname)
{
  std::vector<B5> vector ;
  std::ifstream is ; Os::open(is,fname) ;
  std::string line ;
  std::getline(is,line) ;
  while (is) {
    std::istringstream iline(line) ;
    std::string keyStr ;
    iline >> keyStr ;
    B5 key = Format::byteA<5>(keyStr) ;
    vector.push_back(key) ;
    std::getline(is,line) ;
  }
  return vector ;
}

boost::optional<B5> Unscramble::discKey(uint8_t const (&playerKey)[5],uint8_t const (&discKeyBlock)[409][5])
{
  uint8_t const (&hash)[5] = discKeyBlock[0] ; 
  for (size_t i=1 ; i<409 ; ++i) {
    uint8_t const (&cipher)[5] = discKeyBlock[i] ;
    boost::optional<B5> key = Decrypt::discKey(playerKey,cipher,hash) ;
    if (key) {
      return (*key) ;
    }
  }
  return boost::optional<B5>() ;
}

boost::optional<B5> Unscramble::discKey(Range<B5 const> playerKeyR,uint8_t const (&discKeyBlock)[409][5])
{
  for (size_t i=0 ; i<playerKeyR.size() ; ++i) {
    B5 const &playerKey = playerKeyR[i] ;
    boost::optional<B5> key = discKey(playerKey.carray(),discKeyBlock) ;
    if (key) {
      return (*key) ;
    }
  }
  return boost::optional<B5>() ;
}

TitleKeyMap Unscramble::TitleKeyMap::readFile(std::string const &fname)
{
  std::ifstream is ; Os::open(is,fname) ;
  base_type result ;
  std::string line ;
  std::getline(is,line) ;
  while (is) {
    std::istringstream iline(line) ;
    std::string noStr ;
    std::string keyStr ;
    iline >> noStr >> keyStr ;
    VNo no = Format::number<VNo::Type>(noStr) ;
    B5 key = Format::byteA<5>(keyStr) ;
    result.insert(std::make_pair(no,key)) ;
    std::getline(is,line) ;
  }
  return result ;
}

TitleKeyMap Unscramble::TitleKeyMap::readDisc(std::string const &device,unsigned int timeout,uint8_t const (&nonce)[10])
{
  Os::Fd fd = Os::Fd::openRo(device) ;
  Feature::V volume(device) ; 
  Feature f = Feature::read(&volume) ;
  base_type result ;
  for (Feature::VNo::Type no=Feature::VNo::lower ; no<=Feature::VNo::upper ; ++no) {
    std::vector<uint32_t> lbaV = f.tellVob(Feature::VNo(no)) ;
    if (lbaV.empty())
      continue ;
    boost::optional<Mmc::Title> title = Handshake::title(fd,nonce,timeout,lbaV.front()) ;
    if (!title)
      continue ;
    result.insert(std::make_pair(no,title->key)) ;
  }
  return result ;
}

boost::optional<B5> Unscramble::TitleKeyMap::find(VNo no) const
{
  base_type::const_iterator i = this->base_type::find(no) ;
  return (i == this->end())
    ? boost::optional<B5>()
    : i->second ;
}

TitleKeyMap Unscramble::TitleKeyMap::decrypt(uint8_t const (&discKey)[5]) const
{
  base_type map = (*this) ;
  for (base_type::iterator i=map.begin() ; i!=map.end() ; ++i) {
    B5 &cipher = i->second ;
    static uint8_t const none[] = { 0,0,0,0,0 } ;
    if (cipher != none)
      cipher = Decrypt::titleKey(discKey,cipher) ;
  }
  return map ;
}

bool Unscramble::feature(uint8_t const (&titleKey)[5],uint8_t (*block)[0x800])
{
  bool scrambled = (*block)[0x14] & 0x30 ;
  if (scrambled) {
    (*block)[0x14] &= ~0x30 ;
    B5 seed = titleKey ;
    for (unsigned i=0 ; i<5 ; ++i) {
      seed[i] ^= (*block)[0x54+i] ;
    }
    Range<uint8_t> cipher(block,0x80,0x800) ;
    Decrypt::feature(seed,cipher,cipher) ;
  }
  return scrambled ;
}

void Unscramble::feature(TitleKeyMap const &keyM,Feature::V *volume,std::ostream *os)
{
  Feature f = Feature::read(volume) ;
  // map 32-bit lba to vno (0..99)
  uint32_t nblocks = volume->count() ;
  std::vector< boost::optional<VNo> > map(nblocks) ;
  for (Feature::VNo::Type no=Feature::VNo::lower ; no<=Feature::VNo::upper ; ++no) {
    std::vector<uint32_t> lbaV = f.tellVob(Feature::VNo(no)) ;
    for (std::vector<uint32_t>::const_iterator i=lbaV.begin() ; i!=lbaV.end() ; ++i) {
      map.at(*i) = no ;
    }
    // well, this is lazy and expensive (fairly simple though)
  }
  os->seekp(0) ;
  for (uint32_t lba=0 ; lba<nblocks ; ++lba) {
    uint8_t block[Feature::V::blockSize] ;
    volume->read(lba,&block) ;
    boost::optional<VNo> no = map.at(lba) ;
    if (no) {
      boost::optional<B5> key = keyM.find(*no) ;
      if (key) {
	feature(*key,&block) ;
	// no error if encrypted but no title-key found
	// you may read the resulting file to check whether encrypted packs remain
      }
    } // no vno means not part of any *VOB
    Os::writeAll(*os,block) ;
  }
}

