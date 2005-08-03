/***************************************************************************
                          mapwidget.cpp  -  description
                             -------------------
    begin                : Tue Jun 18 2005
    copyright            : (C) 2005 by Gabor Torok
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

#include "mapwidget.h"
#include "scourge.h"

/**
  *@author Gabor Torok
  */

MapWidget::MapWidget( Scourge *scourge, int x, int y, int x2, int y2 ) : Canvas( x, y, x2, y2, this ) {
  selX = selY = 0;
  this->scourge = scourge;
  //this->canvas = new Canvas( x, y, x2, y2, this );
}

MapWidget::~MapWidget() {
  //delete canvas;
}

bool MapWidget::handleEvent(SDL_Event *event) {
  return false;
}

void MapWidget::drawWidget(Widget *w) {
  glBegin( GL_QUADS );
  glColor4f( 1, 0, 0, 0 );
  glVertex2f( 0, 0 );
  glColor4f( 1, 1, 0, 0 );
  glVertex2f( 0, getHeight() );
  glColor4f( 1, 0, 1, 0 );
  glVertex2f( getWidth(), getHeight() );
  glColor4f( 0, 0, 1, 0 );
  glVertex2f( getWidth(), 0 );
  glEnd();
}

