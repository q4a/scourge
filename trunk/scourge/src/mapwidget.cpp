/***************************************************************************
            mapwidget.cpp  -  Widget that displays a draggable map
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

#include "common/constants.h"
#include "mapwidget.h"
#include "scourge.h"
#include "gui/canvas.h"
#include "shapepalette.h"

using namespace std;

/**
  *@author Gabor Torok
  */

//#define MAP_GRID_HEIGHT ( Constants::MAP_GRID_TILE_HEIGHT * Constants::MAP_GRID_TILE_PIXEL_HEIGHT )
//#define MAP_GRID_WIDTH ( Constants::MAP_GRID_TILE_WIDTH * Constants::MAP_GRID_TILE_PIXEL_WIDTH )

#define MAP_GRID_HEIGHT ( BITMAP_SIZE * BITMAPS_PER_COL )
#define MAP_GRID_WIDTH ( BITMAP_SIZE * BITMAPS_PER_ROW )

MapWidget::MapWidget( Scourge* scourge, Window* parent, int x, int y, int x2, int y2, bool editable ) 
		: Canvas( x, y, x2, y2 ) {
	this->scourge = scourge;
	this->parent = parent;
	this->editable = editable;
	markedX = markedY = oldSelX = oldSelY = selX = selY = 0;
	oldx = oldy = 0;
	dragging = false;
	showRegions = false;
	attach( Widget::Draw, &MapWidget::onDraw, this );
	calculateValues();
}

MapWidget::~MapWidget() {
	for( map<int, Texture>::iterator e = textures.begin(); e != textures.end(); ++e ) {
		Texture tex = e->second;
		tex.clear();
	}	
}

bool MapWidget::handleEvent( Window* parent, SDL_Event* event, int x, int y ) {
	switch ( event->type ) {
	case SDL_MOUSEMOTION:
		if ( dragging ) {
			selX = oldSelX - ( x - oldx );
			selY = oldSelY - ( y - oldy );
			calculateValues();
		}
		break;
	case SDL_MOUSEBUTTONUP:
		dragging = false;
		if ( editable && isInside( x, y ) ) {
			markedX = selX + x - getX();
			markedY = selY + y - getY();
		}
		this->parent->setMouseLock( NULL );
		return isInside( x, y );
	case SDL_MOUSEBUTTONDOWN:
		if ( event->button.button != SDL_BUTTON_LEFT ) return false;
		if ( isInside( x, y ) ) {
			oldx = x;
			oldy = y;
			oldSelX = selX;
			oldSelY = selY;
			dragging = true;
			this->parent->setMouseLock( this );
		}
		break;
	}
	return false;
}

void MapWidget::setSelection( int x, int y ) {
	markedX = x;
	markedY = y;
	selX = x - getWidth() / 2;
	selY = y - getHeight() / 2;
	calculateValues();
}

void MapWidget::calculateValues() {

	if ( selX < 0 ) selX = 0;
	if ( selX >= MAP_GRID_WIDTH ) selX = MAP_GRID_WIDTH - 1;

	if ( selY < 0 ) selY = 0;
	if ( selY >= MAP_GRID_HEIGHT ) selY = MAP_GRID_HEIGHT - 1;

	Canvas *canvas = this;

	// figure out what needs to show
	int sx = selX;
	int sy = selY;
	int ex = selX + canvas->getWidth();
	int ey = selY + canvas->getHeight();

	if ( sx < 0 ) {
		ex = canvas->getWidth();
		sx = 0;
	}
	if ( sy < 0 ) {
		sy = 0;
		ey = canvas->getHeight();
	}
	if ( ex > MAP_GRID_WIDTH ) {
		sx = MAP_GRID_WIDTH - canvas->getWidth();
		ex = MAP_GRID_WIDTH;
	}
	if ( ey > MAP_GRID_HEIGHT ) {
		sy = MAP_GRID_HEIGHT - canvas->getHeight();
		ey = MAP_GRID_HEIGHT;
	}

	gx = sx / BITMAP_SIZE;
	gy = sy / BITMAP_SIZE;
	tx = sx % BITMAP_SIZE;
	ty = sy % BITMAP_SIZE;

	/*
	cerr << "pixel=" << sx << "-" << ex << " , " << sy << "-" << ey <<
	  " out of " << MAP_GRID_WIDTH << "," << MAP_GRID_HEIGHT <<
	  " starting tile=" << gx << "," << gy <<
	  " starting offset=-1 * (" << tx << "," << ty << ")" << endl;
	*/
}

