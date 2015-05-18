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
#include "Revert.h"
#include "Ui.h"

static void head(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: keystream=byte:3+ [--invertA 0|1] [--invertB 0|1]" << std::endl 
      << "output: the seed for each matching generator start state" << std::endl ;
    argL.pop() ;
    return ;
  }
  std::vector<uint8_t> s = Format::byteV(argL.pop()) ;
  bool invertA = Format::number<bool>(argL.option("--invertA","0")) ;
  bool invertB = Format::number<bool>(argL.option("--invertB","0")) ;
  argL.finalize() ;
  if (s.size() < 3) {
    throw Ui::Error("at least 3-byte keystream expected") ;
  }
  Range<uint8_t const> head(s,0,3) ;
  Range<uint8_t const> tail(s,3,s.size()) ;
  std::vector<ByteGen> v = Revert::head(head.sub<3>(0),invertA,invertB) ;
  std::vector<size_t> lenV = Revert::match(&v,tail) ;
  for (size_t i=0 ; i!=v.size() ; ++i) {
    if (lenV[i]+3 != s.size())
      continue ;
    ByteGen g0 = v[i] ;
    g0.unshift(s) ;
    std::cout << Format::hex(g0.k()) << std::endl ;
  }
}

static void snippet(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: keystream=byte:3+ [--invertA 0|1] [--invertB 0|1]" << std::endl 
      << "output: valid start state (0|1), seed, LFSR-A, LFSR-B, carry" << std::endl ;
    argL.pop() ;
    return ;
  }
  std::vector<uint8_t> s = Format::byteV(argL.pop()) ;
  bool invertA = Format::number<bool>(argL.option("--invertA","0")) ;
  bool invertB = Format::number<bool>(argL.option("--invertB","0")) ;
  argL.finalize() ;
  if (s.size() < 3) {
    throw Ui::Error("at least three bytes expected") ;
  }
  Range<uint8_t const> head(s,0,3) ;
  Range<uint8_t const> tail(s,3,s.size()) ;
  std::vector<ByteGen> v = Revert::snippet(head.sub<3>(0),invertA,invertB) ;
  std::vector<size_t> lenV = Revert::match(&v,tail) ;
  for (size_t i=0 ; i!=v.size() ; ++i) {
    if (lenV[i]+3 != s.size())
      continue ;
    ByteGen g0 = v[i] ;
    g0.unshift(s) ;
    std::cout << g0.valid()      << " "
	      << Format::hex(g0.k()) << " "
	      << "0x" << Format::hex(g0.lfsrA.value()) << " "
	      << "0x" << Format::hex(g0.lfsrB.value()) << " "
	      << g0.getCarry()  << std::endl ;
  }
}

void Ui::revertInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "head:    Find all valid start states for a keystream (head)" << std::endl
      << "snippet: Find all start states for a keystream (snippet)" << std::endl ;
  }
  else if (arg == "head") {
    head(argL) ;
  }
  else if (arg == "snippet") {
    snippet(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
