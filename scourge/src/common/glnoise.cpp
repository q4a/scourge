/***************************************************************************
           glnoise.cpp -  primitive OpenGL error checking
                             -------------------
    begin                : Mon Oct 6 2008
    copyright            : (C) 2008 by Vambola Kotkas
    email                : vambola.kotkas@proekspert.ee
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "constants.h"
#include "glnoise.h"

/// puts any positive glGetError() result to cerr
void noiseGL( void ) {
	GLenum ret = glGetError();
	if ( ret == GL_NO_ERROR )
		return;

	std::cerr << "glGetError() said: ";

	switch ( ret ) {
	default:
		std::cerr << "Error #" << ret;
		break;
	case GL_INVALID_ENUM:
		std::cerr << "GL_INVALID_ENUM";
		break;
	case GL_INVALID_VALUE:
		std::cerr << "GL_INVALID_VALUE";
		break;
	case GL_INVALID_OPERATION:
		std::cerr << "GL_INVALID_OPERATION";
		break;
	case GL_STACK_OVERFLOW:
		std::cerr << "GL_STACK_OVERFLOW";
		break;
	case GL_STACK_UNDERFLOW:
		std::cerr << "GL_STACK_UNDERFLOW";
		break;
	case GL_OUT_OF_MEMORY:
		std::cerr << "GL_OUT_OF_MEMORY";
		break;
	}
	std::cerr << std::endl;

}