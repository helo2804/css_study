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

#include "Format.h"
#include "Mmc.h"
#include "Os.h"
#include "Ui.h"

#include <boost/integer.hpp>

using namespace Mmc ;

static void capacity(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  argL.finalize() ;
  Capacity capacity = readCapacity(fd,timeout) ;
  std::cout << "lastLBA: " << capacity.lba << " "
	    << "(0x" << Format::hex(capacity.lba) << ")" << std::endl
	    << "blockSize: " << capacity.bsize << " "
	    << "(0x" << Format::hex(capacity.bsize) << ")" << std::endl ;
}

static void cmi(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: device start=LBA [--count LBAs]" << std::endl 
      << "output: lines with LBA and CMI" << std::endl ;
      ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  uint32_t start = Format::number<uint32_t>(argL.pop()) ; 
  uint32_t n = Format::number<uint32_t>(argL.option("--count","1")) ; 
  argL.finalize() ;
  for (uint32_t i=start ; i<start+n ; ++i) {
    Cmi cmi = readDiscCmi(fd,i,timeout) ;
    std::cout << "0x" << Format::hex(i) << " "
	      << "0x" << Format::hex(cmi.flags) << std::endl ;
  }
}

static void copyright(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  argL.finalize() ;
  Copyright c = readDiscCopyright(fd,timeout) ;
  char const *type = "unknown" ;
  switch (c.type) {
  case 0: type = "none"     ; break ;
  case 1: type = "CSS/CPPM" ; break ;
  case 2: type = "CPRM"     ; break ;
  }
  std::cout << "type: " << int(c.type) << " (" << type << ")" << std::endl ;
  std::string regionCode = "" ;
  for (int i=0 ; i<8 ; ++i) {
    if ((c.regionCode & (1<<i)) == 0) {
      if (regionCode[0])
	regionCode += "," ;
      regionCode += Format::sprintf("%d",i+1) ;
    }
  }
  std::cout << "region-code: 0x" << Format::hex(c.regionCode) 
	    << " (" << regionCode << ")" << std::endl ;
}

static void discInfo(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device [--layer N]" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  uint8_t layer = Format::number<uint8_t>(argL.option("--layer","0")) ; 
  argL.finalize() ;
  std::list<Record> list = readDiscInfo(fd,layer,timeout) ;
  std::list<Record>::const_iterator i = list.begin() ; 
  while (i != list.end()) {
    std::cout << i->key   << ": "
	      << i->value << " ("
	      << i->info  << ")" << std::endl ;
    ++i ;
  }
}

static void discKey(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3 buskey=byte:5" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ; 
  B5 busKey = Format::byteA<5>(argL.pop()) ; 
  argL.finalize() ;
  DiscKey discKey = readDiscKey(fd,agid,timeout) ;
  discKey.deobfuscate(busKey) ;
  for (size_t i=0 ; i<discKey.size ; ++i) {
    std::cout << Format::hex(discKey[i]) << std::endl ;
  }
}

static void inquiry(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  argL.finalize() ;
  Inquiry inquiry = Mmc::inquiry(fd,timeout) ;
  std::cout << "vendor: "     << inquiry.vendor     << std::endl
	    << "productId: "  << inquiry.productId  << std::endl
	    << "productRev: " << inquiry.productRev << std::endl ;
}

static void invalidateAgid(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ; 
  argL.finalize() ;
  reportInvalidateAgid(fd,agid,timeout) ;
}

static void read(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout 
      << "arguments: "
      << "device "
      << "[--fua 0|1] "
      << "[--start LBA] "
      << "[--count LBAs] "
      << "[--cluster LBAs] "
      << "[--write filename]" 
      << std::endl 
      << "note: cluster specifies the number of blocks to read together" << std::endl
      << "output: if read errors: the LBA, the sense key and the sense code" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  Capacity capacity = readCapacity(fd,timeout) ;
  if (capacity.bsize != 0x800)
    throw Ui::Error("block size must be 2048") ;
  bool fua = Format::number<bool>(argL.option("--fua","0")) ;
  uint32_t start = Format::number<uint32_t>(argL.option("--start","0")) ; 
  uint32_t count = Format::number<uint32_t>(argL.option("--count","0x" + Format::hex(capacity.lba-start+1))) ;
  uint16_t cluster = Format::number<uint16_t>(argL.option("--cluster","1")) ; 
  if (cluster == 0)
    throw Ui::Error("--cluster (size) cannot be zero") ;
  boost::optional<std::string> ofname = argL.option("--write") ;
  std::ofstream os ; 
  if (ofname) 
    Os::open(os,*ofname) ;
  argL.finalize() ;
  uint32_t i = 0 ;
  while (i < count) {
    uint16_t nblocks = cluster ;
    if (nblocks > count - i)
      nblocks = count - i ;
    try {
      std::vector<uint8_t> v = Mmc::read10(fd,start+i,nblocks,fua,timeout) ;
      if (ofname) {
	Os::writeAll(os,v) ;
      }
    }
    catch (Mmc::Error &e) {
      if (ofname) {
	std::vector<uint8_t> null(nblocks * 0x800,0) ;
	Os::writeAll(os,null) ;
      }
      std::cout << "0x" << Format::hex(start+i) << " " 
		<< "0x" << Format::hex(e.status.senseKey()) << " "
		<< "0x" << Format::hex(e.status.senseCode()) << std::endl ;
    }
    i += nblocks ;
  }
}

