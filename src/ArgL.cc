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

#include "ArgL.h"

ArgL ArgL::make(int argc,char *argv[])
{
  std::list<std::string> l ;
  for (int i=0 ; i<argc ; ++i) {
    l.push_back(argv[i]) ;
  }
  return ArgL(l) ;
}

boost::optional<std::string> ArgL::option(std::string const& arg)
{
  if (!this->list.empty()) {
    if (this->list.front() == arg) {
      this->list.pop_front() ;
      if (this->list.empty())
	throw Error(std::string("missing argument for option:") + arg) ;
      std::string option = this->list.front() ;
      this->list.pop_front() ;
      return boost::optional<std::string>(option) ;
    }
  }
  return boost::optional<std::string>() ;
}

std::string ArgL::option(std::string const& arg,std::string const &def) 
{
  boost::optional<std::string> option = this->option(arg) ;
  return option ? (*option) : def ;
}

std::string ArgL::pop()
{
  if (this->list.empty())
    throw Error("missing argument") ;
  std::string arg = this->list.front() ;
  this->list.pop_front() ;
  return arg ;
}

std::string const& ArgL::peek() const
{
  if (this->list.empty())
    throw Error("missing argument") ;
  return this->list.front() ;
}

bool ArgL::empty() const
{
  return this->list.empty() ;
}

void ArgL::finalize() const
{
  if (!this->list.empty())
    throw Error("too many arguments") ;
}

