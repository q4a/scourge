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

bool checkFile(const char *dir, const char *file) {
	 char path[300];
	 strcpy(path, dir);
	 strcat(path, file);
	 fprintf(stderr, "\tchecking path: %s\n", path);
	 bool ret = true;
	 FILE *fp = fopen(path, "rb");
	 if(!fp || ferror(fp)) ret = false;
	 if(fp) fclose(fp);
	 return ret;
}

void findResources(const char *appPath) {
	 // Where are we running from?
	 strcpy(rootDir, appPath);	 
	 while(1) {
		  char *p = strrchr(rootDir, SEPARATOR);
		  if(!p) {
				fprintf(stderr, "Can't find data dir!\n");
				exit(1);
		  }	
		  *(p + 1) = 0;
		  fprintf(stderr, "Looking at: rootDir=%s\n", rootDir);
		  if(checkFile(rootDir, "data/cursor.bmp")) return;
		  // remove the last separator
		  *(p) = 0;
	 }
}

int main(int argc, char *argv[]) {
	 fprintf(stderr, "argv[0]=%s\n", argv[0]);
	 findResources(argv[0]);
  if(argc > 1 && !strcmp(argv[1], "--fullscreen")) {
    new Scourge(800, 600, 32, true);
  } else {
    new Scourge(800, 600, 32, false);
  }
  return EXIT_SUCCESS;
}
