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
#include "Logging.h"
#include "Os.h"
#include "Sg.h"

#include <cassert>
#include <cstring> // memset
#include <iomanip> // std::setfill,std::setw
#include <limits> // std::numeric_limits
#include <scsi/scsi.h> // GOOD,CHECK_CONDITION,...
#include <sstream>

int Sg::version(int fd)
{
  int version ;
  Os::ioctl(fd,SG_GET_VERSION_NUM,&version) ;
  return version ;
}

Sg::Status Sg::sgctl(int fd,Range<uint8_t const> cmd_,unsigned int timeout,int direction,Range<uint8_t> xfer)
{
  struct sg_io_hdr io ;
  memset(&io,0,sizeof(io)) ;
  io.interface_id = 'S' ;
  // command, note that SG requires a non-const command buffer
  std::vector<uint8_t> cmd(cmd_.begin(),cmd_.end()) ;
  io.cmdp = &cmd[0] ;
  io.cmd_len = cmd.size() ;
  // sense buffer
  uint8_t sense[32] ;
  io.sbp = &sense[0] ;
  io.mx_sb_len = sizeof(sense) ;
  // send data/receive data/no data
  io.dxfer_direction = direction ;
  io.dxferp = &xfer[0] ;
  if (xfer.size() > std::numeric_limits<unsigned int>::max())
    throw Error("xfer size too large") ;
  io.dxfer_len = xfer.size() ;
  // timeout in milli seconds
  io.timeout = timeout ;
  if (Logging::level() >= 3) 
    Logging::out(__FILE__,__LINE__,std::string("command:") + Format::hex(cmd)) ;
  if (Logging::level() >= 4) 
    if (direction == SG_DXFER_TO_DEV)
      Logging::out(__FILE__,__LINE__,std::string("argument:") + Format::hex(xfer)) ;
  // invoke 
  Os::ioctl(fd,SG_IO,&io) ;
  if (io.sb_len_wr > sizeof(sense))  
    throw Error("ioctl() returned mismatched sense length") ; 
  // results
  Status s(io,Range<uint8_t const>(sense).sub(0,io.sb_len_wr)) ;
  if (Logging::level() >= 3) 
    Logging::out(__FILE__,__LINE__,s.toString()) ;
  if (Logging::level() >= 4) 
    if (direction == SG_DXFER_FROM_DEV)
      Logging::out(__FILE__,__LINE__,std::string("result:") + Format::hex(xfer)) ;
  return s ;
}

Sg::Status Sg::invoke(int fd,Range<uint8_t const> cmd,unsigned int timeout) 
{
  std::vector<uint8_t> null ;
  return sgctl(fd,cmd,timeout,SG_DXFER_NONE,&null) ;
}

Sg::Status Sg::invokeRecv(int fd,Range<uint8_t const> cmd,unsigned int timeout,Range<uint8_t> xfer)
{
  return sgctl(fd,cmd,timeout,SG_DXFER_FROM_DEV,xfer) ;
}

Sg::Status Sg::invokeSend(int fd,Range<uint8_t const> cmd,unsigned int timeout,Range<uint8_t const> xfer_)
{
  // SG requires a non-const transfer buffer
  std::vector<uint8_t> xfer(xfer_.begin(),xfer_.end()) ;
  return sgctl(fd,cmd,timeout,SG_DXFER_TO_DEV,&xfer) ;
}

bool Sg::Status::success() const
{
  return (this->io.info & SG_INFO_OK_MASK) == SG_INFO_OK ;
}

uint8_t Sg::Status::senseKey() const
{
  if (this->sense.size() > 0)
    if ((this->sense[0] == 0x70) || (this->sense[0] == 0x71))
      if (this->sense.size() >= 3)
	return sense[2] & 0x0f ;
  return 0 ;
}

uint16_t Sg::Status::senseCode() const
{
  if (this->sense.size() > 0)
    if ((this->sense[0] == 0x70) || (this->sense[0] == 0x71) || (this->sense[0] >= 0x80))
      if (this->sense.size() >= 14)
	return (sense[12]<<8) + sense[13] ;
  return 0 ;
}

