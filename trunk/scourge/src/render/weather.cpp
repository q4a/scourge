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
#include "../configlang.h"

using namespace std;

Weather::Weather( Session *session ) {
	this->session = session;

	currentWeather = WEATHER_CLEAR;

	lastWeatherUpdate = lastWeatherRoll = SDL_GetTicks();
	lastWeatherChange = 0;
	thunderOnce = false;

	int climateIndex;

	ConfigLang *config = ConfigLang::load( "config/climate.cfg" );

	vector<ConfigNode*> *climates = config->getDocument()->getChildrenByName( "climate" );
	
	for ( unsigned int c = 0; climates && c < climates->size(); c++ ) {
		ConfigNode *node = ( *climates )[c];
		string id = node->getValueAsString( "id" );

		if ( id == "boreal" ) {
			climateIndex = CLIMATE_BOREAL;
		} else if ( id == "alpine" ) {
			climateIndex = CLIMATE_ALPINE;
		} else if ( id == "temperate" ) {
			climateIndex = CLIMATE_TEMPERATE;
		} else if ( id == "subtropical" ) {
			climateIndex = CLIMATE_SUBTROPICAL;
		} else if ( id == "tropical" ) {
			climateIndex = CLIMATE_TROPICAL;
		} else {
			climateIndex = CLIMATE_TEMPERATE;
		}

		vector<ConfigNode*> *times = node->getChildrenByName( "rain" );

		if ( times ) {
			ConfigNode *rainNode = ( *times )[0];
			for ( map<string, ConfigValue*>::iterator t = rainNode->getValues()->begin();
			        t != rainNode->getValues()->end(); ++t ) {
				string name = t->first;
				ConfigValue *value = t->second;

				if ( name == "allyear" ) {
					for ( int m = 0; m < 12; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winterhalfyear" ) {
					for ( int m = 0; m < 2; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					for ( int m = 8; m < 12; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summerhalfyear" ) {
					for ( int m = 2; m < 8; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winter" ) {
					for ( int m = 0; m < 2; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					rainByMonths[ 11 ][ climateIndex ] = value->getAsFloat();
				} else if ( name == "spring" ) {
					for ( int m = 2; m < 5; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summer" ) {
					for ( int m = 5; m < 8; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "autumn" ) {
					for ( int m = 8; m < 11; m++ ) { rainByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "daytime" ) {
					for ( int h = 6; h < 18; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "nighttime" ) {
					for ( int h = 18; h < 24; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
					for ( int h = 0; h < 6; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "morning" ) {
					for ( int h = 6; h < 10; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "noon" ) {
					for ( int h = 10; h < 14; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "afternoon" ) {
					for ( int h = 14; h < 18; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "evening" ) {
					for ( int h = 18; h < 24; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "night" ) {
					for ( int h = 0; h < 6; h++ ) { rainByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				}

			}
		}

		times = node->getChildrenByName( "snow" );

		if ( times ) {
			ConfigNode *snowNode = ( *times )[0];
			for ( map<string, ConfigValue*>::iterator t = snowNode->getValues()->begin();
			        t != snowNode->getValues()->end(); ++t ) {
				string name = t->first;
				ConfigValue *value = t->second;

				if ( name == "allyear" ) {
					for ( int m = 0; m < 12; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winterhalfyear" ) {
					for ( int m = 0; m < 2; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					for ( int m = 8; m < 12; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summerhalfyear" ) {
					for ( int m = 2; m < 8; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winter" ) {
					for ( int m = 0; m < 2; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					snowByMonths[ 11 ][ climateIndex ] = value->getAsFloat();
				} else if ( name == "spring" ) {
					for ( int m = 2; m < 5; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summer" ) {
					for ( int m = 5; m < 8; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "autumn" ) {
					for ( int m = 8; m < 11; m++ ) { snowByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "daytime" ) {
					for ( int h = 6; h < 18; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "nighttime" ) {
					for ( int h = 18; h < 24; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
					for ( int h = 0; h < 6; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "morning" ) {
					for ( int h = 6; h < 10; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "noon" ) {
					for ( int h = 10; h < 14; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "afternoon" ) {
					for ( int h = 14; h < 18; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "evening" ) {
					for ( int h = 18; h < 24; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "night" ) {
					for ( int h = 0; h < 6; h++ ) { snowByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				}

			}
		}

		times = node->getChildrenByName( "thunder" );

		if ( times ) {
			ConfigNode *thunderNode = ( *times )[0];
			for ( map<string, ConfigValue*>::iterator t = thunderNode->getValues()->begin();
			        t != thunderNode->getValues()->end(); ++t ) {
				string name = t->first;
				ConfigValue *value = t->second;

				if ( name == "allyear" ) {
					for ( int m = 0; m < 12; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winterhalfyear" ) {
					for ( int m = 0; m < 2; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					for ( int m = 8; m < 12; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summerhalfyear" ) {
					for ( int m = 2; m < 8; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winter" ) {
					for ( int m = 0; m < 2; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					thunderByMonths[ 11 ][ climateIndex ] = value->getAsFloat();
				} else if ( name == "spring" ) {
					for ( int m = 2; m < 5; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summer" ) {
					for ( int m = 5; m < 8; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "autumn" ) {
					for ( int m = 8; m < 11; m++ ) { thunderByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "daytime" ) {
					for ( int h = 6; h < 18; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "nighttime" ) {
					for ( int h = 18; h < 24; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
					for ( int h = 0; h < 6; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "morning" ) {
					for ( int h = 6; h < 10; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "noon" ) {
					for ( int h = 10; h < 14; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "afternoon" ) {
					for ( int h = 14; h < 18; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "evening" ) {
					for ( int h = 18; h < 24; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "night" ) {
					for ( int h = 0; h < 6; h++ ) { thunderByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				}

			}
		}

		times = node->getChildrenByName( "fog" );

		if ( times ) {
			ConfigNode *fogNode = ( *times )[0];
			for ( map<string, ConfigValue*>::iterator t = fogNode->getValues()->begin();
			        t != fogNode->getValues()->end(); ++t ) {
				string name = t->first;
				ConfigValue *value = t->second;

				if ( name == "allyear" ) {
					for ( int m = 0; m < 12; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winterhalfyear" ) {
					for ( int m = 0; m < 2; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					for ( int m = 8; m < 12; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summerhalfyear" ) {
					for ( int m = 2; m < 8; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "winter" ) {
					for ( int m = 0; m < 2; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
					fogByMonths[ 11 ][ climateIndex ] = value->getAsFloat();
				} else if ( name == "spring" ) {
					for ( int m = 2; m < 5; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "summer" ) {
					for ( int m = 5; m < 8; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "autumn" ) {
					for ( int m = 8; m < 11; m++ ) { fogByMonths[ m ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "daytime" ) {
					for ( int h = 6; h < 18; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "nighttime" ) {
					for ( int h = 18; h < 24; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
					for ( int h = 0; h < 6; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "morning" ) {
					for ( int h = 6; h < 10; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "noon" ) {
					for ( int h = 10; h < 14; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "afternoon" ) {
					for ( int h = 14; h < 18; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "evening" ) {
					for ( int h = 18; h < 24; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				} else if ( name == "night" ) {
					for ( int h = 0; h < 6; h++ ) { fogByHours[ h ][ climateIndex ] = value->getAsFloat(); }
				}

			}
		}

	}

	delete config;
}

Weather::~Weather() {
}

#define WEATHER_ROLL_INTERVAL 24000
#define WEATHER_ROLL_CHANCE 0.2f

#define WEATHER_CHANGE_DURATION 6000

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

	bool weatherChanging = ( now - lastWeatherChange ) > WEATHER_CHANGE_DURATION ? false : true;

	float rainIntensity, snowIntensity, thunderIntensity, fogIntensity;

	if ( !weatherChanging ) {
		rainIntensity = currentWeather & WEATHER_RAIN ? 1.0f : 0.0f;
		snowIntensity = currentWeather & WEATHER_SNOW ? 1.0f : 0.0f;
		thunderIntensity = currentWeather & WEATHER_THUNDER ? 1.0f : 0.0f;
		fogIntensity = currentWeather & WEATHER_FOG ? 1.0f : 0.0f;
		if ( ( now - lastWeatherRoll ) > WEATHER_ROLL_INTERVAL ) {
			if ( Util::roll( 0.0f, 1.0f ) <= WEATHER_ROLL_CHANCE ) changeWeather( generateWeather() );
			lastWeatherRoll = now;
		}
	} else {
		if ( ( currentWeather & WEATHER_RAIN ) && !( oldWeather & WEATHER_RAIN ) ) {
			rainIntensity = (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION;
		} else if ( !( currentWeather & WEATHER_RAIN ) && ( oldWeather & WEATHER_RAIN ) ) {
			rainIntensity = 1.0f - ( (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION );
		} else if ( ( currentWeather & WEATHER_RAIN ) && ( oldWeather & WEATHER_RAIN ) ) {
			rainIntensity = 1.0f;
		}
		if ( ( currentWeather & WEATHER_SNOW ) && !( oldWeather & WEATHER_SNOW ) ) {
			snowIntensity = (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION;
		} else if ( !( currentWeather & WEATHER_SNOW ) && ( oldWeather & WEATHER_SNOW ) ) {
			snowIntensity = 1.0f - ( (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION );
		} else if ( ( currentWeather & WEATHER_SNOW ) && ( oldWeather & WEATHER_SNOW ) ) {
			snowIntensity = 1.0f;
		}
		if ( ( currentWeather & WEATHER_THUNDER ) && !( oldWeather & WEATHER_THUNDER ) ) {
			thunderIntensity = (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION;
		} else if ( !( currentWeather & WEATHER_THUNDER ) && ( oldWeather & WEATHER_THUNDER ) ) {
			thunderIntensity = 1.0f - ( (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION );
		} else if ( ( currentWeather & WEATHER_THUNDER ) && ( oldWeather & WEATHER_THUNDER ) ) {
			thunderIntensity = 1.0f;
		}
		if ( ( currentWeather & WEATHER_FOG ) && !( oldWeather & WEATHER_FOG ) ) {
			fogIntensity = (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION;
		} else if ( !( currentWeather & WEATHER_FOG ) && ( oldWeather & WEATHER_FOG ) ) {
			fogIntensity = 1.0f - ( (float)(now - lastWeatherChange) / WEATHER_CHANGE_DURATION );
		} else if ( ( currentWeather & WEATHER_FOG ) && ( oldWeather & WEATHER_FOG ) ) {
			fogIntensity = 1.0f;
		}
	}

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );

	glEnable( GL_ALPHA_TEST );
	glAlphaFunc( GL_NOTEQUAL, 0 );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_TEXTURE_2D );

	// Draw the fog
	if ( shouldDrawWeather && fogIntensity ) {
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 0, 0, 0 );
		glColor4f( 1.0f, 1.0f, 1.0f, 0.4f * fogIntensity );

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
	if ( shouldDrawWeather && rainIntensity ) {

		deltaY = static_cast<int>( static_cast<float>( now - lastWeatherUpdate ) * ( static_cast<float>( RAIN_DROP_SPEED ) / 1000 ) );
		deltaX = deltaY / 4;

		int rainDropCount = ( int )( RAIN_DROP_COUNT * ( 1.0f - session->getMap()->getZoomPercent() ) * rainIntensity );
		if ( rainDropCount > RAIN_DROP_COUNT ) rainDropCount = RAIN_DROP_COUNT;
		else if ( ( rainDropCount < MIN_RAIN_DROP_COUNT ) && !weatherChanging ) rainDropCount = MIN_RAIN_DROP_COUNT;

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

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Draw the fog clouds
	if ( shouldDrawWeather && fogIntensity ) {

		session->getShapePalette()->fogCloudTexture.glBind();
		glColor4f( 1.0f, 1.0f, 1.0f, 0.6f * fogIntensity );
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

		if ( thunderOnce || ( shouldDrawWeather && thunderIntensity ) ) {
			if ( lightningTime < 101 ) {
				brightness = ( ( float )lightningTime / 100 ) * lightningBrightness * thunderIntensity;
			} else {
				brightness = ( ( 201 - ( float )lightningTime ) / 100 ) * lightningBrightness * thunderIntensity;
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
			if ( thunderOnce || ( outside && thunderIntensity ) ) {
				session->getSound()->playThunderSound( underRoof );
				thunderOnce = false;
			}
		}
		lastLightningRoll = now;
	}

	lastWeatherUpdate = now;

	glDisable( GL_ALPHA_TEST );

	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );
	glDisable( GL_BLEND );
}

/// Determines which type of weather the map will have.

int Weather::generateWeather() {
	int weather;
	weather = Util::pickOne( 1, MAX_WEATHER );

	return weather;
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

void Weather::changeWeather( int newWeather ) {
	oldWeather = currentWeather;
	currentWeather = newWeather;

	lastWeatherChange = SDL_GetTicks();

	if ( ( currentWeather & WEATHER_RAIN ) && !( oldWeather & WEATHER_RAIN ) ) {
		session->getSound()->startRain();
	} else if ( ( oldWeather & WEATHER_RAIN ) && !( currentWeather & WEATHER_RAIN ) ) {
		session->getSound()->stopRain();
	}
	
}
