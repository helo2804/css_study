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

#include "CArray.hpp"
#include "Ifo.h"
#include "Os.h"

static uint16_t msbf(uint8_t const (&src)[2])
{
  uint32_t dst = src[0] ;
  dst <<= 8 ; dst |= src[1] ;
  return dst ;
}
 
static uint32_t msbf(uint8_t const (&src)[4])
{
  uint32_t dst = src[0] ;
  dst <<= 8 ; dst |= src[1] ;
  dst <<= 8 ; dst |= src[2] ;
  dst <<= 8 ; dst |= src[3] ;
  return dst ;
}
 
std::vector<uint8_t> Ifo::readTable(V *volume,Lba start) 
{
  uint8_t block[V::blockSize] ; 
  volume->read(start,&block) ;
  uint32_t nVts = msbf(CArray::sub<0,2>(block)) ;
  uint32_t lastByte = msbf(CArray::sub<4,8>(block)) ;
  if (nVts*12+8-1 != lastByte)
    throw Error("number of titles does not align with table size") ;
  Lba nblocks = (lastByte+V::blockSize) / V::blockSize ;
  std::vector<uint8_t> table(nblocks * V::blockSize,0x00) ;
  std::copy(&block[0],&block[V::blockSize],&table[0]) ;
  for (Lba i=1 ; i<nblocks ; ++i) {
    volume->read(start+i,&Range<uint8_t>(&table).sub<V::blockSize>(i*V::blockSize)) ;
  }
  return std::vector<uint8_t>(&table[8],&table[lastByte+1]) ;
}

Ifo::Map Ifo::readMap(V *volume,Lba start,Range<uint8_t const> table)
{
  Ifo::Map map ;
  for (uint32_t i=0 ; i<table.size() ; i+=12) {
    VtsNo vno = table[i+6] ;
    Ifo::Lba ofs = msbf(table.sub<4>(i+8)) ;
    Ifo::Map::iterator j = map.find(vno) ;
    if (j != map.end()) {
      if (j->second.start != ofs)
	throw Error("VTS address ambiguous") ;
    }
    else {
      uint8_t block[V::blockSize] ; 
      volume->read(start+ofs,&block) ;
      Lba ifoL      = msbf(CArray::sub<0x1c,0x20>(block)) ;
      Lba vobMenuS  = msbf(CArray::sub<0xc0,0xc4>(block)) ;
      Lba vobTitleS = msbf(CArray::sub<0xc4,0xc8>(block)) ;
      Lba bupL      = msbf(CArray::sub<0x0c,0x10>(block)) ;
      Vts vts(ofs,ifoL,vobMenuS,vobTitleS,bupL) ;
      map.insert(std::make_pair(vno,vts)) ;
    }
  }
  return map ;
}

Ifo Ifo::read(V *volume,Lba start)
{
  uint8_t block[V::blockSize] ; 
  volume->read(start,&block) ;
  Lba ifoL     = msbf(CArray::sub<0x1c,0x20>(block)) ;
  Lba vobF     = msbf(CArray::sub<0xc0,0xc4>(block)) ;
  Lba bupL     = msbf(CArray::sub<0x0c,0x10>(block)) ;
  Lba tableOfs = msbf(CArray::sub<0xc4,0xc8>(block)) ;
  std::vector<uint8_t> table = readTable(volume,start+tableOfs) ;
  Map map = readMap(volume,start,table) ;
  return Ifo(start,ifoL,vobF,bupL,map) ;
}
