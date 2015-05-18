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

#include "Spread.h"

struct Item 
{
  ByteGen gn ; // generator after n keystream bytes
  uint8_t sn ;  // last generated keystream byte
  size_t nbits ; // number of (unique) keystream bits
  Item *next ; // another item with the same keystream bits 
  // (set to NULL after split() finished)
  Item(ByteGen const &gn,Item *next) : gn(gn),sn(0),nbits(0),next(next) {}
  static std::vector<Item> prepare(Range<ByteGen const> g) ;
  static void split(Item *item) ;
} ;

std::vector<Item> Item::prepare(Range<ByteGen const> g)
{
  std::vector<Item> v ; v.reserve(g.size()) ;
  for (size_t i=0 ; i<g.size()-1 ; ++i) {
    v.push_back(Item(g[i],&v[i+1])) ;
  }
  v.push_back(Item(g[g.size()-1],NULL)) ;
  return v ;
}

void Item::split(Item *item)
{
  // <item> is the first entry in a linked list
  if (item->next == NULL)
    return ;
  Item *next[2] = { NULL,NULL } ;
  unsigned bofs = item->nbits % 8 ;
  do {  
    if (bofs == 0) {
      item->sn = item->gn.shift() ;
    }
    int ix = 0x1 & (item->sn >> bofs) ;
    ++item->nbits ;
    // add item to linked list in next[ix]
    std::swap(item,next[ix]) ;
    std::swap(item,next[ix]->next) ;
  }
  while (item != NULL) ;
  // at this point all <item> have been placed either in next[0] or in next[1] list
  if (next[0] != NULL) split(next[0]) ;
  if (next[1] != NULL) split(next[1]) ;
  // recursively called until no Item has a next pointer
}

std::vector<Spread::Record> Spread::split(uint8_t const (&head)[3],bool invertA,bool invertB,bool valid)
{
  std::vector<ByteGen> gV = valid
    ? Revert::head   (head,invertA,invertB)
    : Revert::snippet(head,invertA,invertB) ;
  std::vector<Item> iV = Item::prepare(gV) ;
  Item::split(&iV[0]) ;
  std::vector<Record> result ; result.reserve(iV.size()) ;
  for (std::vector<Item>::const_iterator i=iV.begin() ; i!=iV.end() ; ++i) {
    result.push_back(Record(i->gn,3*8+i->nbits)) ;
    // 3 bytes were already covered by the keystream head
  }
  return result ;
}
