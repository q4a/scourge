/***************************************************************************
                       weather.cpp  -  Weather engine
                             -------------------
    begin                : Wed Apr 29 2009
    copyright            : (C) 2009 by Dennis Murczak
    email                : dmurczak@googlemail.com
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
#include "weather.h"
#include "map.h"
#include "../shapepalette.h"
#include "../sound.h"

using namespace std;

Weather::Weather( Session *session ) {
	this->session = session;

	currentWeather = WEATHER_CLEAR;

	lastWeatherUpdate = SDL_GetTicks();
	thunderOnce = false;
}

Weather::~Weather() {
}

#define MIN_RAIN_DROP_COUNT 50
#define MIN_CLOUD_COUNT 3

/// Draws the weather effects.

void Weather::drawWeather() {

#define RAIN_DROP_SPEED 1200

	// Draw weather effects only on outdoor maps, when not inside a house
	bool shouldDrawWeather =  session->getMap()->isHeightMapEnabled() && !session->getMap()->getCurrentlyUnderRoof();

	Uint32 now = SDL_GetTicks();

	int lightningTime = now - lastLightning;
	if ( lastLightningRoll == 0 ) lastLightningRoll = now;

	int screenW = session->getGameAdapter()->getScreenWidth();
	int screenWPlusMore = static_cast<int>( static_cast<float>( screenW ) * 1.25f );
	int screenH = session->getGameAdapter()->getScreenHeight();

	bool outside = session->getMap()->isHeightMapEnabled();
	float mapZoom = session->getMap()->getZoom();
	bool underRoof = session->getMap()->getCurrentlyUnderRoof();

	float deltaY;
	float deltaX;
	float cloudDelta;

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_TEXTURE_2D );

	// Draw the fog
	if ( shouldDrawWeather && ( currentWeather & WEATHER_FOG ) ) {
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 0, 0, 0 );
		glColor4f( 1.0f, 1.0f, 1.0f, 0.4f );

		glBegin( GL_TRIANGLE_STRIP );
		glVertex2i( 0, 0 );
		glVertex2i( screenW, 0 );
		glVertex2i( 0, screenH );
		glVertex2i( screenW, screenH );
		glEnd();

		glPopMatrix();
	}
	
	if ( outside ) {
		session->getSound()->setWeatherVolume( underRoof );
	}

	glEnable( GL_TEXTURE_2D );

	// Draw the rain drops
	if ( shouldDrawWeather && ( currentWeather & WEATHER_RAIN ) ) {

		deltaY = static_cast<int>( static_cast<float>( now - lastWeatherUpdate ) * ( static_cast<float>( RAIN_DROP_SPEED ) / 1000 ) );
		deltaX = deltaY / 4;

		int rainDropCount = ( int )( RAIN_DROP_COUNT * ( 1.0f - session->getMap()->getZoomPercent() ) );
		if ( rainDropCount > RAIN_DROP_COUNT ) rainDropCount = RAIN_DROP_COUNT;
		else if ( rainDropCount < MIN_RAIN_DROP_COUNT ) rainDropCount = MIN_RAIN_DROP_COUNT;

		glPushMatrix();
		session->getShapePalette()->getRaindropTexture().glBind();

		for ( int i = 0; i < rainDropCount; i++ ) {
			if ( lightningTime < 501 && ( currentWeather & WEATHER_THUNDER ) ) {
				glColor4f( 1, 1, 1, rainDropZ[i] );
			} else {
				//glColor4f( 0, 0.8f, 1, 0.5f );
				glColor4f( 0, 0.5f, 0.7f, rainDropZ[i] );
			}
			glLoadIdentity();
			glTranslatef( rainDropX[i], rainDropY[i], 0 );
			glScalef( mapZoom, mapZoom, mapZoom );
			glRotatef( 15, 0, 0, 1 );

			glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2i( 0, 0 );
			glVertex2i( 0, 0 );
			glTexCoord2i( 1, 0 );
			glVertex2i( RAIN_DROP_SIZE, 0 );
			glTexCoord2i( 0, 1 );
			glVertex2i( 0, RAIN_DROP_SIZE );
			glTexCoord2i( 1, 1 );
			glVertex2i( RAIN_DROP_SIZE, RAIN_DROP_SIZE );
			glEnd();

			rainDropY[i] += ( deltaY * rainDropZ[i] );
			rainDropX[i] -= ( deltaX * rainDropZ[i] );

			if ( ( rainDropX[i] < -RAIN_DROP_SIZE ) || ( rainDropX[i] > screenWPlusMore ) || ( rainDropY[i] > screenH ) || ( rainDropY[i] < -screenH ) ) {
				rainDropX[i] = Util::pickOne( -RAIN_DROP_SIZE, screenWPlusMore );
				// Start new drops somewhere above the screen.
				// It prevents them sometimes aligning in horizontal "waves".
				rainDropY[i] = -Util::pickOne( RAIN_DROP_SIZE, screenH );
			}
		}
		glPopMatrix();
	}

	glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );

	// Draw the fog clouds
	if ( shouldDrawWeather && currentWeather & WEATHER_FOG ) {

		session->getShapePalette()->cloud.glBind();
		glColor4f( 1.0f, 1.0f, 0.7f, 0.8f );
		glPushMatrix();

		int cloudCount = ( int )( CLOUD_COUNT * ( 1.0f - session->getMap()->getZoomPercent() ) );
		if ( cloudCount > CLOUD_COUNT ) cloudCount = CLOUD_COUNT;
		else if ( cloudCount < MIN_CLOUD_COUNT ) cloudCount = MIN_CLOUD_COUNT;

		for ( int i = 0; i < cloudCount; i++ ) {

			cloudDelta = static_cast<float>( now - lastWeatherUpdate ) * ( static_cast<float>( cloudSpeed[i] ) / 1000 );

			glLoadIdentity();
			glTranslatef( cloudX[i], cloudY[i], 10 );
			glScalef( mapZoom, mapZoom, mapZoom );

			glBegin( GL_TRIANGLE_STRIP );
			glTexCoord2i( 0, 0 );
			glVertex2i( 0, 0 );
			glTexCoord2i( 1, 0 );
			glVertex2i( cloudSize[i] * 256.0f, 0 );
			glTexCoord2i( 0, 1 );
			glVertex2i( 0, cloudSize[i] * 128.0f );
			glTexCoord2i( 1, 1 );
			glVertex2i( cloudSize[i] * 256.0f, cloudSize[i] * 128.0f );
			glEnd();

			cloudX[i] -= cloudDelta;

			if ( cloudX[i] < -( 256.0f * mapZoom * cloudSize[i] ) || cloudX[i] > ( screenW * 2 ) || cloudY[i] < -( 128.0f * cloudSize[i] ) || cloudY[i] > screenH ) {
				cloudX[i] = Util::pickOne( screenW, screenW * 2 );
				cloudY[i] = Util::pickOne( -( int )( 128.0f * cloudSize[i] ), screenH );
				cloudSize[i] = 4.0f + ( Util::mt_rand() * 12 );
				cloudSpeed[i] = Util::pickOne( 10, 40 );
			}

		}

		glPopMatrix();

	}

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_TEXTURE_2D );

	// Draw the lightning
	if ( lightningTime < 201 ) {

		float brightness;

		if ( thunderOnce || ( shouldDrawWeather && ( currentWeather & WEATHER_THUNDER ) ) ) {
			if ( lightningTime < 101 ) {
				brightness = ( ( float )lightningTime / 100 ) * lightningBrightness;
			} else {
				brightness = ( ( 201 - ( float )lightningTime ) / 100 ) * lightningBrightness;
			}

			glPushMatrix();
			glLoadIdentity();
			glTranslatef( 0, 0, 0 );
			glColor4f( 1, 1, 1, brightness );

			glBegin( GL_TRIANGLE_STRIP );
			glVertex2i( 0, 0 );
			glVertex2i( screenW, 0 );
			glVertex2i( 0, screenH );
			glVertex2i( screenW, screenH );
			glEnd();

			glPopMatrix();
		}
	}

	if ( now > ( lastLightningRoll + 500 ) ) {
		if ( Util::dice( 25 ) == 0 ) {
			lastLightning = now;
			lightningBrightness = 0.3f + ( Util::mt_rand() * 0.5f );
			if ( thunderOnce || ( outside && ( currentWeather & WEATHER_THUNDER ) ) ) {
				session->getSound()->playThunderSound( underRoof );
				thunderOnce = false;
			}
		}
		lastLightningRoll = now;
	}

	lastWeatherUpdate = now;

	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );
}

/// Determines which type of weather the map will have.

int Weather::generateWeather() {
	if ( Util::dice( 3 ) == 0 && session->getMap()->isHeightMapEnabled() ) {
		currentWeather = Util::pickOne( 1, MAX_WEATHER );
	} else {
		currentWeather = WEATHER_CLEAR;
	}
	return currentWeather;
}

void Weather::thunder() {
	thunderOnce = true;
	lastLightning = SDL_GetTicks();
	lastLightningRoll = 0;
}

void Weather::generateRain() {
	for ( int i = 0; i < RAIN_DROP_COUNT; i++ ) {
		rainDropX[i] = Util::pickOne( -RAIN_DROP_SIZE, session->getGameAdapter()->getScreenWidth() );
		rainDropY[i] = Util::pickOne( -RAIN_DROP_SIZE, session->getGameAdapter()->getScreenHeight() );
		rainDropZ[i] = Util::roll( 0.25f, 1.0f );
	}
}

void Weather::generateClouds() {
	for ( int i = 0; i < CLOUD_COUNT; i++ ) {
		cloudSize[i] = 4.0f + ( Util::mt_rand() * 12 );
		cloudSpeed[i] = Util::pickOne( 10, 40 );
		cloudX[i] = Util::pickOne( -( int )( 256.0f * cloudSize[i] ), session->getGameAdapter()->getScreenWidth() );
		cloudY[i] = Util::pickOne( -( int )( 128.0f * cloudSize[i] ), session->getGameAdapter()->getScreenWidth() );
	}
}
