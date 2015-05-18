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

#ifndef _Ifo_h_
#define _Ifo_h_

#include "Domain.hpp"
#include "Range.hpp"
#include "Volume.hpp"
#include <map>
#include <vector>

struct Ifo
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Ifo:" + s) {}
  } ;

  typedef Domain<unsigned,1,99> VtsNo ; 

  typedef uint32_t Lba ; // logical block address

  typedef Volume<Lba,uint32_t,0x800> V ;

  Lba start ; // abs. offset of the VMG IFO (from the start of the volume)
  Lba ifoL ;  // rel. offset to last IFO block 
  Lba vobF ;  // rel. offset to first menu-block (zero if none)
  Lba bupL ;  // rel. offset to last BUP block

  struct Vts
  {
    Lba start ;     // abs. offset of this VTS from VMG-start
    Lba ifoL ;      // rel. offset to last IFO block
    Lba vobMenuF ;  // rel. offset to first menu-block (zero if none)
    Lba vobTitleF ; // rel. offset to first title-block
    Lba bupL ;      // rel. offset to last BUP block

    Vts(Lba start,Lba ifoL,Lba vobMenuF,Lba vobTitleF,Lba bupL) 
      : start(start),ifoL(ifoL),vobMenuF(vobMenuF),vobTitleF(vobTitleF),bupL(bupL) {}
  } ;

  typedef std::map<VtsNo,Vts> Map ;

  Map map ;

  static Ifo read(V *volume,Lba start) ;

private:

  Ifo(Lba start,Lba ifoL,Lba vobF,Lba bupL,Map const &map)
    : start(start),ifoL(ifoL),vobF(vobF),bupL(bupL),map(map) {}

  static std::vector<uint8_t> readTable(V *volume,Lba start) ;

  static Map readMap(V *volume,Lba start,Range<uint8_t const> table) ;

} ;

#endif // _Ifo_h_
