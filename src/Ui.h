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

#ifndef _Ui_h_
#define _Ui_h_

#include "ArgL.h"

namespace Ui
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("Ui:" + s) {}
  } ;

  void       authInvoke(ArgL &argL) ;
  void    decryptInvoke(ArgL &argL) ;
  void       discInvoke(ArgL &argL) ;
  void    featureInvoke(ArgL &argL) ;
  void  generatorInvoke(ArgL &argL) ;
  void  handshakeInvoke(ArgL &argL) ;
  void        ifoInvoke(ArgL &argL) ;
  void        isoInvoke(ArgL &argL) ;
  void      lfsrAinvoke(ArgL &argL) ;
  void      lfsrBinvoke(ArgL &argL) ;
  void        mmcInvoke(ArgL &argL) ;
  void       packInvoke(ArgL &argL) ;
  void    predictInvoke(ArgL &argL) ;
  void    recoverInvoke(ArgL &argL) ;
  void     revertInvoke(ArgL &argL) ;
  void         sgInvoke(ArgL &argL) ;
  void     spreadInvoke(ArgL &argL) ;
  void      studyInvoke(ArgL &argL) ;
  void unscrambleInvoke(ArgL &argL) ;
} 

#endif // _Ui_h_
