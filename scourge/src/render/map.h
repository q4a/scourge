/***************************************************************************
                  map.h  -  Manages and renders the level map
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

#ifndef MAP_H
#define MAP_H
#pragma once

#include "render.h"
#include <vector>
#include <set>
#include <sstream>
#include "location.h"
#include "maprenderhelper.h"
#include "renderedlocation.h"

class CFrustum;
class RenderedProjectile;
class Location;
class EffectLocation;
class Shape;
class RenderedCreature;
class RenderedItem;
class RenderedProjectile;
class GLShape;
class Shapes;
class MapAdapter;
class Location;
class Effect;
class DisplayInfo;
class MapRenderHelper;
class MapRender;
class MapSettings;
class MapMemoryManager;

enum {
	GROUND_LAYER = 0,
	ROAD_LAYER,

	MAX_OUTDOOR_LAYER
};

/// It's a trap!
class Trap {
public:
	SDL_Rect r;
	int type;
	bool discovered;
	bool enabled;
	std::vector<CVector2*> hull;
};

/// A 3D point with texture coordinates and color.
class CVectorTex {
public:
	float x, y, z, u, v, r, g, b, a;
	Texture tex;
};


#define SWAP(src, dst) { int _t; _t = src; src = dst; dst = _t; }
#define POS_CACHE_WIDTH    5
#define POS_CACHE_DEPTH    5
#define POS_CACHE_HEIGHT   10
#define MAX_POS_CACHE (POS_CACHE_WIDTH * POS_CACHE_DEPTH * POS_CACHE_HEIGHT)

/// A nice, fluffy rug.

struct Rug {
	Texture texture;
	bool isHorizontal;
	float angle;
};

// how many water points per 1 floor tile
#define WATER_TILE_X 8
#define WATER_TILE_Y 8

#define WATER_AMP 0.25f
#define WATER_ANIM_SPEED 10.0f
#define WATER_HEIGHT 1.2f
#define WATER_STEP 0.07f


/// An indoor water tile.
struct WaterTile {
	float z[WATER_TILE_X][WATER_TILE_Y];
	float step[WATER_TILE_X][WATER_TILE_Y];
	Uint32 lastTime[WATER_TILE_X][WATER_TILE_Y];
};

/**
 *@author Gabor Torok
 */

/// This huge class contains a level map.

class Map {
private:
	friend class MapRender;
	friend class Indoor;
	friend class Outdoor;
	MapRender *indoor, *outdoor;
	MapAdapter *adapter;
	Preferences *preferences;
	Shapes *shapes;
	static MapMemoryManager mapMemoryManager;
	bool mapChanged;
	bool resortShapes;
	float zoom;
	bool zoomIn, zoomOut;
	Sint16 x;
	Sint16 y;
	int viewX, viewY, viewWidth, viewHeight;
	float mapx, mapy;
	Location *pos[MAP_WIDTH][MAP_DEPTH][MAP_VIEW_HEIGHT];
	Location *itemPos[MAP_WIDTH][MAP_DEPTH];
	EffectLocation *effect[MAP_WIDTH][MAP_DEPTH][MAP_VIEW_HEIGHT];
	Location *posCache[MAX_POS_CACHE];
	signed int nbPosCache;
	Rug rugPos[MAP_CHUNKS_X][MAP_CHUNKS_Y];
	Shape *floorPositions[MAP_WIDTH][MAP_DEPTH];

	std::map<Uint32, WaterTile*> water;
	bool debugGridFlag;
	bool drawGridFlag;
	float xrot, yrot, zrot;
	float xpos, ypos, zpos;
	float xRotating, yRotating, zRotating;
	float fShadowMatrix[16];
	//bool alwaysCenter;
	static const int X_CENTER_TOLERANCE = 8;
	static const int Y_CENTER_TOLERANCE = 8;

	bool lightMapChanged;
	int lightMap[MAP_CHUNKS_X][MAP_CHUNKS_Y];
	bool groundVisible;

	// FIXME: either make this value adjustable or find a faster way to blast it onscreen?
	static const int SHADE_SIZE = 20;

