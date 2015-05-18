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
#include "Iso9660.h"
#include <algorithm> // std::min

using namespace Iso9660 ;

static uint32_t msbf(uint8_t const(&src)[4])
{
  uint32_t dst = src[0] ;
  dst <<= 8 ; dst |= src[1] ;
  dst <<= 8 ; dst |= src[2] ;
  dst <<= 8 ; dst |= src[3] ;
  return dst ;
}

Record::Optional Iso9660::Record::find(V *volume,std::string const &name) const
{
  FileSizeType seek = 0 ;
  Record::Optional record = this->list(volume,&seek) ;
  while (record) {
    if (Record::name(record->id()) == name)
      return (*record) ;
    record = this->list(volume,&seek) ;
  }
  return Optional() ;
}

Record::Optional Iso9660::Record::find(V *volume,Path const &path) const
{
  Optional record(*this) ;
  for (Path::const_iterator i=path.begin() ; i!=path.end() && record ; ++i) {
    std::string const &name = (*i) ;
    record = record->find(volume,name) ;
  }
  return record ;
}

std::string Iso9660::Record::id() const
{
  if (this->data.size() < 33u) 
    throw Error("record too short") ;
  uint8_t len = this->data[32] ;
  if (this->data.size() < 33u+len) 
    throw Error("record too short") ;
  return std::string(&this->data[33],&this->data[33+len]) ;
}

bool Iso9660::Record::isDir() const 
{
  if (this->data.size() < 25u+1) 
    throw Error("record too short") ;
  return (this->data[25] & 0x02) == 0x02 ;
}

Lba Iso9660::Record::lba() const 
{
  if (this->data.size() < 6u+4) 
    throw Error("record too short") ;
  return msbf(Range<uint8_t const>(this->data).sub<4>(6)) ;
}

Record::Optional Iso9660::Record::list(V *volume,FileSizeType *seek) const 
{
  uint8_t block[V::blockSize] ; 
  V::BlockSizeType blockOfs ;
  uint8_t recordLen ;
  while (true) {
    if ((*seek) >= this->size())
      return Record::Optional() ;
    Lba lbaOfs = (*seek) / V::blockSize ;
    blockOfs = (*seek) % V::blockSize ;
    volume->read(this->lba() + lbaOfs,&block) ;
    // this reads the same sector again and again which is less than ideal
    V::BlockSizeType left = std::min(this->size()-lbaOfs*V::blockSize,V::blockSize) ;
    recordLen = block[blockOfs] ;
    if (recordLen > left - blockOfs) {
      throw Error("record exceeds directory") ;
    }
    if (recordLen != 0) {
      break ;
    }
    // directory records do not wander across sector boundaries. that 
    // is, the end of a sector is padded with zeros. hence, we continue
    // with the next sector (w/o verifying whether really zero-padded).
    (*seek) += V::blockSize - blockOfs ;
    // this would also accept empty sectors (not forbidden though)
  }
  std::vector<uint8_t> data(&block[blockOfs],&block[blockOfs+recordLen]) ;
  (*seek) += recordLen ;
  return Record(data) ;
}

std::string Iso9660::Record::name(std::string const &id)
{
  return id.substr(0,id.rfind(';')) ;
}

Record::FileSizeType Iso9660::Record::size() const 
{
  if (this->data.size() < 14u+4) 
    throw Error("record too short") ;
  return msbf(Range<uint8_t const>(this->data).sub<4>(14)) ;
}

Record::Version Iso9660::Record::version(std::string const &id)
{
  Version version = 0 ;
  size_t pos = id.rfind(';') ;
  if (pos != std::string::npos) {
    version = Format::number<typename Version::Type>(id.substr(pos+1)) ;
    if (version < 1) {
      throw Error("not supported file version number") ;
    }
  }
  return version ;
  // note: according to the ISO specification, all file names (but not 
  // directory names) contain at the end a version number. That is, the
  // file name ends with a ";" (semi-colon) followed by a number in the 
  // range 1..32767. However, actually, there are ISO images which do
  // not conform (semicolon and number are missing).
}

// ----[ Primary ISO Descriptor ]--------------------------------------

Descriptor Iso9660::Descriptor::find(V *volume) 
{
  Data data ; 
  Lba lba = 16 ;
  do {
    volume->read(lba,&data.carray()) ;
    static std::string const id("CD001\x01") ;
    if (id != std::string(&data[1],&data[7])) {
      throw Error("unknown descriptor type") ;
    }
    if (data[0] == 0xff) {
      throw Error("primary descriptor not found") ;
    }
    ++lba ;
  }
  while (data[0] != 0x01) ;
  return Descriptor(data) ;
}

std::string Iso9660::Descriptor::id() const 
{
  std::string s(&this->data[40],&this->data[72]) ;
  return s.substr(0,s.find(' ')) ;
}

std::string Iso9660::Descriptor::date() const 
{
  std::string s(&this->data[813],&this->data[830]) ;
  return s.substr(0,s.find(' ')) ;
}

Record Iso9660::Descriptor::root() const
{
  Record root(std::vector<uint8_t>(&this->data[0x9c],&this->data[0xbe])) ;
  if (root.id() != std::string("\x00",1)) {
    throw Error("invalid root directory id") ;
  }
  return root ;
}
