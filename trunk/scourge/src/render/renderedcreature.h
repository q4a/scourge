/***************************************************************************
            renderedcreature.h  -  Map related creature functions
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

#ifndef RENDERED_CREATURE_H
#define RENDERED_CREATURE_H
#pragma once

#include "render.h"
#include <vector>
#include <string>
#include <set>

class RenderedItem;
class Effect;
class GLShape;
class Shapes;
class Map;

/**
 * @author Gabor Torok
 *
 * A creature rendered on the map.
 */

/// A map position where a damage counter is displayed.
struct DamagePos {
	int damage;
	float pos;
	Uint32 lastTime;
};

#define MAX_RECENT_DAMAGE 100

/// A creature rendered on the map.
class RenderedCreature {
protected:
	GLfloat x, y, z;
	GLuint effectDuration;
	GLuint damageEffectCounter;
	Effect *effect;
	int effectType;
	Preferences *preferences;
	Shapes *shapes;
	Map *levelMap;
	DamagePos recentDamages[MAX_RECENT_DAMAGE];
	int recentDamagesCount;

	char speech[2000];
	std::vector<std::string> speechWrapped;
	Uint32 talkStartTime;
	float offsX, offsY, offsZ;

public:
	RenderedCreature( Preferences *preferences,
	                  Shapes *shapes,
	                  Map *levelMap );
	virtual ~RenderedCreature();

	inline void setOffset( float x, float y, float z ) {
		this->offsX = x; this->offsY = y; this->offsZ = z;
	}
	inline float getOffsetX() {
		return offsX;
	}
	inline float getOffsetY() {
		return offsY;
	}
	inline float getOffsetZ() {
		return offsZ;
	}

	virtual inline GLfloat getX() {
		return x;
	}
	virtual inline GLfloat getY() {
		return y;
	}
	virtual inline GLfloat getZ() {
		return z;
	}
	virtual inline void moveTo( GLfloat x, GLfloat y, GLfloat z ) {
		this->x = x; this->y = y; this->z = z;
	}

	virtual inline int getRecentDamageCount() {
		return recentDamagesCount;
	}
	virtual inline DamagePos *getRecentDamage( int i ) {
		return &( recentDamages[i] );
	}
	virtual bool addRecentDamage( int damage );
	virtual void removeRecentDamage( int i );

	virtual void say( char const* text );
	virtual void clearSpeech();
	virtual bool isTalking();
	virtual char *getSpeech();
	std::vector<std::string> *getSpeechWrapped();

	virtual bool isBoss() = 0;
	virtual bool isNpc() = 0;
	virtual bool getStateMod( int mod ) = 0;
	virtual void pickUpOnMap( RenderedItem *item ) = 0;
	virtual GLShape *getShape() = 0;
	virtual char *getName() = 0;
	virtual bool isWanderingHero() = 0;
	virtual bool isMonster() = 0;
	virtual bool isHarmlessAnimal() = 0;
	virtual bool isPartyMember() = 0;
	virtual bool isPlayer() = 0;
	virtual char *getType() = 0;
	virtual CreatureInfo *save() = 0;
	virtual bool canAttack( RenderedCreature *creature, int *cursor = NULL ) = 0;
	virtual bool setSelXY( int x, int y, bool cancelIfNotPossible = true ) = 0;
	virtual void setMapChanged() = 0;

	// effects
	virtual Effect *getEffect();
	virtual void startEffect( int effect_type, int duration = Constants::DAMAGE_DURATION, GLuint delay = 0 );
	virtual inline void setEffectType( int n ) {
		this->effectType = n;
	}
	virtual inline int getEffectType() {
		return effectType;
	}
	virtual inline int getDamageEffect() {
		return damageEffectCounter;
	}
	virtual inline void resetDamageEffect() {
		damageEffectCounter = SDL_GetTicks();
	}
	virtual inline bool isEffectOn() {
		return ( SDL_GetTicks() - damageEffectCounter < effectDuration ? true : false );
	}

	virtual void findPlace( int x, int y, int *finalX = NULL, int *finalY = NULL );
	virtual void findPlace_old( int x, int y, int *finalX = NULL, int *finalY = NULL );


protected:
	bool doFindStart( int *startx, int *starty );
	bool isEmptyUnSeenPlace( int x, int y, std::set<int> *seen );
	bool doFindPlace( int startx, int starty, int *finalX, int *finalY, std::set<int> *seen );
	bool canReach( int startx, int starty, int firstx, int firsty, int xx, int yy, std::map<int, bool> *seen );

};


#endif

