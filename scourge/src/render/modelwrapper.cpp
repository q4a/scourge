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

ModelLoader::ModelLoader( bool headless, GLuint *textureGroup ) {
	this->headless = headless;
	this->textureGroup = textureGroup;
}

ModelLoader::~ModelLoader() {
}

GLShape *ModelLoader::getCreatureShape( char *model_name, 
																				char *skin_name, 
																				float scale ) {

  cerr << "=====================================================" << endl << 
		"getCreatureShape: model_name=" << model_name << 
		" skin=" << ( skin_name ? skin_name : "NULL" ) << endl;
  
	char skinPath[300];
  Md2ModelInfo *model_info;
  string model_name_str = model_name;

	// construct skin name
	sprintf( skinPath, "%s%s/%s", rootDir, model_name, skin_name );

	// load the model the new way
	if( creature_models.find( model_name_str ) == creature_models.end() ) {
		model_info = (Md2ModelInfo*)malloc( sizeof( Md2ModelInfo ) );

		model_info->wrapper.loadModel( model_name, skin_name, this );
		strcpy( model_info->name, model_name );
		model_info->scale = 1.0f;

		creature_models[ model_name_str ] = model_info;
	} else {
		model_info = creature_models[ model_name_str ];
	}

	// increment its ref. count
	if(loaded_models.find(model_info) == loaded_models.end()) {
		loaded_models[model_info] = 1;
	} else {
		loaded_models[model_info] = loaded_models[model_info] + 1;
	}    
  //  cerr << "Creating creature shape with model: " << model << " and skin: " << skin << endl;

  // create the shape.
  // FIXME: shapeindex is always FIGHTER. Does it matter?
  AnimatedShape *shape = 
		model_info->wrapper.createShape( loadSkinTexture( skinPath ), 
																		 (scale == 0.0f ? model_info->scale : scale),
																		 textureGroup, model_info->name, 
																		 -1, 0xf0f0ffff, 0);
	return shape;
}

GLuint ModelLoader::loadSkinTexture( char *skin_name ) {

	// md3-s load their own
	cerr << "&&&&&&&&&& Trying texture: " << skin_name << endl;
	if( skin_name[ strlen( skin_name ) - 4 ] != '.' ) {
		cerr << "\t&&&&&&&&&& skipping it." << endl;
		return 0;
	}

  // find or load the skin
  string skin = skin_name;
  GLuint skin_texture;
  if(creature_skins.find(skin) == creature_skins.end()){
    if( !headless ) {
      cerr << "&&&&&&&&&& Loading texture: " << skin_name << endl;
      CreateTexture(&skin_texture, skin_name, 0);
			cerr << "\t&&&&&&&&&& Loaded texture: " << skin_texture << endl;
      creature_skins[skin] = skin_texture;
    }
  } else {
    skin_texture = creature_skins[skin];
  }

  // increment its ref. count
  if(loaded_skins.find(skin_texture) == loaded_skins.end()) {
    loaded_skins[skin_texture] = 1;
  } else {
    loaded_skins[skin_texture] = loaded_skins[skin_texture] + 1;
	}
	cerr << "&&&&&&&&&& Texture ref count at load for id: " << skin_texture << 
		" count: " << loaded_skins[skin_texture] << endl;
	
	return skin_texture;
}

void ModelLoader::unloadSkinTexture( char *skin_name ) {

	// md3-s unload their own
	if( skin_name[ strlen( skin_name ) - 4 ] != '.' ) return;

  string skin = skin_name;
  GLuint skin_texture;
  if( creature_skins.find(skin) == creature_skins.end() ) {
		cerr << "&&&&&&&&&& WARNING: could not find skin: " << skin << endl;
		return;
	} else {
		skin_texture = creature_skins[skin];
	}

  if (loaded_skins.find(skin_texture) == loaded_skins.end()) {
    cerr << "&&&&&&&&&& WARNING: could not find skin id=" << skin_texture << endl;
    return;
  }

  loaded_skins[skin_texture] = loaded_skins[skin_texture] - 1;
	cerr << "&&&&&&&&&& Texture ref count at load for id: " << skin_texture << 
		" count: " << loaded_skins[skin_texture] << endl;
	// unload texture if no more references
  if (loaded_skins[skin_texture] == 0) {
    cerr << "&&&&&&&&&& Deleting texture: " << skin_texture << endl;
    loaded_skins.erase(skin_texture);
    creature_skins.erase(skin);
    glDeleteTextures(1, &skin_texture);
  }

}

