/***************************************************************************
                      progress.cpp  -  Progress bar widget
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

#include "../common/constants.h"
#include "progress.h"
#include "widget.h"

#define TEXTURE_BORDER 100
#define HIGHLIGHT_BORDER 24

Progress::Progress( ScourgeGui *scourgeGui, Texture* texture, Texture* highlight,
                    int maxStatus, bool clearScreen, bool center, bool opaque ) {
	this->scourgeGui = scourgeGui;
	this->texture = texture;
	this->highlight = highlight;
	this->maxStatus = maxStatus;
	this->clearScreen = clearScreen;
	this->center = center;
	this->status = 0;
	this->opaque = opaque;
}

Progress::~Progress() {
}

void Progress::updateStatusLight( const char *message, int n, int max ) {
	if ( n != -1 ) status = n;
	if ( max != -1 ) maxStatus = max;

	int w = 10;
	int gap = 3;

	int width = maxStatus *  ( w + gap ) + 20;

	// display as % if too large
	int maxWidth = scourgeGui->getScreenWidth() - 50;
	if ( width >= maxWidth ) {
		maxStatus = static_cast<int>( static_cast<float>( maxStatus * maxWidth ) / static_cast<float>( width ) );
		status = static_cast<int>( static_cast<float>( status * maxWidth ) / static_cast<float>( width ) );
		width = maxWidth;
	}

	glColor4f( 1, 1, 1, 1 );
	if ( message && strlen( message ) ) scourgeGui->texPrint( 0, 22, message );

	for ( int i = 0; i < 2; i++ ) {
		switch ( i ) {
		case 0: glColor4f( 0.5f, 0.5f, 0.5f, 0.8f ); break;
		case 1: glColor4f( 1, 1, 1, 0.8f ); break;
		}
		Widget::drawBorderedTexture( highlight,
		                             0, 0,
		                             HIGHLIGHT_BORDER * 2 + ( i == 0 ? maxStatus : status ) * ( w + gap ),
		                             20,
		                             HIGHLIGHT_BORDER, HIGHLIGHT_BORDER, 255 );
	}
	//status++;

}

void Progress::updateStatus( const char *message, bool updateScreen, int n, int max, int alt, bool drawScreen ) {
	if ( n != -1 ) status = n;
	if ( max != -1 ) maxStatus = max;

	glPushAttrib( GL_ENABLE_BIT );

	//if(updateScreen)
	glLoadIdentity();

	if ( clearScreen ) {
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
		glClearColor( 0, 0, 0, 0 );
	}

	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glDisable( GL_CULL_FACE );

	int w = 10;
//  int h = 22;
	int gap = 3;

	int width = maxStatus *  ( w + gap ) + 20;
//  int height = 35 + h + 10;

	// display as % if too large
	int maxWidth = scourgeGui->getScreenWidth() - 50;
	if ( width >= maxWidth ) {
		maxStatus = static_cast<int>( static_cast<float>( maxStatus * maxWidth ) / static_cast<float>( width ) );
		status = static_cast<int>( static_cast<float>( status * maxWidth ) / static_cast<float>( width ) );
		if ( alt > -1 )
			alt = static_cast<int>( static_cast<float>( alt * maxWidth ) / static_cast<float>( width ) );
		width = maxWidth;
	}

	int x = ( center ? scourgeGui->getScreenWidth() / 2 - width / 2 : ( texture ? TEXTURE_BORDER : 0 ) );
	//int y = (center ? scourgeGui->getScreenHeight() / 3 - height / 2 : ( texture ? TEXTURE_BUFFER_TOP : 0 ));
	int y = 0;
	glTranslatef( x, y, 0 );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glColor4f( 1, 1, 1, 0.8f );
	Widget::drawBorderedTexture( texture,
	                             -TEXTURE_BORDER,
	                             0,
	                             width + ( 2 * TEXTURE_BORDER ),
	                             60,
	                             TEXTURE_BORDER, TEXTURE_BORDER,
	                             256 );

	glColor4f( 1, 1, 1, 1 );
	if ( message ) scourgeGui->texPrint( 0, 22, message );

	for ( int i = 0; i < 3; i++ ) {
		if ( i == 2 && alt <= 0 ) continue;
		glPushMatrix();
		//if(updateScreen)
		glLoadIdentity();
		glTranslatef( x + 10, y + 32, 0 );
		switch ( i ) {
		case 0: glColor4f( 0.5f, 0.5f, 0.5f, 0.8f ); break;
		case 1: glColor4f( 1, 1, 1, 0.8f ); break;
		case 2: glColor4f( 1, 0.4f, 0, 0.8f ); break;
		}
		Widget::drawBorderedTexture( highlight,
		                             -HIGHLIGHT_BORDER, 0,
		                             HIGHLIGHT_BORDER * 2 + ( i == 0 ? maxStatus : ( i == 1 ? status : ( status < alt ? status : alt ) ) ) * ( w + gap ),
		                             //22,
		                             20,
		                             HIGHLIGHT_BORDER, HIGHLIGHT_BORDER, 255 );
		glPopMatrix();
	}
	//glDisable( GL_TEXTURE_2D );
	/* Draw it to the screen */
	if ( updateScreen ) {
		if ( drawScreen ) {
			scourgeGui->processEventsAndRepaint();
		} else SDL_GL_SwapBuffers( );
	}
	status++;
	//sleep(1);

	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );

	glPopAttrib();
}