static void reportAgid(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  argL.finalize() ;
  AGID agid = reportAgid(fd,timeout) ;
  std::cout << int(agid) << std::endl ;
}

static void reportAsf(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ; 
  argL.finalize() ;
  bool success = reportAsf(fd,agid,timeout) ;
  std::cout << success << std::endl ;
}

static void reportChallenge(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ; 
  argL.finalize() ;
  B10 nonce1 = reportChallenge(fd,agid,timeout) ;
  std::cout << Format::hex(nonce1) << std::endl ;
}

static void reportKey(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ; 
  argL.finalize() ;
  B5 key1 = reportKey(fd,agid,timeout) ;
  std::cout << Format::hex(key1) << std::endl ;
}

static void reportRpc(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing argument. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "argument: device" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  argL.finalize() ;
  std::list<Record> list = reportRpcState(fd,timeout) ;
  std::list<Record>::const_iterator i = list.begin() ; 
  while (i != list.end()) {
    std::cout << i->key   << ": "
	      << i->value << " ("
	      << i->info  << ")" << std::endl ;
    ++i ;
  }
}

static void reportTitle(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3 LBA buskey=byte:5" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ; 
  uint32_t lba = Format::number<uint32_t>(argL.pop()) ; 
  B5 busKey = Format::byteA<5>(argL.pop()) ; 
  argL.finalize() ;
  Title title = reportTitle(fd,agid,lba,timeout) ;
  title.deobfuscate(busKey) ;
  std::cout << "0x" << Format::hex(title.cmi.flags) << " " << Format::hex(title.key) << std::endl ;
}

static void sendChallenge(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3 nonce1=byte:10" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ;
  B10 nonce = Format::byteA<10>(argL.pop()) ; 
  argL.finalize() ;
  sendChallenge(fd,agid,nonce,timeout) ;
}

static void sendKey(ArgL &argL,unsigned int timeout)
{
  if (argL.empty()) {
    throw Ui::Error("missing arguments. you may try help.") ;
  }
  if (argL.peek() == "help") {
    std::cout << "arguments: device AGID=0..3 KEY2=byte:5" << std::endl ;
    argL.pop() ;
    return ;
  }
  Os::Fd fd = Os::Fd::openRo(argL.pop()) ;
  AGID agid = Format::number<AGID::Type>(argL.pop()) ; 
  B5 key2 = Format::byteA<5>(argL.pop()) ; 
  argL.finalize() ;
  sendKey(fd,agid,key2,timeout) ;
}

void Ui::mmcInvoke(ArgL &argL)
{
  if (argL.empty()) {
    throw Ui::Error("missing option. you may try help.") ;
  }
  unsigned timeout = Format::number<unsigned>(argL.option("--timeout","10000")) ;
  std::string arg = argL.pop() ;
  if (arg == "help") {
    std::cout 
      << "usage: [--timeout milli-seconds] option ..." << std::endl
      << "supported options are:" << std::endl 
      << "capacity:         Print disc capacity" << std::endl
      << "cmi:              Print disc copyright management information (CPR_MAI) for an LBA" << std::endl
      << "copyright:        Print disc copyright mechanism (CSS, region-code)" << std::endl
      << "disc-info:        Print disc information" << std::endl
      << "disc-key:         Handshake(6): retrieve the disc-key-block" << std::endl
      << "inquiry:          Print drive's information" << std::endl
      << "invalidate-agid:  Handshake: invalidate AGID" << std::endl
      << "read:             Read disc block(s)" << std::endl
      << "report-agid:      Handshake(1): allocate new AGID" << std::endl
      << "report-asf:       Handshake(7): retrieve ASF" << std::endl
      << "report-challenge: Handshake(4): retrieve nonce for KEY2" << std::endl
      << "report-key:       Handshake(3): retrieve KEY1" << std::endl
      << "report-rpc:       Print drive's regional information" << std::endl
      << "report-title:     Handshake(6): retrieve title (CPR_MAI+key) for an LBA" << std::endl
      << "send-challenge    Handshake(2): sende nonce for KEY1" << std::endl
      << "send-key          Handshake(5): send KEY2" << std::endl ;
    return ;
  }
  if (arg == "capacity") {
    capacity(argL,timeout) ;
  }
  else if (arg == "cmi") {
    cmi(argL,timeout) ;
  }
  else if (arg == "copyright") {
    copyright(argL,timeout) ;
  }
  else if (arg == "disc-info") {
    discInfo(argL,timeout) ;
  }
  else if (arg == "disc-key") {
    discKey(argL,timeout) ;
  }
  else if (arg == "inquiry") {
    inquiry(argL,timeout) ;
  }
  else if (arg == "invalidate-agid") {
    invalidateAgid(argL,timeout) ;
  }
  else if (arg == "read") {
    read(argL,timeout) ;
  }
  else if (arg == "report-agid") {
    reportAgid(argL,timeout) ;
  }
  else if (arg == "report-asf") {
    reportAsf(argL,timeout) ;
  }
  else if (arg == "report-challenge") {
    reportChallenge(argL,timeout) ;
  }
  else if (arg == "report-key") {
    reportKey(argL,timeout) ;
  }
  else if (arg == "report-rpc") {
    reportRpc(argL,timeout) ;
  }
  else if (arg == "report-title") {
    reportTitle(argL,timeout) ;
  }
  else if (arg == "send-challenge") {
    sendChallenge(argL,timeout) ;
  }
  else if (arg == "send-key") {
    sendKey(argL,timeout) ;
  }
  else {
    throw Ui::Error("not supported option. you may try help.") ;
  }
}
