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
#include "Format.h"

std::string const& Feature::name(FNo fno)
{
  static bool valid = false ;
  static std::string nameA[FNo::upper+1] ;
  if (!valid) {
    // vno = 0
    nameA[0] = "VIDEO_TS/VIDEO_TS.IFO" ;
    nameA[1] = "VIDEO_TS/VIDEO_TS.VOB" ;
    nameA[2] = "VIDEO_TS/VIDEO_TS.BUP" ;
    for (unsigned vtsNo=VtsNo::lower ; vtsNo<=VtsNo::upper ; ++vtsNo) {
      unsigned ofs = 12 * (vtsNo-1) + 3 ;
      nameA[ofs+0] = Format::sprintf("VIDEO_TS/VTS_%02u_0.IFO",vtsNo) ;
      for (int sq=0 ; sq<10 ; ++sq)
	nameA[ofs+1+sq] = Format::sprintf("VIDEO_TS/VTS_%02u_%d.VOB",vtsNo,sq) ;
      nameA[ofs+11] = Format::sprintf("VIDEO_TS/VTS_%02u_0.BUP",vtsNo) ;
    }
    valid = true ;
  }
  return nameA[fno] ;
}

Feature Feature::read(V *volume) 
{
  Iso9660::Descriptor d = Iso9660::Descriptor::find(volume) ;
  FTable table = read(volume,d) ;
  return Feature(table) ;
}

Feature::FTable Feature::read(V *volume,Iso9660::Descriptor const &d) 
{
  FTable table ;
  for (unsigned i=0 ; i<table.size ; ++i) {
    // it would be faster to read the directory; however, this here is simpler
    Iso9660::Record::Optional r = d.root().find(volume,Path::make(name(i))) ;
    if (r) {
      if (0 != (r->size() % V::blockSize))
	throw Error("file size not multiple of 2048:" + name(i)) ;
      table[i] = r ;
    }
  }
  return table ;
}

bool Feature::exists(VNo vno) const
{
  bool yes = false ;
  if (vno == 0) {
    yes |= bool(this->table[0]) ;
    yes |= bool(this->table[1]) ;
    yes |= bool(this->table[2]) ;
  }
  else {
    int ofs = 3 + 12*(vno-1) ;
    for (unsigned i=0 ; i<12 ; ++i)
      yes |= bool(this->table[ofs+i]) ;
  }
  return yes ;
}

std::vector<Feature::FNo> Feature::list() const
{
  std::vector<FNo> index ; 
  index.reserve(FNo::upper+1) ;
  for (FNo::Type i=FNo::lower ; i<=FNo::upper ; ++i)
    if (this->table[i])
      index.push_back(i) ;
  return index ;
}

std::vector<Feature::Lba> Feature::tellFile(FNo fno) const
{
  std::vector<Lba> result ;
  Iso9660::Record::Optional record = this->table[fno] ;
  if (record) {
    size_t size = record->size() / V::blockSize ;
    result.reserve(size) ;
    for (Lba i=0 ; i<size ; ++i)
      result.push_back(record->lba() + i) ;
  }
  return result ;
} 

std::vector<Feature::Lba> Feature::tellVob(VtsNo vtsNo) const
{
  std::vector<Lba> v[10] ;
  unsigned ofs = 3 + 12*(vtsNo-1) + 1 ;
  size_t size = 0 ;
  for (unsigned i=0 ; i<10 ; ++i) {
    v[i] = this->tellFile(ofs+i) ;
    size += v[i].size() ;
  }
  std::vector<Lba> r ; r.reserve(size) ;
  for (unsigned i=0 ; i<10 ; ++i) {
    r.insert(r.end(),v[i].begin(),v[i].end()) ;
  }
  return r ;
}

std::vector<Feature::Lba> Feature::tellVob(VNo vno) const
{
  if (vno == 0)
    return this->tellFile(1) ;
  return this->tellVob(VtsNo(vno)) ;
}

std::vector<Feature::Lba> Feature::tellVob() const
{
  std::vector<Lba> v[VNo::upper+1] ;
  size_t size = 0 ;
  for (VNo::Type i=VNo::lower ; i<=VNo::upper ; ++i) {
    v[i] = this->tellVob(VNo(i)) ;
    size += v[i].size() ;
  }
  std::vector<Lba> r ; r.reserve(size) ;
  for (VNo::Type i=VNo::lower ; i<=VNo::upper ; ++i) {
    r.insert(r.end(),v[i].begin(),v[i].end()) ;
  }
  return r ;
}



