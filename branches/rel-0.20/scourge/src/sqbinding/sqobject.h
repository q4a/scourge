/***************************************************************************
                          sqobject.h  -  description
                             -------------------
    begin                : Sat Oct 8 2005
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

#ifndef SQOBJECT_H
#define SQOBJECT_H

#include "sqbinding.h"
#include <set>

/**
 * The interface all squirrel objects implement.
 */
class SqObject {
public:
  SqObject();
  virtual ~SqObject();

  virtual const char *getInstanceName() = 0;
  virtual const char *getClassName() = 0;
  virtual SquirrelClassDecl *getClassDeclaration() = 0;

  void documentSOM( char *path, std::set<std::string> *names );

private:
  char *describeTypeMask( const SQChar *typemask, 
                          char *buffer );
  char *describeReturnType( const char *returnType, 
                            std::set<std::string> *names, 
                            char *buffer, size_t bufferSize );

};

#endif

