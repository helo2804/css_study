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

#include "CArray.hpp"
#include "Format.h"
#include "Logging.h"
#include "Mmc.h"

#include <fcntl.h>
#include <scsi/sg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

using namespace Mmc ;

static void msbf(uint16_t src,uint8_t(*dst)[2])
{
  (*dst)[1] = src ; src >>= 8 ;
  (*dst)[0] = src ;
}

static void msbf(uint32_t src,uint8_t(*dst)[4])
{
  (*dst)[3] = src ; src >>= 8 ;
  (*dst)[2] = src ; src >>= 8 ;
  (*dst)[1] = src ; src >>= 8 ;
  (*dst)[0] = src ;
}

static uint32_t msbf(uint8_t const (&src)[4])
{
  uint32_t dst = src[0] ;
  dst <<= 8 ; dst |= src[1] ;
  dst <<= 8 ; dst |= src[2] ;
  dst <<= 8 ; dst |= src[3] ;
  return dst ;
}
 
static void eor(uint8_t (*y)[5],uint8_t const (&x)[5])
{
  for (size_t i=0 ; i<5 ; ++i) 
    (*y)[i] ^= x[i] ; 
}

// --------------------------------------------------------------------
// 12 : INQUIRY
// --------------------------------------------------------------------

Inquiry Mmc::inquiry(int fd,unsigned int timeout) 
{
  uint8_t cmd[6] = { 0 } ;
  cmd[0] = 0x12 ;
  uint8_t xfer[36] ;
  msbf(sizeof(xfer),CArray::sub<3,5>(&cmd)) ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"inquiry") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  return Inquiry(std::string(&xfer[ 8],&xfer[16]),
		 std::string(&xfer[16],&xfer[32]),
		 std::string(&xfer[32],&xfer[36])) ;
}

// --------------------------------------------------------------------
// 25 : READ CAPACITY                                   
// --------------------------------------------------------------------

Capacity Mmc::readCapacity(int fd,unsigned int timeout)
{
  uint8_t xfer[8] ;
  uint8_t cmd[10] = { 0 } ;
  cmd[0] = 0x25 ; 
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"read capacity") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  uint32_t last  = msbf(CArray::sub<0,4>(xfer)) ;
  uint32_t bsize = msbf(CArray::sub<4,8>(xfer)) ;
  return Capacity(last,bsize) ;
}

// --------------------------------------------------------------------
// 28 : READ10
// --------------------------------------------------------------------

std::vector<uint8_t> Mmc::read10(int fd,uint32_t lba,uint16_t nblocks,bool fua,unsigned int timeout) 
{
  uint8_t cmd[10] = { 0 } ;
  cmd[0] = 0x28 ; 
  cmd[1] = fua ; cmd[1] <<= 3 ;
  msbf(lba,CArray::sub<2,6>(&cmd)) ;
  msbf(nblocks,CArray::sub<7,9>(&cmd)) ;
  size_t nbytes = nblocks ; nbytes *= 0x800 ;
  std::vector<uint8_t> xfer(nbytes) ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"read(10)") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,&xfer) ;
  if (!status.success())
    throw Error(status) ;
  return xfer ;
}

// -------------------------------------------------------------------- 
// A3:01 : SEND KEY : challenge (for KEY1)
// -------------------------------------------------------------------- 

void Mmc::sendChallenge(int fd,AGID agid,uint8_t const (&nonce)[10],unsigned int timeout)
{
  uint8_t xfer[16] = { 0 } ;
  msbf(sizeof(xfer)-2,CArray::sub<0,2>(&xfer)) ;
  CArray::copy(nonce,CArray::sub<4,14>(&xfer)) ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa3 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = agid ; cmd[10] <<= 6 ; cmd[10] |= 0x01 ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"send key:challenge (for key1)") ;
  Sg::Status status = Sg::invokeSend(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
}

// -------------------------------------------------------------------- 
// A3:03 : SEND KEY : KEY2
// -------------------------------------------------------------------- 