	Location *selectedDropTarget;

	int accessMap[MAP_CHUNKS_X][MAP_CHUNKS_Y];
	std::map<Uint32, bool> locked;
	std::map<Uint32, Uint32> doorToKey;
	std::map<Uint32, Uint32> keyToDoor;

	int LIGHTMAP_ENABLED;
	int lastOutlinedX, lastOutlinedY, lastOutlinedZ;

	MapRenderHelper *helper;

	static int dir_index[];

	int mapViewWidth, mapViewDepth;
	enum { DEBUG_SIZE = 200 };
	char mapDebugStr[ DEBUG_SIZE ];

	char name[80];
	bool edited; // is this a non-random (edited) map?
	bool hasWater;

	MapSettings *settings;

	bool mouseMoveScreen;
	Uint16 move;
	bool mouseRot, mouseZoom;
	int mouseRotDir;

	Uint16 cursorMapX, cursorMapY, cursorMapZ, cursorFlatMapX, cursorFlatMapY;
	Uint16 cursorChunkX, cursorChunkY;

	int floorTexWidth, floorTexHeight;
	Texture floorTex;

	RenderedCreature *mapCenterCreature;
	std::map<int, bool> secretDoors;

	bool quakesEnabled;

	bool heightMapEnabled;
	OutdoorTexture outdoorTex[MAP_TILES_X][MAP_TILES_Y][MAX_OUTDOOR_LAYER];
	float ground[MAP_WIDTH][MAP_DEPTH];
	Texture groundTex[MAP_WIDTH][MAP_DEPTH];
	bool refreshGroundPos;
	int debugHeightPosXX[4], debugHeightPosYY[4];
	CVectorTex groundPos[MAP_WIDTH][MAP_DEPTH];
	Texture outdoorShadow;
	Texture outdoorShadowTree;
	Texture waterTexture;
	Location *hackBlockingPos;

	std::map<Uint32, Uint8> trapPos;
	std::vector<Trap> trapList;
	std::set<Uint8> trapSet;
	int selectedTrapIndex;
	bool isRoofShowing, isCurrentlyUnderRoof;
	Uint32 roofAlphaUpdate;
	float roofAlpha;
	bool gridEnabled;

	int weather;

	std::set<Location*> gates, teleporters;
	std::map<RenderedCreature*, RenderedLocation*> creatureMap, creatureEffectMap, creatureLightMap;
	std::vector<std::set<Location*> > houses;
	std::set<Location*> *currentHouse;
	
	// world region coordinates
	int regionX, regionY;
	

public:
	bool useFrustum;
	static bool debugMd2Shapes;

	int cursorWidth, cursorDepth, cursorHeight, cursorZ;
	int startx, starty;
	int mapGridX, mapGridY;

	Map( MapAdapter *adapter, Preferences *preferences, Shapes *shapes );
	~Map();
	
	inline int getRegionX() { return regionX; }
	inline void setRegionX( int n ) { this->regionX = n; }
	inline int getRegionY() { return regionY; }
	inline void setRegionY( int n ) { this->regionY = n; }
	
	
	bool isOnFloorTile( int px, int py );

	inline void setGridEnabled( bool b ) {
		this->gridEnabled = b;
	}
	inline bool isGridEnabled() {
		return this->gridEnabled;
	}

	/// Levels can have occasional quakes.
	inline void setQuakesEnabled( bool b ) {
		quakesEnabled = b;
	}
	inline bool areQuakesEnabled() {
		return quakesEnabled;
	}
	void quake();

	void setRoofShowing( bool b );
	/// Unused.
	inline bool getRoofShowing() {
		return isRoofShowing;
	}

	/// Are we under a roof (inside a house)?
	inline bool getCurrentlyUnderRoof() {
		return isCurrentlyUnderRoof;
	}
	
	/// Which weather will the map have?
	inline void setWeather( int i ) {
		weather = i;
	}
	/// The current weather conditions.
	inline int getWeather() {
		return weather;
	}
	int generateWeather();

