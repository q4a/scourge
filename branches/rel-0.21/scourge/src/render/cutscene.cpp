/***************************************************************************
                  cutscene.cpp  -  Utilities for movie mode
                             -------------------
    begin                : Tue May 13 2008
    copyright            : (C) 2008 by Dennis Murczak
    email                : dmurczak@versanet.de
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
#include "cutscene.h"
#include "map.h"

using namespace std;

#define LETTERBOX_DURATION 300

Cutscene::Cutscene( Session *session ) {
	this->session = session;

	// FIXME: perhaps this should be the hdtv aspect ration given the current pixel width?
	letterboxHeight = ( int )( ( float )session->getPreferences()->getH() / 8 );

	inMovieMode = false;
	endingMovie = false;
	cameraMoving = false;
}

Cutscene::~Cutscene() {
}

/// Enters movie mode.

void Cutscene::startMovieMode() {
	fromX = originalX = session->getMap()->getMapX();
	fromY = originalY = session->getMap()->getMapY();
	fromZ = originalZ = 0;

	fromXRot = originalXRot = session->getMap()->getXRot();
	fromYRot = originalYRot = session->getMap()->getYRot();
	fromZRot = originalZRot = session->getMap()->getZRot();

	fromZoom = originalZoom = session->getMap()->getZoom();

	inMovieMode = true;
	endingMovie = false;
	cameraMoving = false;

	cameraStartTime = cameraDuration = letterboxStartTime = letterboxEndTime = 0;

	startLetterbox();
}

/// Starts leaving movie mode.

/// Initiates end of the movie. Note that movie mode does
/// not end until the letterbox has faded out completely.

void Cutscene::endMovieMode() {
	endingMovie = true;
	// start resetting the camera
	animateCamera( originalX, originalY, originalZ,
	               originalXRot, originalYRot, originalZRot,
	               originalZoom, LETTERBOX_DURATION );
	endLetterbox();
}

/// Starts fading in the black bars.

void Cutscene::startLetterbox() {
	letterboxStartTime = SDL_GetTicks();
}

/// Starts fading out the black bars.

void Cutscene::endLetterbox() {
	letterboxEndTime = SDL_GetTicks();
}

/// Check whether in movie mode (fading of the black bars counts too).

bool Cutscene::isInMovieMode() {
	if ( inMovieMode && endingMovie ) {
		Uint32 now = SDL_GetTicks();
		// Did the movie end?
		if ( now > ( letterboxEndTime + LETTERBOX_DURATION ) ) {
			inMovieMode = false; endingMovie = false;
			placeCamera( originalX, originalY, originalZ,
	                             originalXRot, originalYRot, originalZRot,
	                             originalZoom );
			updateCameraPosition();
		}
	}

	return inMovieMode;
}

/// Returns current height of the black bars.

int Cutscene::getCurrentLetterboxHeight() {
	Uint32 now = SDL_GetTicks();
	int h;

	if ( endingMovie ) {

		if ( ( now - letterboxEndTime ) > LETTERBOX_DURATION ) {
			h = letterboxHeight;
		} else {
			float percent = ( float )( now - letterboxEndTime ) / LETTERBOX_DURATION;
			h = ( float )letterboxHeight * ( 1.0f - percent );
		}

	} else {

		if ( ( now - letterboxStartTime ) > LETTERBOX_DURATION ) {
			h = letterboxHeight;
		} else {
			float percent = ( float )( now - letterboxStartTime ) / LETTERBOX_DURATION;
			h = ( float )letterboxHeight * ( percent );
		}

	}

	return h;
}

/// Place the camera within the scene (level map coordinates).

void Cutscene::placeCamera( float x, float y, float z, float xRot, float yRot, float zRot, float zoom ) {
	Uint32 now = SDL_GetTicks();

	fromX = toX = x;
	fromY = toY = y;
	fromZ = toZ = z;

	fromXRot = toXRot = xRot;
	fromYRot = toYRot = yRot;
	fromZRot = toZRot = zRot;

	fromZoom = toZoom = zoom;

	cameraStartTime = now;
	cameraDuration = 0;
	cameraMoving = false;
}

/// Starts moving/rotating the camera towards a target position/orientation.

void Cutscene::animateCamera( float targetX, float targetY, float targetZ, float targetXRot, float targetYRot, float targetZRot, float targetZoom, Uint32 duration ) {
	Uint32 now = SDL_GetTicks();

	toX = targetX;
	toY = targetY;
	toZ = targetZ;

	toXRot = targetXRot;
	toYRot = targetYRot;
	toZRot = targetZRot;

	toZoom = targetZoom;

	cameraStartTime = now;
	cameraDuration = duration;
	cameraMoving = true;
}

/// Synchronizes the ingame camera with the scripted camera.

void Cutscene::updateCameraPosition() {
	session->getMap()->setPos( getCameraX(), getCameraY(), getCameraZ() );
	session->getMap()->setRot( getCameraXRot(), getCameraYRot(), getCameraZRot() );
	session->getMap()->setZoom( getCameraZoom() );
}

/// Returns whether the camera currently, well, moves.

bool Cutscene::isCameraMoving() {
	Uint32 now = SDL_GetTicks();

	if ( cameraMoving ) {
		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
		}
	}

	return cameraMoving;
}

float Cutscene::getCameraX() {
	Uint32 now = SDL_GetTicks();
	float x;

	if ( cameraMoving ) {

		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
			x = fromX;
		} else {
			float percent = ( float )( now - cameraStartTime ) / ( float )cameraDuration;
			x = fromX + ( percent * ( toX - fromX ) );
		}

	} else {
		x = fromX;
	}

	return x;
}

float Cutscene::getCameraY() {
	Uint32 now = SDL_GetTicks();
	float y;

	if ( cameraMoving ) {

		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
			y = fromY;
		} else {
			float percent = ( float )( now - cameraStartTime ) / ( float )cameraDuration;
			y = fromY + ( percent * ( toY - fromY ) );
		}

	} else {
		y = fromY;
	}

	return y;
}

float Cutscene::getCameraZ() {
	Uint32 now = SDL_GetTicks();
	float z;

	if ( cameraMoving ) {

		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
			z = fromZ;
		} else {
			float percent = ( float )( now - cameraStartTime ) / ( float )cameraDuration;
			z = fromZ + ( percent * ( toZ - fromZ ) );
		}

	} else {
		z = fromZ;
	}

	return z;
}

float Cutscene::getCameraXRot() {
	Uint32 now = SDL_GetTicks();
	float r;

	if ( cameraMoving ) {

		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
			r = fromXRot;
		} else {
			float percent = ( float )( now - cameraStartTime ) / ( float )cameraDuration;
			float diff = Util::diffAngle( toXRot, fromXRot );
			r = fromXRot + ( diff * percent );
			while ( r >= 360 ) r -= 360;
		}

	} else {
		r = fromXRot;
	}

	return r;
}

float Cutscene::getCameraYRot() {
	Uint32 now = SDL_GetTicks();
	float r;

	if ( cameraMoving ) {

		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
			r = fromYRot;
		} else {
			float percent = ( float )( now - cameraStartTime ) / ( float )cameraDuration;
			float diff = Util::diffAngle( toYRot, fromYRot );
			r = fromYRot + ( diff * percent );
			while ( r >= 360 ) r -= 360;
		}

	} else {
		r = fromYRot;
	}

	return r;
}

float Cutscene::getCameraZRot() {
	Uint32 now = SDL_GetTicks();
	float r;

	if ( cameraMoving ) {

		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
			r = fromZRot;
		} else {
			float percent = ( float )( now - cameraStartTime ) / ( float )cameraDuration;
			float diff = Util::diffAngle( toZRot, fromZRot );
			r = fromZRot + ( diff * percent );
			while ( r >= 360 ) r -= 360;
		}

	} else {
		r = fromZRot;
	}

	return r;
}

float Cutscene::getCameraZoom() {
	Uint32 now = SDL_GetTicks();
	float m;

	if ( cameraMoving ) {

		if ( ( now - cameraStartTime ) > cameraDuration ) {
			placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
			m = fromZoom;
		} else {
			float percent = ( float )( now - cameraStartTime ) / ( float )cameraDuration;
			m = fromZoom + ( percent * ( toZoom - fromZoom ) );
		}

	} else {
		m = fromZoom;
	}

	return m;
}

/// Draws the black bars at the top and bottom of the screen.

void Cutscene::drawLetterbox() {
	int w = session->getGameAdapter()->getScreenWidth();
	int h = getCurrentLetterboxHeight();

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );

	glColor3f( 0.0f, 0.0f, 0.0f );

	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0, 0, 0 );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( 0, 0 );
	glVertex2i( w, 0 );
	glVertex2i( 0, h );
	glVertex2i( w, h );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0, session->getGameAdapter()->getScreenHeight() - h, 0 );
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2i( 0, 0 );
	glVertex2i( w, 0 );
	glVertex2i( 0, h );
	glVertex2i( w, h );
	glEnd();
	glPopMatrix();

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1, 1, 1, 1 );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );
}
