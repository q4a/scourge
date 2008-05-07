/***************************************************************************
                          vbo.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef VBO_H
#define VBO_H

#include "render.h"

/**
 * Video buffer objects. Original code from Nehe tutorials:
 * http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=45
 */
/*******************************************
*           Linux / SDL port of            *
*   Paul Frazee's Vertex Array Example     *
*           nehe.gamedev.net               *
*          Ilias Maratos 2004              *
*                                          *
*******************************************/

#ifndef CDS_FULLSCREEN											// CDS_FULLSCREEN Is Not Defined By Some
#define CDS_FULLSCREEN 4										// Compilers. By Defining It This Way,
#endif															// We Can Avoid Errors

#ifndef GL_BGR
#define GL_BGR  0x80E0
#endif

// #define NO_VBOS												// If Defined, VBOs Will Be Forced Off

// VBO Extension Definitions, From glext.h
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4

typedef void (APIENTRY * PFNGLBINDBUFFERARBPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY * PFNGLDELETEBUFFERSARBPROC) (GLsizei n, const GLuint *buffers);
typedef void (APIENTRY * PFNGLGENBUFFERSARBPROC) (GLsizei n, GLuint *buffers);
// typedef void (APIENTRY * PFNGLBUFFERDATAARBPROC) (GLenum target, int size, const GLvoid *data, GLenum usage);


// VBO Extension Function Pointers
PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;					// VBO Name Generation Procedure
PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;					// VBO Bind Procedure
PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;					// VBO Data Loading Procedure
PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;			// VBO Deletion Procedure


class VBO {
private:
	static bool initialized;
	static bool g_fVBOSupported;

	// Vertex Buffer Object Names
	unsigned int	m_nVBOVertices;								// Vertex VBO Name
	unsigned int	m_nVBOTexCoords;							// Texture Coordinate VBO Name
	int vertexCount;
	float *vertices;
	float *texCoords;

public:
	VBO( int vertexCount, float *vertices, float *texCoords );
	~VBO();
	void draw();

protected:
	bool isExtensionSupported( char* szTargetExtension );
	void initialize();
	void buildVBOs();
};

#endif
