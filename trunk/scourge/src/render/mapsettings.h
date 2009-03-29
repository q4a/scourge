/***************************************************************************
                  mapsettings.h  -  Manages and renders the level map
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

#ifndef MAPSETTINGS_H_
#define MAPSETTINGS_H_
#pragma once

/// General map settings.

class MapSettings {

public:
	MapSettings() {
	}

	virtual ~MapSettings() {
	}

	virtual bool isLightMapEnabled() = 0;
	virtual bool isGridShowing() = 0;
	virtual bool isPlayerEnabled() = 0;
	virtual bool isItemPosEnabled() = 0;
	virtual float getMinZoomIn() = 0;
	virtual float getMaxZoomOut() = 0;
	virtual float getMaxYRot() = 0;
	virtual void setVisibleMapSize( int *mapViewWidth, int *mapViewDepth ) {
		*mapViewWidth = *mapViewDepth = 100;
	}
};

/// General map settings (ingame).

class GameMapSettings : public MapSettings {

public:
	GameMapSettings();
	virtual ~GameMapSettings();

	bool isLightMapEnabled();
	bool isGridShowing();
	bool isPlayerEnabled();
	bool isItemPosEnabled();
	float getMinZoomIn();
	float getMaxZoomOut();
	float getMaxYRot();
};

/// General map settings for the map editor.

class EditorMapSettings : public MapSettings {

public:
	EditorMapSettings();
	virtual ~EditorMapSettings();

	bool isLightMapEnabled();
	bool isGridShowing();
	bool isPlayerEnabled();
	bool isItemPosEnabled();
	float getMinZoomIn();
	float getMaxZoomOut();
	float getMaxYRot();
	
	virtual void setVisibleMapSize( int *mapViewWidth, int *mapViewDepth ) {
		*mapViewWidth = *mapViewDepth = 200;
	}
};


#endif /*MAPSETTINGS_H_*/
