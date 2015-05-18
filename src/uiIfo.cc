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
#include "Ifo.h"
#include "Iso9660.h"
#include "Ui.h"

static Ifo::Lba vmgStart(Ifo::V &volume,Iso9660::Descriptor const &d)
{
  std::string name = "VIDEO_TS/VIDEO_TS.IFO" ;
  Iso9660::Record::Optional record = d.root().find(&volume,Path::make(name)) ;
  if (!record) {
    throw Error(name + " not found") ;
  }
  return record->lba() ;
}

static void verifyVmg(Feature const &f,Ifo const &i,std::ostream &os) 
{
  Iso9660::Record::Optional const &ifo = f.record(0) ;
  Iso9660::Record::Optional const &vob = f.record(1) ;
  Iso9660::Record::Optional const &bup = f.record(2) ;
  if (!ifo) {
    os << "Error:VMG:IFO-file missing" << std::endl ;
    return ;
  }
  if (i.start != ifo->lba()) {
    os << "Error:VMG:IFO-start differs" << std::endl ;
    return ;
  }
  if (i.ifoL != ifo->size() / Ifo::V::blockSize -1) {
    os << "Error:VMG:IFO-size differs" << std::endl ;
    return ;
  }
  if (i.ifoL > i.bupL) {
    os << "Error:VMG:BUP-end before IFO-end" << std::endl ;
    return ;
  }
  Ifo::Lba bupF = i.bupL - i.ifoL ;
  if (i.vobF == 0) {
    if (vob && vob->size() != 0) {
      os << "Error:VMG:VOB-file supposed to be empty" << std::endl ;
      return ;
    }
  }
  else {
    if (i.ifoL >= i.vobF) {
      os << "Error:VMG:VOB-start before IFO-end" << std::endl ;
      return ;
    }
    if (!vob) {
      os << "Error:VMG:VOB-file missing" << std::endl ;
      return ;
    }
    if (i.start + i.vobF != vob->lba()) {
      os << "Error:VMG:VOB-start differs" << std::endl ;
      return ;
    }
    if (i.vobF + vob->size() / Ifo::V::blockSize > bupF) {
      os << "Error:VMG:VOB-file-size beyound BUP-start" << std::endl ;
      return ;
    }
  }
  if (!bup) {
    os << "Error:VMG:BUP-file missing" << std::endl ;
    return ;
  }
  if (i.start + bupF != bup->lba()) {
    os << "Error:VMG:BUP-start differs" << std::endl ;
    return ;
  }
  if (ifo->size() != bup->size()) {
    os << "Error:VMG:IFO-size differs from BUP-file-size" << std::endl ;
    return ;
  }
}

