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
#include "Ui.h"

static void list(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: file" << std::endl 
      << "output: LBA, size in blocks, filename, gap in blocks, file sequence number (0..1190)" << std::endl
      ;
    argL.pop() ;
    return ;
  }
  Feature::V volume(argL.pop()) ;
  argL.finalize() ;
  Feature f = Feature::read(&volume) ;
  typedef std::vector<Feature::FNo> V ;
  V index = f.list() ;
  Feature::Lba prev = 0 ;
  for (V::const_iterator i=index.begin() ; i!=index.end() ; ++i) {
    Feature::FNo fno = (*i) ;
    Iso9660::Record::Optional const &r = f.record(fno) ;
    Feature::Lba nblocks = r->size() / Feature::V::blockSize ;
    std::cout 
      << std::setw( 7) << r->lba()           << " "
      << std::setw( 7) << nblocks            << " "
      << std::setw(21) << Feature::name(fno) << " " 
      << std::setw( 7) << (r->lba()-prev)    << " "
      << std::setw( 4) << fno                << std::endl 
      ;
    prev = r->lba() + nblocks ;
  }
}

void Ui::featureInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout
      << "usage: option ..."      << std::endl
      << "supported options are:" << std::endl 
      << "list: list feature details" << std::endl ;
  }
  else if (arg == "list") {
    list(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
