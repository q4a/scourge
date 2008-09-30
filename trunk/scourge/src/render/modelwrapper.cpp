/***************************************************************************
             modelwrapper.cpp  -  Generic character model loader
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
#include "shapes.h"
#include "../shapepalette.h"

using namespace std;

CLoadMD2 md2Loader;

// #define DEBUG_LOADING 1

enum {
	LOAD_MD2=0,
	LOAD_MD3
};

ModelLoader::ModelLoader( ShapePalette *shapePal, bool headless, GLuint *textureGroup ) {
	this->shapePal = shapePal;
	this->headless = headless;
	this->textureGroup = textureGroup;
}

ModelLoader::~ModelLoader() {
}

void ModelLoader::clearModel( t3DModel *pModel ) {
  pModel->numOfObjects = 0;
  pModel->numOfMaterials = 0;
  pModel->numOfAnimations = 0;
  pModel->numOfTags = 0;
  pModel->pLinks.clear();
  pModel->pTags = NULL;
  pModel->movex = 0;
  pModel->movey = 0;
  pModel->movez = 0;
  pModel->vertices = NULL;
  pModel->numVertices = 0;
  pModel->pGlCommands = NULL;
}

GLShape *ModelLoader::getCreatureShape( char *model_name, 
																				char *skin_name, 
																				float scale ) {

#ifdef DEBUG_LOADING
  cerr << "=====================================================" << endl << 
		"getCreatureShape: model_name=" << model_name << 
		" skin=" << ( skin_name ? skin_name : "NULL" ) << endl;
#endif
  
  Md2ModelInfo *model_info;
  string model_name_str = model_name;
	string skinPath = rootDir + model_name_str + "/" + skin_name;

	// load the model the new way
	if( creature_models.find( model_name_str ) == creature_models.end() ) {
		model_info = new Md2ModelInfo;

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

  // create the shape.
  // FIXME: shapeindex is always FIGHTER. Does it matter?
  AnimatedShape *shape = 
		model_info->wrapper.createShape( loadSkinTexture( skinPath ), 
																		 (scale == 0.0f ? model_info->scale : scale),
																		 textureGroup, model_info->name, 
																		 -1, 0xf0f0ffff, 0,
																		 model_name, skin_name, 
                                     this );
	return shape;
}

GLuint ModelLoader::loadSkinTexture( const string& skin_name ) {
	// md3-s load their own
#ifdef DEBUG_LOADING
	cerr << "&&&&&&&&&& Trying texture: " << skin_name << endl;
#endif
	if( skin_name[skin_name.length() - 4] != '.' ) {
#ifdef DEBUG_LOADING
		cerr << "\t&&&&&&&&&& skipping it." << endl;
#endif
		return 0;
	}

  // find or load the skin
  string skin = skin_name;
  GLuint skin_texture;
  if(creature_skins.find(skin) == creature_skins.end()){
    if( !headless ) {
#ifdef DEBUG_LOADING
      cerr << "&&&&&&&&&& Loading texture: " << skin_name << endl;
#endif

      skin_texture = shapePal->loadTexture( skin, true, false );

#ifdef DEBUG_LOADING
      cerr << "\t&&&&&&&&&& Loaded texture: " << skin_texture << endl;
#endif
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
#ifdef DEBUG_LOADING
	cerr << "&&&&&&&&&& Texture ref count at load for id: " << skin_texture << 
		" count: " << loaded_skins[skin_texture] << endl;
#endif
	
	return skin_texture;
}

void ModelLoader::unloadSkinTexture( const string& skin_name ) {

	// md3-s unload their own
	if( skin_name[ skin_name.length() - 4 ] != '.' ) {
#ifdef DEBUG_LOADING
		cerr << "\t&&&&&&&&&& skipping it." << endl;
#endif
		return;
	}

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
#ifdef DEBUG_LOADING
	cerr << "&&&&&&&&&& Texture ref count at load for id: " << skin_texture << 
		" count: " << loaded_skins[skin_texture] << endl;
#endif
	// unload texture if no more references
  if (loaded_skins[skin_texture] == 0) {
#ifdef DEBUG_LOADING
    cerr << "&&&&&&&&&& Deleting texture: " << skin_texture << endl;
#endif
    loaded_skins.erase(skin_texture);
    creature_skins.erase(skin);
    glDeleteTextures(1, &skin_texture);
  }

}

void ModelLoader::decrementSkinRefCount( char *model_name, char *skin_name ) {

#ifdef DEBUG_LOADING
  cerr << "=====================================================" << endl << 
		"decrementSkinRefCount: model_name=" << model_name << 
		" skin=" << ( skin_name ? skin_name : "NULL" ) << endl;
#endif

	string skinPath = rootDir + model_name + "/" + skin_name;
	unloadSkinTexture( skinPath );

  string model = model_name;
  Md2ModelInfo *model_info;
  if (creature_models.find(model) == creature_models.end()) {
    cerr << "&&&&&&&&&& WARNING: Not unloading model: " << model << endl;
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
#ifdef DEBUG_LOADING
    cerr << "&&&&&&&&&& Deleting model: " << model << endl;
#endif
    loaded_models.erase(model_info);
    creature_models.erase(model);

    model_info->wrapper.unloadModel();

    delete model_info;
  }
}

void ModelLoader::debugModelLoader() {
#ifdef DEBUG_LOADING
	cerr << "****************************************" << endl;
	cerr << "Loaded models: " << endl;
	for( map<string, Md2ModelInfo*>::iterator i = creature_models.begin();
			 i != creature_models.end(); ++i ) {
		string key = i->first;
		Md2ModelInfo *model_info = i->second;
		cerr << "\t" << key << " " << loaded_models[ model_info ] << " references." << endl;
	}
	cerr << "Loaded skins: " << endl;
	for( map<string, GLuint>::iterator i = creature_skins.begin();
			 i != creature_skins.end(); ++i ) {
		string key = i->first;
		GLuint id = i->second;
		cerr << "\t" << key << " " << loaded_skins[ id ] << " references." << endl;
	}
	cerr << "****************************************" << endl;
#endif
}



// ----------------------------------------------------------
// ----------------------------------------------------------
// ----------------------------------------------------------



void ModelWrapper::loadModel( const string& path, char *name, ModelLoader *loader ) {	
	int load = -1;
  string full = rootDir + path;
	// if name ends in .bmp (or other image extension) it's an md2
  if( !name || name[ strlen( name ) - 4 ] == '.' ) {
		if( path.substr(path.length() - 4).compare(".md2" ) != 0 )
			full += "/tris.md2";
		load = LOAD_MD2;
	} else {
		load = LOAD_MD3;
	}
    
#ifdef DEBUG_LOADING
	cerr << "&&&&&&&&&& Loading animated model: " << 
		" type=" << ( load == LOAD_MD2 ? "md2" : ( load == LOAD_MD3 ? "md3" : "unknown" ) ) <<
			" path: " << path << 
			" name: " << ( name ? name : "NULL" ) << 
			" full: " << full << endl;  
#endif
  
	if( load == LOAD_MD2 ) {		
	  t3DModel *t3d = new t3DModel;
	  md2Loader.ImportMD2( t3d, full ); 
	  this->md2 = t3d;
	  this->md3 = NULL;
	} else if( load == LOAD_MD3 ) {
		CModelMD3 *md3 = new CModelMD3( loader );
		md3->LoadModel( full );
		//md3->loadSkins( full, name );
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
																					GLuint texture[], char *name, 
																					int descriptionGroup,
																					Uint32 color, Uint8 shapePalIndex,
																					char *model_name, char *skin_name,
                                          ModelLoader *loader ) {
  int width, depth, height;
	normalizeModel( &width, &depth, &height, div, name );

	if( md2 ) {
		return new MD2Shape( md2, textureId, div, texture, width, depth, height,
												 name, descriptionGroup, color, shapePalIndex );
	} else if( md3 ) {
		MD3Shape *shape = 
			new MD3Shape( md3, loader, div, texture, width, depth, height,
										name, descriptionGroup, color, shapePalIndex );
		string full = rootDir + model_name;
		md3->loadSkins( full, skin_name, shape );
		return shape;
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
	float fw = max[2] * div / MUL;
	float fd = max[0] * div / MUL;
	float fh = max[1] * div / MUL;
		
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

#ifdef DEBUG_LOADING
		cerr << "Creating shape of type=" << ( md2 ? "md2" : ( md3 ? "md3" : "unknown" ) ) << 
		" for model=" << name << 
		" width=" << *width << " depth=" << *depth << " height=" << *height << endl;
#endif
	
	if( md2 ) CLoadMD2::normalize( md2, min, max );
	else if( md3 ) md3->normalize( min, max );
	else cerr << "*** Error: can't normalize this type of model." << endl;
	
}