	void addSecretDoor( int x, int y );
	void removeSecretDoor( int x, int y );
	bool isSecretDoor( Location *pos );
	bool isSecretDoor( int x, int y );
	bool isSecretDoorDetected( Location *pos );
	bool isSecretDoorDetected( int x, int y );
	void setSecretDoorDetected( Location *pos );
	void setSecretDoorDetected( int x, int y );

	/// The creature on which the main view should be centered.
	inline void setMapCenterCreature( RenderedCreature *c ) {
		mapCenterCreature = c;
	}
	/// The creature on which the main view should be centered.
	inline RenderedCreature *getMapCenterCreature() {
		return mapCenterCreature;
	}

	void setMapRenderHelper( MapRenderHelper *helper );
	inline MapRenderHelper *getMapRenderHelper() {
		return helper;
	}

	inline MapAdapter *getAdapter() {
		return adapter;
	}

	/// Gets the game's user-set preferences.
	inline Preferences *getPreferences() {
		return preferences;
	}

	inline MapRenderHelper *getHelper() {
		return helper;
	}

	inline Shapes *getShapes() {
		return shapes;
	}

	inline MapSettings *getSettings() {
		return settings;
	}

	/// Returns the frustum (the viewing area).
	inline CFrustum *getFrustum() {
		return frustum;
	}

	/// Which floor texture will we use (indoors)?
	inline void setFloor( int tw, int th, Texture texture ) {
		floorTexWidth = tw; floorTexHeight = th; floorTex = texture;
	}

	/// Is this an edited (instead of randomly generated) map?
	inline bool isEdited() {
		return edited;
	}
	inline char *getName() {
		return name;
	}


	void saveMap( const std::string& name, std::string& result, bool absolutePath = false, int referenceType = REF_TYPE_NAME );
	bool loadMap( const std::string& name, std::string& result, StatusReport *report = NULL,
	              int level = 1, int depth = 0,
	              bool changingStory = false, bool fromRandom = false,
	              bool goingUp = false, bool goingDown = false,
	              std::vector< RenderedItem* > *items = NULL,
	              std::vector< RenderedCreature* > *creatures = NULL,
	              bool absolutePath = false,
	              char *templateMapName = NULL );
	void loadMapLocation( const std::string& name, std::string& result, int *gridX, int *gridY, int depth = 0 );
	void initForCave( char *themeName = NULL );

	/// Returns the mouse cursor's map position.
	inline Uint16 getCursorMapX() {
		return cursorMapX;
	}
	inline Uint16 getCursorMapY() {
		return cursorMapY;
	}
	inline Uint16 getCursorMapZ() {
		return cursorMapZ;
	}

	/// Returns the mouse cursor's map position (using flat map math).
	inline Uint16 getCursorFlatMapX() {
		return cursorFlatMapX;
	}
	inline Uint16 getCursorFlatMapY() {
		return cursorFlatMapY;
	}

	inline Uint16 getCursorChunkX() {
		return cursorChunkX;
	}
	inline Uint16 getCursorChunkY() {
		return cursorChunkY;
	}

	void handleEvent( SDL_Event *event );

	void reset();

	/// Stops moving the view.
	inline void resetMove() {
		move = 0;
	}

	/// Do we currently rotate the view?
	inline bool isMouseRotating() {
		return mouseRot;
	}
	/// Do we currently zoom the view?
	inline bool isMouseZooming() {
		return mouseZoom;
	}
	/// Do we currently scroll the view?
	inline bool isMapMoving() {
		return move != 0;
	}

	inline void setMapSettings( MapSettings *settings ) {
		this->settings = settings;
	}
	inline MapSettings *getMapSettings() {
		return settings;
	}

	/// Is this a flooded level (indoors only)?
	inline bool getHasWater() {
		return hasWater;
	}
	/// Make this a "flooded" level (indoors).
	inline void setHasWater( bool b ) {
		hasWater = b;
	}

	int debugX, debugY, debugZ;

	int toggleLightMap();

	bool floorOnly;

	/// A location where an item is to be dropped by the player.
	inline void setSelectedDropTarget( Location *loc ) {
		selectedDropTarget = loc;
	}
	/// A location where an item is to be dropped by the player.
	inline Location *getSelectedDropTarget() {
		return selectedDropTarget;
	}