void Mmc::sendKey(int fd,AGID agid,uint8_t const (&key)[5],unsigned int timeout)
{
  uint8_t xfer[12] = { 0 } ;
  msbf(sizeof(xfer)-2,CArray::sub<0,2>(&xfer)) ;
  CArray::copy(key,CArray::sub<4,9>(&xfer)) ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa3 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = agid ; cmd[10] <<= 6 ; cmd[10] |= 0x03 ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"send key:key2") ;
  Sg::Status status = Sg::invokeSend(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
}

// --------------------------------------------------------------------
// A4:00 : REPORT KEY : AGID
// --------------------------------------------------------------------

AGID Mmc::reportAgid(int fd,unsigned int timeout)
{
  uint8_t xfer[8] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa4 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"report key:agid") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  return AGID(xfer[7] >> 6) ;
}

// --------------------------------------------------------------------
// A4:01 : REPORT KEY : challenge (for KEY2) 
// --------------------------------------------------------------------

B10 Mmc::reportChallenge(int fd,AGID agid,unsigned int timeout)
{
  uint8_t xfer[16] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa4 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = 0x01 | (agid << 6) ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"report key:challenge (for key2)") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  return B10(CArray::sub<4,14>(xfer)) ;
}

// --------------------------------------------------------------------
// A4:02 : REPORT KEY : KEY1 
// --------------------------------------------------------------------

B5 Mmc::reportKey(int fd,AGID agid,unsigned int timeout)
{
  uint8_t xfer[12] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa4 ;
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = agid ; cmd[10] <<= 6 ; cmd[10] |= 0x02 ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"report key:key1") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  return B5(CArray::sub<4,9>(xfer)) ;
}

// --------------------------------------------------------------------
// A4:04 : REPORT KEY : TITLE KEY (obfuscated and encrypted title key)
// --------------------------------------------------------------------

void Mmc::Title::deobfuscate(uint8_t const (&buskey)[5])
{
  eor(&this->key.carray(),buskey) ;
}

Title Mmc::reportTitle(int fd,AGID agid,uint32_t lba,unsigned int timeout)
{
  uint8_t xfer[12] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa4 ; 
  msbf(lba,CArray::sub<2,6>(&cmd)) ;
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = agid ; cmd[10] <<= 6 ; cmd[10] |= 0x04 ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"report key:title key (data)") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  Cmi cmi(xfer[4]) ;
  B5 key(CArray::sub<5,10>(xfer)) ;
  return Title(cmi,key) ;
}

// --------------------------------------------------------------------
// A4:05 : REPORT KEY : authentication success flag
// --------------------------------------------------------------------

bool Mmc::reportAsf(int fd,AGID agid,unsigned int timeout)
{
  uint8_t xfer[8] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa4 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = agid ; cmd[10] <<= 6 ; cmd[10] |= 0x05 ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"report key:ASF") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  return (xfer[7] & 0x01) == 0x01 ;
}

// --------------------------------------------------------------------
// A4:08 : REPORT KEY : RPC state
// --------------------------------------------------------------------

std::list<Record> Mmc::reportRpcState(int fd,unsigned int timeout)
{
  uint8_t xfer[8] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa4 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = 0x08 ; 
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"report key:RPC state") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  std::list<Record> list ;
  uint32_t v ;
  std::string s ;
  v = (xfer[4] & 0xc0) >> 6 ;
  switch (v) {
  case 0: s = "no drive region setting" ; break ;
  case 1: s = "drive region is set" ; break ;
  case 2: s = "drive region is set. additional restrictions required to make a change." ; break ;
  case 3: s = "drive region has been set permanently, but may be reset by the vendor." ; break ;
  }
  list.push_back(Record("typeCode",v,s)) ;
  v = (xfer[4] & 0x38) >> 3 ;
  s = "0x" + Format::hex(v) ;
  list.push_back(Record("numberOfVendorResetsAvailable",v,s)) ;
  v = (xfer[4] & 0x7) ;
  s = "0x" + Format::hex(v) ;
  list.push_back(Record("numberOfUserControlledChangesAvailable",v,s)) ;
  v = (xfer[5]) ;
  s = "" ;
  for (int i=0 ; i<8 ; ++i) {
    if ((v & (1<<i)) == 0) {
      if (s[0]) s += "," ;
      s += Format::sprintf("%d",i+1) ;
    }
  }
  list.push_back(Record("regionMask",v,s)) ;
  v = (xfer[6]) ;
  switch (v) {
  case 0: s = "drive does not enforce region playback controls (RPC)" ; break ;
  case 1: s = "drive region adheres to all CSS requirements (RPC II)" ; break ;
  default: s = "reserved" ;
  }
  list.push_back(Record("rpcScheme",v,s)) ;
  return list ;
}

