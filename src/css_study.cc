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
#include "Ui.h"

int main(int argc, char *argv[])
{
  try {
    ArgL argL = ArgL::make(argc-1,argv+1) ;
    Logging::set(Format::number<Logging::Level::Type>(argL.option("--log","0"))) ;
    
    if (argL.empty()) {
      throw Ui::Error("missing command. you may try help.") ;
    }

    std::string arg = argL.pop() ;
    if (arg == "help") {
      std::cout 
	<< "usage: [--log 0..4] command option ..." << std::endl 
	<< "supported commands are:" << std::endl 
	<< "auth:       Encryption of authentication keys (KEY1, KEY2, buskey)" << std::endl 
	<< "decrypt:    Decryption of disc-key, title-key and feature" << std::endl 
	<< "feature:    List feature (file-system) details" << std::endl 
	<< "generator:  Keystream generator operations" << std::endl
	<< "handshake:  Retrieval of disc-key-block and title-keys; unlock DVD" << std::endl
	<< "ifo:        List IFO details and verify with feature (file-system) data" << std::endl
	<< "lfsr-a:     LFSR-A operations" << std::endl
	<< "lfsr-b:     LFSR-B operations" << std::endl
	<< "mmc:        SCSI MMC-5 operations (low-level)" << std::endl
	<< "pack:       List PES ID, VTS numbers and examine scrambling values for DVD-Packs" << std::endl
	<< "predict:    Predict a plain-text pattern for DVD-Packs" << std::endl
	<< "recover:    Recover disc-, player- and title-keys by exploit" << std::endl 
	<< "revert:     Find the generator for a keystream (correlation exploit)" << std::endl
	<< "sg:         The Linux SCSI Generic (low-level)" << std::endl
	<< "spread:     Find unique keystreams" << std::endl
	<< "study:      Brute-force recovery of disc- and player-keys" << std::endl
	<< "unscramble: Decrypt disc-key, title-keys and feature" << std::endl ;
    }

    else if (arg ==       "auth") { Ui::      authInvoke(argL) ; }
    else if (arg ==    "decrypt") { Ui::   decryptInvoke(argL) ; }
    else if (arg ==    "feature") { Ui::   featureInvoke(argL) ; }
    else if (arg ==  "generator") { Ui:: generatorInvoke(argL) ; }
    else if (arg ==  "handshake") { Ui:: handshakeInvoke(argL) ; }
    else if (arg ==        "ifo") { Ui::       ifoInvoke(argL) ; }
    else if (arg ==        "iso") { Ui::       isoInvoke(argL) ; }
    else if (arg ==     "lfsr-a") { Ui::     lfsrAinvoke(argL) ; }
    else if (arg ==     "lfsr-b") { Ui::     lfsrBinvoke(argL) ; }
    else if (arg ==        "mmc") { Ui::       mmcInvoke(argL) ; }
    else if (arg ==       "pack") { Ui::      packInvoke(argL) ; }
    else if (arg ==    "predict") { Ui::   predictInvoke(argL) ; }
    else if (arg ==    "recover") { Ui::   recoverInvoke(argL) ; }
    else if (arg ==     "revert") { Ui::    revertInvoke(argL) ; }
    else if (arg ==         "sg") { Ui::        sgInvoke(argL) ; }
    else if (arg ==     "spread") { Ui::    spreadInvoke(argL) ; }
    else if (arg ==      "study") { Ui::     studyInvoke(argL) ; }
    else if (arg == "unscramble") { Ui::unscrambleInvoke(argL) ; }

    else {
      throw Ui::Error("not supported command. you may try help instead.") ;
    }

  }
  catch (Ui::Error &error) {
    std::cerr << error.what() << std::endl ;
  }
}

