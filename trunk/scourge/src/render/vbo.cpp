/***************************************************************************
                          vbo.cpp  -  description
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
 
#include "vbo.h"

using namespace std;

bool VBO::initialized = false;
bool VBO::g_fVBOSupported = false;

VBO::VBO( int vertexCount, float *vertices, float *texCoords ) {
	this->vertexCount = vertexCount;
	this->vertices = vertices;
	this->texCoords = texCoords;
	initialize();
}

VBO::~VBO() {
	// Delete VBOs
	if( g_fVBOSupported ) {
		unsigned int nBuffers[2] = { m_nVBOVertices, m_nVBOTexCoords };
		glDeleteBuffersARB( 2, nBuffers );						// Free The Memory
	}
	// Delete Data
	if( vertices )											// Deallocate Vertex Data
		delete [] vertices;
	vertices = NULL;
	if( texCoords )											// Deallocate Texture Coord Data
		delete [] texCoords;
	texCoords = NULL;
	if(m_nVBOTexCoords)									// Deallocate Vertex Buffer
		glDeleteBuffersARB( 1, &m_nVBOTexCoords);
	if(m_nVBOVertices)
		glDeleteBuffersARB( 1, &m_nVBOVertices);
}

// Based Off Of Code Supplied At OpenGL.org
bool VBO::isExtensionSupported( char* szTargetExtension ) {
	const unsigned char *pszExtensions = NULL;
	const unsigned char *pszStart;
	unsigned char *pszWhere, *pszTerminator;

	// Extension names should not have spaces
	pszWhere = (unsigned char *) strchr( szTargetExtension, ' ' );
	if( pszWhere || *szTargetExtension == '\0' )
		return false;

	// Get Extensions String
	pszExtensions = glGetString( GL_EXTENSIONS );

	// Search The Extensions String For An Exact Copy
	pszStart = pszExtensions;
	for(;;) {
		pszWhere = (unsigned char *) strstr( (const char *) pszStart, szTargetExtension );
		if( !pszWhere ) break;
		pszTerminator = pszWhere + strlen( szTargetExtension );
		if( pszWhere == pszStart || *( pszWhere - 1 ) == ' ' )
			if( *pszTerminator == ' ' || *pszTerminator == '\0' )
				return true;
			pszStart = pszTerminator;
	}
	return false;
}

void VBO::initialize( void ) {
	if( !initialized ) {
		// Check For VBOs Supported
		cerr << "Checking for extensions....." << endl;
#ifndef NO_VBOS
		g_fVBOSupported = isExtensionSupported( "GL_ARB_vertex_buffer_object" );
#else
		g_fVBOSupported = false;
#endif
		if( g_fVBOSupported ) {
			cerr << "VBOs supported, great!" << endl;
		} else {
			cerr << "VBOs not supported , too bad!" << endl;
		}
		initialized = true;
	}

	if( g_fVBOSupported ) {
		// Get Pointers To The GL Functions
		glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress( "glGenBuffersARB" );
		glBindBufferARB = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress( "glBindBufferARB" );
		glBufferDataARB = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress( "glBufferDataARB" );
		glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress( "glDeleteBuffersARB" );
		
		// Load Vertex Data Into The Graphics Card Memory
		buildVBOs();									// Build The VBOs
	}
}

void VBO::buildVBOs() {
	// Generate And Bind The Vertex Buffer
	glGenBuffersARB( 1, &m_nVBOVertices );							// Get A Valid Name
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );			// Bind The Buffer
	// Load The Data
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, vertexCount * 3 * sizeof( float ), vertices, GL_STATIC_DRAW_ARB );

	// Generate And Bind The Texture Coordinate Buffer
	glGenBuffersARB( 1, &m_nVBOTexCoords );							// Get A Valid Name
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOTexCoords );		// Bind The Buffer
	// Load The Data
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, vertexCount * 2 * sizeof( float ), texCoords, GL_STATIC_DRAW_ARB );

	// Our Copy Of The Data Is No Longer Necessary, It Is Safe In The Graphics Card
	delete [] vertices; vertices = NULL;
	delete [] texCoords; texCoords = NULL;
}

void VBO::draw() {
	// Enable Pointers
	glEnableClientState( GL_VERTEX_ARRAY );						// Enable Vertex Arrays
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );				// Enable Texture Coord Arrays
	
	// Set Pointers To Our Data
	if( g_fVBOSupported ) {
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOVertices );
		glVertexPointer( 3, GL_FLOAT, 0, (char*)NULL );		// Set The Vertex Pointer To The Vertex Buffer
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, m_nVBOTexCoords );
		glTexCoordPointer( 2, GL_FLOAT, 0, (char*)NULL );		// Set The TexCoord Pointer To The TexCoord Buffer
	} else {
		glVertexPointer( 3, GL_FLOAT, 0, vertices ); // Set The Vertex Pointer To Our Vertex Data
		glTexCoordPointer( 2, GL_FLOAT, 0, texCoords ); // Set The Vertex Pointer To Our TexCoord Data
	}

	// Render
	glDrawArrays( GL_TRIANGLES, 0, vertexCount );	// Draw All Of The Triangles At Once

	// Disable Pointers
	glDisableClientState( GL_VERTEX_ARRAY );					// Disable Vertex Arrays
	glDisableClientState( GL_TEXTURE_COORD_ARRAY );				// Disable Texture Coord Arrays
}

