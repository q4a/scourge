/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Sat May  3 19:39:34 EDT 2003
    copyright            : (C) 2003 by Gabor Torok
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream.h>
#include <stdlib.h>
#include <strings.h>

#include "scourge.h"

int main(int argc, char *argv[]) {
  // Where are we running from?
  fprintf(stderr, "argv[0]=%s\n", argv[0]);
  strcpy(rootDir, argv[0]);
  char *p = strrchr(rootDir, SEPARATOR);
  if(!p) {
	  fprintf(stderr, "Can't parse argv[0]=%s\n", argv[0]);
	  exit(1);
  }
  *(p + 1) = 0;
  fprintf(stderr, "rootDir=%s\n", rootDir);

  if(argc > 1 && !strcmp(argv[1], "--fullscreen")) {
    new Scourge(800, 600, 32, true);
  } else {
    new Scourge(800, 600, 32, false);
  }
  return EXIT_SUCCESS;
}
