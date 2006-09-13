/***************************************************************************
                          modelwrapper.h  -  description
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

#ifndef MODEL_WRAPPER_H
#define MODEL_WRAPPER_H

#include "render.h"
#include <map>
#include <string>

class CModelMD3;
class AnimatedShape;
class GLShape;
class ModelLoader;

class ModelWrapper  {
private:
	t3DModel *md2;
	CModelMD3 *md3;

public:
	inline t3DModel *getMd2() { return md2; }
	inline CModelMD3 *getMd3() { return md3; }

	void loadModel( char *path, char *name, ModelLoader *loader );
	void unloadModel();
	AnimatedShape *createShape( GLuint textureId, float div,
															GLuint texture[], char *name, int descriptionGroup,
															Uint32 color, Uint8 shapePalIndex,
															char *model_name, char *skin_name );

	void normalizeModel( int *width, int *depth, int *height, float div, char *name );
	
};

typedef struct _Md2ModelInfo {
	ModelWrapper wrapper;
  char name[100];
  char filename[100];
  float scale;	
} Md2ModelInfo;

class ModelLoader {
private:
	bool headless;
	GLuint *textureGroup;
	std::map<std::string, GLuint> creature_skins;
	std::map<GLuint, int> loaded_skins;
	std::map<std::string, Md2ModelInfo*> creature_models;
	std::map<Md2ModelInfo*, int> loaded_models;

public:
	ModelLoader( bool headless, GLuint *textureGroup );

	virtual ~ModelLoader();

	virtual GLShape *getCreatureShape( char *model_name, 
																		 char *skin_name, 
																		 float scale=0.0f );
	virtual void decrementSkinRefCount( char *model_name, 
																			char *skin_name );
	GLuint loadSkinTexture( char *skin_name );
	void unloadSkinTexture( char *skin_name );
	void debugModelLoader();
};

#endif
