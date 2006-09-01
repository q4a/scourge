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

class CModelMD3;

class ModelWrapper  {
private:
	t3DModel *md2;
	CModelMD3 *md3;	

public:

	void loadModel( char *file_name );
	void unloadModel();
	
	inline t3DModel *getMd2() { return md2; }
	inline CModelMD3 *getMd3() { return md3; }
	
};

#endif
