/***************************************************************************
                map.cpp  -  Manages and renders the level map
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

#include "mapsettings.h"

// can't use, undefined: using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

GameMapSettings::GameMapSettings() {
}

GameMapSettings::~GameMapSettings() {
}

/// Does it have a "light map" (stores which parts of the level are visible)?

bool GameMapSettings::isLightMapEnabled() {
	return true;
}

bool GameMapSettings::isGridShowing() {
	return false;
}

bool GameMapSettings::isPlayerEnabled() {
	return true;
}

bool GameMapSettings::isItemPosEnabled() {
	return true;
}

/// How far can the camera zoom in?

float GameMapSettings::getMinZoomIn() {
	return 0.85f;
}

/// How far can the camera zoom out?

float GameMapSettings::getMaxZoomOut() {
	return 2.8f;
}

/// How "high" can the camera be (degrees)?

float GameMapSettings::getMaxYRot() {
	return 55.0f;
}





EditorMapSettings::EditorMapSettings() {
}

EditorMapSettings::~EditorMapSettings() {
}

bool EditorMapSettings::isLightMapEnabled() {
	return false;
}

bool EditorMapSettings::isGridShowing() {
	return true;
}

bool EditorMapSettings::isPlayerEnabled() {
	return false;
}

bool EditorMapSettings::isItemPosEnabled() {
	return false;
}

float EditorMapSettings::getMinZoomIn() {
	return 0.05f;
}

float EditorMapSettings::getMaxZoomOut() {
	return 2.8f;
}

float EditorMapSettings::getMaxYRot() {
	return 90.0f;
}
