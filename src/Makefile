# Copyright (c) 2015, helo2804. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions 
# are met:
#
# * Redistributions of source code must retain the above copyright 
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright 
#   notice, this list of conditions and the following disclaimer in 
#   the documentation and/or other materials provided with the 
#   distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
# WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.

# This projected is hosted at https://github.com/helo2804/css_study

LSRC=	ArgL.cc \
	Auth.cc \
	Css.cc \
	Decrypt.cc \
	Feature.cc \
	Format.cc \
	Handshake.cc \
	Ifo.cc \
	Iso9660.cc \
	Logging.cc \
	Mmc.cc \
	Os.cc \
	Pack.cc \
	Path.cc \
	Predict.cc \
	Reader.cc \
	Recover.cc \
	Revert.cc \
	Sg.cc \
	Spread.cc \
	uiAuth.cc \
	uiDecrypt.cc \
	uiFeature.cc \
	uiGenerator.cc \
	uiHandshake.cc \
	uiIfo.cc \
	uiIso.cc \
	uiLfsrA.cc \
	uiLfsrB.cc \
	uiMmc.cc \
	uiPack.cc \
	uiPredict.cc \
	uiRecover.cc \
	uiRevert.cc \
	uiSg.cc \
	uiStudy.cc \
	uiSpread.cc \
	uiUnscramble.cc \
	Unscramble.cc

BSRC=	css_study.cc 

css_study : css_study.o

SRC=$(LSRC) $(BSRC)

# ---------------------------------------------------------------------

%.cc.d : %.cc
	g++ -E $(@:.d=) -MMD -MF $@ > /dev/null

DEP=$(SRC:=.d)

-include $(DEP)

# ---------------------------------------------------------------------

LOBJ=$(LSRC:.cc=.o) 

BOBJ=$(BSRC:.cc=.o) 

OBJ=$(LOBJ) $(BOBJ)

BIN=$(BOBJ:.o=)

# ---------------------------------------------------------------------

CC=g++ -std=c++98 -Wall -Wextra -pedantic -g3 -O3

$(OBJ): 
	$(CC) -o $@ -c $(@:.o=.cc)

LD=g++ 

$(BIN): $(LOBJ)
	$(LD) -o $@ $^

# ---------------------------------------------------------------------

all: $(BIN)

clean:
	rm -f $(DEP) $(OBJ) $(BIN)
