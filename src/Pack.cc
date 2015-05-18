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

#include "Pack.h"

std::vector<uint8_t> Pack::PES::makePadding(Offset offset)
{
  uint16_t size = Offset::upper - offset + 1 ;
  std::vector<uint8_t> pes(6+size,0xff) ;
  pes[0] = 0x00 ;
  pes[1] = 0x00 ;
  pes[2] = 0x01 ;
  pes[3] = 0xbe ;
  pes[4] = (size >> 8) ;
  pes[5] = size ;
  return pes ;
}

Pack Pack::parse(uint8_t const (&data)[0x800])
{
  Reader r(data) ;
  return parse_program_stream(&r) ;
}

Pack Pack::parse_program_stream(Reader *r)
{
  r->expect(4,0x000001BA) ;
  parse_pack_header(r) ;
  // first and mandatory PES packet
  PES pes1 = parse_PES_packet(r) ;
  if (r->eof()) {
    return Pack(pes1) ;
  }
  if (pes1.scrambled() && (r->tell() > 0x80-6)) {
    // don't dare parsing since that will hit the scrambled part
    return Pack(pes1) ;
  }
  PES pes2 = parse_PES_packet2(r) ;
  if (!r->eof()) {
    throw Error("unexpected data at pack end") ;
  }
  return Pack(pes1,pes2) ;
}

void Pack::parse_pack_header(Reader *r)
{
  r->pop(9) ;
  uint8_t pack_stuffing_length = 0x07 & r->get(1) ;
  for (uint8_t i=0 ; i<pack_stuffing_length ; ++i) {
    r->expect(1,0xff) ;
  }
  if (r->peek(4) == 0x000001BB) {
    r->pop(4) ;
    parse_system_header(r) ;
  }
}

void Pack::parse_system_header(Reader *r)
{
  uint16_t header_length = r->get(2) ;
  r->pop(header_length) ;
}
  
Pack::PES Pack::parse_PES_packet(Reader *r)
{
  r->expect(3,0x000001) ;
  uint8_t id = r->get(1) ;
  uint16_t pes_packet_length = r->get(2) ;
  uint16_t size ;
  boost::optional<PES::Ext> ext ;
  if ( (id == 0xbd) || ((0xc0 <= id) && (id <= 0xc7)) || (id == 0xe0) ) {
    if (pes_packet_length < 3) {
      throw Error("packet too short") ;
    }
    uint8_t copyright = 0x30 & r->get(1) ;
    bool scrambled = (0x00 != copyright) ;
    ext = PES::Ext(scrambled,r->tell()-1) ;
    r->get(1) ; // not used here
    uint8_t header_length = r->get(1) ;
    if (header_length + 3 > pes_packet_length) {
      throw Error("invalid header length") ;
    }
    r->pop(header_length) ;
    size = pes_packet_length - 3 - header_length ;
  }
  else if ((id == 0xbe) || (id == 0xbf)) {
    size = pes_packet_length ;
  }
  else {
    throw Error("stream id not supported") ;
  }
  uint16_t payload = r->tell() ;
  r->pop(size) ;
  return PES(id,ext,payload,size) ;
}

Pack::PES Pack::parse_PES_packet2(Reader *r)
{
  r->expect(3,0x000001) ;
  uint8_t id = r->get(1) ;
  uint16_t pes_packet_length = r->get(2) ;
  if ((id != 0xbe) && (id != 0xbf)) 
    throw Error("stream id not supported in 2nd PES") ;
  uint16_t size = pes_packet_length ;
  uint16_t payload = r->tell() ;
  r->pop(size) ;
  return PES(id,boost::optional<PES::Ext>(),payload,size) ;
}

Pack::PES const* Pack::at0x80() const
{
  Pack::PES const *pes = &this->pes1 ;
  if (pes->payload > 0x80) {
    return NULL ;
    // there 'should' be always a payload before 0x80
  }
  if (pes->payload + pes->size >= 0x80) {
    // this 'should' actually be 0x800 if there is no 2nd PES
    return pes ;
  }
  if (!this->pes2) {
    return NULL ; 
    // this 'should' never happen at this point. 
  }
  pes = &(*this->pes2) ;
  if (pes->payload > 0x80) {
    return NULL ;
    // if 1st PES ends shortly before the 0x80 border, the header of
    // the 2nd PES may consume the space at the 0x80 border. so there
    // is no payload imemdiately before 0x80 at all.
  }
  if (pes->payload + pes->size >= 0x80) {
    // this 'should' actually be always 0x800 for 2nd PES
    return pes ;
  }
  return NULL ;
  // this 'should' never happen at this point
}

