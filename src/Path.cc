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

#include "Path.h"

Path Path::make(std::string const &s)
{
  Path::L list ;
  size_t head = 0 ;
  size_t tail = s.find('/') ;
  while (tail != s.npos) {
    if (head < tail)
      list.push_back(s.substr(head,tail-head)) ;
    // else: ignore starting and consecutive '/' in input
    tail = s.find('/',head=tail+1) ;
  }
  if (head < s.length())
    list.push_back(s.substr(head)) ;
  // else: ignore final '/' in input (or empty input)
  return Path(list) ;
}

std::string Path::toString() const
{
  std::ostringstream os ;
  os << (*this) ;
  return os.str() ;
}

std::ostream& operator<< (std::ostream &os,Path const &path)
{
  Path::L::const_iterator i = path.begin() ;
  if (i != path.end()) {
    os << (*i) ;
    while (++i != path.end()) {
      os << '/' << (*i) ;
    }
  }
  return os ;
}
