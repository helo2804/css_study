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

#include "Auth.h"
#include "Format.h"
#include "Ui.h"

typedef Array<uint8_t,10> B10 ;

static void buskey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: variant=0..31 key1=byte:5 key2=byte:5" << std::endl ;
    argL.pop() ;
    return ;
  }
  Auth::Variant variant = Format::number<Auth::Variant::Type>(argL.pop()) ;
  Auth::B5 key1 = Format::byteA<5>(argL.pop()) ;
  Auth::B5 key2 = Format::byteA<5>(argL.pop()) ;
  argL.finalize() ;
  Auth::B5 buskey = Auth::makeBusKey(variant,key1,key2) ;
  std::cout << Format::hex(buskey) << std::endl ;
}

static void key1(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: variant=0..31 nonce1=byte:10" << std::endl ;
    return ;
  }
  Auth::Variant variant = Format::number<Auth::Variant::Type>(argL.pop()) ;
  B10 nonce1 = Format::byteA<10>(argL.pop()) ;
  argL.finalize() ;
  Auth::B5 key1 = Auth::makeKey1(variant,nonce1) ;
  std::cout << Format::hex(key1) << std::endl ;
}

static void key2(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: variant=0..31 nonce2=byte:10" << std::endl ;
    argL.pop() ;
    return ;
  }
  Auth::Variant variant = Format::number<Auth::Variant::Type>(argL.pop()) ;
  B10 nonce2 = Format::byteA<10>(argL.pop()) ;
  argL.finalize() ;
  Auth::B5 key2 = Auth::makeKey2(variant,nonce2) ;
  std::cout << Format::hex(key2) << std::endl ;
}

static void variant(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: key1=byte:5 nonce1=byte:10" << std::endl ;
    argL.pop() ;
    return ;
  }
  Auth::B5 key1 = Format::byteA<5>(argL.pop()) ;
  B10 nonce1 = Format::byteA<10>(argL.pop()) ;
  argL.finalize() ;
  for (unsigned variant=Auth::Variant::lower ; variant<=Auth::Variant::upper ; ++variant) {
    Auth::B5 key = Auth::makeKey1(variant,nonce1) ;
    if (key1 == key) {
      std::cout << variant << std::endl ;
    }
  }
}

void Ui::authInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "buskey:  make buskey"   << std::endl 
      << "key1:    make KEY1"     << std::endl  
      << "key2:    make KEY2"     << std::endl
      << "variant: find variant"  << std::endl ;
  }
  else if (arg == "buskey") {
    buskey(argL) ;
  }
  else if (arg == "key1") {
    key1(argL) ;
  }
  else if (arg == "key2") {
    key2(argL) ;
  }
  else if (arg == "variant") {
    variant(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
