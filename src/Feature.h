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

#ifndef _Feature_h_
#define _Feature_h_

#include "Array.hpp"
#include "Domain.hpp"
#include "Iso9660.h"

struct Feature
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Feature:" + s) {}
  } ;

  typedef uint32_t Lba ; // logical block address

  typedef Volume<Lba,uint32_t,0x800u> V ;

  // VTS/VMG number; 0=VMG,1..99=VTS
  typedef Domain<unsigned,0,99> VNo ; 

  // VTS numbers only
  typedef Domain<unsigned,1,99> VtsNo ; 

  // file number
  //    0 = VIDEO_TS.IFO
  //    1 = VIDEO_TS.VOB
  //    3 = VIDEO_TS.BUP
  //    4 = VTS_01_0.IFO (3+ (1-1)*12+ 0)
  //  ...
  // 1189 = VTS_99_9.VOB (3+(99-1)*12+10)
  // 1190 = VTS_99_0.BUP (3+(99-1)*12+11)
  typedef Domain<unsigned,0u,1190u> FNo ; 

  // file name including directory, e.g. for #5: "VIDEO_TS/VTS_01_1.VOB"
  static std::string const& name(FNo fno) ;

  // return true if any related file exists (IFO,*VOB or BUP)
  bool exists(VNo vno) const ; 
  
  // return file numbers for existing files in ascending file-number order
  std::vector<FNo> list() const ; 

  // return all LBAs for the given single file
  std::vector<Feature::Lba> tellFile(FNo fno) const ;

  // return all LBAs of all related *VOB files
  std::vector<Lba> tellVob(VNo vno) const ;

  // return all LBAs for all related *VOB files
  std::vector<Lba> tellVob(VtsNo vtsNo) const ;

  // return all LBAs of all *VOB files
  std::vector<Lba> tellVob() const ;

  static Feature read(V *volume) ;

  Iso9660::Record::Optional const& record(FNo fno) const { return this->table[fno] ; }

private:

  // one optional entry for each file
  typedef Array<Iso9660::Record::Optional,FNo::upper+1> FTable ;

  static FTable read(V *volume,Iso9660::Descriptor const &d) ;

  FTable table ;

  Feature(FTable const &table) : table(table) {}

} ;

#endif // _Feature_h_
