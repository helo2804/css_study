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

#include "Os.h"
#include "Unscramble.h"
#include "Ui.h"

#include <sstream>
#include <fstream>

using namespace Unscramble ;

static void discKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: player-keys=file disc-key-block=file" << std::endl ;
    argL.pop() ;
    return ;
  }
  std::vector<B5> keyV = readKeys(argL.pop()) ;
  std::vector<B5> cipherV = readKeys(argL.pop()) ;
  if (cipherV.size() != 409)
    throw Ui::Error("409 keys expected") ;
  uint8_t cipherA[409][5] ; std::copy(&cipherV[0].carray(),&cipherV[409].carray(),&cipherA[0]) ;
  argL.finalize() ;
  Range<B5 const> keyR = keyV ;
  boost::optional<B5> discKey = Unscramble::discKey(keyR,cipherA) ;
  if (discKey) {
    std::cout << Format::hex(*discKey) << std::endl ;
  }
}

static void feature(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: title-keys=file encrypted-feature=file decrypted-feature=file" << std::endl ;
    argL.pop() ;
    return ;
  }
  TitleKeyMap keys = TitleKeyMap::readFile(argL.pop()) ;
  Feature::V volume(argL.pop()) ;
  std::ofstream os ; Os::open(os,argL.pop()) ;
  argL.finalize() ;
  Unscramble::feature(keys,&volume,&os) ;
}

static void titleKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: disc-key=byte:5 ciphers=file" << std::endl ;
    argL.pop() ;
    return ;
  }
  B5 discKey = Format::byteA<5>(argL.pop()) ; 
  TitleKeyMap cipher = TitleKeyMap::readFile(argL.pop()) ;
  argL.finalize() ;
  TitleKeyMap plain = cipher.decrypt(discKey) ;
  for (TitleKeyMap::const_iterator i=plain.begin() ; i!=plain.end() ; ++i) {
    VNo const &no = i->first ;
    B5 const &titleKey = i->second ;
    std::cout << "0x" << Format::hex(no.value()) << " " << Format::hex(titleKey) << std::endl ;
  }
}

void Ui::unscrambleInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "disc-key:  Decrypt the disc-key from a disc-key block with a list of title-keys" << std::endl 
      << "feature:   Decrypt a feature with a list of title-keys" << std::endl 
      << "title-key: Decrypt a list of title-keys with a disc-key" << std::endl ;
  }
  else if (arg == "disc-key") {
    discKey(argL) ;
  }
  else if (arg == "feature") {
    feature(argL) ;
  }
  else if (arg == "title-key") {
    titleKey(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