static void verifyVts(Feature const &f,Ifo const &i,Ifo::VtsNo no,std::ostream &os) 
{
  Ifo::Map::const_iterator it = i.map.find(no) ;
  if (it == i.map.end()) {
    if (f.exists(Feature::VNo(no))) 
      os << "Error:VTS#" << no << ":not in VMG table" << std::endl ;
    return ;
  }
  else if (!f.exists(Feature::VNo(no))) {
    os << "Error:VTS#" << no << ":files missing" << std::endl ;
    return ;
  }
  Ifo::Vts const &vts = it->second ;
  int ofs = 3 + 12 * (no-1) ;
  Iso9660::Record::Optional const &ifo  = f.record(ofs+ 0) ;
  Iso9660::Record::Optional const &vob0 = f.record(ofs+ 1) ;
  Iso9660::Record::Optional const &vob1 = f.record(ofs+ 2) ;
  Iso9660::Record::Optional const &bup  = f.record(ofs+11) ;
  if (!ifo) {
    os << "Error:VTS#" << no << ":IFO-file missing or too small" << std::endl ;
    return ;
  }
  if (i.start + vts.start != ifo->lba()) {
    os << "Error:VMG:IFO-start differs" << std::endl ;
    return ;
  }
  if (vts.ifoL != ifo->size() / Ifo::V::blockSize -1) {
    os << "Error:VTS#" << no << ":IFO-size differs" << std::endl ;
    return ;
  }
  if (vts.ifoL > vts.bupL) {
    os << "Error:VTS#" << no << ":BUP-end before IFO-end" << std::endl ;
    return ;
  }
  Ifo::Lba bupF = vts.bupL - vts.ifoL ;
  if (vts.vobMenuF == 0) {
    if (vob0 && vob0->size() != 0) {
      os << "Error:VTS#" << no << ":VOB0-file supposed to be empty" << std::endl ;
      return ;
    }
  }
  else {
    if (vts.ifoL >= vts.vobMenuF) {
      os << "Error:VTS#" << no << ":VOBM-start before IFO-end" << std::endl ;
      return ;
    }
    if (!vob0) {
      os << "Error:VTS#" << no << ":VOB0-file missing" << std::endl ;
      return ;
    }
    if (i.start + vts.start + vts.vobMenuF != vob0->lba()) {
      os << "Error:VTS#" << no << ":VOB0-start differs" << std::endl ;
      return ;
    }
  }
  if (vts.vobMenuF != 0) {
    if (vts.vobMenuF + vob0->size() / Ifo::V::blockSize > vts.vobTitleF) {
      os << "Error:VTS#" << no << ":VOBM-file-size beyound VOBT-start" << std::endl ;
      return ;
    }
  }  
  if (!vob1) {
    os << "Error:VTS#" << no << ":VOB#1 missing" << std::endl ; 
    return ;
  }
  if (i.start + vts.start + vts.vobTitleF != vob1->lba()) {
    os << "Error:VTS#" << no << ":VOBT-start differs" << std::endl ;
    return ;
  }
  Ifo::Lba blocks = vob1->size() / Ifo::V::blockSize ;
  bool eot = false ;
  for (int j=3 ; j<11 ; ++j) {
    Iso9660::Record::Optional const &vobx  = f.record(ofs+j) ;
    if (vobx) {
      if (eot)
	os << "Warning:VTS#" << no << ":gap before VOB#" << (j-1) << std::endl ; 
      if (vobx->lba() != vob1->lba() + blocks) {
	os << "Error:VTS#" << no << ":not contiguous to VOB#" << (j-1) << std::endl ; 
	return ;
      }
      blocks += vobx->size() / Ifo::V::blockSize ;
    }
    eot = !vobx ;
  }
  if (!bup) {
    os << "Error:VTS#" << no << ":BUP-file missing" << std::endl ;
    return ;
  }
  if (i.start + vts.start + bupF != bup->lba()) {
    os << "Error:VTS#" << no << ":BUP-start differs" << std::endl ;
    return ;
  }
  if (ifo->size() != bup->size()) {
    os << "Error:VTS#" << no << ":IFO- and BUP-file-size differ" << std::endl ;
    return ;
  }
}

// --------------------------------------------------------------------

static void list(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: file" << std::endl
      << "output first line (VMG): first IFO, last IFO, first VOB, last BUP" << std::endl 
      << "output next lines (VTS): number, first IFO, last IFO, first VOBM, first VOBT, last BUP" << std::endl
      << "all values as LBA, first IFO LBA absolutely, other LBAs as offset" << std::endl ;
    argL.pop() ;
    return ;
  }
  Ifo::V volume(argL.pop()) ; 
  argL.finalize() ;

  Iso9660::Descriptor d = Iso9660::Descriptor::find(&volume) ;
  Ifo::Lba start = vmgStart(volume,d) ;
  Ifo ifo = Ifo::read(&volume,start) ;

  std::cout << ifo.start << " "
	    << ifo.ifoL  << " "
	    << ifo.vobF  << " "
	    << ifo.bupL << std::endl ;

  for (Ifo::Map::const_iterator i=ifo.map.begin() ; i!=ifo.map.end() ; ++i) {
    Ifo::VtsNo vtsNo = i->first ;
    Ifo::Vts const &vts = i->second ;
    std::cout << vtsNo << " " 
	      << vts.start << " "
	      << vts.ifoL  << " "
	      << vts.vobMenuF  << " "
	      << vts.vobTitleF << " "
	      << vts.bupL  << std::endl ;
  }
}

static void verify(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: file" << std::endl 
      << "output: any inconsistencies (no output if none)" << std::endl ;
    argL.pop() ;
    return ;
  }
  Ifo::V volume(argL.pop()) ; 
  argL.finalize() ;
  Iso9660::Descriptor d = Iso9660::Descriptor::find(&volume) ;
  Ifo::Lba start = vmgStart(volume,d) ;
  Ifo ifo = Ifo::read(&volume,start) ;
  Feature feature = Feature::read(&volume) ;
  verifyVmg(feature,ifo,std::cout) ;
  for (unsigned i=Ifo::VtsNo::lower ; i<=Ifo::VtsNo::upper ; ++i)
    verifyVts(feature,ifo,i,std::cout) ;
}

// --------------------------------------------------------------------

void Ui::ifoInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "list:   List IFO details" << std::endl
      << "verify: Verify IFO data with feature (file-system) data" << std::endl ;
  }
  else if (arg == "list") {
    list(argL) ;
  }
  else if (arg == "verify") {
    verify(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
