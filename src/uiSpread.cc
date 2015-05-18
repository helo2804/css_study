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

#include "Format.h"
#include "Spread.h"
#include "Ui.h"
#include <map>

typedef Array<uint8_t,3> B3 ;

static ByteGen unshift(ByteGen const &gn,size_t n,uint8_t s0)
{
  ByteGen g0 = gn ;
  g0.unshift(n) ;
  g0.setCarry(false) ;
  ByteGen g1 = g0 ; 
  uint8_t s = g1.shift() ;
  if (s != s0) {
    g0.setCarry() ;
    g1 = g0 ; 
    s = g1.shift() ;
    assert(s == s0) ;
  }
  return g0 ;
}

static void sum(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: start=byte:3 end=byte:3 [--invertA 0|1] [--invertB 0|1] [--valid 0|1]" << std::endl 
      << "output: the keystream followed by a list of pairs: each pair holds" << std::endl 
      << "        the number of unique keystream bits and their occurrence." << std::endl ;
    argL.pop() ;
    return ;
  }
  B3 start = Format::byteA<3>(argL.pop()) ;
  B3 end = Format::byteA<3>(argL.pop()) ;
  bool invertA = Format::number<bool>(argL.option("--invertA","0")) ;
  bool invertB = Format::number<bool>(argL.option("--invertB","0")) ;
  bool valid = Format::number<bool>(argL.option("--valid","0")) ;
  argL.finalize() ;
  B3 head = start ; goto Enter ;
  while (head != end) {
    if (++head[2]==0) if (++head[1]==0)	++head[0] ;
  Enter:
    std::vector<Spread::Record> v = Spread::split(head,invertA,invertB,valid) ;
    std::map<size_t,size_t> map ;
    for (std::vector<Spread::Record>::const_iterator i=v.begin() ; i!=v.end() ; ++i) {
      ++map[i->nbits] ;
    }
    std::cout << Format::hex(head) ;
    for (std::map<size_t,size_t>::const_iterator i=map.begin() ; i!=map.end() ; ++i) {
      size_t const &length = i->first ;
      size_t const &count = i->second ;
      std::cout << " | " << length << " " << count ;
    }
    std::cout << std::endl ;
  }
}

static void verbose(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: keystream=byte:3 [--invertA 0|1] [--invertB 0|1] [--valid 0|1]" << std::endl 
      << "output: keystream, number of unique bits, LFSR-A, LFSR-B, carry-over," << std::endl 
      << "        valid start state (0|1), generator seed" << std::endl ;
    argL.pop() ;
    return ;
  }
  B3 head = Format::byteA<3>(argL.pop()) ;
  bool invertA = Format::number<bool>(argL.option("--invertA","0")) ;
  bool invertB = Format::number<bool>(argL.option("--invertB","0")) ;
  bool valid = Format::number<bool>(argL.option("--valid","0")) ;
  argL.finalize() ;
  std::vector<Spread::Record> v = Spread::split(head,invertA,invertB,valid) ;
  for (std::vector<Spread::Record>::const_iterator i=v.begin() ; i!=v.end() ; ++i) {
    int n = (i->nbits+7) / 8 ;
    ByteGen g0 = unshift(i->gn,n,head[0]) ;
    ByteGen gn = g0 ;
    std::vector<uint8_t> s(n) ; gn.shift(&s) ;
    std::cout 
      << Format::hex(s) << " "
      << i->nbits << " "
      << "0x" << Format::hex(g0.lfsrA.value()) << " "
      << "0x" << Format::hex(g0.lfsrB.value()) << " " 
      << g0.getCarry() << " " 
      << g0.valid() << " " 
      << Format::hex(g0.k()) << std::endl ;
  }
}

void Ui::spreadInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..." << std::endl
      << "supported options are:" << std::endl 
      << "sum:     A summary for each 3-uint8_t keystream of the given range" << std::endl 
      << "verbose: Each unique keystream that starts with the given bytes" << std::endl ;
  }
  else if (arg == "sum") {
    sum(argL) ;
  }
  else if (arg == "verbose") {
    verbose(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
