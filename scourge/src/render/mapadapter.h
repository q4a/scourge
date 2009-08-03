/***************************************************************************
               mapadapter.h  -  Map related utility interface
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

#ifndef MAP_ADAPTER_H
#define MAP_ADAPTER_H
#pragma once

#include "render.h"

class RenderedCreature;
class Shape;
class Location;
class RenderedItem;
class Texture;

/// Utility functions for level maps.

class MapAdapter {
public:
	MapAdapter();
	virtual ~MapAdapter();

	/// Updates the progress meter (when loading a map etc.)
	virtual void setUpdate( char *message, int n, int total ) = 0;
	/// Mouse cursor visible?
	virtual void setCursorVisible( bool b ) = 0;
	/// Screen width in pixels.
	virtual int getScreenWidth() = 0;
	/// Screen height in pixels.
	virtual int getScreenHeight() = 0;
	/// X position of the mouse.
	virtual Uint16 getMouseX() = 0;
	/// Y position of the mouse.
	virtual Uint16 getMouseY() = 0;
	/// Has the mouse moved over the main 3D view?
	virtual bool isMouseIsMovingOverMap() = 0;
	/// The active party character.
	virtual RenderedCreature *getPlayer() = 0;
	virtual void setDebugStr( char *s ) = 0;
	/// Is it the mission creature?
	virtual bool isMissionCreature( RenderedCreature *creature ) = 0;
	/// Does a party exist?
	virtual bool hasParty() = 0;
	/// How many characters are in the party?
	virtual int getPartySize() = 0;
	/// Gets a character from the party by index.
	virtual RenderedCreature *getParty( int index ) = 0;
	/// Creates an item by name.
	virtual RenderedItem *createItem( char *item_name, int level, int depth ) = 0;
	/// Creates a monster by name.
	virtual RenderedCreature *createMonster( char *monster_name ) = 0;
	/// Creates an item (detailed version).
	virtual RenderedItem *createItem( ItemInfo *info ) = 0;
	/// Creates an NPC/monster (detailed version).
	virtual RenderedCreature *createMonster( CreatureInfo *info ) = 0;
	/// Loads a saved map from disk.
	virtual void loadMapData( const std::string& name ) = 0;
	/// Saves the map to disk.
	virtual void saveMapData( const std::string& name ) = 0;
	/// Return the magic school name for a location (typically a pool).
	virtual char const* getMagicSchoolIndexForLocation( Location *pos ) = 0;
	virtual void setMagicSchoolIndexForLocation( Location *pos, char const* magicSchoolName ) = 0;
	/// Prints text to the screen.
	virtual void texPrint( GLfloat x, GLfloat y, const char *fmt, ... ) = 0;
	/// Notifies Squirrel that a shape has been added to a map pos.
	virtual void shapeAdded( const char *shapeName, int x, int y, int z ) = 0;

	/// What color to outline this location with? If NULL is returned the location is not outlined.
	virtual Color *getOutlineColor( Location *pos ) = 0;

	/// Set up the opengl view.
	virtual void setView() = 0;

	/// Is the oval cutout "shade" shown over the map?
	virtual bool isLevelShaded() = 0;

	/// Rectangle intersection test.
	virtual bool intersects( int x, int y, int w, int h,
	                         int x2, int y2, int w2, int h2 ) = 0;

	/// Adds a message to the log scroller using a custom color.
	virtual void addDescription( char const* description, float r = 1.0f, float g = 1.0f, float b = 0.4f, int logLevel = Constants::LOGLEVEL_FULL ) = 0;
	/// Adds a message of the specified type to the log scroller.
	virtual void writeLogMessage( char const* message, int messageType = Constants::MSGTYPE_NORMAL, int logLevel = Constants::LOGLEVEL_FULL ) = 0;

	/// Gets a texture from the shape palette by name.
	virtual Texture const& getNamedTexture( char *name ) = 0;

	/// Are we in movie mode?
	virtual bool isInMovieMode() = 0;
	
	virtual void saveMapRegions() = 0;
	virtual void mapRegionsChanged( float party_x, float party_y ) = 0;
	
	// save/load generators
	virtual int getGeneratorCount( int region_x, int region_y ) = 0;
	virtual GeneratorInfo *getGeneratorInfo( int region_x, int region_y, int index ) = 0;
	virtual void loadGenerator( GeneratorInfo *info ) = 0;
};

#endif
