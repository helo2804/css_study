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

#include "Feature.h"
#include "Os.h"
#include "Pack.h"
#include "Predict.h"
#include "Ui.h"

static std::pair<size_t,size_t> libdvdcss(uint8_t const (&block)[0x800])
{
  // the libdvdcss prediction (hopefully its the same here)
  size_t mwi = 0 ; // max word index
  size_t mci = 0 ; // max cycle index
  // word [0x80-i] .. [0x7f]
  for (size_t wi=2 ; wi<0x30 ; ++wi) {
    size_t ci = wi + 1 ;
    // find cylce start
    while ((ci<0x80) && (block[0x7f-(ci%wi)] == block[0x7f-ci])) {
      if (ci>mci) {
	mci = ci ;
	mwi = wi ;
      }
      ++ci ;
    }
  }
  return std::make_pair(mwi,mci) ;
}

static std::pair<size_t,size_t> plus(uint8_t const (&block)[0x800])
{
  // the "Ethan Hawke" DeCSSplus prediction (hopefully its the same here)
  size_t mwi = 0 ; // max word index
  size_t mci = 0 ; // max cycle index
  // word [0x80-i] .. [0x7f]
  for (size_t wi=2 ; wi<0x30 ; ++wi) { 
    // find cylce start...
    size_t ci = wi ;
    while ((ci<0x80) && (block[0x7f-(ci%wi)] == block[0x7f-ci]))
      ++ci ;
    // ...cycle starting at [0x80-ci]
    if ((ci>mci) && (ci>wi)) {
      mci = ci ;
      mwi = wi ;
    }
  }
  return std::make_pair(mwi,mci) ;
}

// --------------------------------------------------------------------

static void cycle(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: feature" << std::endl  
      << "output: For each DVD-Pack with a prediction quality greater than zero" << std::endl 
      << "        the LBA, the PES payload start, the PES payload end, and then" << std::endl 
      << "        a triplet for each prediction: word size, quality, match size." << std::endl ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ; 
  argL.finalize() ;
  std::vector<Feature::Lba> lbaV = Feature::read(&volume).tellVob() ;
  for (std::vector<Feature::Lba>::const_iterator i=lbaV.begin() ; i!=lbaV.end() ; ++i) {
    Feature::Lba lba = (*i) ;
    uint8_t block[0x800] ; volume.read(lba,&block) ;
    Pack pack = Pack::parse(block) ;
    Pack::PES const *pes = pack.at0x80() ;
    if (pes == NULL) continue ;
    Range<uint8_t const> prefix(block,pes->payload,0x80) ;
    Predict::Cycle::Vector l = Predict::findCycle(prefix) ;
    if (l.empty()) continue ;
    Range<uint8_t const> postfix(block,0x80,pes->payload + pes->size) ;
    std::cout << lba << " "
	      << pes->payload << " "
	      << pes->payload + pes->size ;
    for (Predict::Cycle::Vector::const_iterator i=l.begin() ; i!=l.end() ; ++i) {
      Range<uint8_t const> word(block,0x80 - i->wsize,0x80) ;
      size_t match = Predict::match(word,postfix) ;
      std::cout << " | " << i->wsize << " " << i->msize << " " << match ;
    }
    std::cout << std::endl ;
  }
}

static void libdvdcss(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: feature" << std::endl  
      << "output: For each DVD-Pack the LBA, the word size, the cycle size, the" << std::endl
      << "        libdvdcss prediction (0/?/1) and the actual match (0/1)." << std::endl
      << "        flags: '0' = no match, '?' = match (flaw), '1' = match." << std::endl ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ; 
  argL.finalize() ;
  std::vector<Feature::Lba> lbaV = Feature::read(&volume).tellVob() ;
  for (std::vector<Feature::Lba>::const_iterator i=lbaV.begin() ; i!=lbaV.end() ; ++i) {
    Feature::Lba lba = (*i) ;
    uint8_t block[0x800] ; volume.read(lba,&block) ;
    std::pair<size_t,size_t> p = libdvdcss(block) ;
    std::cout << lba << " " << p.first << " " << p.second << " " ;
    bool equal = false ;
    // the selection below is also from the libdvdcss prediction
    if (p.first == 0) { 
      std::cout << "0" ;
    }
    else if ((p.second<=3) || (p.second/p.first<2)) {
      std::cout << "0" ;
    }
    else {
      size_t len = (p.second / p.first) * p.first ;
      if (len < 10) {
	// this would cross the encryption border at 0x80
	std::cout << "?" ; 
      }
      else {
	std::cout << "1" ;
	equal = std::equal(&block[0x80],&block[0x80+10],&block[0x80-len]) ;
      }
    }
    std::cout << " " << equal << std::endl ;
  }
}

static void plus(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: feature" << std::endl  
      << "output: For each DVD-Pack the LBA, the word size, the cycle size, the" << std::endl
      << "        DeCSSplus prediction (0/1) and the actual match (0/1)." << std::endl
      << "        flags: '0' = no match, '1' = match." << std::endl ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ; 
  argL.finalize() ;
  std::vector<Feature::Lba> lbaV = Feature::read(&volume).tellVob() ;
  for (std::vector<Feature::Lba>::const_iterator i=lbaV.begin() ; i!=lbaV.end() ; ++i) {
    Feature::Lba lba = (*i) ;
    uint8_t block[0x800] ; volume.read(lba,&block) ;
    std::pair<size_t,size_t> p = plus(block) ;
    std::cout << lba << " " << p.first << " " << p.second << " " ;
    bool equal = false ;
    if (p.first == 0) { 
      std::cout << "0" ;
    }
    else if ((p.second<=20) || (p.second/p.first<2)) {
      std::cout << "0" ;
    }
    else {
      std::cout << "1" ;
      size_t len = (p.second / p.first) * p.first ;
      equal = std::equal(&block[0x80],&block[0x80+10],&block[0x80-len]) ;
    }
    std::cout << " " << equal << std::endl ;
  }
}

// --------------------------------------------------------------------

void Ui::predictInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "cycle:     Predict a cycle with a quality above zero" << std::endl 
      << "libdvdcss: Libdvdcss prediction " << std::endl 
      << "plus:      DeCSS plus prediction " << std::endl ;
  }
  else if (arg == "cycle") {
    cycle(argL) ;
  }
  else if (arg == "libdvdcss") {
    libdvdcss(argL) ;
  }
  else if (arg == "plus") {
    plus(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