	inline void setZoomIn( bool b ) {
		zoomIn = b;
	}
	inline void setZoomOut( bool b ) {
		zoomOut = b;
	}
	/// Current camera zoom.
	inline float getZoom() {
		return zoom;
	}
	float getZoomPercent();

	/// Camera rotation.
	inline void setXRot( float b ) {
		xRotating = b;
	}
	inline void setYRot( float b ) {
		yRotating = b;
	}
	inline void setZRot( float b ) {
		zRotating = b;
	}

	inline float getXRot() {
		return xrot;
	}
	inline float getYRot() {
		return yrot;
	}
	inline float getZRot() {
		return zrot;
	}

	/// Move the camera one tile east.
	inline void addXPos( float f ) {
		xpos += f;
	}
	/// Move the camera one tile south.
	inline void addYPos( float f ) {
		ypos += f;
	}
	/// Make the camera point at a higher z pos.
	inline void addZPos( float f ) {
		zpos += f;
	}

	/// X position of tile the view is centered on.
	inline float getXPos() {
		return xpos;
	}
	/// Y position of tile the view is centered on.
	inline float getYPos() {
		return ypos;
	}
	/// Z position of tile the view is centered on.
	inline float getZPos() {
		return zpos;
	}

	// camera control
	void setPos( float x, float y, float z );
	/// Sets the camera rotation.
	inline void setRot( float x, float y, float z ) {
		xrot = x; yrot = y; zrot = z;
	}
	inline void setZoom( float zoom ) {
		this->zoom = zoom;
	}

	bool isLocationVisible( int x, int y );

	bool isLocationInLight( int x, int y, Shape *shape );

	void draw();
	void preDraw();
	void postDraw();

	/// The map top left x coordinate
	inline int getX() {
		return x;
	}

	/// The map top left y coordinate
	inline int getY() {
		return y;
	}

	inline float getMapX() {
		return mapx;
	}
	inline float getMapY() {
		return mapy;
	}

	inline void setXY( int x, int y ) {
		this->x = x; this->y = y;
	}

	void center( Sint16 x, Sint16 y, bool force = false );

	void stopEffect( Sint16 x, Sint16 y, Sint16 z );
	void startEffect( Sint16 x, Sint16 y, Sint16 z,
	                  int effect_type, GLuint duration = Constants::DAMAGE_DURATION,
	                  int width = 1, int height = 1,
	                  GLuint delay = 0, bool forever = false,
	                  DisplayInfo *di = NULL );

	void setPosition( Sint16 x, Sint16 y, Sint16 z, Shape *shape, DisplayInfo *di = NULL );
	Shape *removePosition( Sint16 x, Sint16 y, Sint16 z );
	Shape *removeLocation( Sint16 x, Sint16 y, Sint16 z );

	bool hasOutdoorTexture( int x, int y, int width, int height );

	void setOutdoorTexture( int x, int y, float offsetX, float offsetY,
	                        int ref, float angle, bool horizFlip,
	                        bool vertFlip, int z );
	void removeOutdoorTexture( int x, int y, float width, float height, int z );

	Shape *removeItemPosition( Sint16 x, Sint16 y );

	void setItem( Sint16 x, Sint16 y, Sint16 z, RenderedItem *item );
	RenderedItem *removeItem( Sint16 x, Sint16 y, Sint16 z );

	void setCreature( Sint16 x, Sint16 y, Sint16 z, RenderedCreature *creature );
	RenderedCreature *removeCreature( Sint16 x, Sint16 y, Sint16 z );

	Location *moveCreature( Sint16 x, Sint16 y, Sint16 z, Uint16 dir,
	                        RenderedCreature *newCreature );
	Location *moveCreature( Sint16 x, Sint16 y, Sint16 z,
	                        Sint16 nx, Sint16 ny, Sint16 nz,
	                        RenderedCreature *newCreature );

	void setFloorPosition( Sint16 x, Sint16 y, Shape *shape );
	Shape *removeFloorPosition( Sint16 x, Sint16 y );

