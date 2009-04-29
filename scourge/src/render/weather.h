/***************************************************************************
                        weather.h  -  Weather engine
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

#ifndef WEATHER_H
#define WEATHER_H
#pragma once

#include "../session.h"

class Session;

/// A "weather" object that manages and controls the weather in the outdoors.

class Weather {
private:
	Session *session;
	int oldWeather;
	int currentWeather;

#define RAIN_DROP_COUNT 300
#define RAIN_DROP_SIZE 32
	float rainDropX[RAIN_DROP_COUNT];
	float rainDropY[RAIN_DROP_COUNT];
	float rainDropZ[RAIN_DROP_COUNT];
#define CLOUD_COUNT 15
	float cloudX[CLOUD_COUNT];
	float cloudY[CLOUD_COUNT];
	float cloudSize[CLOUD_COUNT];
	int cloudSpeed[CLOUD_COUNT];
	Uint32 lastWeatherUpdate;
	Uint32 lastLightning;
	Uint32 lastLightningRoll;
	float lightningBrightness;
	bool thunderOnce;

public:
	Weather( Session *session );
	~Weather();

	inline void setWeather( int i ) { currentWeather = i; }
	inline int getOldWeather() { return oldWeather; }
	inline int getCurrentWeather() { return currentWeather; }

	void thunder();

	int generateWeather();
	void generateRain();
	void generateClouds();
	void drawWeather();

	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
