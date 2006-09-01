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

enum {
	LOAD_MD2=0,
	LOAD_MD3
};

void ModelWrapper::loadModel( char *path, char *name ) {	

	int load = -1;
  char full[300];
  sprintf( full, "%s%s", rootDir, path );
  if( !name || name[ strlen( name ) - 4 ] == '.' ) {
		if( strcasecmp( path + strlen( path ) - 4, ".md2" ) ) strcat( full, "/tris.md2" );
		load = LOAD_MD2;
	} else {
		load = LOAD_MD3;
	}
    
	cerr << "&&&&&&&&&& Loading animated model: " << 
			" path: " << path << 
			" name: " << ( name ? name : "NULL" ) << 
			" full: " << full << endl;  
  
	if( load == LOAD_MD2 ) {		
	  t3DModel *t3d = new t3DModel;
	  md2Loader.ImportMD2( t3d, full ); 
	  this->md2 = t3d;
	  this->md3 = NULL;
	} else if( load == LOAD_MD3 ) {
		CModelMD3 *md3 = new CModelMD3();
		md3->LoadModel( full, name );		
		md3->SetTorsoAnimation( "TORSO_STAND" );
		md3->SetLegsAnimation( "LEGS_IDLE" );
	  this->md2 = NULL;
	  this->md3 = md3;
	} else {
		cerr << "Don't know how to load model. path=" << path << " name=" << name << endl;
	}
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
