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
#include "rpg/character.h"
#include "rpg/monster.h"
#include "constants.h"
#include "effect.h"

using namespace std;

#define MAX_CLOSED_NODES 50

class Map;
class Scourge;
class Effect;
class Item;

/**
  *@author Gabor Torok

  This class is both the UI representation (shape) and state (inventory, etc.) of a character or monster.
  All creatures of the same type (character or monster) share the same instance of the prototype class (Character of Monster)

  */

#define MAX_INVENTORY_SIZE 200

// how many times to attempt to move to range
#define MAX_FAILED_MOVE_ATTEMPTS 10

class Creature {
  
 private:
  // gui information
  Sint16 x, y, z;
  Creature *next;
  GLShape *shape;
  char *model_name, *skin_name;
  Uint16 dir;
  Scourge *scourge;
  GLUquadric *quadric;
  int motion;
  float minRange, maxRange;
  int failedToMoveWithinRangeAttemptCount;
  int facingDirection;
  int formation;
  int index;
  int tx, ty;
  int selX, selY;
  int bestPathPos;
  vector<Location> bestPath;
  Creature *targetCreature;
  Sint16 cornerX, cornerY;
  bool arrived; // true if no particular destination set for this creature
  
  // inventory
  Item *inventory[MAX_INVENTORY_SIZE];
  int inventory_count;
  int equipped[Character::INVENTORY_COUNT];

  // character information
  char *name;
  int level, exp, hp, startingHp, ac, thirst, hunger, money, expOfNextLevel;
  Character *character;
  int skills[Constants::SKILL_COUNT], skillMod[Constants::SKILL_COUNT];
  GLuint stateMod;
  Monster *monster;

  char description[300];
  GLint lastTick;
  int speed;
  int armor;
  int moveRetrycount;
  int availableSkillPoints;
  
  static const int MAX_MOVE_RETRY = 15;
  int lastTurn;

  int effectDuration;
  int damageEffectCounter;
  Effect *effect;
  int effectType;
  
 public:
  static const int DIAMOND_FORMATION = 0;
  static const int STAGGERED_FORMATION = 1;
  static const int SQUARE_FORMATION = 2;
  static const int ROW_FORMATION = 3;
  static const int SCOUT_FORMATION = 4;
  static const int CROSS_FORMATION = 5;
  static const int FORMATION_COUNT = 6;
  
  Creature(Scourge *scourge, Character *character, char *name);
  Creature(Scourge *scourge, Monster *monster);
  ~Creature();

  inline void setLastTurn(int n) { lastTurn = n; }
  inline int getLastTurn() { return lastTurn; }

  inline bool isMonster() { return (monster ? TRUE : FALSE); }
  
  inline GLUquadric *getQuadric() { return quadric; }

  void setTargetCreature(Creature *c);
  inline Creature *getTargetCreature() { return targetCreature; }
  
  inline void setMotion(int motion) { this->motion = motion; }  
  inline int getMotion() { return this->motion; }

  inline void setDistanceRange(float min, float max) { minRange = min; maxRange = max; }  

  /**
	 Return true only if a range is specified and we're within it.
  */
  bool Creature::isInRange();
    
  inline void setFacingDirection(int direction) { this->facingDirection = direction;}
  inline int getFacingDirection() { return this->facingDirection; }
  
  
  inline char *getDescription() { return description; }

  inline void setLastTick(GLint n) { this->lastTick = n; }
  inline GLint getLastTick() { return lastTick; }

  // FIXME: should be modified by inventory (boots of speed, etc.)
  inline int getSpeed() { return speed; }
  
  /**
	 The movement functions return true if movement has occured, false if it has not.
   */
  bool move(Uint16 dir, Map *map);
  bool follow(Map *map);
  bool moveToLocator(Map *map);
  void stopMoving();

  inline float getDistanceToTargetCreature() {
	if(!getTargetCreature()) return 0.0f;
	return Constants::distance(getX(),  getY(), 
							   getShape()->getWidth(), getShape()->getDepth(),
							   getTargetCreature()->getX(), getTargetCreature()->getY(),
							   getTargetCreature()->getShape()->getWidth(), 
							   getTargetCreature()->getShape()->getDepth());
  }
  
  inline void moveTo(Sint16 x, Sint16 y, Sint16 z) { this->x = x; this->y = y; this->z = z; }
  inline Sint16 getX() { return x; }
  inline Sint16 getY() { return y; }
  inline Sint16 getZ() { return z; }
  inline char *getModelName() { return model_name; }
  inline char *getSkinName() { return skin_name; }
  inline GLShape *getShape() { return shape; }
  inline void setFormation(int formation) { this->formation = formation; }
  inline int getFormation() { return formation; }
  void setNext(Creature *next, int index);
  void setNextDontMove(Creature *next, int index);
  inline Uint16 getDir() { return dir; }
  inline void setDir(Uint16 dir) { this->dir = dir; }
  
  inline void draw() { getShape()->draw(); }  
    
  /**
	 Used to move away from the player. Find the nearest corner of the map.
  */
  void findCorner(Sint16 *px, Sint16 *py, Sint16 *pz);
  
  void setSelXY(int x, int y, bool force=false);
  inline int getSelX() { return selX; }
  inline int getSelY() { return selY; }
  
  bool anyMovesLeft();
  
