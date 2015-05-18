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

#include "Handshake.h"
#include "Os.h"
#include "Ui.h"
#include "Unscramble.h"

#include <iostream>
#include <vector>

static void key(unsigned timeout,uint8_t const (&nonce)[10],ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  argL.finalize() ;
  boost::optional<Mmc::DiscKey> key = Handshake::unlock(fd,nonce,timeout) ;
  if (key) {
    for (size_t i=0 ; i<key->size ; ++i) {
      std::cout << Format::hex((*key)[i]) << std::endl ;
    }
  }
  else {
    std::cout << "not CSS protected" << std::endl ;
  }
}

static void title(unsigned timeout,uint8_t const (&nonce)[10],ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device start=LBA [--count LBAs]" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  uint32_t start = Format::number<uint32_t>(argL.pop()) ;
  uint32_t count = Format::number<uint32_t>(argL.option("--count","1")) ;
  argL.finalize() ;
  for (uint32_t i=start ; i<start+count ; ++i) {
    boost::optional<Mmc::Title> title = Handshake::title(fd,nonce,timeout,i) ;
    std::cout << "0x" << i << " " ;
    if (title) std::cout << Format::hex(title->key) << " 0x" << Format::hex(title->cmi.flags) ;
    else std::cout << "none" ;
    std::cout << std::endl  ;
  }
}

static void titleSet(unsigned timeout,uint8_t const (&nonce)[10],ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  std::string device = argL.pop() ;
  argL.finalize() ;
  Os::Fd fd = Os::Fd::openRo(device) ;
  boost::optional<Mmc::DiscKey> key = Handshake::unlock(fd,nonce,timeout) ;
  if (!key) {
    throw Ui::Error("cannot unlock.") ;
  }
  Unscramble::TitleKeyMap keys = Unscramble::TitleKeyMap::readDisc(device,timeout,nonce) ;
  for (Unscramble::TitleKeyMap::const_iterator i=keys.begin() ; i!=keys.end() ; ++i) {
    std::cout << "0x" << Format::hex(i->first.value()) << " " << Format::hex(i->second) << std::endl ;
  }
}

static void unlock(unsigned timeout,uint8_t const (&nonce)[10],ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  argL.finalize() ;
  boost::optional<Mmc::DiscKey> key = Handshake::unlock(fd,nonce,timeout) ;
  std::cout << (key ? "unlocked" : "not-CSS-protected") << std::endl ;
}

void Ui::handshakeInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }

  unsigned timeout = Format::number<unsigned>(argL.option("--timeout","10000")) ;
  Array<uint8_t,10> nonce = Format::byteA<10>(argL.option("--nonce","00:00:00:00:00:00:00:00:00:00")) ;

  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: [--timeout milli-seconds] [--nonce byte:10] option ..." << std::endl 
      << "supported options are: " << std::endl 
      << "key:       retrieve the disc-key-block" << std::endl
      << "title:     retrieve title-key(s)" << std::endl 
      << "title-set: retrieve all VMG+VTS title-keys" << std::endl 
      << "unlock:    unlock locked blocks" << std::endl ;
  }
  else if (arg == "key") {
    key(timeout,nonce,argL) ;
  }
  else if (arg == "title") {
    title(timeout,nonce,argL) ;
  }
  else if (arg == "title-set") {
    titleSet(timeout,nonce,argL) ;
  }
  else if (arg == "unlock") {
    unlock(timeout,nonce,argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
