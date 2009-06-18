/***************************************************************************
                     glteleporter.cpp  -  Teleporter shape
                             -------------------
    begin                : Thu Jul 10 2003
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
#include "glteleporter.h"

Color GLTeleporter::lightColor( 0.4f, 0.45f, 1.0f, 0.5f );
float GLTeleporter::lightRadius = 15.0f;

/**
  *@author Gabor Torok
  */

GLTeleporter::GLTeleporter( Texture texture[], Texture flameTex,
                            int width, int depth, int height,
                            char const* name, int descriptionGroup,
                            Uint32 color, Uint8 shapePalIndex,
                            int teleporterType ) :
		GLShape( texture, width, depth, height, name, descriptionGroup, color, shapePalIndex ) {
	commonInit( flameTex );
	this->teleporterType = teleporterType;
}

void GLTeleporter::commonInit( Texture flameTex ) {
	this->flameTex = flameTex;
	for ( int i = 0; i < MAX_RINGS; i++ ) {
		ring[i] = 0;
	}
	for ( int i = 0; i < MAX_STARS; i++ ) {
		star[i][0] = star[i][1] = -1;
	}
}

GLTeleporter::~GLTeleporter() {
}


void GLTeleporter::draw() {
	float r = ( static_cast<float>( width ) * MUL ) / 2.0f;
	for ( int i = 0; i < MAX_STARS; i++ ) {
		// reposition
		if ( star[i][0] == -1 ) {
			starAngle[i] = Util::roll( 0.0f, 360.0f );
			starSpeed[i] = Util::roll( 2.0f, 10.0f );
		}
		star[i][0] = r + ( r * Constants::cosFromAngle( starAngle[i] ) );
		star[i][1] = r + ( r * Constants::sinFromAngle( starAngle[i] ) );

		// draw
		glsDisable( GLS_CULL_FACE );
		glPushMatrix();


		float w = ( static_cast<float>( width ) * MUL ) / 10.0f;
		float d = ( static_cast<float>( depth ) * MUL ) / 10.0f;
		float h = 1.25f * MUL;

		if ( flameTex.isSpecified() ) flameTex.glBind();

		glColor4f( 1, 1, 1, 1 );

		glBegin( GL_TRIANGLE_STRIP );
		// front
		if ( flameTex.isSpecified() ) glTexCoord2f( 0.0f, 0.0f );
		glVertex3f( star[i][0], star[i][1], h );
		if ( flameTex.isSpecified() ) glTexCoord2f( 1.0f, 0.0f );
		glVertex3f( star[i][0] + w, star[i][1], h );
		if ( flameTex.isSpecified() ) glTexCoord2f( 0.0f, 1.0f );
		glVertex3f( star[i][0], star[i][1] + d, h );
		if ( flameTex.isSpecified() ) glTexCoord2f( 1.0f, 1.0f );
		glVertex3f( star[i][0] + w, star[i][1] + d, h );
		glEnd();

		glPopMatrix();
		glsEnable( GLS_CULL_FACE );

		// move
		starAngle[i] += starSpeed[i];
		if ( starAngle[i] >= 360.0f ) starAngle[i] -= 360.0f;
	}

	for ( int i = 0; !locked && i < MAX_RINGS; i++ ) {
		// reposisition
		if ( ring[i] <= ( 1.0f * MUL ) || ring[i] >= ( static_cast<float>( height ) * MUL ) ) {
			ring[i] = ( static_cast<float>( height - 1 ) * MUL ) / 2.0f + Util::roll( -10.0f, 10.0f );
			delta[i] = Util::dice( 2 ) ? 3.0f : -3.0f;
		}

		// draw
		glsDisable( GLS_CULL_FACE );
		glPushMatrix();

		float w = ( static_cast<float>( width ) * MUL );
		float d = ( static_cast<float>( depth ) * MUL );
		float h = ring[i];
		//      if(h == 0) h = 0.25 * MUL;

		if ( flameTex.isSpecified() ) flameTex.glBind();

		float red = static_cast<float>( ( this->color & 0xff000000 ) >> ( 3 * 8 ) ) / static_cast<float>( 0xff );
		float green = static_cast<float>( ( this->color & 0x00ff0000 ) >> ( 2 * 8 ) ) / static_cast<float>( 0xff );
		float blue = static_cast<float>( ( this->color & 0x0000ff00 ) >> ( 1 * 8 ) ) / static_cast<float>( 0xff );
		float max = ( static_cast<float>( height - 1 ) * MUL ) / 2.0f;
		// float dist = 1.5f - abs(max - ring[i]) / max;
		float dist = 1.5f - abs( static_cast<int>( max - ring[i] ) ) / max;
		glColor4f( red, green, blue, dist );

		glBegin( GL_TRIANGLE_STRIP );
		// front
		if ( flameTex.isSpecified() ) glTexCoord2i( 0, 0 );
		glVertex3f( 0, 0, h );
		if ( flameTex.isSpecified() ) glTexCoord2i( 1, 0 );
		glVertex3f( w, 0, h );
		if ( flameTex.isSpecified() ) glTexCoord2i( 0, 1 );
		glVertex3f( 0, d, h );
		if ( flameTex.isSpecified() ) glTexCoord2i( 1, 1 );
		glVertex3f( w, d, h );
		glEnd();

		glPopMatrix();
		glsEnable( GLS_CULL_FACE );

		// move
		ring[i] += delta[i];
	}

}
