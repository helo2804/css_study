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

#ifndef _Range_hpp_
#define _Range_hpp_

#include "Array.hpp"
#include <vector>

template <typename T>
struct Range
{
  template<typename U> friend class Range ;

  template<typename U> Range(Range<U> range) : n(range.n),p(range.p) {} 

  template<typename U,std::size_t N> Range(U (&carray)[N]) : n(N),p(carray) {} 

  template<typename U,std::size_t N> Range(U (&carray)[N],size_t pos1,size_t pos2) : n(pos2-pos1),p(&carray[pos1])
  {
    assert(pos1<=pos2) ;
    assert(pos2<=N) ;
  }

  template<std::size_t N> Range(T (*carray)[N]) : n(N),p(&(*carray)[0]) {} 

  template<std::size_t N> Range(T (*carray)[N],size_t pos1,size_t pos2) : n(pos2-pos1),p(&(*carray)[pos1])
  {
    assert(pos1<=pos2) ;
    assert(pos2<=N) ;
  }

  template<typename U,std::size_t N> Range(Array<U,N> const &array) : n(N),p(&array[0]) {} 

  template<std::size_t N> Range(Array<T,N> *array) : n(N),p(&(*array)[0]) {} 

  template<typename U> Range(std::vector<U> const &vector) : n(vector.size()),p(&vector[0]) {} 

  template<typename U> Range(std::vector<U> const &vector,size_t pos1,size_t pos2) : n(pos2-pos1),p(&vector[pos1]) 
  {
    assert(pos1<=pos2) ;
    assert(pos2<=vector.size()) ;
  } 

  Range(std::vector<T> *vector) : n(vector->size()),p(&(vector->front())) {} 

  Range(std::vector<T> *vector,size_t pos1,size_t pos2) : n(pos2-pos1),p(&(*vector)[pos1])
  {
    assert(pos1<=pos2) ;
    assert(pos2<=vector->size()) ;
  } 

  std::size_t size() const { return this->n ; }

  T& operator[] (std::size_t i) const { return this->p[i] ; }
    
  T* begin() const { return this->p         ; }
  T*   end() const { return this->p+this->n ; }

  Range sub(std::size_t pos1,std::size_t pos2) const
  { 
    assert(pos1 <= pos2) ;
    assert(pos2 <= this->n) ;
    return Range(pos2-pos1,this->p+pos1) ;
  }

  template <size_t N> T (&sub(size_t pos)const)[N]
  { 
    assert(pos+N <= this->n) ;
    return reinterpret_cast<T(&)[N]>(this->p[pos]) ;
  }

private:

  Range(std::size_t n,T *p) : n(n),p(p) {}

  std::size_t n ;

  T *p ;
} ;

template<typename T> bool operator== (Range<T> x,Range<T> y) 
{ 
  if (x.size() != y.size())
    return false ;
  return std::equal(x.begin(),x.end(),y.begin()) ; 
}

template<typename T> bool operator< (Range<T> x,Range<T> y) 
{ 
  return std::lexicographical_compare(x.begin(),x.end(),y.begin(),y.end()) ; 
}

template<typename T> bool operator!= (Range<T> x,Range<T> y) { return ! (x == y) ; }
template<typename T> bool operator>  (Range<T> x,Range<T> y) { return    y <  x  ; }
template<typename T> bool operator<= (Range<T> x,Range<T> y) { return ! (y <  x) ; }
template<typename T> bool operator>= (Range<T> x,Range<T> y) { return ! (x <  y) ; }

#endif // _Range_hpp_