  // inventory
  // get the item at the given equip-index (inventory location)
  float inventoryWeight;
  float maxInventoryWeight;
  inline float getInventoryWeight() { return inventoryWeight;  }
  inline float getMaxInventoryWeight() { return maxInventoryWeight; }  
  inline float computeMaxInventoryWeight () { return (float) skills[Constants::POWER] + 25.0f; }  
  Item *getEquippedInventory(int index);
  inline void setMaxInventoryWeight(float w) { maxInventoryWeight = w; }
  
  inline Item *getInventory(int index) { return inventory[index]; }
  inline int getInventoryCount() { return inventory_count; }
  inline void setInventory(int index, Item *item) { 
	if(index < inventory_count) inventory[index] = item; 
  }
  // returns the index of the last item added
  bool addInventory(Item *item);
  Item *removeInventory(int index);  
  // returns true if ate/drank item completely and false else
  bool eatDrink(int index);  
  bool computeNewItemWeight(RpgItem * rpgItem);
  // equip or doff if already equipped
  void equipInventory(int index);
  int doff(int index);
  // return the equip index (inventory location) for an inventory index
  int getEquippedIndex(int index);
  bool isItemInInventory(Item *item);
  // return the item at location Character::INVENTORY_LEFT_HAND, etc.
  Item *getItemAtLocation(int location);

  // return the best equipped weapon that works on this distance, 
  // or NULL if none are available
  Item *getBestWeapon(float dist);  

  inline char *getName() { return name; }
  inline Character *getCharacter() { return character; }  
  inline Monster *getMonster() { return monster; }  
  inline int getLevel() { return level; }
  inline int getExpOfNextLevel() { return expOfNextLevel; }
  inline int getExp() { return exp; }
  inline int getMoney() { return money; }
  inline int getHp() { return hp; }
  inline int getStartingHp() { return hp; }
  int getMaxHp();
  inline int getAc() { return ac; }
  inline int getThirst() { return thirst; }
  inline int getHunger() { return hunger; }
  inline int getSkill(int index) { return skills[index]; }
  inline bool getStateMod(int mod) { return (stateMod & (1 << mod) ? true : false); }  

  inline void setName(char *s) { name = s; }
  inline void setCharacter(Character *c) { character = c; }  
  inline void setLevel(int n) { level = n; }
  inline void setExp(int n) { exp = n; }
  inline void setMoney(int n) { money = n; }
  inline void setHp(int n) { hp = n; }
  inline void setThirst(int n)  { if(n<0)n=0; if(n>10)n=10; thirst = n; }
  inline void setHunger(int n)  { if(n<0)n=0; if(n>10)n=10; hunger = n; } 
  inline void setHp() { hp = getCharacter()->getStartingHp(); }
  inline void setAc(int n) { ac = n; }

  bool incSkillMod(int index);
  bool decSkillMod(int index);
  void applySkillMod();
  inline int getSkillMod(int index) { return skillMod[index]; }

  inline void setSkill(int index, int value) { skills[index] = value; }
  inline void setStateMod(int mod, bool setting) { 
	if(setting) stateMod |= (1 << mod);  
	else stateMod &= ((GLuint)0xffff - (GLuint)(1 << mod)); 
  }

  // return the initiative for a battle round (0-10), the lower the faster the attack
  // the method can return negative numbers if the weapon skill is very high (-10 to 10)
  int getInitiative(Item *weapon);

  // roll the die for the toHit number. returns a value between 0(total miss) - 100(best hit)
  int getToHit(Item *weapon);

  // get the armor value of the creature (0-100) (this is the max armor)
  inline int getArmor() { return armor; }

  // get the armor as modified by each item's assc. skill
  int getSkillModifiedArmor();
  
  // return the damage as:
  // rand(weapon + power + (skill - 50 % weapon))
  int getDamage(Item *weapon);

  // take damage
  // return true if the creature dies
  bool takeDamage(int damage, int effect_type = Constants::EFFECT_GLOW);

  // returns exp gained
  int addExperience(Creature *creature_killed);

  int addExperience(int exp);

  // returns coins gained
  int addMoney(Creature *creature_killed);

  inline void getDetailedDescription(char *s) {
	sprintf(s, "%s (Hp:%d A:%d)", 
			getDescription(), 
			getHp(), 
			getArmor());
  }

  // effects
  void startEffect(int effect_type, int duration = Constants::DAMAGE_DURATION);
  inline void setEffectType(int n) { this->effectType = n; }
  inline int getEffectType() { return effectType; }  
  inline Effect *getEffect() { return effect; }
  inline int getDamageEffect() { return damageEffectCounter; }
  inline void resetDamageEffect() { damageEffectCounter = SDL_GetTicks(); }
  inline bool isEffectOn() { return (SDL_GetTicks() - damageEffectCounter < effectDuration ? true : false); }

  inline int getAvailableSkillPoints() { return availableSkillPoints; }
  inline void setAvailableSkillPoints(int n) { availableSkillPoints = n; }
  
  int getMaxProjectileCount(Item *item);

 protected:

  /**
	 Check that we're within range (if range specified).
	 If not, try n times, then wait n times before trying again.
   */
  void adjustMovementToRange();

  /**
   * Get the position of this creature in the formation.
   * returns -1,-1 if the position cannot be set (if the person followed is not moving)
   */
  void getFormationPosition(Sint16 *px, Sint16 *py, Sint16 *pz);

  void commonInit();
  void calculateExpOfNextLevel();
  void monsterInit();
  void recalcAggregateValues();

  bool gotoPosition(Map *map, Sint16 px, Sint16 py, Sint16 pz, char *debug);
};


#endif

