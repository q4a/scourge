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

Progress::Progress(Scourge *scourge, int maxStatus, bool clearScreen, bool center, bool opaque) {
  this->scourge = scourge;
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

  int x = (center ? scourge->getScreenWidth() / 2 - (maxStatus * w + 20) : 0);
  int y = (center ? scourge->getScreenHeight() / 2 - (35 + h + 10) / 2 : 0);
  glTranslatef( x, y, 0 );

  if(!opaque) {
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  glColor4f( 0.25f, 0.20f, 0.15f, 0.8f );
  glBegin( GL_QUADS );
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, 35 + h + 10, 0);
  glVertex3f( maxStatus * 2 * w + 20, 35 + h + 10, 0 );
  glVertex3f( maxStatus * 2 * w + 20, 0, 0 );
  glEnd();
  glDisable( GL_BLEND );

  glColor4f(1, 1, 1, 1);
  if(message) scourge->getSDLHandler()->texPrint(20, 25, message);
  for (int i = 0; i < maxStatus; i++) {
    if( i < alt ) glColor4f(0.75f, 1.0f, 0.0f, 1);
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

