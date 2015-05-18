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
#include "Decrypt.h"
#include "Format.h"
#include "Ui.h"

typedef Decrypt::B5 B5 ;

static bool increment(B5 *key)
{
  bool c ;
  size_t i = key->size ;
  do {
    --i ;
    c = 0 == (++(*key)[i]) ;
  }
  while (c && (i != 0)) ;
  return c ;
}

static void discKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: hash=byte:5 [--begin byte:5] [--end byte:5] [--interval I]" << std::endl ;
    argL.pop() ;
    return ;
  }
  B5 hash = Format::byteA<5>(argL.pop()) ;
  B5 begin = Format::byteA<5>(argL.option("--begin","00:00:00:00:00")) ;
  B5 end = Format::byteA<5>(argL.option("--end","00:00:00:00:00")) ;
  size_t interval = Format::number<size_t>(argL.option("--interval","0x100000")) ;
  argL.finalize() ;
  B5 key = begin ;
  size_t i = interval ;
  goto Enter ;
  while (key != end) {
    increment(&key) ;
    ++i ;
  Enter:
    if (i == interval) {
      i = 0 ;
      std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" << Format::hex(key) << std::flush ;
    }
    B5 self = Decrypt::discKey(key,hash) ;
    if (self == key) {
      std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" << Format::hex(key) << std::endl ;
    }
  }
  std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" 
	    << "              " << std::endl ;
}

static void playerKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: disc-key=byte:5 cipher=byte:5 [--begin byte:5] [--end byte:5] [--interval I]" << std::endl ;
    argL.pop() ;
    return ;
  }
  B5 dkey = Format::byteA<5>(argL.pop()) ;
  B5 ciph = Format::byteA<5>(argL.pop()) ;
  B5 begin = Format::byteA<5>(argL.option("--begin","00:00:00:00:00")) ;
  B5 end = Format::byteA<5>(argL.option("--end","00:00:00:00:00")) ;
  size_t interval = Format::number<size_t>(argL.option("--interval","0x100000")) ;
  argL.finalize() ;
  B5 pkey = begin ;
  size_t i = interval ;
  goto Enter ;
  while (pkey != end) {
    increment(&pkey) ;
    ++i ;
  Enter:
    if (i == interval) {
      i = 0 ;
      std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" << Format::hex(pkey) << std::flush ;
    }
    B5 self = Decrypt::discKey(pkey,ciph) ;
    if (self == dkey) {
      std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" << Format::hex(pkey) << std::endl ;
    }
  }
  std::cout << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b" 
	    << "              " << std::endl ;
} 

void Ui::studyInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "disc-key:   brute-force disc-keys from their hash value" << std::endl 
      << "player-key: brute-force player-keys from a disc-key and the cipher in the" << std::endl 
      << "            disc-key-block" << std::endl ;
  }
  else if (arg == "disc-key") {
    discKey(argL) ;
  }
  else if (arg == "player-key") {
    playerKey(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