static std::string scsiText(uint8_t v)
{
  // INQUIRY: scsi status
  switch (v) {
  case GOOD                 : /* 0x00 */ return "good" ;
  case CHECK_CONDITION      : /* 0x01 */ return "check condition" ;
  case CONDITION_GOOD       : /* 0x02 */ return "condition good" ;
  case BUSY                 : /* 0x04 */ return "busy" ;
  case INTERMEDIATE_GOOD    : /* 0x08 */ return "intermediate good" ;
  case INTERMEDIATE_C_GOOD  : /* 0x0a */ return "intermediate c good" ;
  case RESERVATION_CONFLICT : /* 0x0c */ return "reservation conflict" ;
  case COMMAND_TERMINATED   : /* 0x11 */ return "command terminated" ;
  case QUEUE_FULL           : /* 0x14 */ return "queue full" ;
  }
  return "{no description}" ; 
}

static std::string hostText(uint8_t v)
{
  // INQUIRY: host status
  switch (v) {
  case /* SG_ERR_DID_OK          */ 0x00: return "No error" ;
  case /* SG_ERR_DID_NO_CONNECT  */ 0x01: return "Couldn't connect before timeout period" ;
  case /* SG_ERR_DID_BUS_BUSY    */ 0x02: return "BUS stayed busy through time out period" ;
  case /* SG_ERR_DID_TIME_OUT    */ 0x03: return "TIMED OUT for other reason" ;
  case /* SG_ERR_DID_BAD_TARGET  */ 0x04: return "BAD target, device not responding?" ;
  case /* SG_ERR_DID_ABORT       */ 0x05: return "Told to abort for some other reason" ;
  case /* SG_ERR_DID_PARITY      */ 0x06: return "Parity error" ;
  case /* SG_ERR_DID_ERROR       */ 0x07: return "Internal error [DMA underrun on aic7xxx]" ;
  case /* SG_ERR_DID_RESET       */ 0x08: return "Reset by somebody" ;
  case /* SG_ERR_DID_BAD_INTR    */ 0x09: return "Got an interrupt we weren't expecting" ;
  case /* SG_ERR_DID_PASSTHROUGH */ 0x0a: return "Force command past mid-layer" ;
  case /* SG_ERR_DID_SOFT_ERROR  */ 0x0b: return "The low level driver wants a retry" ;
  }
  return "{no description}" ; 
}

static std::string driverText(uint8_t v)
{
  std::string m ;
  // INQUIRY: driver_status
  switch (v & 0x0f) {
  case /* SG_ERR_DRIVER_OK      */ 0x00: m = "ok"      ; break ;
  case /* SG_ERR_DRIVER_BUSY    */ 0x01: m = "busy"    ; break ;
  case /* SG_ERR_DRIVER_SOFT    */ 0x02: m = "soft"    ; break ;
  case /* SG_ERR_DRIVER_MEDIA   */ 0x03: m = "media"   ; break ;
  case /* SG_ERR_DRIVER_ERROR   */ 0x04: m = "error"   ; break ;
  case /* SG_ERR_DRIVER_INVALID */ 0x05: m = "invalid" ; break ;
  case /* SG_ERR_DRIVER_TIMEOUT */ 0x06: m = "timeout" ; break ;
  case /* SG_ERR_DRIVER_HARD    */ 0x07: m = "hard"    ; break ;
  case /* SG_ERR_DRIVER_SENSE   */ 0x08: m = "sense"   ; break ; // implies sense output
  default                              : m = "{no description}" ;
  }
  switch (v & 0xf0) {
  case /* SG_ERR_SUGGEST_RETRY */ 0x10: m += ":retry"  ; break ;
  case /* SG_ERR_SUGGEST_ABORT */ 0x20: m += ":abort"  ; break ;
  case /* SG_ERR_SUGGEST_REMAP */ 0x30: m += ":remap"  ; break ;
  case /* SG_ERR_SUGGEST_DIE   */ 0x40: m += ":die"    ; break ;
  case /* SG_ERR_SUGGEST_SENSE */ 0x80: m += ":sense"  ; break ;
  default                             : m += ":{no action}" ; 
  }
  return m ;
}

static std::string senseKeyText(uint8_t v)
{
  static char const* p[] = {
    "no sense"       , "recovered error",
    "not ready"      , "medium error",
    "hardware error" , "illegal request",
    "unit attention" , "data protect",
    "blank check"    , "vendor specific",
    "copy aborted"   , "aborted command",
    "{obsolete}"     , "volume overflow",
    "miscompare"     , "{reserved}" } ;
  return p[v & 0xf] ;
}

struct ScsiError
{
  uint16_t senseCode ; // additional sense code + qualifier
  char const* text ; // error description text
} ;

