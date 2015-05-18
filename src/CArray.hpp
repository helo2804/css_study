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

#ifndef _CArray_hpp_
#define _CArray_hpp_

#include <algorithm> // std::copy,size_t
#include <cassert>
#include <boost/static_assert.hpp>

namespace CArray
{
  template <size_t POS1,size_t POS2,typename T,size_t N> 
  inline T (*sub(T (*x)[N]))[POS2-POS1]
  { 
    BOOST_STATIC_ASSERT(POS1<=POS2) ;
    BOOST_STATIC_ASSERT(POS2<=N) ;
    return reinterpret_cast<T(*)[POS2-POS1]>(&(*x)[POS1]) ;
  }
  
  template <size_t POS1,size_t POS2,typename T,size_t N> 
  inline T const (&sub(T const (&x)[N]))[POS2-POS1]
  { 
    BOOST_STATIC_ASSERT(POS1<=POS2) ;
    BOOST_STATIC_ASSERT(POS2<=N) ;
    return reinterpret_cast<T const(&)[POS2-POS1]>(x[POS1]) ;
  }

  template <size_t M,typename T,size_t N> 
  inline T const (&sub(T const (&x)[N],size_t pos))[M]
  { 
    assert(pos+M<=N) ;
    return reinterpret_cast<T const(&)[M]>(x[pos]) ;
  }
  
  template <typename T,size_t N> 
  inline void copy(T const (&a)[N],T (*b)[N])
  { 
    std::copy(&a[0],&a[N],&(*b)[0]) ;
  }

  template <typename T,size_t N> 
  inline bool equal(T const (&a)[N],T const (&b)[N])
  { 
    return std::equal(&a[0],&a[N],&b[0]) ;
  }

}

#endif // _CArray_hpp_
