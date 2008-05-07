/***************************************************************************
                          zipfile.h  -  description
                             -------------------
    begin                : Sat Jul 29, 2005
    copyright            : (C) 2005 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ZIP_FILE_H
#define ZIP_FILE_H

#include <zlib.h>
#include "file.h"

#define CHUNK 16384

                   
/**
 * Save file with zip compression. Uses zlib. Code adapted from zlib example code:
 * http://www.zlib.net/zpipe.c
 */
class ZipFile : public File {
private:
  int mode;
  z_stream strm;
  char zipBuff[ CHUNK ], tmpBuff[ CHUNK ];
  int tmpBuffOffset;
  int ret;

public:
  enum {
    ZIP_READ=0,
    ZIP_WRITE
  };

  ZipFile( FILE *fp, int mode = ZIP_WRITE, int level = Z_DEFAULT_COMPRESSION );
  virtual ~ZipFile();

protected:
  virtual int write( void *buff, size_t size, int count );
  virtual int writeZip( int flush );

  virtual int read( void *buff, size_t size, int count );
  virtual int inflateMore();
};

#endif

