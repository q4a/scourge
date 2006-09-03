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
#include "animatedshape.h"
#include "md2shape.h"
#include "md3shape.h"
#include "Md2.h"
#include "Md3.h"
#include "glshape.h"

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

// factory method to create shape
AnimatedShape *ModelWrapper::createShape( GLuint textureId, float div,
																					GLuint texture[], char *name, int descriptionGroup,
																					Uint32 color, Uint8 shapePalIndex) {
  int width, depth, height;
	normalizeModel( &width, &depth, &height, div, name );

	if( md2 ) {
		return new MD2Shape( md2, textureId, div, texture, width, depth, height,
												 name, descriptionGroup, color, shapePalIndex );
	} else if( md3 ) {
		return new MD3Shape( md3, div, texture, width, depth, height,
												 name, descriptionGroup, color, shapePalIndex );
	} else {
		cerr << "*** Error: Can't create animated shape." << endl;
		return NULL;
	}
}

void ModelWrapper::normalizeModel( int *width, int *depth, int *height, float div, char *name ) {
	vect3d min, max;
	min[0] = min[1] = min[2] = 100000.0f;	// BAD!!
	max[0] = max[1] = max[2] = 0.0f;
	// bogus initial value
	*width = *depth = *height = 1;

	if( md2 ) CLoadMD2::findBounds( md2, min, max );
	else if( md3 ) md3->findBounds( min, max );
	else cerr << "*** Error: can't find bounds for this type of model." << endl;
			
	for (int t = 0; t < 3; t++)	max[t] -= min[t];

	if( md3 ) cerr << "*** 1 max=" << max[2] << "," << max[0] << "," << max[1] << endl;
	if( md3 ) cerr << "*** 1 min=" << min[2] << "," << min[0] << "," << min[1] << endl;
		
	// set the dimensions
	float fw = max[2] * div * DIV;
	float fd = max[0] * div * DIV;
	float fh = max[1] * div * DIV;
		
	// make it a square
	if (fw > fd) fd = fw;
	else fw	= fd;

	if( md3 ) cerr << "*** 2 fw=" << fw << " fd=" << fd << " fh=" << fh << endl;
		
	// make it a min size (otherwise pathing has issues)
	if ( fw < 3 )	fw = 3;
	if ( fd < 3 )	fd = 3;
		
	// set the shape's dimensions
	*width = (int)( fw + ( (float)((int)fw) == fw ? 0 : 1 ) );
	*depth = (int)( fd + ( (float)((int)fd) == fd ? 0 : 1 ) );
	*height = toint(fh);	

	if( md3 )
		cerr << "Creating shape for model=" << name << 
		" width=" << *width << " depth=" << *depth << " height=" << *height << endl;
	
	if( md2 ) CLoadMD2::normalize( md2, min, max );
	else if( md3 ) md3->normalize( min, max );
	else cerr << "*** Error: can't normalize this type of model." << endl;
	
}
