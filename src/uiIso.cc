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

#include "Format.h"
#include "Iso9660.h"
#include "Os.h"
#include "Ui.h"

#include <fstream>

using namespace Iso9660 ;

static void date(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: file" << std::endl ;
    argL.pop() ;
    return ;
  }
  Iso9660::V volume(argL.pop()) ;
  argL.finalize() ;
  Descriptor d = Descriptor::find(&volume) ;
  std::string date = d.date() ;
  std::cout << date.substr(0,4) << "-" 
	    << date.substr(4,2) << "-" 
	    << date.substr(6,2) << std::endl ;
}

static void id(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: file" << std::endl ;
    argL.pop() ;
    return ;
  }
  Iso9660::V volume(argL.pop()) ;
  argL.finalize() ;
  Descriptor d = Descriptor::find(&volume) ;
  std::cout << d.id() << std::endl ;
}

static void list(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "argument: file" << std::endl
      << "output: LBA, size in bytes, size in blocks, record-name" << std::endl
      << "  directory names end with '/'" << std::endl 
      << "  file names end with '#' followed by the numeric ID (0..32767)" << std::endl 
      ;
    argL.pop() ;
    return ;
  }
  Iso9660::V volume(argL.pop()) ;
  argL.finalize() ;
  Descriptor d = Descriptor::find(&volume) ;
  typedef std::pair<Path,Record> Pair ;
  std::list<Pair> queue(1,std::make_pair(Path(),d.root())) ;
  while (!queue.empty()) {
    Pair pair = queue.front() ; queue.pop_front() ;
    Path const &path = pair.first ;
    Record const &r = pair.second ;
    uint32_t nblocks = (r.size() + V::blockSize - 1) / V::blockSize ;
    std::cout 
      << std::setw( 7) << r.lba()  << " " 
      << std::setw(10) << r.size() << " "
      << std::setw( 7) << nblocks  << " " 
      << Format::escape(path.toString()) // \x00 and \x01 for directories
      ;
    if (r.isDir()) std::cout << "/" ;
    else  	   std::cout << "#" << Record::version(r.id()) ;
    std::cout << std::endl ;
    if (!r.isDir())
      continue ;
    if (!path.empty()) {
      if (path.back() == std::string(1,'\x00'))
	continue ;
      if (path.back() == std::string(1,'\x01'))
	continue ;
    }
    Record::FileSizeType seek = 0 ;
    Record::Optional o = r.list(&volume,&seek) ;
    while (o) {
      Path child(path) ; 
      child.push_back(Record::name(o->id())) ;
      queue.push_back(std::make_pair(child,*o)) ;
      o = r.list(&volume,&seek) ;
    }
  }
}

static void path(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: path" << std::endl ;
    argL.pop() ;
    return ;
  }
  std::string path = argL.pop() ;
  argL.finalize() ;
  std::cout << Path::make(path).toString() << std::endl ;
}

void Ui::isoInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: option ..." << std::endl
      << "supported options are..." << std::endl 
      << "date: Show date of the ISO-9660 image" << std::endl
      << "id:   Show id of the ISO-9660 image" << std::endl
      << "list: List all files and directories in the ISO-9660 image" << std::endl 
      << "path: Convert string to internal path and back" << std::endl ;
  }
  else if (arg == "date") {
    date(argL) ;
  }
  else if (arg == "id") {
    id(argL) ;
  }
  else if (arg == "list") {
    list(argL) ;
  }
  else if (arg == "path") {
    path(argL) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
