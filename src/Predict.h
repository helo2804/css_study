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

#ifndef _Predict_h_
#define _Predict_h_

// Prediction of a pattern in a DVD pack. The predicted pattern and the 
// encrypted feature data is used as basis to recover the title-key.
//
// We assume, there is a repeating snippet of data (the word w) in the 
// feature data. so the data contains a cycle w{2,n} where the word
// w appears twice or more. If this cycle crosses over the encryption
// boundary at 0x80 in a pack, we've got our plain-text prediction.
//
// Example:
//
//   offset: ... 70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F
//   -----------------------------------------------------------
//     data: ... 00 00 00 00 00 00 00 00 01 02 03 04 05 06 01 02
//
//   Here, the word <w> is '01 02 03 04 05 06'. 
//     The head <h> is the data before the border: '01 02' 
//     The tail <t> is the data after the border: '03 04 05 06'
//
//   We would predict the subsequent data as:
//
//   offset: 80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F ...
//   -----------------------------------------------------------
//     data: 03 04 05 06 01 02 03 04 05 06 01 02 03 04 05 06 ...
//
// Since the procedure to recover the title-key is costly (in terms of 
// cpu time), it may be helpful to estimate the quality of a certain 
// prediction, for example use the number of repetions (match size).

#include "Range.hpp"
#include <stdint.h>
#include <vector>
#include <boost/optional.hpp>

namespace Predict
{
  struct Cycle
  {
    typedef std::vector<Cycle> Vector ;
    Cycle(size_t wsize,size_t msize) : wsize(wsize),msize(msize) {}
    size_t wsize ; // word size (at least 1)
    size_t msize ; // match size (at least 1)
  } ;

  // find all possible cycles with at least one matching digit
  Cycle::Vector findCycle(Range<uint8_t const> text,size_t msize=1) ;
  // note: a word in this implementation does never contain a nested 
  //   cycle (e.g. word is never "abab")

  // match the word with the start of the text. return the number of
  // matching bytes. if the complete word matches, continue with the
  // start of the word again, until mismatch, or until text ends.
  size_t match(Range<uint8_t const> word,Range<uint8_t const> text) ;
}

#endif // _Predict_h_
