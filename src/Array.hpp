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

#ifndef _Array_hpp_
#define _Array_hpp_

#include <algorithm> // std::copy,size_t
#include <cassert>
#include <boost/static_assert.hpp>

template <typename T,size_t N> struct Array
{
  template<typename U,size_t M> friend class Array ;

  typedef T Type ;

  static size_t const size = N ;

  Array() {} 

  Array(T const (&array)[N]) { std::copy(array,array+N,this->a) ; } 

  // this is already covered by implicit operator conversion...
  // T      & operator[] (size_t i)       { return this->a[i] ; }
  // T const& operator[] (size_t i) const { return this->a[i] ; }
  // however, if it would be defined, the compiler might complain about
  // ambiguos [] definitions (if the index type is 'int' which can 
  // either be promoted to size_t, or the implicit conversion to a C-
  // array could be performed which uses 'int' as [] index)

  typedef T       (&reference_type      )[N] ;
  typedef T const (&reference_type_const)[N] ;

  operator reference_type_const() const { return this->a ; }
  operator reference_type()             { return this->a ; }

  reference_type_const carray() const { return this->a ; }
  reference_type       carray()       { return this->a ; }

private:

  T a[N] ;

} ;

template<typename T,size_t N> bool operator== (Array<T,N> const &a,Array<T,N> const &b) 
{ 
  return std::equal(&a[0],&a[N],&b[0]) ; 
}

template<typename T,size_t N> bool operator< (Array<T,N> const &a,Array<T,N> const &b) 
{ 
  return std::lexicographical_compare(&a[0],&a[N],&b[0],&b[N]) ; 
}

template<typename T,size_t N> bool operator!= (Array<T,N> const &a,Array<T,N> const &b) { return ! (a == b) ; }
template<typename T,size_t N> bool operator>  (Array<T,N> const &a,Array<T,N> const &b) { return    b <  a  ; }
template<typename T,size_t N> bool operator<= (Array<T,N> const &a,Array<T,N> const &b) { return ! (b <  a) ; }
template<typename T,size_t N> bool operator>= (Array<T,N> const &a,Array<T,N> const &b) { return ! (a <  b) ; }

template<typename T,size_t N,size_t M> Array<T,N+M> cling(T const (&a)[N],T const (&b)[M])
{ 
  Array<T,N+M> y ;
  std::copy(&a[0],&a[N],&y[0]) ;
  std::copy(&b[0],&b[M],&y[N]) ;
  return y ;
}

#endif // _Array_hpp_
