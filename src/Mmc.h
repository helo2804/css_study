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

#ifndef _Mmc_h_
#define _Mmc_h_

#include "Array.hpp"
#include "Domain.hpp"
#include "Sg.h"

#include <list>
#include <string>
#include <vector>

namespace Mmc
{
  typedef Array<uint8_t, 5>  B5 ;
  typedef Array<uint8_t,10> B10 ;

  // authentification grant identifier
  typedef Domain<unsigned,0,3> AGID ;

  struct Error : public ::Error
  {
    Sg::Status status ;
    Error(Sg::Status const &status) : ::Error("Mmc:" + status.toString()), status(status) {}
  } ;

  // (key,value,info) tuple
  struct Record {
    std::string key ;
    uint32_t value ;
    std::string info ;
    Record(std::string const &key,uint32_t value, std::string const &info)
    : key(key),value(value),info(info) {}
  } ;

  struct Cmi { // copyright management information (CPR_MAI)
    uint8_t flags ;
    int copyright() const ; 
    // 0 : no copyright
    // 1 : copying is permitted without restriction
    // 2 : reserved
    // 3 : one generation of copies may be made
    // 4 : no copying is allowed
    int mode() const ; 
    // 0 : no copyright protection mode
    // 1 : CSS
    // 2 : CPPM
    // 3..16 : not defined
    Cmi(uint8_t flags) : flags(flags) {}
  } ;

  // (12) inquiry
  struct Inquiry {
    std::string vendor ;     //  8 characters
    std::string productId ;  // 16 characters
    std::string productRev ; //  4 characters
    Inquiry(std::string const &vendor,
	    std::string const &productId, 
	    std::string const &productRev)
    : vendor(vendor),productId(productId),productRev(productRev) {}
  } ;
  Inquiry inquiry(int fd,unsigned int timeout=10000) ;

  // (25) capacity of medium
  struct Capacity {
    uint32_t lba ; // last lba
    uint32_t bsize ; // logical block size (always 2048)
    Capacity(uint32_t lba,uint32_t bsize) : lba(lba),bsize(bsize) {}
  } ;
  Capacity readCapacity(int fd,unsigned int timeout=10000) ;

  // (28) read(10)
  std::vector<uint8_t> read10(int fd,uint32_t lba,uint16_t nblocks,bool fua=0,unsigned int timeout=10000) ;

  // (A3:01) send nonce #1 for key #1 to LU
  void sendChallenge(int fd,AGID agid,uint8_t const (&nonce)[10],unsigned int timeout=10000) ;
  // 05:2C00 illegal request: command sequence error

  // (A3:03) send key #2 to LU
  void sendKey(int fd,AGID agid,uint8_t const (&key)[5],unsigned int timeout=10000) ;
  // 05:2C00 illegal request: command sequence error
  // 05:6F00 illegal request: copy protection key exchange failure - authentication failure

  // (A4:00) get authentification grant identifier from LU
  AGID reportAgid(int fd,unsigned int timeout=10000) ;
  // 05:2C00 illegal request: command sequence error
  // 05:3002 illegal request: cannot read medium - incompatible format (for DVD-R on HL-DT-ST/GDR8164B/0L06)

  // (A4:01) get nonce #2 for key #2 from LU
  B10 reportChallenge(int fd,AGID agid,unsigned int timeout=10000) ;
  // 05:2C00 illegal request: command sequence error

  // (A4:02) get key #1 from LU
  B5 reportKey(int fd,AGID agid,unsigned int timeout=10000) ;
  // 05:2C00 illegal request: command sequence error
  // 05:6F01 illegal request: copy protection key exchange failure - key not present (on HL-DT-ST/GDR8164B/0L06)

