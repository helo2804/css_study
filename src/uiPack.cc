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
#include "Pack.h"
#include "Ui.h"

static bool isPadded(Range<uint8_t const> a)
{
  for (size_t i=0 ; i<a.size() ; ++i) {
    if (a[i] != 0xff) {
      return false ;
    }
  }
  return true ;
}

// --------------------------------------------------------------------

static void id(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: feature" << std::endl  
      << "output: for each pack the LBA followed by one or two PES" << std::endl
      << "        triplets: ID, user data offset, user data size" << std::endl ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ; 
  argL.finalize() ;
  std::vector<Feature::Lba> lbaV = Feature::read(&volume).tellVob() ;
  for (std::vector<Feature::Lba>::const_iterator i=lbaV.begin() ; i!=lbaV.end() ; ++i) {
    Feature::Lba lba = (*i) ;
    std::cout << lba ;
    uint8_t block[0x800] ; volume.read(lba,&block) ;
    try {
      Pack pack = Pack::parse(block) ;
      std::cout << " | 0x"
		<< Format::hex(pack.pes1.id) << " " 
		<< pack.pes1.payload << " "
		<< pack.pes1.size ;
      if (pack.pes2) {
	std::cout << " | 0x" 
		  << Format::hex(pack.pes2->id) << " " 
		  << pack.pes2->payload << " "
		  << pack.pes2->size ;
      }
    }
    catch (Error &e) {
      std::cout << " E " << e.what() ;
    }
    std::cout << std::endl ;
  }
}

static void scrambling(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: feature" << std::endl  
      << "output: for each pack the LBA followed by the VTS number (zero for VMG)," << std::endl
      << "        the bits 4 and 5 at offset 20, the offset of the MPEG scrambling-" << std::endl
      << "        control-field (if any), S|P (scrambled or plain-text according to" << std::endl
      << "        the scrambling control field)." << std::endl ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ; 
  argL.finalize() ;
  Feature f = Feature::read(&volume) ;
  for (Feature::VNo::Type no=Feature::VNo::lower ; no<=Feature::VNo::upper ; ++no) {
    std::vector<uint32_t> lbaV = f.tellVob(Feature::VNo(no)) ;
    for (std::vector<uint32_t>::const_iterator i=lbaV.begin() ; i!=lbaV.end() ; ++i) {
      uint32_t lba = (*i) ;
      uint8_t block[0x800] ; volume.read(lba,&block) ;
      std::cout << lba << " " 
		<< int(no) << " " 
		<< int((block[0x14] & 0x30) >> 4) << " " ;
      try {
	Pack pack = Pack::parse(block) ;
	if (pack.pes1.ext) std::cout << pack.pes1.ext->ofs ;
	else               std::cout << "-" ;
	std::cout << " " << (pack.pes1.scrambled() ? "S" : "P") 
		  << " " << pack.pes1.payload << " " ;
      }
      catch (Error &e) {
	std::cout << "E " << e.what() ;
      }
      std::cout << std::endl ;
    }
  }
  // note: applications as libdvdcss do always check the flag at offset
  //   0x14 to decide whether scrambled or not. However, this is beyond 
  //   the MPEG-2 specification for two reasons: First, the related
  //   MPEG-2 copyright-field is only defined for extended PES. But DVD
  //   packs may also hold non-extended PES (e.g. 0xBF private stream).
  //   Second, even if there is an extended PES, the location of the 
  //   MPEG-2 copyright-field may vary, and needs not to be exactly at 
  //   offset 0x14. 
  //   Though, it appears the DVD standard makes it somehow sure there
  //   is a valid copyright-field at offset 0x14. 
}

static void verify(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: feature" << std::endl  
      << "output: the LBA for each padding PES that does not consist of FF fillers" << std::endl ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ; 
  argL.finalize() ;
  std::vector<Feature::Lba> lbaV = Feature::read(&volume).tellVob() ;
  for (std::vector<Feature::Lba>::const_iterator i=lbaV.begin() ; i!=lbaV.end() ; ++i) {
    Feature::Lba lba = (*i) ;
    uint8_t block[0x800] ; volume.read(lba,&block) ;
    Pack pack = Pack::parse(block) ;
    bool ok = true ;
    if (pack.pes1.id == 0xbe)
      ok = isPadded(Range<uint8_t const>(block,pack.pes1.payload,pack.pes1.payload+pack.pes1.size)) ;
    if (ok && pack.pes2 && pack.pes2->id == 0xbe) 
      ok = isPadded(Range<uint8_t const>(block,pack.pes2->payload,pack.pes2->payload+pack.pes2->size)) ;
    if (!ok)
      std::cout << lba << std::endl ;
  }
}

// --------------------------------------------------------------------

void Ui::packInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "id:         List the PES ID(s) for each DVD-Pack" << std::endl
      << "scrambling: List the scrambling control field for each DVD-Pack" << std::endl
      << "verify:     List invalid padding PES" << std::endl ;
  }
  else if (arg == "id") {
    id(argL) ;
  }
  else if (arg == "scrambling") {
    scrambling(argL) ;
  }
  else if (arg == "verify") {
    verify(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
