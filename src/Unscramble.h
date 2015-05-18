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

#ifndef _Unscramble_h_
#define _Unscramble_h_

#include "Array.hpp"
#include "Auth.h"
#include "Decrypt.h"
#include "Domain.hpp"
#include "Feature.h"
#include <map>
#include <boost/array.hpp>
#include <boost/optional.hpp>

namespace Unscramble
{
  typedef Array<uint8_t,5> B5 ;

  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Unscramble:" + s) {}
  } ;

  // read keys from file, one key at the begin of each line, while the 
  //   remainder of the line is ignored. this file format is used 
  //   -- for the disc-key-block with 409 encrypted disc-keys
  //   -- for an arbitrary number of player-keys
  std::vector<B5> readKeys(std::string const &fname) ;

  typedef Domain<unsigned,0u,99u> VNo ; // 0=VMG,1..99=VTS

  // title keys
  struct TitleKeyMap : public std::map<VNo,B5>
  {
    typedef std::map<VNo,B5> base_type ;
    // read keys from file, each line contains a video-set-number 
    //   (0..99) followed by a title-key. the remainder of the line 
    //   is ignored. the file format is used for encrypted or
    //   decrypted title-keys.
    static TitleKeyMap readFile(std::string const &fname) ;
    // read keys from DVD
    static TitleKeyMap readDisc(std::string const &device,unsigned int timeout,uint8_t const (&nonce)[10]) ;
    boost::optional<B5> find(VNo no) const ;
    // decrypt all title-keys with the given disc-key
    TitleKeyMap decrypt(uint8_t const (&discKey)[5]) const ;
  private:
    TitleKeyMap(base_type const &map) : base_type(map) {}
  } ;

  // decrypt the disc-key-block with the player-key and return the
  //   result. the disc-key-block contains 1 disc-key hash-value plus
  //   408 encrypted disc-keys, for 408 (different) player-keys.
  boost::optional<B5> discKey(uint8_t const (&playerKey)[5],uint8_t const (&discKeyBlock)[409][5]) ; 

  // same as above, takes however multiple player-keys
  boost::optional<B5> discKey(Range<B5 const> playerKeyR,uint8_t const (&discKeyBlock)[409][5]) ;

  // decrypt the block if encryted, return true if decrypted
  bool feature(uint8_t const (&titleKey)[5],uint8_t (*block)[0x800]) ;

  // create a new decrypted DVD feature from the given one; the given
  //   title-keys must be already decrypted
  void feature(TitleKeyMap const &keys,Feature::V *volume,std::ostream *os) ;
}

#endif // _Unscramble_h_
