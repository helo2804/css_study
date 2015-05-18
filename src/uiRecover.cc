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
#include "Recover.h"
#include "Ui.h"
#include "Domain.hpp"

#include <sstream>
#include <fstream>

typedef Recover::B5 B5 ;

// --------------------------------------------------------------------

static boost::optional<B5> titleByCycle(Feature::V *volume,Feature::Lba lba,size_t minQ,Recover::Status::Counter *counter)
{
  uint8_t data[0x800] ; volume->read(lba,&data) ;
  boost::variant<Recover::Status,B5> u = Recover::titleByCycle(data,minQ) ;
  if (u.which() == 0) {
    counter->increment(boost::get<Recover::Status>(u)) ;
    return boost::optional<B5>() ;
  }
  return boost::get<B5>(u) ;
}

static boost::optional<B5> titleByPadding(Feature::V *volume,Feature::Lba lba,Recover::Status::Counter *counter)
{
  uint8_t data[0x800] ; volume->read(lba,&data) ;
  boost::variant<Recover::Status,B5> u = Recover::titleByPadding(data) ;
  if (u.which() == 0) {
    counter->increment(boost::get<Recover::Status>(u)) ;
    return boost::optional<B5>() ;
  }
  return boost::get<B5>(u) ;
}

static std::string toString(boost::optional<B5> const &titleKey,Recover::Status::Counter const &counter)
{
  std::ostringstream os ;
  if (titleKey) os << Format::hex(*titleKey) ;
  else if (counter.notScrambled == counter.total()) os << "none" ;
  else os << "failed" ;
  os << " " << counter.total()       
     << " " << counter.notScrambled  
     << " " << counter.noPattern     
     << " " << counter.tooSmall      
     << " " << counter.noPrediction  
     << " " << counter.notDecrypted  ;
  return os.str() ;
}

static std::vector<Feature::Lba> shrink(std::vector<Feature::Lba> const &in,Feature::Lba sLba,Feature::Lba eLba)
{
  std::vector<Feature::Lba> out ; out.reserve(in.size()) ;
  for (std::vector<Feature::Lba>::const_iterator i=in.begin() ; i!=in.end() ; ++i) {
    if (sLba <= (*i) && (*i) < eLba)
      out.push_back(*i) ;
  }
  return out ;
}

// --------------------------------------------------------------------

static void discKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: hash=byte:5" << std::endl 
      << "note: execution may take a few seconds" << std::endl ;
    argL.pop() ;
    return ;
  }
  B5 hash = Format::byteA<5>(argL.pop()) ;
  argL.finalize() ;
  std::vector<B5> v = Recover::discKey(hash) ;
  for (size_t i=0 ; i<v.size() ; ++i) {
    std::cout << Format::hex(v[i]) << std::endl ;
  }
}

static void playerKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: disc-key=byte:5 cipher=byte:5" << std::endl 
      << "output: the number of matching keystream followed by related" << std::endl
      << "        groups with the keystream itself plus the number of" << std::endl
      << "        recovered generator seeds (player-keys) followed by" << std::endl
      << "        by the seeds themselves" << std::endl ;
    argL.pop() ;
    return ;
  }
  B5 discKey = Format::byteA<5>(argL.pop()) ;
  B5 cipher = Format::byteA<5>(argL.pop()) ;
  argL.finalize() ;
  std::vector<B5> s = Recover::playerKey(discKey,cipher) ;
  std::cout << s.size() ;
  for (size_t i=0 ; i<s.size() ; ++i) {
    std::vector<B5> k = Recover::playerKey(s[i]) ;
    std::cout << " | " << Format::hex(s[i]) << " " << k.size() ;
    for (size_t j=0 ; j<k.size() ; ++j) {
      std::cout << " " << Format::hex(k[j]) ;
    }
  }
  std::cout << std::endl ;
}

static void titleKey(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: feature [--start LBA] [--end LBA] [--mode 0|1] [--quality Q] [--retry 0|1]" << std::endl 
      << "--mode: The mode 0 (default) is used to find a cycle. The mode 1 is used" << std::endl
      << "    to find a padding stream" << std::endl 
      << "--quality: The quality Q (default 10) defines the minimum number of bytes" << std::endl
      << "    in the cycle that must match. Only then the prediction is used to try" << std::endl
      << "    to recover a title-key (the recovery is quite expensive)." << std::endl
      << "--retry: If no key was found and the retry flag is set (default 0), a" << std::endl
      << "    second run is executed in cycle mode with Q=1." << std::endl
      << "output: (1) The VTS number, 0 for VMG; \"retry\" for a second run" << std::endl
      << "        (2) The title-key or \"failed\"; \"none\" if no pack was scrambled" << std::endl
      << "        (3) The total number of read packs" << std::endl
      << "        (4) Packs that were not scrambled" << std::endl
      << "        (5) Packs without payload at scrambling border" << std::endl
      << "        (6) Packs with less than ten bytes payload after scrambling border" << std::endl
      << "        (7) Packs that do not match the quality Q" << std::endl
      << "        (8) Packs that couldn't be deciphered" << std::endl ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ; 
  Feature::Lba sLba = Format::number<Feature::Lba>(argL.option("--start","0")) ;
  Feature::Lba eLba = Format::number<Feature::Lba>(argL.option("--end","0xffffffff")) ;
  Domain<unsigned,0,2> mode = Format::number<unsigned>(argL.option("--mode","0")) ;
  size_t minQ = Format::number<size_t>(argL.option("--quality","10")) ;
  bool retry = Format::number<bool>(argL.option("--retry","0")) ;
  argL.finalize() ;
  Feature f = Feature::read(&volume) ;
  for (Feature::VNo::Type no=Feature::VNo::lower ; no<=Feature::VNo::upper ; ++no) {
    std::vector<Feature::Lba> lbaV = shrink(f.tellVob(Feature::VNo(no)),sLba,eLba) ;
    if (lbaV.empty()) continue ;
    Recover::Status::Counter counter ;
    boost::optional<B5> titleKey ;
    for (std::vector<Feature::Lba>::const_iterator i=lbaV.begin() ; !titleKey && i!=lbaV.end() ; ++i) {
      titleKey = (mode == 0)
	? titleByCycle  (&volume,*i,minQ,&counter) 
	: titleByPadding(&volume,*i,     &counter) ;
    }
    std::cout << "0x" << Format::hex(no) << " " << toString(titleKey,counter) ;
    if (!titleKey && retry) 
      if (counter.notScrambled < counter.total()) 
	if ((mode==1) || (mode==0 && minQ>1)) {
	  Recover::Status::Counter counter ;
	  for (std::vector<Feature::Lba>::const_iterator i=lbaV.begin() ; !titleKey && i!=lbaV.end() ; ++i) {
	    titleKey = titleByCycle(&volume,*i,1,&counter) ;
	  }
	  std::cout << std::endl << "retry " << toString(titleKey,counter) ;
	}
    std::cout << std::endl ;
  }
}

// --------------------------------------------------------------------

// nice to have: analysis for cipher matches below 10 bytes

void Ui::recoverInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "disc-key:   recover disc-keys from their hash value" << std::endl 
      << "player-key: recover player-keys from a disc-key and the cipher in the" << std::endl 
      << "            disc-key-block" << std::endl 
      << "title-key:  recover title-keys from a feature" << std::endl ;
  }
  else if (arg == "disc-key") {
    discKey(argL) ;
  }
  else if (arg == "player-key") {
    playerKey(argL) ;
  }
  else if (arg == "title-key") {
    titleKey(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