	void setRugPosition( Sint16 x, Sint16 y, Rug *rug );
	void removeRugPosition( Sint16 x, Sint16 y );
	inline bool hasRugAtPosition( Sint16 x, Sint16 y ) {
		return rugPos[x][y].texture.isSpecified();
	}

	Location *isBlocked( Sint16 x, Sint16 y, Sint16 z,
	                     Sint16 shapex, Sint16 shapey, Sint16 shapez,
	                     Shape *shape,
	                     int *newz = NULL,
	                     bool useItemPos = false );

	Location *getPosition( Sint16 x, Sint16 y, Sint16 z );
	/// Returns a position struct for the specified map tile.
	inline Location *getLocation( Sint16 x, Sint16 y, Sint16 z ) {
		return pos[x][y][z];
	}
	/// Returns an item position struct for the x,y map tile.
	inline Location *getItemLocation( Sint16 x, Sint16 y ) {
		return itemPos[x][y];
	}
	/// Gets the shape that is on the floor at x,y.
	inline Shape *getFloorPosition( Sint16 x, Sint16 y ) {
		if ( x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_DEPTH ) return NULL; else return floorPositions[x][y];
	}

	//void showInfoAtMapPos(Uint16 mapx, Uint16 mapy, Uint16 mapz, char *message);
	/// Unused.
	void showCreatureInfo( RenderedCreature *creature, bool player, bool selected, bool groupMode );

	void initMapView( bool ignoreRot = false );

	void moveMap( int dir );
	
	bool isWallBetweenLocations( Location *pos1, Location *pos2 );
	
	bool isWallBetweenShapes( int x1, int y1, int z1,
	                          Shape *shape1,
	                          int x2, int y2, int z2,
	                          Shape *shape2 );

	Shape *isWallBetween( int x1, int y1, int z1,
	                      int x2, int y2, int z2,
	                      Shape *ignoreShape = NULL );

	bool shapeFits( Shape *shape, int x, int y, int z );

	bool shapeFitsOutdoors( GLShape *shape, int x, int y, int z );

	Location *getBlockingLocation( Shape *shape, int x, int y, int z );

	Location *getDropLocation( Shape *shape, int x, int y, int z );

	/// Forces an update of the light map (indoor visibility map).
	inline void updateLightMap() {
		lightMapChanged = resortShapes = true;
	}

	void refresh();

	/// Force to resort the shapes so transparency will work.
	inline void refreshTransparency() {
		resortShapes = true;
	}

	void setViewArea( int x, int y, int w, int h );
	/// Width of the 3D view (usually equal to screen width)
	inline int getViewWidth() {
		return viewWidth;
	}
	/// Height of the 3D view (usually equal to screen height)
	inline int getViewHeight() {
		return viewHeight;
	}

	void dropItemsAbove( int x, int y, int z, RenderedItem *item );

	int getCreaturesInArea( int x, int y, int radius, RenderedCreature *targets[] );

	bool isOnScreen( Uint16 mapx, Uint16 mapy, Uint16 mapz );

	bool isDoor( int x, int y );
	bool isDoor( Shape *shape );
	bool isDoorType( Shape *shape, bool includeCorner = true );

	// ====================================================================
	// Locked doors/chests code
	void setLocked( int doorX, int doorY, int doorZ, bool value );
	/// Unlocks the door at x,y,z.
	inline void removeLocked( int doorX, int doorY, int doorZ ) {
		Uint32 door = createTripletKey( doorX, doorY, doorZ );
		locked.erase( door );
		if ( doorToKey.find( door ) != doorToKey.end() ) {
			Uint32 key = doorToKey[door];
			doorToKey.erase( key );
			keyToDoor.erase( door );
		}
	}

	/// Clear the locked objects (doors and levers).
	inline void clearLocked() {
		if ( !locked.empty() ) locked.clear();
		if ( !doorToKey.empty() ) doorToKey.clear();
		if ( !keyToDoor.empty() ) keyToDoor.clear();
	}

