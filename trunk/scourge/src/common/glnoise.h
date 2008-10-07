/***************************************************************************
            glnoise.h  -  things that add primitive OpenGL error checking
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
#ifndef GLNOISE_H
#define GLNOISE_H
#pragma once

/// puts any positive glGetError() result to cerr
void noiseGL( void );

/// use  DECLARE_NOISY_OPENGL_SUPPORT() macro with defined NOISY_OPENGL 
/// to generate lots of noisy member functions that hide most OpenGL functions. 

#ifndef NOISY_OPENGL
#  define DECLARE_NOISY_OPENGL_SUPPORT()
#else
#  define DECLARE_NOISY_OPENGL_SUPPORT()                     \
	private:                                               \
	static void glAlphaFunc( GLenum func, GLclampf ref ) {        \
		::glAlphaFunc( func, ref ); noiseGL();             \
	}                                                      \
	static void glBindTexture( GLenum target, GLuint texture ) {  \
		::glBindTexture( target, texture ); noiseGL();     \
	}                                                      \
	static void glBlendFunc( GLenum sfactor, GLenum dfactor ) {   \
		::glBlendFunc( sfactor, dfactor ); noiseGL();      \
	}                                                      \
	static void glClear( GLbitfield mask ) {                      \
		::glClear( mask ); noiseGL();                      \
	}                                                      \
	static void glClearColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha ) {     \
		::glClearColor( red, green, blue, alpha ); noiseGL();      \
	}                                                      \
	static void glClearDepth( GLclampd depth ) {                  \
		::glClearDepth( depth ); noiseGL();                \
	}                                                      \
	static void glClearStencil( GLint s ) {                       \
		::glClearStencil( s ); noiseGL();                  \
	}                                                      \
	static void glColorMask( GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha ) {   \
		::glColorMask( red, green, blue, alpha ); noiseGL();      \
	}                                                      \
	static void glColorMaterial( GLenum face, GLenum mode ) {     \
		::glColorMaterial( face, mode ); noiseGL();        \
	}                                                      \
	static void glCopyTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border ) {   \
		::glCopyTexImage2D( target, level, internalFormat, x, y, width, height, border ); noiseGL();      \
	}                                                      \
	static void glCopyTexSubImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height ) {    \
		::glCopyTexSubImage2D( target, level, xoffset, yoffset, x, y, width, height ); noiseGL();      \
	}                                                      \
	static void glCullFace( GLenum mode ) {                       \
		::glCullFace( mode ); noiseGL();                   \
	}                                                      \
	static void glDeleteLists( GLuint list, GLsizei range ) {     \
		::glDeleteLists( list, range ); noiseGL();         \
	}                                                      \
	static void glDeleteTextures( GLsizei n, const GLuint *textures ) {    \
		::glDeleteTextures( n, textures ); noiseGL();      \
	}                                                      \
	static void glDepthFunc( GLenum func ) {                      \
		::glDepthFunc( func ); noiseGL();                  \
	}                                                      \
	static void glDepthMask( GLboolean flag ) {                   \
		::glDepthMask( flag ); noiseGL();                  \
	}                                                      \
	static void glDisable( GLenum cap ) {                         \
		::glDisable( cap ); noiseGL();                     \
	}                                                      \
	static void glEnable( GLenum cap ) {                          \
		::glEnable( cap ); noiseGL();                      \
	}                                                      \
	static void glEnd( void ) {                                   \
		::glEnd(); noiseGL();                              \
	}                                                      \
	static void glEndList( void ) {                               \
		::glEndList(); noiseGL();                          \
	}                                                      \
	static void glFlush( void ) {                                 \
		::glFlush(); noiseGL();                            \
	}                                                      \
	static void glFrontFace( GLenum mode ) {                      \
		::glFrontFace( mode ); noiseGL();                  \
	}                                                      \
	static GLuint glGenLists( GLsizei range ) {                   \
		GLuint ret =::glGenLists( range ); noiseGL(); return ret;      \
	}                                                      \
	static void glGenTextures( GLsizei n, GLuint *textures ) {    \
		::glGenTextures( n, textures ); noiseGL();         \
	}                                                      \
	static void glGetBooleanv( GLenum pname, GLboolean *params ) {        \
		::glGetBooleanv( pname, params ); noiseGL();       \
	}                                                      \
	static void glGetDoublev( GLenum pname, GLdouble *params ) {  \
		::glGetDoublev( pname, params ); noiseGL();        \
	}                                                      \
	static void glGetFloatv( GLenum pname, GLfloat *params ) {    \
		::glGetFloatv( pname, params ); noiseGL();         \
	}                                                      \
	static void glGetIntegerv( GLenum pname, GLint *params ) {    \
		::glGetIntegerv( pname, params ); noiseGL();       \
	}                                                      \
	static const GLubyte * glGetString( GLenum name ) {           \
		const GLubyte *ret =::glGetString( name ); noiseGL(); return ret;      \
	}                                                      \
	static void glHint( GLenum target, GLenum mode ) {            \
		::glHint( target, mode ); noiseGL();               \
	}                                                      \
	static GLboolean glIsEnabled( GLenum cap ) {                  \
		GLboolean ret =::glIsEnabled( cap ); noiseGL(); return ret;      \
	}                                                      \
	static void glLineWidth( GLfloat width ) {                    \
		::glLineWidth( width ); noiseGL();                 \
	}                                                      \
	static void glLoadIdentity( void ) {                          \
		::glLoadIdentity(); noiseGL();                     \
	}                                                      \
	static void glMatrixMode( GLenum mode ) {                     \
		::glMatrixMode( mode ); noiseGL();                 \
	}                                                      \
	static void glMultMatrixf( const GLfloat *m ) {               \
		::glMultMatrixf( m ); noiseGL();                   \
	}                                                      \
	static void glNewList( GLuint list, GLenum mode ) {           \
		::glNewList( list, mode ); noiseGL();              \
	}                                                      \
	static void glOrtho( GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar ) {    \
		::glOrtho( left, right, bottom, top, zNear, zFar ); noiseGL();      \
	}                                                      \
	static void glPixelStorei( GLenum pname, GLint param ) {      \
		::glPixelStorei( pname, param ); noiseGL();        \
	}                                                      \
	static void glPixelZoom( GLfloat xfactor, GLfloat yfactor ) { \
		::glPixelZoom( xfactor, yfactor ); noiseGL();      \
	}                                                      \
	static void glPolygonMode( GLenum face, GLenum mode ) {       \
		::glPolygonMode( face, mode ); noiseGL();          \
	}                                                      \
	static void glPopAttrib( void ) {                             \
		::glPopAttrib(); noiseGL();                        \
	}                                                      \
	static void glPopClientAttrib( void ) {                       \
		::glPopClientAttrib( ); noiseGL();                 \
	}                                                      \
	static void glPopMatrix( void ) {                             \
		::glPopMatrix(); noiseGL();                        \
	}                                                      \
	static void glPopName( void ) {                               \
		::glPopName(); noiseGL();                          \
	}                                                      \
	static void glPrioritizeTextures( GLsizei n, const GLuint *textures, const GLclampf *priorities ) {     \
		::glPrioritizeTextures( n, textures, priorities ); noiseGL();      \
	}                                                      \
	static void glPushAttrib( GLbitfield mask ) {                 \
		::glPushAttrib( mask ); noiseGL();                 \
	}                                                      \
	static void glPushClientAttrib( GLbitfield mask ) {           \
		::glPushClientAttrib( mask ); noiseGL();           \
	}                                                      \
	static void glPushMatrix( void ) {                            \
		::glPushMatrix(); noiseGL();                       \
	}                                                      \
	static void glPushName( GLuint name ) {                       \
		::glPushName( name ); noiseGL();                   \
	}                                                      \
	static void glReadPixels( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels ) {     \
		::glReadPixels( x, y, width, height, format, type, pixels ); noiseGL();      \
	}                                                      \
	static void glRotatef( GLfloat angle, GLfloat x, GLfloat y, GLfloat z ) {      \
		::glRotatef( angle, x, y, z ); noiseGL();          \
	}                                                      \
	static void glScalef( GLfloat x, GLfloat y, GLfloat z ) {     \
		::glScalef( x, y, z ); noiseGL();                  \
	}                                                      \
	static void glScissor( GLint x, GLint y, GLsizei width, GLsizei height ) {      \
		::glScissor( x, y, width, height ); noiseGL();     \
	}                                                      \
	static void glShadeModel( GLenum mode ) {                     \
		::glShadeModel( mode ); noiseGL();                 \
	}                                                      \
	static void glStencilFunc( GLenum func, GLint ref, GLuint mask ) {      \
		::glStencilFunc( func, ref, mask ); noiseGL();     \
	}                                                      \
	static void glStencilOp( GLenum fail, GLenum zfail, GLenum zpass ) {       \
		::glStencilOp( fail, zfail, zpass ); noiseGL();    \
	}                                                      \
	static void glTexEnvf( GLenum target, GLenum pname, GLfloat param ) {      \
		::glTexEnvf( target, pname, param ); noiseGL();    \
	}                                                      \
	static void glTexEnvi( GLenum target, GLenum pname, GLint param ) {       \
		::glTexEnvi( target, pname, param ); noiseGL();    \
	}                                                      \
	static void glTexImage2D( GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels ) {     \
		::glTexImage2D( target, level, internalformat, width, height, border, format, type, pixels ); noiseGL();      \
	}                                                      \
	static void glTexParameterf( GLenum target, GLenum pname, GLfloat param ) {      \
		::glTexParameterf( target, pname, param ); noiseGL();      \
	}                                                      \
	static void glTexParameteri( GLenum target, GLenum pname, GLint param ) {    \
		::glTexParameteri( target, pname, param ); noiseGL();      \
	}                                                      \
	static void glTranslated( GLdouble x, GLdouble y, GLdouble z ) {     \
		::glTranslated( x, y, z ); noiseGL();              \
	}                                                      \
	static void glTranslatef( GLfloat x, GLfloat y, GLfloat z ) { \
		::glTranslatef( x, y, z ); noiseGL();              \
	}                                                      \
	static void glViewport( GLint x, GLint y, GLsizei width, GLsizei height ) {     \
		::glViewport( x, y, width, height ); noiseGL();    \
	}                                                      \
	 
#endif //NOISY_OPENGL

#endif //GLNOISE_H
