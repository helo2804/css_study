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

#include "Predict.h"

Predict::Cycle::Vector Predict::findCycle(Range<uint8_t const> text,size_t msize)
{
  std::vector<bool> drop(text.size(),false) ; 
  // set to true if wsize is already covered by a cycle of smaller wsize
  Cycle::Vector v ;
  for (size_t wsize=1 ; wsize<text.size() ; ++wsize) { 
    if (drop[wsize])
      continue ;
    size_t i = 0 ; 
    while ((i+wsize < text.size()) && (text[text.size()-1-i] == text[text.size()-1-(i+wsize)])) {
      ++i ;
    }
    if (i >= msize) {
      v.push_back(Cycle(wsize,i)) ;
    }
    if (i >= 1) {
      // if i covers more than one word (i.e. a cycle of words) then we 
      // return only the smallest word and skip multiple words
      for (size_t j=2 ; j*wsize<wsize+i ; ++j) {
	drop[j*wsize] = true ;
      }
    }
  }
  return v ;
}

size_t Predict::match(Range<uint8_t const> word,Range<uint8_t const> text)
{
  size_t i = 0 ;
  if (word.size() > 0) {
    while ((i < text.size()) && (text[i] == word[i % word.size()])) {
      ++i ;
    }
  }
  return i ;
} 