	/// Is the door at x,y,z locked?
	inline bool isLocked( int doorX, int doorY, int doorZ ) {
		Uint32 door = createTripletKey( doorX, doorY, doorZ );
		return ( locked.find( door ) != locked.end() ? locked[door] : false );
	}

	/// Gets the position of the door that is connected to the lever at x,y,z.
	inline void getDoorLocation( int keyX, int keyY, int keyZ,
	                             int *doorX, int *doorY, int *doorZ ) {
		Uint32 key = createTripletKey( keyX, keyY, keyZ );
		if ( keyToDoor.find( key ) != keyToDoor.end() ) {
			decodeTripletKey( keyToDoor[key], doorX, doorY, doorZ );
		} else {
			*doorX = *doorY = *doorZ = -1;
		}
	}

	/// Gets the position of the lever that is connected to the door at x,y,z.
	inline void getKeyLocation( int doorX, int doorY, int doorZ,
	                            int *keyX, int *keyY, int *keyZ ) {
		Uint32 key = createTripletKey( doorX, doorY, doorZ );
		if ( doorToKey.find( key ) != doorToKey.end() ) {
			decodeTripletKey( doorToKey[ key ], keyX, keyY, keyZ );
		} else {
			*keyX = *keyY = *keyZ = -1;
		}
	}

	/// Moves a door and tries to keep it connected to its unlock lever.
	inline void updateDoorLocation( int oldDoorX, int oldDoorY, int oldDoorZ,
	                                int newDoorX, int newDoorY, int newDoorZ ) {
		//cerr << "**********************" << endl;
		Uint32 oldDoor = createTripletKey( oldDoorX, oldDoorY, oldDoorZ );
		Uint32 newDoor = createTripletKey( newDoorX, newDoorY, newDoorZ );
		//cerr << "oldDoor=" << oldDoor << " pos: " << oldDoorX << "," << oldDoorY << "," << oldDoorZ << endl;
		//cerr << "newDoor=" << newDoor << " pos: " << newDoorX << "," << newDoorY << "," << newDoorZ << endl;
		if ( locked.find( oldDoor ) != locked.end() ) {
			//cerr << "\tfound locked." << endl;
			locked[newDoor] = locked[oldDoor];
			locked.erase( oldDoor );
		}
		if ( doorToKey.find( oldDoor ) != doorToKey.end() ) {
			//cerr << "\tfound door<->key." << endl;
			Uint32 key = doorToKey[oldDoor];
			int keyX, keyY, keyZ;
			decodeTripletKey( key, &keyX, &keyY, &keyZ );
			//cerr << "\tkey=" << key << " pos: " << keyX << "," << keyY << "," << keyZ << endl;
			doorToKey[newDoor] = key;
			doorToKey.erase( oldDoor );
			keyToDoor[key] = newDoor;
		}
		//cerr << "**********************" << endl;
	}

	/// Creates a door/unlock lever pair.
	void setKeyLocation( int doorX, int doorY, int doorZ,
	                     int keyX, int keyY, int keyZ ) {
		Uint32 door = createTripletKey( doorX, doorY, doorZ );
		Uint32 key = createTripletKey( keyX, keyY, keyZ );
		doorToKey[door] = key;
		keyToDoor[key] = door;
	}
	// ====================================================================

	// access map methods for locked doors/chests
	void configureAccessMap( int fromX, int fromY );
	bool isPositionAccessible( int atX, int atY );

	void getMapXYAtScreenXY( Uint16 x, Uint16 y, Uint16 *mapx, Uint16 *mapy );
	void getScreenXYAtMapXY( Uint16 mapx, Uint16 mapy, Uint16 *screenx, Uint16 *screeny );
	int getPanningFromMapXY( Uint16 mapx, Uint16 mapy );

