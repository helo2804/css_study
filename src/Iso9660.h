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

// ====================================================================
// Very basic read-only access to ISO-9660 file system / volumes
//   (e.g. without support for file interleaving)
//
// For ISO-9660 see ECMA-119 (1987):
// "Volume and File Structure of CDROM for Information Interchange"
// ====================================================================

#ifndef _Iso9660_h_
#define _Iso9660_h_

#include "Domain.hpp"
#include "Error.h"
#include "Path.h"
#include "Volume.hpp"

#include <stdint.h>
#include <string>
#include <vector>

#include <boost/optional.hpp>

namespace Iso9660 // ISO-9660 file-system
{
  struct Error : public ::Error
  {
    Error(std::string const &s) : ::Error("ISO9660:" + s) {}
  } ;

  typedef uint32_t Lba ; // logical block address

  typedef Volume<Lba,uint32_t,0x800u> V ;

  struct Descriptor ; 

  struct Record // ISO-9660 directory record
  {
    // note: the root record is constructed by the Descriptor

    typedef boost::optional<Record> Optional ;

    typedef uint32_t FileSizeType ;

    typedef Domain<unsigned,0u,0x7fffu> Version ;

    // true if the record concerns a directory (otherwise it's a file)
    bool isDir() const ;

    // starting logical block address (a block holds 2048 bytes)
    Lba lba() const ;

    // file or directory size in bytes
    FileSizeType size() const ;

    // file/dir name including version number (e.g. "VIDEO_TS.BUP;1")
    std::string id() const ;
    // -- "\x00" for parent directory ("..")
    // -- "\x01" for current directory (".")

    // file name w/o version number (e.g. "VIDEO_TS.BUP" w/o ";1")
    static std::string name(std::string const &id) ;

    // file name's version number in the range 0|1..32767
    //   e.g. return 1 for "VIDEO_TS.BUP;1" 
    //   note: directory names do never have version numbers
    static Version version(std::string const &id) ;
    // there are volumes without any file's version numbers (which
    // does not conform to the specification). in that case 0 (zero) 
    // is returned.

    // return the named record (name w/o version number) in the file-
    // system. 'this' must be a directory.
    Optional find(V *volume,std::string const &name) const ;

    // return the named record (name w/o version number) in the file-
    // system. 'this' must be a directory. path is relative.
    Optional find(V *volume,Path const &path) const ;

    // list record's directory contents. 'this' must be a directory.
    // set (*seek)=0 ; iteratively call list() until no record is 
    // returned.
    Optional list(V *volume,FileSizeType *seek) const ;
    // note: a method which returns the complete directory list can be
    // kind of dangerous if the directory is really huge or if the 
    // file system is corrupted. This might lead to some kind of resource
    // bottleneck.
    // as alternative: return the directory entries one by one (as 
    // above), or, count the number of directory entries beforehand.

  private:

    friend class Descriptor ; // holds a record for the root directory

    // directory-record data-block (copied from the volume)
    std::vector<uint8_t> data ;

    Record(Range<uint8_t const> r) : data(r.begin(),r.end()) {}
  } ;


  struct Descriptor // primary ISO descriptor of the volume
  {
    // search for the descriptor on the volume
    static Descriptor find(V *volume) ;

    // volume date (may not be set / empty)
    std::string date() const ;

    // volume id (not neccessary unique)
    std::string id() const ;

    // volume's root directory
    Record root() const ;

  private:

    typedef Array<uint8_t,V::blockSize> Data ;
    
    // primary-descriptor data-block (copied from the volume)
    Data data ;

    Descriptor(Data const &data) : data(data) {}

  } ;
}

#endif // _Iso9660_h_
