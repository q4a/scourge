/***************************************************************************
                          creature.h  -  description
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

#ifndef CREATURE_H
#define CREATURE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
//#include <conio.h>   // for getch
#include <vector>    // STL for Vector
#include <algorithm> // STL for Heap
#include "glshape.h"
#include "md2shape.h"
#include "map.h"
#include "scourge.h"
#include "util.h"
#include "character.h"
#include "constants.h"

using namespace std;

// if this is defined, party members will exchange places with each other
// can't get stuck, but can move-off screen.
// fix this to keep player centered
//#define ENABLE_PARTY_SWAP 1

#define MAX_CLOSED_NODES 50

class Map;
class Scourge;

/**
  *@author Gabor Torok
  */

class Creature {

public:
    enum {
        SWORD_WEAPON = 0,
        AXE_WEAPON,
        BOW_WEAPON,
        SHIELD_DEFEND,
        ARMOR_DEFEND,
        WEAPON_DEFEND,
        MATERIAL_SPELL,
        ILLUSION_SPELL,
        PSYCHIC_SPELL,
        OPEN_LOCK,
        FIND_TRAP,
        MOVE_UNDETECTED,
        SKILL_0, SKILL_1, SKILL_2, SKILL_3, SKILL_4, SKILL_5, SKILL_6, SKILL_7, SKILL_8, SKILL_9,

        SKILL_COUNT
    };
    static const char *SKILL_NAMES[];

private:
    Sint16 x, y, z;
	Creature *next;
	GLShape *shape;
	//Uint8 shapePosition;
	Uint16 dir;
	Scourge *scourge;
    Character *character;

    int motion;

	int formation;
	int index;

    int tx, ty;
    int selX, selY;
    int bestPathPos;
    vector<Location> bestPath;

    // character stats
    char *name;
    int portraitIndex;
    int level;
    GLuint exp;
    int attrib[7];
    int hp;
    GLuint stateMod;
    int skills[SKILL_COUNT];
  
public:
    enum { strength, dexterity, health, intel, willpower, charisma, luck };
    enum { blessed, empowered, enraged, ac_protected, magic_protected, drunk, poisoned, cursed, possessed, blinded, charmed, changed };
    static const char ATTR_NAMES[][80];         
    static const char STATE_NAMES[][80];
    static const char STATE_SHORT_NAMES[][10];

    static const int ATTR_COUNT = 7;
    static const int STATE_MOD_COUNT = 12;  

	static const int DIAMOND_FORMATION = 0;
    static const int STAGGERED_FORMATION = 1;
	static const int SQUARE_FORMATION = 2;
	static const int ROW_FORMATION = 3;
	static const int SCOUT_FORMATION = 4;
	static const int CROSS_FORMATION = 5;
	static const int FORMATION_COUNT = 6;
    
	Creature(Scourge *scourge, GLShape *shape);
	~Creature();

    inline void setMotion(int motion) { this->motion = motion; }
    inline int getMotion() { return this->motion; }
										
	//inline void setShapeIndex(Uint8 shapePosition) { this->shapePosition = shapePosition; }
	//inline Uint8 getShapeIndex() { return shapePosition; }
	bool move(Uint16 dir, Map *map);
	void follow(Map *map);
    void gotoPosition(Map *map, Sint16 px, Sint16 py, Sint16 pz);
    void moveToLocator(Map *map);
  
	inline void moveTo(Sint16 x, Sint16 y, Sint16 z) { this->x = x; this->y = y; this->z = z; }
	inline Sint16 getX() { return x; }
	inline Sint16 getY() { return y; }
	inline Sint16 getZ() { return z; }
	inline GLShape *getShape() { return shape; }
	inline void setFormation(int formation) { this->formation = formation; }
	inline int getFormation() { return formation; }
	void setNext(Creature *next, int index);
    inline Uint16 getDir() { return dir; }


	inline void draw() { getShape()->draw(); }  

	/**
	 * Get the position of this creature in the formation.
	 */
	void getFormationPosition(Sint16 *px, Sint16 *py, Sint16 *pz);

    /**
        Used to move away from the player. Find the nearest corner of the map.
    */
    void findCorner(Sint16 *px, Sint16 *py, Sint16 *pz);


  
    inline void setName(char *s) { name = strdup(s); }
    inline char *getName() { return name; };
    inline void setPortraitIndex(int n) { portraitIndex = n; }
    inline int getPortraitIndex() { return portraitIndex; };    
    inline void setCharacter(Character *c) { character = c; }
    inline Character *getCharacter() { return character; }
    inline void setLevel(int n) { level = n; }
    inline int getLevel() { return level; };
    inline void setExp(GLuint n) { exp = n; }
    inline GLuint getExp() { return exp; };
    void rollAttributes();
    inline int getAttr(int index) { return attrib[index]; }
    inline void setHp() { hp = getCharacter()->getStartingHp(); }
    inline int getHp() { return hp; }  
    inline void setStateMod(int mod, bool setting) { if(setting) stateMod |= (1 << mod); else stateMod &= ((GLuint)0xffff - (GLuint)(1 << mod)); }
    inline bool getStateMod(int mod) { return (stateMod & (1 << mod) ? true : false); }
    inline void setSkill(int skillIndex, int value) { skills[skillIndex] = value; }
    inline int getSkill(int skillIndex) { return skills[skillIndex]; }
    inline void setSelXY(int x, int y) { selX = x; selY = y; }

protected:
  

};


#endif