	/// Sets the ground height at x,y (outdoors).
	inline void setGroundHeight( int x, int y, float h ) {
		this->ground[x][y] = h; refreshGroundPos = true;
	}
	/// Gets the ground height at x,y.
	inline float getGroundHeight( int x, int y ) {
		return this->ground[x][y];
	}
	/// Does this map have an uneven floor (e.g. is it outdoors)?
	inline void setHeightMapEnabled( bool b ) {
		this->heightMapEnabled = b;
	}
	/// Does this map have an uneven floor (e.g is it outdoors)?
	inline bool isHeightMapEnabled() {
		return this->heightMapEnabled;
	}
	/// Do we need to setup the ground textures?
	inline void setRefreshGroundPos( bool b ) {
		this->refreshGroundPos = b;
	}
	/// Do we need to setup the ground textures?
	inline bool isRefreshGroundPos() {
		return this->refreshGroundPos;
	}
	/// Sets the ground texture at x,y (outdoors).
	inline void setGroundTex( int x, int y, Texture tex ) {
		this->groundTex[x][y] = tex;
	}
	/// Returns the ground texture of tile x,y.
	inline Texture getGroundTex( int x, int y ) {
		return this->groundTex[x][y];
	}

	float findMaxHeightPos( float x, float y, float z, bool findMax = false );


	// Traps
	int addTrap( int x, int y, int w, int h );
	void removeTrap( int trap );
	int getTrapAtLoc( int x, int y );
	Trap *getTrapLoc( int trap );
	/// Returns an array of trap indices.
	inline std::set<Uint8> *getTrapsShown() {
		return &trapSet;
	}
	/// The currently selected trap (mouse cursor is over it).
	inline int getSelectedTrapIndex() {
		return selectedTrapIndex;
	}

	bool canFit( int x, int y, Shape *shape );
	bool isEmpty( int x, int y );

	bool inMapEditor();
	bool coversDoor( Shape *shape, int x, int y );

	inline std::set<Location*> *getGates() {
		return &gates;
	}
	inline std::set<Location*> *getTeleporters() {
		return &teleporters;
	}
	
	bool isFacingLight( Surface *surface, Location *p, Location *lightPos );
	bool isValidPosition( int x, int y, int z );
	inline MapRender *getRender() { return helper->isIndoors() ? indoor : outdoor; }
	void flattenChunk( Sint16 mapX, Sint16 mapY, float height = 0 );
	void startHouse();
	void endHouse();
	void clearHouses();

protected:
	bool checkLightMap( int chunkX, int chunkY );
	void clearTraps();

	void setPositionInner( Sint16 x, Sint16 y, Sint16 z,
	                       Shape *shape,
	                       RenderedItem *item,
	                       RenderedCreature *creature );

	/// Set which direction to move the map in.

	/// @param n is a bitfield.
	/// See constants for direction values.
	inline void setMove( Uint16 n ) {
		move |= n;
	};

	/// Stop moving the map in the given direction(s).

	/// @param n is a bitfield.
	/// See constants for directions values.
	inline void removeMove( Uint16 n ) {
		move &= ( 0xffff - n );
	}

	/// Creates a unique key from a x,y,z map position.

	/// This assumes that MAP_WIDTH >= MAP_HEIGHT
	/// and that MAP_WIDTH^3 < 2^32.
	inline Uint32 createTripletKey( int x, int y, int z ) {
		Uint32 key =
		  ( ( Uint32 )x * ( Uint32 )MAP_WIDTH * ( Uint32 )MAP_WIDTH ) +
		  ( ( Uint32 )y * ( Uint32 )MAP_WIDTH ) +
		  ( ( Uint32 )z );
		//cerr << "DEBUG: createTripletKey, x=" << x << " y=" << y << " z=" << z << " key=" << key << endl;
		return key;
	}

	/// Decodes a triplet key to a x,y,z map position.
	inline void decodeTripletKey( Uint32 key, int *x, int *y, int *z ) {
		*x = static_cast<int>( key / ( ( Uint32 )MAP_WIDTH * ( Uint32 )MAP_WIDTH ) );
		*y = static_cast<int>( ( key % ( ( Uint32 )MAP_WIDTH * ( Uint32 )MAP_WIDTH ) ) / ( Uint32 )MAP_WIDTH );
		*z = static_cast<int>( ( key % ( ( Uint32 )MAP_WIDTH * ( Uint32 )MAP_WIDTH ) ) % ( Uint32 )MAP_WIDTH );
		//cerr << "DEBUG: decodeTripletKey, key=" << key << " x=" << (*x) << " y=" << (*y) << " z=" << (*z) << endl;
	}

