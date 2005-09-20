/***************************************************************************
                          progress.cpp  -  description
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

#include "progress.h"

Progress::Progress(ScourgeGui *scourgeGui, GLuint texture, int maxStatus, bool clearScreen, bool center, bool opaque) {
  this->scourgeGui = scourgeGui;
  this->texture = texture;
  this->maxStatus = maxStatus;
  this->clearScreen = clearScreen;
  this->center = center;
  this->status = 0;
  this->opaque = opaque;
}

Progress::~Progress() {
}

void Progress::updateStatus(const char *message, bool updateScreen, int n, int max, int alt) {
  if(n != -1) status = n;
  if(max != -1) maxStatus = max;

  glPushAttrib( GL_ENABLE_BIT );
  
  if(updateScreen) glLoadIdentity();

  if(clearScreen) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    glClearColor( 0, 0, 0, 0 );
  }

  glDisable( GL_DEPTH_TEST );
  glDepthMask(GL_FALSE);
  glDisable( GL_BLEND );
  glDisable( GL_CULL_FACE );
  glDisable( GL_TEXTURE_2D );

  int w = 10;  
  int h = 20;

  int width = maxStatus * 2 * w + 20;
  int height = 35 + h + 10;

  // display as % if too large
  int maxWidth = scourgeGui->getScreenWidth() - 50;
  if( width >= maxWidth ) {
	//	cerr << "BEFORE: width=" << width << " maxStatus=" << maxStatus << " status=" << status << endl;
	//	maxStatus = (int)((float)( maxWidth - 20 ) / (float)w / 2.0f);
	maxStatus = (int)((float)( maxStatus * maxWidth ) / (float)width);
	status = (int)((float)( status * maxWidth ) / (float)width);
	if( alt > -1 )
	  alt = (int)((float)( alt * maxWidth ) / (float)width);
	width = maxWidth;
	//	cerr << "AFTER: width=" << width << " maxStatus=" << maxStatus << " status=" << status << endl;
  }

  int x = (center ? scourgeGui->getScreenWidth() / 2 - width / 2 : 0);
  int y = (center ? scourgeGui->getScreenHeight() / 2 - height / 2 : 0);
  glTranslatef( x, y, 0 );

  if(!opaque) {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  glColor4f( 0.25f, 0.20f, 0.15f, 0.8f );
  glBegin( GL_QUADS );
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, height, 0);
  glVertex3f( width, height, 0 );
  glVertex3f( width, 0, 0 );
  glEnd();
  glDisable( GL_BLEND );

  glColor4f(1, 1, 1, 1);
  if(message) scourgeGui->texPrint(20, 25, message);
  for (int i = 0; i < maxStatus; i++) {
    if( i < alt && i < status ) glColor4f(1, 0.4f, 0.0f, 1);
    else if(i < status) glColor4f(0.7f, 0.10f, 0.15f, 1);
    else glColor4f(0.5f, 0.5f, 0.5f, 1);
    glPushMatrix();
    if(updateScreen) glLoadIdentity();
    glTranslatef( x + i * 2 * w + 20, y + 35, 0 );
    glBegin( GL_QUADS );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, h, 0 );
    glVertex3f( w, h, 0 );
    glVertex3f( w, 0, 0 );
    glEnd();
    glPopMatrix();
  }

  /* Draw it to the screen */
  if(updateScreen) SDL_GL_SwapBuffers( );
  status++;
  //sleep(1);

  glEnable( GL_DEPTH_TEST );
  glDepthMask(GL_TRUE);
  
  glPopAttrib();
}

