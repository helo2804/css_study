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

#include "Decrypt.h"
#include "Format.h"
#include "Os.h"
#include "Ui.h"

static void discKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: player-key=byte:5 cipher=byte:5" << std::endl ;
    argL.pop() ;
    return ;
  }
  Decrypt::B5 pkey = Format::byteA<5>(argL.pop()) ; 
  Decrypt::B5 ciph = Format::byteA<5>(argL.pop()) ; 
  argL.finalize() ;
  Decrypt::B5 dkey = Decrypt::discKey(pkey,ciph) ;
  std::cout << Format::hex(dkey) << std::endl ;
}

static void titleKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: disc-key=byte:5 cipher=byte:5" << std::endl ;
    argL.pop() ;
    return ;
  }
  Decrypt::B5 dkey = Format::byteA<5>(argL.pop()) ; 
  Decrypt::B5 ciph = Format::byteA<5>(argL.pop()) ;
  argL.finalize() ;
  Decrypt::B5 tkey = Decrypt::titleKey(dkey,ciph) ;
  std::cout << Format::hex(tkey) << std::endl ;
}

static void feature(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: seed=byte:5 input=filename output=filename" << std::endl 
	      << "note that the complete file is read into memory" << std::endl ;
    argL.pop() ;
    return ;
  }
  Decrypt::B5 seed = Format::byteA<5>(argL.pop()) ;
  std::ifstream is ; Os::open(is,argL.pop()) ;
  std::ofstream os ; Os::open(os,argL.pop()) ;
  argL.finalize() ;
  std::streamsize n = Os::size(is) ;
  std::vector<uint8_t> buffer(n) ;
  Os::readAll(is,&buffer) ;
  Decrypt::feature(seed,buffer,&buffer) ;
  Os::writeAll(os,buffer) ;
}

void Ui::decryptInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."            << std::endl
      << "supported options are: "      << std::endl 
      << "disc-key:  decrypt disc-key"  << std::endl 
      << "title-key: decrypt title-key" << std::endl  
      << "feature:   decrypt feature"   << std::endl ;
  }
  else if (arg == "disc-key") {
    discKey(argL) ;
  }
  else if (arg == "title-key") {
    titleKey(argL) ;
  }
  else if (arg == "feature") {
    feature(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