	/// Creates a unique key from a map position.
	inline Uint32 createPairKey( int x, int y ) {
		Uint32 key =
		  ( ( Uint32 )x * ( Uint32 )MAP_WIDTH ) +
		  ( ( Uint32 )y );
		return key;
	}

	/// Decodes a pair key to a map position.
	inline void decodePairKey( Uint32 key, int *x, int *y ) {
		*x = static_cast<int>( key / ( ( Uint32 )MAP_WIDTH ) );
		*y = static_cast<int>( key % ( ( Uint32 )MAP_WIDTH ) );
	}

	CFrustum *frustum;

	/// A chunk (of several tiles).
	struct ChunkInfo {
		float x, y;
		int cx, cy;
	};

	ChunkInfo chunks[1000];
	int chunkCount;
	RenderedLocation later[100], stencil[1000], other[1000], damage[1000], roof[1000], lights[50];
	int laterCount, stencilCount, otherCount, damageCount, roofCount, lightCount;
	std::map<Uint32, EffectLocation*> currentEffectsMap;
	std::map<Location*, RenderedLocation*> renderedLocations;
	
	inline RenderedLocation *getRenderedLocation( Location *pos ) {
		if( renderedLocations.find( pos ) == renderedLocations.end() ) {
			return NULL;
		} else {
			return renderedLocations[ pos ]; 
		}
	}

	void setupShapes( bool forGround, bool forWater, int *csx = NULL, int *cex = NULL, int *csy = NULL, int *cey = NULL );
	void setupPosition( int posX, int posY, int posZ,
	                    float xpos2, float ypos2, float zpos2,
	                    Location *pos,
	                    EffectLocation *effect );
	void setupLocation( Location *location, Uint16 drawSide, int chunkStartX, int chunkStartY, int chunkOffsetX, int chunkOffsetY );
	Shape *isWall( int x, int y, int z );

	void configureLightMap();
	void traceLight( int chunkX, int chunkY, int lightMap[MAP_CHUNKS_X][MAP_CHUNKS_Y], bool onlyLockedDoors );
	bool isLocationBlocked( int x, int y, int z, bool onlyLockedDoors );

	void removeEffect( Sint16 x, Sint16 y, Sint16 z );
	void removeAllEffects();

	void moveCreaturePos( Sint16 nx, Sint16 ny, Sint16 nz,
	                      Sint16 ox, Sint16 oy, Sint16 oz,
	                      RenderedCreature *creature );

	void calculateChunkInfo( int *chunkOffsetX, int *chunkOffsetY,
	                         int *chunkStartX, int *chunkStartY,
	                         int *chunkEndX, int *chunkEndY );
	void getChunk( int mapX, int mapY, int *chunkX, int *chunkY );
	void calculateLocationInfo( Location *location,
	                            int chunkStartX, int chunkStartY,
	                            int chunkOffsetX, int chunkOffsetY,
	                            Uint16 drawSide,
	                            int *posX, int *posY, int *posZ,
	                            float *xpos, float *ypos, float *zpos,
	                            int *chunkX, int *chunkY,
	                            bool *lightEdge );
	bool checkUnderRoof();

	void removeCurrentEffects();

	void getMapXYAtScreenXY( Uint16 *mapx, Uint16 *mapy );
	void getMapXYZAtScreenXY( Uint16 *mapx, Uint16 *mapy, Uint16 *mapz, Location **pos );
		
	inline float distance( Location *pos1, Location *pos2 ) {
		return Constants::distance( pos1->x, pos1->y - 1 - pos1->shape->getDepth(), pos1->shape->getWidth(), pos1->shape->getDepth(), 
		                            pos2->x, pos2->y - 1 - pos2->shape->getDepth(), pos2->shape->getWidth(), pos2->shape->getDepth() );
	}
	
	void doQuake();
	
	DECLARE_NOISY_OPENGL_SUPPORT();
};

#endif
