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
#include <zlib.h>
#include "zipfile.h"

#define CHUNK 16384
 
ZipFile::ZipFile( FILE *fp ) : File( fp ) {
}

ZipFile::~ZipFile() {
}
  
int ZipFile::write( void *buff, size_t size, int count ) {
  return File::write( buff, size, count );
}

int ZipFile::read( void *buff, size_t size, int count ) {
  return File::read( buff, size, count );
}

void ZipFile::close() {
  File::close();
}
