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
#include "gui/canvas.h"

/**
  *@author Gabor Torok
  */
  
#define MAP_GRID_HEIGHT ( Constants::MAP_GRID_TILE_HEIGHT * Constants::MAP_GRID_TILE_PIXEL_HEIGHT )
#define MAP_GRID_WIDTH ( Constants::MAP_GRID_TILE_WIDTH * Constants::MAP_GRID_TILE_PIXEL_WIDTH )

MapWidget::MapWidget( Scourge *scourge, int x, int y, int x2, int y2 ) {
  selX = selY = 0;
  this->scourge = scourge;
  this->canvas = new Canvas( x, y, x2, y2, this );
}

MapWidget::~MapWidget() {
  delete canvas;
}

void MapWidget::setPosition( int x, int y ) {
  selX += ( x - ( canvas->getWidth() / 2 ) );
  selY += ( y - ( canvas->getHeight() / 2 ) );
}

void MapWidget::drawWidget(Widget *w) {

  // figure out what needs to show
  int sx = selX - canvas->getWidth() / 2;
  int sy = selY - canvas->getHeight() / 2;
  int ex = selX + canvas->getWidth() / 2;
  int ey = selY + canvas->getHeight() / 2;

  if( sx < 0 ) {
    ex = canvas->getWidth();
    sx = 0;
  }
  if( sy < 0 ) {
    sy = 0;
    ey = canvas->getHeight();
  }
  if( ex >= MAP_GRID_WIDTH ) {
    sx = MAP_GRID_WIDTH - canvas->getWidth();
    ex = MAP_GRID_WIDTH;
  }
  if( ey >= MAP_GRID_HEIGHT ) {
    sx = MAP_GRID_HEIGHT - canvas->getHeight();
    ex = MAP_GRID_HEIGHT;
  }

  int gx = sx / Constants::MAP_GRID_TILE_PIXEL_WIDTH;
  int gy = sy / Constants::MAP_GRID_TILE_PIXEL_HEIGHT;
  int tx = sx % Constants::MAP_GRID_TILE_PIXEL_WIDTH;
  int ty = sy % Constants::MAP_GRID_TILE_PIXEL_HEIGHT;

  /*
  cerr << "pixel=" << sx << "-" << ex << " , " << sy << "-" << ey <<
    " gx=" << gx << " gy=" << gy <<
    " tx=" << tx << " ty=" << ty << endl;
  */

  glEnable( GL_TEXTURE_2D );
  for( int xx = 0; xx < canvas->getWidth() / Constants::MAP_GRID_TILE_PIXEL_WIDTH + 1; xx++ ) {
    for( int yy = 0; yy < canvas->getWidth() / Constants::MAP_GRID_TILE_PIXEL_HEIGHT + 1; yy++ ) {
      glPushMatrix();
      int xp = xx * Constants::MAP_GRID_TILE_PIXEL_WIDTH - tx;
      int yp = yy * Constants::MAP_GRID_TILE_PIXEL_HEIGHT - ty;
      glTranslatef( xp, yp, 0 );
//      cerr << "\txx=" << xx << " yy=" << yy << 
//        " gird=" << ( gx + xx ) << "," << ( gy + yy ) << endl;
      glBindTexture( GL_TEXTURE_2D, 
                     scourge->getShapePalette()->
                     getMapGridTile( gx + xx, gy + yy ) );
      glColor4f( 1, 1, 1, 1 );
      glBegin( GL_QUADS );
      glNormal3f( 0, 0, 1 );
      glTexCoord2f( 0, 0 );
      glVertex2f( 0, 0 );
      glTexCoord2f( 0, 1 );
      glVertex2f( 0, Constants::MAP_GRID_TILE_PIXEL_HEIGHT );
      glTexCoord2f( 1, 1 );
      glVertex2d( Constants::MAP_GRID_TILE_PIXEL_WIDTH, Constants::MAP_GRID_TILE_PIXEL_HEIGHT );
      glTexCoord2f( 1, 0 );
      glVertex2d( Constants::MAP_GRID_TILE_PIXEL_WIDTH, 0 );
      glEnd();
      glPopMatrix();
    }
  }  
}

