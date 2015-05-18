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

#include "BitGen.h"
#include "ByteGen.h"
#include "Format.h"
#include "Ui.h"

static void shift(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: register=0..131071 [--verbose 0|1]" << std::endl 
      << "output: the complete LFSR cycle if verbose, otherwise the LFSR period" << std::endl ;
      ;
    argL.pop() ;
    return ;
  }
  BitGen::LfsrA::Register r(Format::number<unsigned long>(argL.pop())) ;
  bool verbose = Format::number<bool>(argL.option("--verbose","0")) ;
  argL.finalize() ;
  BitGen::LfsrA lfsr(r) ;
  size_t n = 0 ;
  do {
    if (verbose)
      std::cout << lfsr.r << " 0x" << Format::hex(lfsr.r.to_ulong()) << std::endl ;
    lfsr.shift() ;
    ++n ;
  }
  while (r != lfsr.r) ;
  if (!verbose)
    std::cout << n << std::endl ;
}

static void shiftVerify(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: register=0..131071 [--verbose 0|1]" << std::endl 
      << "output: the LFSR cycle if verbose, otherwise the LFSR period; the word \"mismatch\" on error" << std::endl ;
      ;
    argL.pop() ;
    return ;
  }
  uint32_t start = Format::number<uint32_t>(argL.pop()) ;
  bool verbose = Format::number<bool>(argL.option("--verbose","0")) ;
  argL.finalize() ;
  BitGen ::LfsrA::Register r1(start) ;
  BitGen ::LfsrA lfsr1(r1) ;
  ByteGen::LfsrA lfsr2 = ByteGen::LfsrA::make(start) ;
  size_t n = 0 ;
  uint8_t s8 = 0 ;
  do {
    if (verbose)
      std::cout << lfsr1.r << " 0x" << Format::hex(lfsr1.r.to_ulong()) << std::endl ;
    bool s1 = lfsr1.shift() ;
    if ((n%8) == 0)
      s8 = lfsr2.shift() ;
    if (s1 != (1 & (s8 >> (n%8)))) {
      std::cout << "mismatch" << std::endl ;
      return ;
    }
    ++n ;
  }
  while (lfsr1.r != r1) ;
  if (!verbose)
    std::cout << n << std::endl ;
}

static void unshift(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
    argL.pop() ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: register=0..131071 [--verbose 0|1]" << std::endl 
      << "output: the complete LFSR cycle if verbose, otherwise the LFSR period" << std::endl ;
      ;
    argL.pop() ;
    return ;
  }
  BitGen::LfsrA::Register r(Format::number<unsigned long>(argL.pop())) ;
  bool verbose = Format::number<bool>(argL.option("--verbose","0")) ;
  argL.finalize() ;
  BitGen::LfsrA lfsr(r) ;
  size_t n = 0 ;
  do {
    if (verbose)
      std::cout << lfsr.r << " 0x" << Format::hex(lfsr.r.to_ulong()) << std::endl ;
    lfsr.unshift() ;
    ++n ;
  }
  while (r != lfsr.r) ;
  if (!verbose)
    std::cout << n << std::endl ;
}

static void unshiftVerify(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: register=0..131071 [--verbose 0|1]" << std::endl 
      << "output: the LFSR cycle if verbose, otherwise the LFSR period; the word \"mismatch\" on error" << std::endl ;
      ;
    argL.pop() ;
    return ;
  }
  uint32_t start = Format::number<uint32_t>(argL.pop()) ;
  bool verbose = Format::number<bool>(argL.option("--verbose","0")) ;
  argL.finalize() ;
  BitGen ::LfsrA::Register r1(start) ;
  BitGen ::LfsrA lfsr1(r1) ;
  ByteGen::LfsrA lfsr2 = ByteGen::LfsrA::make(start) ;
  size_t n = 0 ;
  uint8_t s8 = 0 ;
  do {
    if (verbose)
      std::cout << lfsr1.r << " 0x" << Format::hex(lfsr1.r.to_ulong()) << std::endl ;
    bool s1 = lfsr1.unshift() ;
    if ((n%8) == 0)
      s8 = lfsr2.unshift() ;
    if (s1 != (1 & (s8 >> (7-(n%8))))) {
      std::cout << "mismatch" << std::endl ;
      return ;
    }
    ++n ;
  }
  while (lfsr1.r != r1) ;
  if (!verbose)
    std::cout << n << std::endl ;
}

void Ui::lfsrAinvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are..." << std::endl 
      << "shift:          Execute LFSR cycle forward (bit-based)" << std::endl
      << "shift-verify:   Execute LFSR cycle forward (bit- and byte-based)" << std::endl
      << "unshift:        Execute LFSR cycle backwards (bit-based)" << std::endl
      << "unshift-verify: Execute LFSR cycle backwards (bit- and byte-based)" << std::endl ;
  }
  else if (arg == "shift") {
    shift(argL) ;
  }
  else if (arg == "shift-verify") {
    shiftVerify(argL) ;
  }
  else if (arg == "unshift") {
    unshift(argL) ;
  }
  else if (arg == "unshift-verify") {
    unshiftVerify(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