// --------------------------------------------------------------------
// A4:3F : REPORT KEY : invalidate AGID
// --------------------------------------------------------------------

void Mmc::reportInvalidateAgid(int fd,AGID agid,unsigned int timeout)
{
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xa4 ; 
  cmd[10] = agid ; cmd[10] <<= 6 ; cmd[10] |= 0x3f ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"report key:invalidate agid") ;
  Sg::Status status = Sg::invoke(fd,cmd,timeout) ;
  if (!status.success())
    throw Error(status) ;
}

// --------------------------------------------------------------------
// AD:00 : READ DISC STRUCTURE : physical format information
// --------------------------------------------------------------------

std::list<Record> Mmc::readDiscInfo(int fd,uint8_t layer,unsigned int timeout) 
{
  uint8_t xfer[4+17] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xad ; 
  cmd[6] = layer ;
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"read disc structure:physical format information") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  std::list<Record> list ;
  uint32_t v ;
  std::string s ;
  v = ( xfer[4] & 0xf0 ) >> 4 ;
  switch (v) {
  case  0:  s = "DVD-ROM" ; break ;
  case  1:  s = "DVD-RAM" ; break ;
  case  2:  s = "DVD-R"   ; break ;
  case  3:  s = "DVD-RW"  ; break ;
  case  9:  s = "DVD+RW"  ; break ;
  case 10:  s = "DVD+R"   ; break ;
  default: s = "reserved" ;
  }
  list.push_back(Record("diskCategory",v,s)) ;
  v = ( xfer[4] & 0x0f ) ;
  list.push_back(Record("partVersion",v,"0x" + Format::hex(v))) ;
  v = ( xfer[4] & 0x0f ) >> 4 ;
  switch (v) {
  case 0:  s = "120 mm" ; break ;
  case 1:  s = "80 mm"  ; break ;
  default: s = "n/a"    ;
  }
  list.push_back(Record("discSize",v,s)) ;
  v = ( xfer[5] & 0x0f ) ;
  switch (v) {
  case  0: s = "2.52 Mbps"     ; break ;
  case  1: s = "5.04 Mbps"     ; break ;
  case  2: s = "10.08 Mbps"    ; break ;
  case 15: s = "not specified" ; break ;
  default: s = "reserved"      ;
  }
  list.push_back(Record("maximumRate",v,s)) ;
  v = ( xfer[6] & 0x60 ) >> 5 ;
  switch (v) {
  case  0: s = "1" ; break ;
  case  1: s = "2" ; break ;
  default: s = "not specified" ; 
  }
  list.push_back(Record("numberOfLayers",v,s)) ;
  v = ( xfer[6] & 0x10 ) >> 4 ;
  s = (v==0) 
  ? "Parallel Track Path (PTP)" 
  : "Opposite Track Path (OTP)" ;
  list.push_back(Record("track",v,s)) ;
  v = ( xfer[6] & 0x0f ) ;
  s = "" ;
  if (v & 0x01) {
    s = "embossed" ;
  }
  if (v & 0x02) {
    if (s[0]) s += "+" ;
    s += "recordable" ;
  }
  if (v & 0x04) {
    if (s[0]) s += "+" ;
    s += "rewritable" ;
  }
  if (v & 0x08) {
    if (s[0]) s += "+" ;
    s += "reserved" ;
  }
  if (!s[0])
    s = "none" ;
  list.push_back(Record("layerType",v,s)) ;
  v = ( xfer[7] & 0xf0 ) >> 4 ;
  switch (v) {
  case  0: s = "0.267 um/bit"          ; break ;
  case  1: s = "0.293 um/bit"          ; break ;
  case  2: s = "0.409 to 0.435 um/bit" ; break ;
  case  4: s = "0.280 to 0.291 um/bit" ; break ;
  case  8: s = "0.353 um/bit"          ; break ;
  default: s = "reserved" ;
  }
  list.push_back(Record("linearyDensity",v,s)) ;
  v = ( xfer[7] & 0x0f ) ;
  switch (v) {
  case  0: s = "0.74 um/bit"  ; break ;
  case  1: s = "0.80 um/bit"  ; break ;
  case  2: s = "0.615 um/bit" ; break ;
  default: s = "reserved" ;
  }
  list.push_back(Record("trackDensity",v,s)) ;
  v = msbf(CArray::sub<8,12>(xfer)) ;
  list.push_back(Record("startSector",v,"0x" + Format::hex(v))) ;
  v = msbf(CArray::sub<12,16>(xfer)) ;
  list.push_back(Record("endSector",v,"0x" + Format::hex(v))) ;
  v = msbf(CArray::sub<16,20>(xfer)) ;
  list.push_back(Record("endSectorLayer0",v,"0x" + Format::hex(v))) ;
  v = (xfer[20] & 0x80) >> 7 ;
  s = (v==0) 
  ? "BCA data does not exist"
  : "BCA data exist" ;
  list.push_back(Record("bca",v,s)) ;
  return list ;
}

