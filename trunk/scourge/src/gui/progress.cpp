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

Progress::Progress(Scourge *scourge, int maxStatus) {
  this->scourge = scourge;
  this->maxStatus = maxStatus;
  this->status = 0;
}

Progress::~Progress() {
}

void Progress::updateStatus(const char *message) {
  glLoadIdentity();
  //  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  //  glClearColor( 0, 0, 0, 0 );

  glDisable( GL_DEPTH_TEST );
  glDepthMask(GL_FALSE);
  glDisable( GL_BLEND );
  glDisable( GL_CULL_FACE );
  glDisable( GL_TEXTURE_2D );

  int w = 10;  
  int h = 20;

  glColor4f( 0.25f, 0.20f, 0.15f, 0.15f );
  glBegin( GL_QUADS );
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, 35 + h + 10, 0);
  glVertex3f( maxStatus * 2 * w + 20, 35 + h + 10, 0 );
  glVertex3f( maxStatus * 2 * w + 20, 0, 0 );
  glEnd();

  glColor4f(1, 1, 1, 1);
  scourge->getSDLHandler()->texPrint(20, 25, message);
  for (int i = 0; i < maxStatus; i++) {
    if (i < status) glColor4f(0.7f, 0.10f, 0.15f, 1);
    else glColor4f(0.5f, 0.5f, 0.5f, 1);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( i * 2 * w + 20, 35, 0 );
    glBegin( GL_QUADS );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, h, 0 );
    glVertex3f( w, h, 0 );
    glVertex3f( w, 0, 0 );
    glEnd();
    glPopMatrix();
  }

  /* Draw it to the screen */
  SDL_GL_SwapBuffers( );
  status++;
  //sleep(1);

  glEnable( GL_DEPTH_TEST );
  glDepthMask(GL_TRUE);
}

