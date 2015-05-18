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

#ifndef _Recover_h_
#define _Recover_h_

#include "Decrypt.h"
#include <vector>
#include <boost/variant.hpp>

namespace Recover
{
  typedef Array<uint8_t,5> B5 ;

  // generate the key-stream that match the disc-key/cipher
  std::vector<B5> playerKey(uint8_t const (&discKey)[5],uint8_t const (&cipher)[5]) ;
  // generate the player-key from the key-stream
  std::vector<B5> playerKey(uint8_t const (&s)[5]) ;

  // generate the disc-keys that match the disc-key-hash
  //   (this may take a few seconds)
  std::vector<B5> discKey(uint8_t const (&hash)[5]) ;

  // title recovery statistics
  struct Status
  {
    enum type {
      notScrambled,
      noPattern,     // not suitable to predict a cycle
      tooSmall,      // this impl. expects at least 10 encrypted bytes
      noPrediction,  // prediction returned no matches
      notDecrypted   // decryption of predicted pattern failed
    } e ;
    explicit Status(type e) : e(e) {}
    operator type() { return this->e ; }

    struct Counter
    {
      size_t notScrambled ;
      size_t noPattern ;
      size_t tooSmall ;
      size_t noPrediction ;
      size_t notDecrypted ;
      size_t total() const { return 
	  this->notScrambled +
	  this->noPattern +
	  this->tooSmall +
	  this->noPrediction +
	  this->notDecrypted ; }
      Counter() : notScrambled(0),noPattern(0),tooSmall(0),noPrediction(0),notDecrypted(0) {}
      void increment(Status const &status) ;
    } ;
  } ;

  // predict a pattern and try to recover the title-key; don't try to 
  //   recover if the prediction quality is less than <minMatch>.
  //   return the recovered title-key; or the Status if not recovered
  boost::variant<Status,B5> titleByCycle  (uint8_t const (&data)[0x800],size_t minMatch) ;

  // similar as above, however an (MPEG PES) padding stream is used 
  //   for prediction
  boost::variant<Status,B5> titleByPadding(uint8_t const (&data)[0x800]) ;
}

#endif // _Recover_h_
