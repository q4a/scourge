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

#include <iostream.h>
#include <stdlib.h>
#include <strings.h>

#include "scourge.h"

bool checkFile(const char *dir, const char *file) {
  char path[300];
  strcpy(path, dir);
  strcat(path, file);
  //fprintf(stderr, "\tchecking path: %s\n", path);
  bool ret = true;
  FILE *fp = fopen(path, "rb");
  if(!fp || ferror(fp)) ret = false;
  if(fp) fclose(fp);
  return ret;
}

// this function is used to be able to run scourge while developing
void findLocalResources(const char *appPath, char *dir) {
  // Where are we running from?
  strcpy(dir, appPath);	 
  // Look in this and the parent dir for a 'data' folder
  for(int i = 0; i < 2; i++) {
	char *p = strrchr(dir, SEPARATOR);
	if(!p) {
	  dir[0] = '\0';
	  cerr << "*** Can't find local version of data dir. You're running a distribution." << endl;
	  return;
	}	
	*(p + 1) = 0;
	//	fprintf(stderr, "*** Looking at: dir=%s\n", dir);
	if(checkFile(dir, "data/cursor.bmp")) return;
	// remove the last separator
	*(p) = 0;
  }
}

int main(int argc, char *argv[]) {

  // Check to see if there's a local version of the data dir
  // (ie. we're running in the build folder and not in a distribution)
  char dir[300];
  findLocalResources(argv[0], dir);
  if(strlen(dir)) {
	cerr << "*** Using local data dir. Not running a distribution." << endl;
	sprintf(rootDir, "%sdata", dir);
  }
  
  // config check
  if(argc >= 2 && !strcmp(argv[1], "--test-config")) {
	cerr << "Configuration:" << endl;
	char dir[300];
	char file[500];
	int dir_res = get_config_dir_name( dir, 300 );
	int file_res = get_config_file_name( file, 500 );
	cerr << "starting app: " << argv[0] << endl;
	cerr << "rootDir=" << rootDir << 
	  "\nconfigDir=" << configDir << 
	  "\nconfigFile=" << CONFIG_FILE << 
	  "\ndir=" << dir << " dir_res=" << dir_res <<
	  "\nfile=" << file << " file_res=" << file_res <<	endl;
	exit(0);
  }

  // do a final sanity check before running the game
  if(!checkFile(rootDir, "/cursor.bmp")) {
	cerr << "ERROR: check for files failed in data dir: " << rootDir << endl;
	cerr << "Either install the data files at the above location, or rebuild with ./configure --with-data-dir=<new location> or run the game from the source distribution's main directory (the one that contains src,data,etc.)" << endl;
	exit(1);
  }

  new Scourge(argc, argv);
  return EXIT_SUCCESS;
}
