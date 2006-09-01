/***************************************************************************
                          modelwrapper.cpp  -  description
                             -------------------
    begin                : Thu Aug 31 2006
    copyright            : (C) 2006 by Gabor Torok
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

#include <string>  
#include "modelwrapper.h"
#include "Md2.h"
#include "Md3.h"

using namespace std;

CLoadMD2 md2Loader;

void ModelWrapper::loadModel( char *file_name ) {	
  char path[300];
  sprintf(path, "%s%s", rootDir, file_name );
  if( file_name[ strlen( file_name ) - 4 ] != '.' ) strcat( path, "/tris.md2" );
  
  char *tmp = strdup( path );
  char *name = tmp;
  name = strrchr( name, '/' );
  if( !name ) {
  	name = tmp;
  	name = strrchr( name, '\\' );
  	if( !name ) {
  		cerr << "*** Error: can't determine model name. file=" << path << endl;
  	}
  }
  
  if( name ) {
  	path[ strlen( path ) - strlen( name ) ] = '\0';
  }
  
  char full[300];
  sprintf( full, "%s%s", path, name );
  
	cerr << "&&&&&&&&&& Loading animated model: " << file_name << 
			" path: " << path << 
			" name: " << name << 
			" full: " << full << endl;  
  
	char *extension = name + strlen( name ) - 3;  
	
	if( !strcasecmp( extension, "md2" ) ) {		
	  t3DModel *t3d = new t3DModel;
	  md2Loader.ImportMD2( t3d, full ); 
	  this->md2 = t3d;
	  this->md3 = NULL;
	} else if( !strcasecmp( extension, "md3" ) ) {
		CModelMD3 *md3 = new CModelMD3();
		md3->LoadModel( path, name );		
	  this->md2 = NULL;
	  this->md3 = md3;
	} else {
		cerr << "Don't know how to load model of type: " << extension << " file=" << file_name << endl;
	}
	
	free( tmp );
}

void ModelWrapper::unloadModel() {
	if( md2 ) {
	  md2Loader.DeleteMD2( md2 );	
	  delete md2;
	  md2 = NULL;
	} else if( md3 ) {
		delete md3;
		md3 = NULL;
	} else {
		cerr << "Don't know how to unload model." << endl;
	} 
}