// --------------------------------------------------------------------
// AD:01 : READ DISC STRUCTURE : DVD Copyright Information
// --------------------------------------------------------------------

Copyright Mmc::readDiscCopyright(int fd,unsigned int timeout) 
{
  uint8_t xfer[4+4] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xad ; 
  cmd[7] = 0x01 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"read disc structure:dvd copyright information") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  return Copyright(xfer[4+0],xfer[4+1]) ;
}

// --------------------------------------------------------------------
// AD:02 : READ DISC STRUCTURE : disc key (obfuscated and encrypted)
// --------------------------------------------------------------------

void DiscKey::deobfuscate(uint8_t const (&buskey)[5])
{
  for (size_t i=0 ; i<this->size ; ++i) 
    eor(&(*this)[i].carray(),buskey) ;
}

DiscKey Mmc::readDiscKey(int fd,AGID agid,unsigned int timeout) 
{
  uint8_t xfer[4+0x800] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xad ; 
  cmd[7] = 0x02 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  cmd[10] = agid ; cmd[10] <<= 6 ; 
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"read disc structure:disc key") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  Array<B5,409> keyA ;
  for (size_t i=0 ; i<keyA.size ; ++i) {
    size_t ofs = 4+5*i ;
    keyA[i] = B5(CArray::sub<5>(xfer,ofs)) ;
  }
  return DiscKey(keyA) ;
}

// --------------------------------------------------------------------
// AD:05 : READ DISC STRUCTURE : Copyright Management Information
// --------------------------------------------------------------------

int Cmi::copyright() const 
{ 
  int r = 0 ;
  if (this->flags & 0x80) 
    r = 1 + ((this->flags >> 4) & 0x03) ;
  return r ;
}

int Cmi::mode() const 
{ 
  int r = 0 ;
  if ((this->flags & 0xc0) == 0xc0) 
    r = 1 + (this->flags & 0x0f) ;
  return r ;
}

Cmi Mmc::readDiscCmi(int fd,uint32_t lba,unsigned int timeout) 
{
  uint8_t xfer[4+4] ;
  uint8_t cmd[12] = { 0 } ;
  cmd[0] = 0xad ; 
  msbf(lba,CArray::sub<2,6>(&cmd)) ;
  cmd[7] = 0x05 ; 
  msbf(sizeof(xfer),CArray::sub<8,10>(&cmd)) ;
  if (Logging::level() >= 2)
    Logging::out(__FILE__,__LINE__,"read disc structure:dvd copyright information") ;
  Sg::Status status = Sg::invokeRecv(fd,cmd,timeout,xfer) ;
  if (!status.success())
    throw Error(status) ;
  return Cmi(xfer[4]) ;
}