static ScsiError const error[] = { 

  { 0x0006, "i/o process terminated" },
  { 0x0011, "audio play operation in progress" },
  { 0x0012, "audio play operation paused" },
  { 0x0013, "audio play operation successfully completed" },
  { 0x0014, "audio play operation stopped due to error" },
  { 0x0015, "no current audio status to return" },
  { 0x0017, "cleaning requested" },
  { 0x0200, "no seek complete" },
  { 0x0400, "logical unit not ready, cause not reportable" },
  { 0x0401, "logical unit is in process of becoming ready" },
  { 0x0402, "logical unit not ready, initializing cmd. required" },
  { 0x0403, "logical unit not ready, manual intervention required" },
  { 0x0404, "logical unit not ready, format in progress" },
  { 0x0407, "logical unit not ready, operation in progress" },
  { 0x0408, "logical unit not ready, long write in progress" },
  { 0x0409, "write error recovery needed" },
  { 0x0500, "logical unit does not respond to selection" },
  { 0x0600, "no reference position found" },
  { 0x0700, "multiple peripheral devices selected" },
  { 0x0800, "logical unit communication failure" },
  { 0x0801, "logical unit communication timeout" },
  { 0x0802, "logical unit communication parity error" },
  { 0x0803, "logical unit communication crc error (ultra-dma/32)" },
  { 0x0900, "track following error" },
  { 0x0901, "tracking servo failure" },
  { 0x0902, "focus servo failure" },
  { 0x0903, "spindle servo failure" },
  { 0x0904, "head select fault" },
  { 0x0a00, "error log overflow" },
  { 0x0b00, "warning" },
  { 0x0b02, "warning – enclosure degraded" },
  { 0x0c00, "write error" },
  { 0x0c07, "write error – recovery needed" },
  { 0x0c08, "write error – recovery failed" },
  { 0x0c09, "write error – loss of streaming" },
  { 0x0c0a, "write error – padding blocks added" },
  { 0x1100, "unrecovered read error" },
  { 0x1101, "read retries exhausted" },
  { 0x1102, "error too long to correct" },
  { 0x1105, "l-ec uncorrectable error" },
  { 0x1106, "circ unrecovered error" },
  { 0x110f, "error reading upc/ean number" },
  { 0x1110, "error reading isrc number" },
  { 0x1111, "read error – loss of streaming" },
  { 0x1500, "random positioning error" },
  { 0x1501, "mechanical positioning error" },
  { 0x1502, "positioning error detected by read of medium" },
  { 0x1700, "recovered data with no error correction applied" },
  { 0x1701, "recovered data with retries" },
  { 0x1702, "recovered data with positive head offset" },
  { 0x1703, "recovered data with negative head offset" },
  { 0x1704, "recovered data with retries and/or circ applied" },
  { 0x1705, "recovered data using previous sector id" },
  { 0x1707, "recovered data without ecc - recommend reassignment" },
  { 0x1708, "recovered data without ecc - recommend rewrite" },
  { 0x1709, "recovered data without ecc - data rewritten" },
  { 0x1800, "recovered data with error correction applied" },
  { 0x1801, "recovered data with error corr. & retries applied" },
  { 0x1802, "recovered data - data auto-reallocated" },
  { 0x1803, "recovered data with circ" },
  { 0x1804, "recovered data with l-ec" },
  { 0x1805, "recovered data - recommend reassignment" },
  { 0x1806, "recovered data - recommend rewrite" },
  { 0x1808, "recovered data with linking" },
  { 0x1a00, "parameter list length error" },
  { 0x1b00, "synchronous data transfer error" },
  { 0x1d00, "miscompare during verify operation" },
  { 0x2000, "invalid command operation code" },
  { 0x2100, "logical block address out of range" },
  { 0x2101, "invalid element address" },
  { 0x2102, "invalid address for write" },
  { 0x2400, "invalid field in cdb" },
  { 0x2500, "logical unit not supported" },
  { 0x2600, "invalid field in parameter list" },
  { 0x2601, "parameter not supported" },
  { 0x2602, "parameter value invalid" },
  { 0x2603, "threshold parameters not supported" },
  { 0x2604, "invalid release of active persistent reservation" },
  { 0x2700, "write protected" },
  { 0x2701, "hardware write protected" },
  { 0x2702, "logical unit software write protected" },
  { 0x2703, "associated write protect" },
  { 0x2704, "persistent write protect" },
  { 0x2705, "permanent write protect" },
  { 0x2706, "conditional write protect" },
  { 0x2800, "not ready to ready change, medium may have changed" },
  { 0x2801, "import or export element accessed" },
  { 0x2900, "power on, reset, or bus device reset occurred" },
  { 0x2901, "power on occurred" },
  { 0x2902, "scsi bus reset occurred" },
  { 0x2903, "bus device reset function occurred" },
  { 0x2904, "device internal reset" },
  { 0x2a00, "parameters changed" },
  { 0x2a01, "mode parameters changed" },
  { 0x2a02, "log parameters changed" },
  { 0x2a03, "reservations preempted" },
  { 0x2b00, "copy cannot execute since initiator cannot disconnect" },
  { 0x2c00, "command sequence error" },
  { 0x2c03, "current program area is not empty" },
  { 0x2c04, "current program area is empty" },
  { 0x2e00, "insufficient time for operation" },
  { 0x2f00, "commands cleared by another initiator" },
  { 0x3000, "incompatible medium installed" },
  { 0x3001, "cannot read medium - unknown format" },
  { 0x3002, "cannot read medium - incompatible format" },
  { 0x3003, "cleaning cartridge installed" },
  { 0x3004, "cannot write medium - unknown format" },
  { 0x3005, "cannot write medium - incompatible format" },
  { 0x3006, "cannot format medium - incompatible medium" },
  { 0x3007, "cleaning failure" },
  { 0x3008, "cannot write - application code mismatch" },
  { 0x3009, "current session not fixated for append" },
  { 0x3010, "medium not formatted" },
  { 0x3011, "cannot write medium - unsupported medium version" },
  { 0x3100, "medium format corrupted" },
  { 0x3101, "format command failed" },
  { 0x3102, "zoned formatting failed due to spare linking" },
  { 0x3400, "enclosure failure" },
  { 0x3500, "enclosure services failure" },
  { 0x3501, "unsupported enclosure function" },
  { 0x3502, "enclosure services unavailable" },
  { 0x3503, "enclosure services transfer failure" },
  { 0x3504, "enclosure services transfer refused" },
  { 0x3700, "rounded parameter" },
  { 0x3900, "saving parameters not supported" },
  { 0x3a00, "medium not present" },
  { 0x3a01, "medium not present - tray closed" },
  { 0x3a02, "medium not present - tray open" },
  { 0x3b0d, "medium destination element full" },
  { 0x3b0e, "medium source element empty" },
  { 0x3b0f, "end of medium reached" },
  { 0x3b11, "medium magazine not accessible" },
  { 0x3b12, "medium magazine removed" },
  { 0x3b13, "medium magazine inserted" },
  { 0x3b14, "medium magazine locked" },
  { 0x3b15, "medium magazine unlocked" },
  { 0x3b16, "mechanical positioning or changer error" },
  { 0x3d00, "invalid bits in identify message" },
  { 0x3e00, "logical unit has not self-configured yet" },
  { 0x3e01, "logical unit failure" },
  { 0x3e02, "timeout on logical unit" },
  { 0x3f00, "target operating conditions have changed" },
  { 0x3f01, "microcode has been changed" },
  { 0x3f02, "changed operating definition" },
  { 0x3f03, "inquiry data has changed" },
//{ 0x40nn, "diagnostic failure on component nn (80h-ffh)" }, 
  { 0x4300, "message error" },
  { 0x4400, "internal target failure" },
  { 0x4500, "select or reselect failure" },
  { 0x4600, "unsuccessful soft reset" },
  { 0x4700, "scsi parity error" },
  { 0x4800, "initiator detected error message received" },
  { 0x4900, "invalid message error" },
  { 0x4a00, "command phase error" },
  { 0x4b00, "data phase error" },
  { 0x4c00, "logical unit failed self-configuration" },
//{ 0x4dnn, "tagged overlapped commands (nn = queue tag)" }, 
  { 0x4e00, "overlapped commands attempted" },
  { 0x5100, "erase failure" },
  { 0x5101, "erase failure - incomplete erase operation detected" },
  { 0x5300, "media load or eject failed" },
  { 0x5302, "medium removal prevented" },
  { 0x5500, "system resource failure" },
  { 0x5700, "unable to recover table-of-contents" },
  { 0x5a00, "operator request or state change input" },
  { 0x5a01, "operator medium removal request" },
  { 0x5a02, "operator selected write protect" },
  { 0x5a03, "operator selected write permit" },
  { 0x5b00, "log exception" },
  { 0x5b01, "threshold condition met" },
  { 0x5b02, "log counter at maximum" },
  { 0x5b03, "log list codes exhausted" },
  { 0x5d01, "failure prediction threshold exceeded - predicted media failure" },
  { 0x5d02, "logical unit failure prediction threshold exceeded" },
  { 0x5d03, "failure prediction threshold exceeded - predicted spare area exhaustion" },
  { 0x5dff, "failure prediction threshold exceeded (false)" },
  { 0x5e00, "low power condition on" },
  { 0x5e01, "idle condition activated by timer" },
  { 0x5e02, "standby condition activated by timer" },
  { 0x5e03, "idle condition activated by command" },
  { 0x5e04, "standby condition activated by command" },
  { 0x6300, "end of user area encountered on this track" },
  { 0x6301, "packet does not fit in available space" },
  { 0x6400, "illegal mode for this track" },
  { 0x6401, "invalid packet size" },
  { 0x6500, "voltage fault" },
  { 0x6f00, "copy protection key exchange failure - authentication failure" },
  { 0x6f01, "copy protection key exchange failure - key not present" },
  { 0x6f02, "copy protection key exchange failure -key not established" },
  { 0x6f03, "read of scrambled sector without authentication" },
  { 0x6f04, "media region code is mismatched to logical unit region" },
  { 0x6f05, "logical unit region must be permanent/region reset count error" },
  { 0x7200, "session fixation error" },
  { 0x7201, "session fixation error writing lead-in" },
  { 0x7202, "session fixation error writing lead-out" },
  { 0x7203, "session fixation error - incomplete track in session" },
  { 0x7204, "empty or partially written reserved track" },
  { 0x7205, "no more track reservations allowed" },
  { 0x7300, "cd control error" },
  { 0x7301, "power calibration area almost full" },
  { 0x7302, "power calibration area is full" },
  { 0x7303, "power calibration area error" },
  { 0x7304, "program memory area update failure" },
  { 0x7305, "program memory area is full" },
  { 0x7306, "rma/pma is almost full" }

} ;