  // (A4:04) get sector title data from LU
  struct /* Sector */ Title /* Data */ {
    Cmi cmi ;
    B5 key ; // the key is obfuscated by the buskey
  Title(Cmi const &cmi,uint8_t const (&key)[5]) : cmi(cmi),key(key) {}
    void deobfuscate(uint8_t const (&buskey)[5]) ;
  } ;
  Title reportTitle(int fd,AGID agid,uint32_t lba,unsigned int timeout=10000) ;
  // 05:6F01 illegal request: copy protection key exchange failure - key not present (not all drives)
  // 05:6F02 illegal request: copy protection key exchange failure - key not established
  // 05:6F04 illegal request: media region code is mismatched to logical unit region

  // (A4:05) get authentication success flag from LU and invalidate AGID
  bool reportAsf(int fd,AGID agid,unsigned int timeout=10000) ;
  // appears that no errors are ever thrown (e.g. sequence error)
  // may report success/true even if the AGID is wrong (AGID seems to be ignored)

  // (A4:08) get drive's region code et al.
  std::list<Record> reportRpcState(int fd,unsigned int timeout=10000) ;
  // reports:
  // - "typeCode": e.g. 1 - "drive region is set"
  // - "numberOfVendorResetsAvailable" e.g. 4 
  // - "numberOfUserControlledChangesAvailable": e.g. 4
  // - "regionMask": e.g. 1111 1101 (for region 2)
  // - "rpcScheme": e.g. 1 - "drive region adheres to all CSS requirements (RPC II)"

  // (A4:3F) invalidate authentification grant identifier
  void reportInvalidateAgid(int fd,AGID agid,unsigned int timeout=10000) ;
  // appears that no errors are ever thrown

  // (AD:00) physical format information
  std::list<Record> readDiscInfo(int fd,uint8_t layer=0,unsigned int timeout=10000) ;
  // reports:
  // - "diskCategory": e.g. 0 - "DVD-ROM"
  // - "partVersion": e.g. 1
  // - "discSize": e.g. 0 - "120 mm"
  // - "maximumRate": e.g. 2 - "10.08 Mbps"
  // - "numberOfLayers": 1 (means two layers)
  // - "track": e.g. 1 - "Opposite Track Path (OTP)"
  // - "layerType": e.g. 1 - "embossed"
  // - "linearyDensity": e.g. 1 - "0.293 um/bit"
  // - "trackDensity": e.g. 0 - "0.74 um/bit"
  // - "startSector": e.g. 0x30000 (always on DVD-ROM)
  // - "endSector": e.g. 0xfcfffc
  // - "endSectorLayer0": e.g. 0x223faf
  // - "bca": 0 - "BCA data does not exist"

  // (AD:01) read (disc) copyright information 
  struct Copyright {
    uint8_t type ; // 0:none 1:CSS/CPPM 2:CPRM
    uint8_t regionCode ; // permitted in region if corresponding bit is zero
    Copyright(uint8_t type,uint8_t regionCode)
    : type(type),regionCode(regionCode) {}
  } ;
  Copyright readDiscCopyright(int fd,unsigned int timeout) ;

  // (AD:02) get disc key (data) from LU
  struct DiscKey /* Block */ : public Array<B5,409> {
    // decrypt: 
    //   disc_key   x key(0)      -> disc_key (identity hash)
    //   player_key x key(1..408) -> disc_key 
    DiscKey(Array<B5,409> const &keyV) : Array<B5,409>(keyV) {}
    // keys are obfuscated by bus key until unscrambled
    void deobfuscate(uint8_t const (&buskey)[5]) ;
  } ;
  DiscKey readDiscKey(int fd,AGID agid,unsigned int timeout=10000) ;
  // 05:6F01 illegal request: copy protection key exchange failure - key not present
  // 05:6F02 illegal request: copy protection key exchange failure - key not established
  // note: surprisingly no command sequence error seen yet

  // (AD:05) Copyright Management Information (disc)
  Cmi readDiscCmi(int fd,uint32_t lba,unsigned int timeout) ;

}

#endif // _Mmc_h_
