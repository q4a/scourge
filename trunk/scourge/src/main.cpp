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

int main(int argc, char *argv[]) {

  if(argc >= 2 && !strcmp(argv[1], "--test-config")) {
	cerr << "Configuration:" << endl;
	char dir[300];
	char file[500];
	int dir_res = get_config_dir_name( dir, 300 );
	int file_res = get_config_file_name( file, 500 );
	cerr << "rootDir=" << rootDir << "\nconfigDir=" << configDir << "\nconfigFile=" << CONFIG_FILE << 
	  "\ndir=" << dir << " dir_res=" << dir_res <<
	  "\nfile=" << file << " file_res=" << file_res <<	endl;
	exit(0);
  }

  new Scourge(argc, argv);
  return EXIT_SUCCESS;
}