// complete table as e.g. http://lxr.free-electrons.com/source/drivers/scsi/constants.c?v=3.2
// see also: http://www.t10.org/lists/1spc-lst.htm

static std::string senseCodeText(uint8_t asc,uint8_t ascq)
{
  if (asc == 0x40) {
    return Format::sprintf("diagnostic failure on component %02x",ascq) ;
  }
  if (asc == 0x4d) {
    return Format::sprintf("tagged overlapped commands (%02x = queue tag)",ascq) ;
  }
  uint16_t senseCode = asc ; senseCode <<= 8 ; senseCode += ascq ;
  for (size_t i=0 ; i<sizeof(error)/sizeof(error[0]) ; ++i) {
    if (error[i].senseCode == senseCode) {
      return error[i].text ;
    }
  }
  return "{no description}" ;
}

std::string Sg::Status::toString() const
{
  std::ostringstream os ;
  if (this->success()) {
    os << "status(success" ;
  }
  else {
    os << "status(failure" ;
    os << ",scsi:(" << scsiText(this->io.masked_status) << ",0x" << Format::hex(this->io.masked_status) << ")" ;
    os << ",host:(" << hostText(this->io.host_status) << ",0x" << Format::hex(this->io.host_status) << ")" ;
    os << ",driver:(" << driverText(this->io.driver_status) << ",0x" << Format::hex(this->io.driver_status) << ")" ;
    os << ",sense:" << Format::hex(this->sense) ;
    if (this->sense.size() > 0) {
      if ((this->sense[0] == 0x70) || (this->sense[0] == 0x71) || (this->sense[0] >= 0x80)) {
	if (this->sense.size() >= 3) {
	  uint8_t sk = sense[2] & 0x0f ;
	  os << ",key:" << senseKeyText(sk) << "(0x" << Format::hex(sk) << ")" ;
	  if (this->sense.size() >= 14) {
	    uint8_t asc = sense[12] ;
	    uint8_t ascq = sense[13] ;
	    os << ",code:(" << senseCodeText(asc,ascq) 
	       << ",0x" << Format::hex(asc) << ",0x" << Format::hex(ascq) << ")" ;
	  }
	}
      }
    }
    os << ")" ; // end of sense
  }
  os << ",duration:" << std::dec << io.duration/1000 << "." << std::setfill('0') << std::setw(3) << io.duration%100 ;
  os << ")"  ; // end of status
  return os.str() ;
}
