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

#include "ByteGen.h"
#include "BitGen.h"
#include "Format.h"
#include "Ui.h"
#include <sstream>

// --------------------------------------------------------------------

static ByteGen generator(ArgL &argL)
{
  bool invertA = Format::number<bool>(argL.option("--invertA","0")) ;
  bool invertB = Format::number<bool>(argL.option("--invertB","0")) ;
  std::string type = argL.pop() ;
  if (type == "--seed") {
    Array<uint8_t,5> seed = Format::byteA<5>(argL.pop()) ;
    return ByteGen::make(seed,invertA,invertB) ;
  }
  if (type == "--register") {
    uint32_t registerA = Format::number<unsigned long>(argL.pop()) ;
    uint32_t registerB = Format::number<unsigned long>(argL.pop()) ;
    bool carry = Format::number<bool>(argL.option("--carry","0")) ;
    return ByteGen::make(ByteGen::LfsrA::make(registerA),ByteGen::LfsrB::make(registerB),invertA,invertB,carry) ;
  }
  throw Ui::Error("Generator must be given by seed or register") ;
}

static std::string toString(ByteGen const &g)
{
  std::ostringstream os ;
  os << Format::hex(g.k()) << " "
     << g.getInvertA() << " "
     << g.getInvertB() << " "
     << g.getCarry()   << " "
     << g.valid()      << " "
     << "0x" << Format::hex(g.lfsrA.value()) << " "
     << "0x" << Format::hex(g.lfsrB.value()) ;
  return os.str() ;
}

// --------------------------------------------------------------------

static void carry(ArgL &argL)
{
  if (!argL.empty() && argL.peek() == "help") {
    std::cout << "arguments: "
	      << "[--invertA 0|1] "
	      << "[--invertB 0|1] "
	      << "[--begin 0..131071] "
	      << "[--end 0..131071]" 
	      << std::endl ;
    argL.pop() ;
    return ;
  }
  bool invertA = Format::number<bool>(argL.option("--invertA","0")) ;
  bool invertB = Format::number<bool>(argL.option("--invertB","0")) ;
  uint32_t begin = Format::number<uint32_t>(argL.option("--begin","0")) ;
  uint32_t end   = Format::number<uint32_t>(argL.option("--end","0x1ffff")) ;
  argL.finalize() ;
  for (uint32_t registerA=begin ; registerA<=end ; ++registerA) {
    std::vector<size_t> v ; v.reserve(0x2000000) ;
    for (uint32_t registerB=0 ; registerB<0x2000000 ; ++registerB) {
      ByteGen g0 = ByteGen::make(ByteGen::LfsrA::make(registerA),ByteGen::LfsrB::make(registerB),invertA,invertB,false) ;
      ByteGen g1 = g0 ; g0.setCarry() ;
      size_t i = 0 ;
      do {
	g0.shift() ;
	g1.shift() ;
	++i ;
      }
      while (g0.getCarry()!=g1.getCarry() && i<0x100) ;
      v.push_back(i) ;
    }
    size_t n = v[0] ;
    for (size_t i=1 ; i<v.size() ; ++i)
      if (v[i] > n)
	n = v[i] ;
    std::cout << "0x" << Format::hex(registerA) ;
    for (size_t i=0 ; i<v.size() ; ++i)
      if (v[i] == n)
	std::cout << " | 0x" << Format::hex(i) << " " << n ;
    std::cout << std::endl ;
  }
}

static void shift(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: "
	      << "[--invertA 0|1] "
	      << "[--invertB 0|1] "
	      << "(--seed byte:5 | --register registerA=0..131071 registerB=0..33554431 [--carry 0|1]) "
	      << "pulses "
	      << "[--keystream 0|1]" 
	      << std::endl ;
    argL.pop() ;
    return ;
  }
  ByteGen g = generator(argL) ;
  size_t pulses = Format::number<size_t>(argL.pop()) ;
  bool showKeystream = Format::number<bool>(argL.option("--keystream","0")) ;
  argL.finalize() ;
  for (size_t i=0 ; i<pulses ; ++i) {
    uint8_t s[1] = { g.shift() } ;
    if (showKeystream) {
      if (i > 0)
	std::cout << ":" ;
      std::cout << Format::hex(s) ;
    }
  }
  if (!showKeystream)
    std::cout << toString(g) ;
  std::cout << std::endl ;
}

static void unshift(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: "
	      << "[--invertA 0|1] "
	      << "[--invertB 0|1] "
	      << "(--seed byte:5 | --register registerA=0..131071 registerB=0..33554431 [--carry 0|1]) "
	      << "pulses" 
	      << std::endl ;
    argL.pop() ;
    return ;
  }
  ByteGen g = generator(argL) ;
  size_t pulses = Format::number<size_t>(argL.pop()) ;
  argL.finalize() ;
  g.unshift(pulses) ;
  std::cout << toString(g) << std::endl ;
}

static void verify(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: "
	      << "registerA=0..131071 "
	      << "registerB=0..33554431 "
	      << "[--invertA 0|1] "
	      << "[--invertB 0|1] "
	      << "[--carry 0|1]" 
	      << std::endl ;
    argL.pop() ;
    return ;
  }
  unsigned long ulA = Format::number<unsigned long>(argL.pop()) ;
  unsigned long ulB = Format::number<unsigned long>(argL.pop()) ;
  bool invertA = Format::number<bool>(argL.option("--invertA","0")) ;
  bool invertB = Format::number<bool>(argL.option("--invertB","0")) ;
  bool carry = Format::number<bool>(argL.option("--carry","0")) ;
  argL.finalize() ;

  BitGen::LfsrA::Register registerA1(ulA) ;
  BitGen::LfsrB::Register registerB1(ulB) ;
  BitGen::LfsrA lfsrA1(registerA1) ;
  BitGen::LfsrB lfsrB1(registerB1) ;
  BitGen g1(lfsrA1,lfsrB1,invertA,invertB,carry) ;

  uint32_t registerA2(ulA) ;
  uint32_t registerB2(ulB) ;
  ByteGen::LfsrA lfsrA2 = ByteGen::LfsrA::make(registerA2) ;
  ByteGen::LfsrB lfsrB2 = ByteGen::LfsrB::make(registerB2) ;
  ByteGen g2 = ByteGen::make(lfsrA2,lfsrB2,invertA,invertB,carry) ;

  size_t n = 0 ;
  uint8_t s8 = 0 ;
  do {
    bool s1 = g1.shift() ;
    if ((n%8) == 0)
      s8 = g2.shift() ;
    if (s1 != (1 & (s8 >> (n%8)))) {
      std::cout << "Generator mismatch" << std::endl ;
      return ;
    }
    ++n ;
  }
  while (g1.lfsrA.r != registerA1 || g1.lfsrB.r != registerB1) ;
  std::cout << "No mismatch after " << n << " pulses" << std::endl ;
}

// --------------------------------------------------------------------

void Ui::generatorInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;

  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "carry:   Calculate after how may pulses the carry-over bit aligns" << std::endl 
      << "shift:   Shift a generator by the given number of pulses" << std::endl  
      << "unshift: Unshift a generator by the given number of pulses" << std::endl
      << "verify:  Verify the byte-based generator with the bit-based generator and report on mismatch" << std::endl ;
  }
  else if (arg == "carry") {
    carry(argL) ;
  }
  else if (arg == "shift") {
    shift(argL) ;
  }
  else if (arg == "unshift") {
    unshift(argL) ;
  }
  else if (arg == "verify") {
    verify(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
