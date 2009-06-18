/***************************************************************************
       texteffect.cpp  -  Class for large text with animated effects
                             -------------------
    begin                : Sept 13, 2005
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
#include "common/constants.h"
#include "texteffect.h"
#include "scourge.h"
#include "sdlhandler.h"

#define MENU_ITEM_WIDTH 256
#define MENU_ITEM_HEIGHT 50
#define MENU_ITEM_ZOOM 1.0f
#define MENU_ITEM_PARTICLE_ZOOM 1.1f
#define MAX_PARTICLE_LIFE 50
#define FONT_OFFSET ( abs( SDLHandler::fontInfos[ Constants::SCOURGE_LARGE_FONT ]->yoffset ) )

TextEffect::TextEffect( Scourge *scourge, int x, int y, char const* text ) {
	this->scourge = scourge;
	strncpy( this->text, text, 254 );
	this->text[ 254 ] = '\0';
	this->x = x;
	this->y = y;

	lastTickMenu = 0;
	for ( int t = 0; t < 20; t++ ) particle[t].life = 0;
	textureInMemory = NULL;
}

TextEffect::~TextEffect() {
	if ( textureInMemory ) {
		glDeleteTextures( 1, texture );
		delete [] textureInMemory;
	}
}

void TextEffect::draw() {
	glsDisable( GLS_DEPTH_TEST );
	glsDisable( GLS_CULL_FACE );

	if ( !textureInMemory ) {
		buildTextures();
	}

	float zoom = MENU_ITEM_ZOOM;
	zoom = ( active ? MENU_ITEM_ZOOM * 1.5f : MENU_ITEM_ZOOM );

	glsEnable( GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	//scourge->setBlendFunc();

	glPushMatrix();
	glLoadIdentity();
	glTranslatef( x + 40, y + FONT_OFFSET, 0 );
	glBindTexture( GL_TEXTURE_2D, texture[0] );

	if ( active ) {
		//glColor4f( 1, 0.6f, 0.5f, 1 );
		glColor4f( 0.9f, 0.7f, 0.15f, 1 );
	} else {
		glColor4f( 1, 1, 1, 1 );
	}

	glsEnable( GLS_TEXTURE_2D );
	glBegin( GL_TRIANGLE_STRIP );
//  glNormal3f( 0, 0, 1 );
	glTexCoord2i( 0, 1 );
	glVertex2f( 0, 0 );
	glTexCoord2i( 1, 1 );
	glVertex2f( MENU_ITEM_WIDTH * zoom, 0 );
	glTexCoord2i( 0, 0 );
	glVertex2f( 0, MENU_ITEM_HEIGHT * zoom );
	glTexCoord2i( 1, 0 );
	glVertex2f( MENU_ITEM_WIDTH * zoom, MENU_ITEM_HEIGHT * zoom );
	glEnd();
	glsDisable( GLS_TEXTURE_2D );
	glsDisable( GLS_BLEND );
	glPopMatrix();

	drawEffect( 4.0f, 20 );

	// move menu
	Uint32 tt = SDL_GetTicks();
	if ( tt - lastTickMenu > 40 ) {
		lastTickMenu = tt;
		for ( int i = 0; i < 20; i++ ) {
			particle[i].x += Constants::cosFromAngle( particle[i].dir ) * particle[i].step;
			particle[i].y += Constants::sinFromAngle( particle[i].dir ) * particle[i].step;
			particle[i].life++;
			if ( particle[i].life >= MAX_PARTICLE_LIFE ) {
				particle[i].life = 0;
			}
		}
	}

	//glsEnable( GLS_DEPTH_MASK );
	glsEnable( GLS_DEPTH_TEST );
}

void TextEffect::drawEffect( float divisor, int count ) {
	float scaledDivisor = 256.0f * divisor;

	glsEnable( GLS_BLEND );
	glBlendFunc( GL_DST_COLOR, GL_ONE );
	glPushMatrix();
	glBindTexture( GL_TEXTURE_2D, texture[0] );

	for ( int i = 0; i < count; i++ ) {
		if ( !( particle[i].life ) ) {
			particle[i].life = Util::dice( MAX_PARTICLE_LIFE );
			particle[i].x = particle[i].y = 0;
			particle[i].r = Util::pickOne( 200, 239 );
			particle[i].g = Util::pickOne( 170, 209 );
			particle[i].b = Util::pickOne( 80, 119 );
			particle[i].dir = Util::roll( 0.0f, 10.0f );
			particle[i].zoom = Util::roll( 2.0f, 4.0f );
			switch ( Util::dice( 4 ) ) {
			case 0: particle[i].dir = 360.0f - particle[i].dir; break;
			case 1: particle[i].dir = 180.0f - particle[i].dir; break;
			case 2: particle[i].dir = 180.0f + particle[i].dir; break;
				//default: // do nothing
			}
			particle[i].step = 4.0f * Util::mt_rand();
		}

		if ( active ) {
			glLoadIdentity();
			glTranslatef( x + particle[i].x, y + particle[i].y, 0 );

			float a = static_cast<float>( MAX_PARTICLE_LIFE - particle[i].life ) / static_cast<float>( MAX_PARTICLE_LIFE );
			//if( i == 0 ) cerr << "life=" << particle[i].life << " a=" << a << endl;
			glColor4f( static_cast<float>( particle[i].r ) / ( scaledDivisor ),
			           static_cast<float>( particle[i].g ) / ( scaledDivisor ),
			           static_cast<float>( particle[i].b ) / ( scaledDivisor ),
			           a / divisor );

			glsEnable( GLS_TEXTURE_2D );
//   glNormal3f( 0, 0, 1 );
			glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2i( 0, 1 );
			glVertex2f( 0, 0 );
			glTexCoord2i( 1, 1 );
			glVertex2f( MENU_ITEM_WIDTH * particle[i].zoom, 0 );
			glTexCoord2i( 0, 0 );
			glVertex2f( 0, MENU_ITEM_HEIGHT * particle[i].zoom );
			glTexCoord2i( 1, 0 );
			glVertex2f( MENU_ITEM_WIDTH * particle[i].zoom, MENU_ITEM_HEIGHT * particle[i].zoom );
			glEnd();
			glsDisable( GLS_TEXTURE_2D );
		}
	}
	glPopMatrix();
	glsDisable( GLS_BLEND );
}

void TextEffect::buildTextures() {

	// must be powers of 2
	int width = 256;
	int height = 64;

	glPushMatrix();
	glLoadIdentity();
	glColor4f( 0, 0, 0, 0 );

	glsEnable( GLS_TEXTURE_2D );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( x, y - FONT_OFFSET );
	glVertex2i( x + width, y - FONT_OFFSET );
	glVertex2i( x, y - FONT_OFFSET + height );
	glVertex2i( x + width, y - FONT_OFFSET + height );
	glEnd();

	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );

	//int width = scourge->getSDLHandler()->textWidth( text );

	// Create texture and copy minimap date from backbuffer on it
	textureInMemory = new unsigned char[width * height * 4];

	glGenTextures( 1, texture );
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); // filtre appliqu� a la texture
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
	glTexImage2D( GL_TEXTURE_2D, 0, ( scourge->getPreferences()->getBpp() > 16 ? GL_RGBA : GL_RGBA4 ), width, height, 0,
	              GL_RGBA, GL_UNSIGNED_BYTE, textureInMemory );

	// Draw image
	//x = x;
	//y = y;
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	scourge->getSDLHandler()->texPrint( x, y, text );
	//y += height;

	// Copy to a texture
	glLoadIdentity();
	glBindTexture( GL_TEXTURE_2D, texture[0] );
	glCopyTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,
	                  x, scourge->getSDLHandler()->getScreen()->h - ( y - FONT_OFFSET + height ),
	                  width, height, 0 );
	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

	glsDisable( GLS_TEXTURE_2D );
	glPopMatrix();
}