Texture MapWidget::loadTexture( int x, int y ) {
	int bitmapIndex = y * BITMAPS_PER_ROW + x;
	
	if( textures.find( bitmapIndex ) == textures.end() ) {
		char bitmapName[3000];
		sprintf( bitmapName, "/mapgrid/map_%02d.png", bitmapIndex );
		cerr << "Loading bitmap=" << bitmapName << " pos=" << x << "," << y << endl;
		
		Texture texture;
		string filename = bitmapName;
		texture.load( filename );
		
		textures[ bitmapIndex ] = texture;
	}
	
	return textures[ bitmapIndex ];
}

bool  MapWidget::onDraw( Widget* ) {
	Canvas *canvas = this;


	glEnable( GL_TEXTURE_2D );
	for ( int xx = 0; xx < canvas->getWidth() / BITMAP_SIZE + 2; xx++ ) {
		if ( gx + xx >= BITMAP_SIZE ) continue;
		for ( int yy = 0; yy < canvas->getHeight() / BITMAP_SIZE + 2; yy++ ) {
			if ( gy + yy >= BITMAP_SIZE ) continue;
			glPushMatrix();
			int xp = xx * BITMAP_SIZE - tx;
			int yp = yy * BITMAP_SIZE - ty;
			glTranslatef( xp, yp, 0 );
//      cerr << "\txx=" << xx << " yy=" << yy <<
//        " gird=" << ( gx + xx ) << "," << ( gy + yy ) << endl;
			loadTexture( gx + xx, gy + yy ).glBind();
			glColor4f( 1, 1, 1, 1 );

			glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2i( 0, 0 );
			glVertex2i( 0, 0 );
			glTexCoord2i( 1, 0 );
			glVertex2i( BITMAP_SIZE, 0 );
			glTexCoord2i( 0, 1 );
			glVertex2i( 0, BITMAP_SIZE );
			glTexCoord2i( 1, 1 );
			glVertex2i( BITMAP_SIZE, BITMAP_SIZE );
			glEnd();
			glPopMatrix();
		}
	}
	glDisable( GL_TEXTURE_2D );
	
	if( showRegions ) {
		glEnable( GL_BLEND );
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix();
		glTranslatef( -tx, -ty, 0 );
		
		glColor4f( 1, 1, 1, 0.3f );
		for( int lx = 0; lx < canvas->getWidth() + BITMAP_SIZE; lx += REGION_SIZE ) {
			glBegin( GL_LINES );
			glVertex2i( lx, 0 );
			glVertex2i( lx, canvas->getHeight() + BITMAP_SIZE );
			glEnd();
		}
		for( int ly = 0; ly < canvas->getHeight() + BITMAP_SIZE; ly += REGION_SIZE ) {
			glBegin( GL_LINES );
			glVertex2i( canvas->getWidth() + BITMAP_SIZE, ly );
			glVertex2i( 0, ly );
			glEnd();
		}
		
		glPopMatrix();
		glDisable( GL_BLEND );
	}

	int shadowSize = 10;
	glEnable( GL_BLEND );
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );

	glBegin( GL_QUADS );
	glColor4f( 0, 0, 0, 0.75f );
	glVertex2i( 0, 0 );
	glColor4f( 0.4f, 0.4f, 0.4f, 0.5f );
	glVertex2i( getWidth(), 0 );
	glVertex2i( getWidth(), shadowSize );
	glColor4f( 0, 0, 0, 0.75f );
	glVertex2i( 0, shadowSize );

	glColor4f( 0, 0, 0, 0.75f );
	glVertex2i( 0, shadowSize );
	glVertex2i( shadowSize, shadowSize );
	glColor4f( 0.4f, 0.4f, 0.4f, 0.5f );
	glVertex2i( shadowSize, getHeight() );
	glColor4f( 0, 0, 0, 0.75f );
	glVertex2i( 0, getHeight() );
	glEnd();
	glDisable( GL_BLEND );

	glPushMatrix();
	glTranslatef( markedX - ( gx * BITMAP_SIZE + tx ),
	              markedY - ( gy * BITMAP_SIZE + ty ),
	              0 );
	glDisable( GL_TEXTURE_2D );
	glColor4f( 1, 0, 0, 0 );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( 0, 0 );
	glVertex2i( 10, 0 );
	glVertex2i( 0, 10 );
	glVertex2i( 10, 10 );
	glEnd();
	glColor4f( 0, 0, 0, 0 );
	glBegin( GL_LINE_LOOP );
	glVertex2i( 0, 0 );
	glVertex2i( 10, 0 );
	glVertex2i( 10, 10 );
	glVertex2i( 0, 10 );
	glEnd();
	glEnable( GL_TEXTURE_2D );
	glPopMatrix();
	return true;
}