void ModelLoader::decrementSkinRefCount( char *model_name, 
																					char *skin_name ) {

	char skinPath[300];
	sprintf( skinPath, "%s%s/%s", rootDir, model_name, skin_name );
	unloadSkinTexture( skinPath );

  string model = model_name;
  Md2ModelInfo *model_info;
  if (creature_models.find(model) == creature_models.end()) {
    cerr << "&&&&&&&&&& Not unloading model: " << model << endl;
    return;
  } else {
    model_info = creature_models[model];
  }

  if (loaded_models.find(model_info) == loaded_models.end()) {
    cerr << "&&&&&&&&&& WARNING: could not find model id=" << model << endl;
    return;
  }

  loaded_models[model_info] = loaded_models[model_info] - 1;

  // unload model if no more references  
  if (loaded_models[model_info] == 0) {
//    cerr << "&&&&&&&&&& Deleting model: " << model << endl;
    loaded_models.erase(model_info);
    creature_models.erase(model);

    model_info->wrapper.unloadModel();

    free(model_info);
  }
}



// ----------------------------------------------------------
// ----------------------------------------------------------
// ----------------------------------------------------------



void ModelWrapper::loadModel( char *path, char *name, ModelLoader *loader ) {	
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
		" type=" << ( load == LOAD_MD2 ? "md2" : ( load == LOAD_MD3 ? "md3" : "unknown" ) ) <<
			" path: " << path << 
			" name: " << ( name ? name : "NULL" ) << 
			" full: " << full << endl;  
  
	if( load == LOAD_MD2 ) {		
	  t3DModel *t3d = new t3DModel;
	  md2Loader.ImportMD2( t3d, full ); 
	  this->md2 = t3d;
	  this->md3 = NULL;
	} else if( load == LOAD_MD3 ) {
		CModelMD3 *md3 = new CModelMD3( loader );
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

	//if( md3 ) cerr << "*** 1 max=" << max[2] << "," << max[0] << "," << max[1] << endl;
	//if( md3 ) cerr << "*** 1 min=" << min[2] << "," << min[0] << "," << min[1] << endl;
		
	// set the dimensions
	float fw = max[2] * div * DIV;
	float fd = max[0] * div * DIV;
	float fh = max[1] * div * DIV;
		
	// make it a square
	if (fw > fd) fd = fw;
	else fw	= fd;

	//if( md3 ) cerr << "*** 2 fw=" << fw << " fd=" << fd << " fh=" << fh << endl;
		
	// make it a min size (otherwise pathing has issues)
	if ( fw < 3 )	fw = 3;
	if ( fd < 3 )	fd = 3;
		
	// set the shape's dimensions
	*width = (int)( fw + ( (float)((int)fw) == fw ? 0 : 1 ) );
	*depth = (int)( fd + ( (float)((int)fd) == fd ? 0 : 1 ) );
	*height = toint(fh);	

	if( md3 )
		cerr << "Creating shape of type=" << ( md2 ? "md2" : ( md3 ? "md3" : "unknown" ) ) << 
		" for model=" << name << 
		" width=" << *width << " depth=" << *depth << " height=" << *height << endl;
	
	if( md2 ) CLoadMD2::normalize( md2, min, max );
	else if( md3 ) md3->normalize( min, max );
	else cerr << "*** Error: can't normalize this type of model." << endl;
	
}
