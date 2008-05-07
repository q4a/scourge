/***************************************************************************
                          sqbinding.h  -  description
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

#ifndef CONSOLE_PRINTER_H
#define CONSOLE_PRINTER_H

#include <iostream>
#include <string>
#include <vector>

/**
 * An interface for console printing.
 */
class ConsolePrinter {
public:
  ConsolePrinter() {
  }

  virtual ~ConsolePrinter() {
  }

  virtual void printToConsole( const char *s ) = 0;
};

#endif

