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

#include "file.h"

class ZipFile : public File {
private:

public:
  ZipFile( FILE *fp );
  virtual ~ZipFile();

  virtual void close();
  
protected:
  virtual int write( void *buff, size_t size, int count );
  virtual int read( void *buff, size_t size, int count );
};

#endif

