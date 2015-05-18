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

#ifndef _ArgL_h_
#define _ArgL_h_

#include "Error.h"
#include <list>
#include <string>
#include <boost/optional.hpp>

struct ArgL
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("ArgL:" + s) {}
  } ;

  static ArgL make(int argc,char *argv[]) ;

  // check if <arg> is present at the top of the list. if so, return 
  //   the next value and remove both from the list
  boost::optional<std::string> option(std::string const &arg) ;

  // same as above. however, if <arg> is not present, return <def>
  std::string option(std::string const &arg,std::string const &def) ;

  // pop first arg from list. throws if empty
  std::string pop() ;

  // returns first arg from list. throws if empty
  std::string const& peek() const ;

  // returns true if empty
  bool empty() const ;

  // throws exception if list is not empty (finalizes argument processing)
  void finalize() const ;

private:

  std::list<std::string> list ;

  ArgL(std::list<std::string> const &list) : list(list) {}

} ;

#endif // _ArgL_h_
