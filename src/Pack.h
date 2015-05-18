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

#ifndef _Pack_h_
#define _Pack_h_

// Basic access to a DVD MPEG-2 program-stream Pack
//
// Supports only the following stream identifiers:
//
// BA    - pack
// BB    - system header
// BD    - PES-Ext. private-stream-1 with non-MPEG-audio or subpictures
// BE    - PES padding-stream (0xff)
// BF    - PES private-stream-2 with navigation data
// C0-C7 - PES-Ext. MPEG-audio-stream 0..7
// E0    - PES-Ext. video-stream 
//
// Each sector (2048 bytes) contains excatly one pack with one or two PES
//   packets as (BD), (BD,BE), (BF,BF), (Cx), (Cx,BE), (E0) or (E0,BE) 
//   where Cx is C0..C7

#include "Domain.hpp"
#include "Reader.h"
#include <vector>
#include <boost/optional.hpp>

struct Pack
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Pack:" + s) {}
  } ;

  struct PES 
  {
    struct Ext
    {
      bool scrambled ;
      uint16_t ofs ;
      Ext(bool scrambled,uint16_t ofs) : scrambled(scrambled), ofs(ofs) {}
    } ;

    uint8_t id ; // stream identifier
    boost::optional<Ext> ext ;
    uint16_t payload ; // actually the offset of the payload start
    uint16_t size ; // size of payload

    bool scrambled() const { return this->ext && this->ext->scrambled ; }

    PES(uint8_t id,boost::optional<Ext> ext,uint16_t payload,uint16_t size)
      : id(id),ext(ext),payload(payload),size(size) {}

    typedef Domain<unsigned,0u,2041u> Offset ;

    static std::vector<uint8_t> makePadding(Offset offset) ;
  } ;

  PES pes1 ;
  boost::optional<PES> pes2 ;

  Pack(PES const &pes1) : pes1(pes1) {}
  Pack(PES const &pes1,PES const &pes2) : pes1(pes1),pes2(pes2) {}

  static Pack parse(uint8_t const (&data)[0x800]) ;
  // note: if not scrambled, pes1 and pes2 correspond to the the actual
  // pack. however, if scrambled and the 2nd pes header would touch the 
  // scrambled area, then pes2 is not set.

  // return the PES with a payload at the 0x80 boundary
  PES const* at0x80() const ;
  // returns NULL if there is no payload at the boundary

private:

  static Pack parse_program_stream(Reader *reader) ;
  static void parse_pack_header   (Reader *reader) ;
  static void parse_system_header (Reader *reader) ;
  static PES  parse_PES_packet    (Reader *reader) ;
  static PES  parse_PES_packet2   (Reader *reader) ;
} ;

#endif // _Pack_h_
