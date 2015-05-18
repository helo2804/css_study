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

#ifndef _Domain_hpp_
#define _Domain_hpp_

#include <stdexcept>
#include <functional>

template <typename T,T L,T U,typename Less=std::less<T> >
struct Domain 
{
  typedef T Type ;

  static Type const lower = L ;

  static Type const upper = U ;

  static bool valid(Type const &v) 
  {
    return ! (Less()(v,lower) || Less()(upper,v)) ;
  }

  Domain() : v(lower) {}

  Domain(Domain const &domain) : v(domain.v) {}

  Domain(Type const &v) : v(v)
  {
    if (!valid(v))
      throw std::domain_error("Domain:out of range") ;
  }

  Domain& operator = (Type const &v) 
  {
    if (!valid(v))
      throw std::domain_error("Domain:out of range");
    this->v = v ;
    return (*this) ;
  }

  operator Type() const { return this->v ; }

  Type value() const { return this->v ; }

private:

  Type v ;
} ;

#endif // _Domain_hpp_
