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

#include "Handshake.h"

static Mmc::AGID reportAgid(int fd,unsigned int timeout,int retries=1)
{
  for (int i=0 ; true ; ++i) {
    try {
      return Mmc::reportAgid(fd,timeout) ;
    }
    catch (Mmc::Error &error) {
      if (error.status.senseKey() == 0x05 && i <= retries) {
	switch (error.status.senseCode()) {
	case 0x2c00: // illegal request: command sequence error
	case 0x6f01: // copy protection key exchange failure - key not present
	case 0x5500: // illegal request: system resource failure
	  for (Mmc::AGID::Type i=Mmc::AGID::lower ; i<=Mmc::AGID::upper ; ++i) {
	    Mmc::reportInvalidateAgid(fd,Mmc::AGID(i),timeout) ;
	  }
	  continue ;
	}
      }
      throw ; 
    }
  }
  // Note: The command to request an AGID fails if all AGIDs are
  // currently in use for another handshake. An AGID becomes available
  // again if these handshakes are completed, or, if the AGID is 
  // invalidated. Though, if an application fails, it may not be able
  // to invalidate the AGID. 
  //
  // So what to do if no AGID is available? Is there really another 
  // application that right now performs a handshake? Or has the AGID
  // been left dead? There's actually no way to know. However, the
  // latter case is most likely. This implementation just retries.
  //
  // Note: The MMC spec defines AGID numbers 0..3. However, many drives 
  // that have been tested support just one AGID (either AGID #0 or #3).
  // Even worse, the actual AGID number in the MMC commands is ignored 
  // by those drives. That is, they go with any value. So there's no 
  // reason to take the AGID management seriously.
}

static Auth::B5 makeBuskey(int fd,Mmc::AGID agid,uint8_t const (&nonce1)[10],unsigned int timeout)
{
  Mmc::sendChallenge(fd,agid,nonce1,timeout) ;
  Auth::B5 key1 = Mmc::reportKey(fd,agid,timeout) ;
  Auth::Variant variant = Auth::getVariant(nonce1,key1) ;
  Mmc::B10 nonce2 = Mmc::reportChallenge(fd,agid,timeout) ;
  Mmc::B5 key2 = Auth::makeKey2(variant,nonce2) ;
  Mmc::sendKey(fd,agid,key2,timeout) ;
  Auth::B5 buskey = Auth::makeBusKey(variant,key1,key2) ;
  return buskey ;
}


boost::optional<Mmc::DiscKey> Handshake::unlock(int fd,uint8_t const (&nonce1)[10],unsigned int timeout)
{
  Mmc::AGID agid = reportAgid(fd,timeout) ;
  Auth::B5 buskey = makeBuskey(fd,agid,nonce1,timeout) ;
  try {
    Mmc::DiscKey discKey = Mmc::readDiscKey(fd,agid,timeout) ;
    bool success = Mmc::reportAsf(fd,agid,timeout) ;
    if (!success) {
      throw Error("authentication handshake with drive failed") ;
    }
    discKey.deobfuscate(buskey) ;
    return discKey ;
  }
  catch (Mmc::Error &error) {
    Sg::Status const &status = error.status ;
    if (status.senseKey() == 0x05 && status.senseCode() == 0x6f01) {
      // copy protection key exchange failure - key not present
      return boost::optional<Mmc::DiscKey>() ;
    }
    throw ;
  }
  return boost::optional<Mmc::DiscKey>() ;
}

boost::optional<Mmc::Title> Handshake::title(int fd,uint8_t const (&nonce1)[10],unsigned int timeout,uint32_t lba)
{
  Mmc::AGID agid = reportAgid(fd,timeout) ;
  Auth::B5 buskey = makeBuskey(fd,agid,nonce1,timeout) ;
  try {
    Mmc::Title title = Mmc::reportTitle(fd,agid,lba,timeout) ;
    // note, even if the sector is not marked as scrambled nor the 
    // disc is CSS protected at all, we might have got a title-key 
    // 00:00:00:00
    title.deobfuscate(buskey) ;
    return title ; 
  }
  catch (Mmc::Error &error) {
    Sg::Status const &status = error.status ;
    if (status.senseKey() == 0x05 && status.senseCode() == 0x6f01) {
      // copy protection key exchange failure - key not present
      return boost::optional<Mmc::Title>() ;
    }
    throw ;
  }
  return boost::optional<Mmc::Title>() ;
}

