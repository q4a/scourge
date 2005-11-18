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
#include <map>
#include <set>
#include <algorithm> // STL for Heap

#include "constants.h"
#include "persist.h"
#include "date.h"
#include "render/renderedcreature.h"
#include "storable.h"

class Map;
class Session;
class Effect;
class Location;
class Battle;
class Monster;
class Character;
class RpgItem;
class Spell;
class SpecialSkill;
class GLShape;
class Item;
class Event;
class RenderedItem;
class NpcInfo;
class SpecialSkill;

/**
  *@author Gabor Torok

  This class is both the UI representation (shape) and state (inventory, etc.) of a character or monster.
  All creatures of the same type (character or monster) share the same instance of the prototype class (Character of Monster)

  */

// how many times to attempt to move to range
#define MAX_FAILED_MOVE_ATTEMPTS 10

class Creature : public RenderedCreature {
  
 private:
  // gui information
  Creature *next;
  GLShape *shape;
  char *model_name, *skin_name;
  Uint16 dir;
  Session *session;
  int motion;
  int failedToMoveWithinRangeAttemptCount;
  int facingDirection;
  int formation;
  int index;
  int tx, ty;
  int selX, selY;
  int proposedX, proposedY, proposedPathIndex;
  int bestPathPos;
  std::vector<Location> bestPath;
  std::vector<Location> proposedPath;
  Creature *targetCreature;
  int targetX, targetY, targetZ;
  Item *targetItem;
  Sint16 cornerX, cornerY;
  bool arrived; // true if no particular destination set for this creature
  std::map<int, Event*> stateModEventMap;
  GLfloat angle, wantedAngle, angleStep;
  int portraitTextureIndex;
  
  // inventory
  Item *inventory[MAX_INVENTORY_SIZE];
  int inventory_count;
  int equipped[Constants::INVENTORY_COUNT];

  // character information
  char *name;
  int level, exp, hp, mp, startingHp, startingMp, ac, thirst, hunger, money, expOfNextLevel;
  Character *character;
  int skills[Constants::SKILL_COUNT], skillMod[Constants::SKILL_COUNT], skillBonus[Constants::SKILL_COUNT];
  GLuint stateMod, protStateMod;
  Monster *monster;

  char description[300];
  GLint lastTick;
  int speed;
  int armor, bonusArmor;
  float avgArmorLevel;
  int moveRetrycount;
  int availableSkillPoints;
  int usedSkillPoints;
  
  static const int MAX_MOVE_RETRY = 15;
  int lastTurn;

  std::vector<Spell*> spells;
  int action;
  Item *actionItem;
  Spell *actionSpell;
  SpecialSkill *actionSkill;
  Creature *preActionTargetCreature;

  int moveCount;
  Uint32 lastMove;
  Battle *battle;

  Date lastEnchantDate;
  int character_model_info_index;
  int deityIndex;

  Storable *quickSpell[12];
  bool loaded;

  NpcInfo *npcInfo;

  std::set<SpecialSkill*> specialSkills;
  
 public:
  static const int DIAMOND_FORMATION = 0;
  static const int STAGGERED_FORMATION = 1;
  static const int SQUARE_FORMATION = 2;
  static const int ROW_FORMATION = 3;
  static const int SCOUT_FORMATION = 4;
  static const int CROSS_FORMATION = 5;
  static const int FORMATION_COUNT = 6;
  
  Creature(Session *session, Character *character, char *name, int character_model_info_index, bool loaded);
  Creature(Session *session, Monster *monster, GLShape *shape, bool loaded);
  ~Creature();

  void evalSpecialSkills();
  inline bool hasSpecialSkill( SpecialSkill *ss ) { return specialSkills.find(ss) != specialSkills.end(); }
  char *useSpecialSkill( SpecialSkill *specialSkill, 
                         bool manualOnly );
  float applyAutomaticSpecialSkills( int event, 
                                     char *varName,
                                     float value );
  
  inline void setQuickSpell( int index, Storable *storable ) { 
    for( int i = 0; i < 12; i++ ) {
      if( quickSpell[ i ] == storable ) {
        // already stored
        return;
      }
    }
    quickSpell[ index ] = storable; 
  }
  inline Storable *getQuickSpell( int index ) { return quickSpell[ index ]; }

  inline void setDeityIndex( int n ) { deityIndex = n; }
  inline int getDeityIndex() { return deityIndex; }

  inline void setPortraitTextureIndex( int n ) { this->portraitTextureIndex = n; }
  inline int getPortraitTextureIndex() { return portraitTextureIndex; }

  inline std::vector<Location> *getPath() { return &bestPath; }
  inline int getPathIndex() { return bestPathPos; }

  inline std::vector<Location> *getProposedPath() { return &proposedPath; }
  inline int getProposedPathIndex() { return proposedPathIndex; }
  inline int getProposedX() { return proposedX; }
  inline int getProposedY() { return proposedY; }

  inline int getCharacterModelInfoIndex() { return character_model_info_index; }

  inline void setLastEnchantDate(Date date) { lastEnchantDate = date; }
  inline Date getLastEnchantDate() { return lastEnchantDate; }

  inline Battle *getBattle() { return battle; }

  CreatureInfo *save();
  static Creature *load(Session *session, CreatureInfo *info);

  inline void setLastTurn(int n) { lastTurn = n; }
  inline int getLastTurn() { return lastTurn; }

  inline bool isMonster() { return (monster ? true : false); }
  
  inline int getTargetX() { if(targetCreature) return toint(targetCreature->getX()); else return targetX; }
  inline int getTargetY() { if(targetCreature) return toint(targetCreature->getY()); else return targetY; }
  inline int getTargetZ() { if(targetCreature) return toint(targetCreature->getZ()); else return targetZ; }
  void setTargetCreature(Creature *c);
  inline Creature *getTargetCreature() { return targetCreature; }
  inline void setTargetLocation(int x, int y, int z) { targetItem = NULL; targetCreature = NULL; targetX = x; targetY = y; targetZ = z; }
  inline void getTargetLocation(int *x, int *y, int *z) { *x = targetX; *y = targetY; *z = targetZ; }
  inline void setTargetItem(int x, int y, int z, Item *item) { setTargetLocation(x, y, z); targetItem = item; }
  inline Item *getTargetItem() { return targetItem; }
  
  inline void setMotion(int motion) { this->motion = motion; }  
  inline int getMotion() { return this->motion; }

  /**
	 Return true only if a range is specified and we're within it.
  */
//  bool Creature::isInRange();
    
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
  void switchDirection(bool force);
  bool follow(Map *map);
  bool moveToLocator(Map *map);
  void stopMoving();
  
  inline char *getModelName() { return model_name; }
  inline char *getSkinName() { return skin_name; }
  inline GLShape *getShape() { return shape; }
  inline void setFormation(int formation) { this->formation = formation; }
  inline int getFormation() { return formation; }
  void setNext(Creature *next, int index);
  void setNextDontMove(Creature *next, int index);
  inline Uint16 getDir() { return dir; }
  inline void setDir(Uint16 dir) { this->dir = dir; }
  
  void draw();
    
  /**
	 Used to move away from the player. Find the nearest corner of the map.
  */
  void findCorner(Sint16 *px, Sint16 *py, Sint16 *pz);
  
  void setSelXY(int x, int y, bool force=false);
  inline int getSelX() { return selX; }
  inline int getSelY() { return selY; }
  void findPath( int x, int y );
  
  bool anyMovesLeft();
  
  // inventory
  // get the item at the given equip-index (inventory location)
  float inventoryWeight;
  inline float getInventoryWeight() { return inventoryWeight;  }
  float getMaxInventoryWeight();
  Item *getEquippedInventory(int index);
  
  inline Item *getInventory(int index) { return inventory[index]; }
  inline int getInventoryCount() { return inventory_count; }
  inline void setInventory(int index, Item *item) { 
	if(index < inventory_count) inventory[index] = item; 
  }
  // returns the index of the last item added
  void pickUpOnMap( RenderedItem *item );
  bool addInventory(Item *item, bool force=false);
  Item *removeInventory(int index);  
  int findInInventory(Item *item);
  // returns true if ate/drank item completely and false else
  bool eatDrink(int index);  
  bool eatDrink(Item *item);
  bool computeNewItemWeight(RpgItem * rpgItem);
  // equip or doff if already equipped
  void equipInventory(int index);
  int doff(int index);
  // return the equip index (inventory location) for an inventory index
  int getEquippedIndex(int index);
  bool isItemInInventory(Item *item);
  // return the item at location Character::INVENTORY_LEFT_HAND, etc.
  Item *getItemAtLocation(int location);
  bool isEquipped( Item *item );
  bool isEquipped( int inventoryIndex );
  bool removeCursedItems();

  // return the best equipped weapon that works on this distance, 
  // or NULL if none are available
  Item *getBestWeapon(float dist);  

  void setNpcInfo( NpcInfo *npcInfo );
  inline NpcInfo *getNpcInfo() { return npcInfo; }


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
  inline int getMp() { return mp; }
  inline int getStartingMp() { return mp; }
  int getMaxMp();
  inline int getThirst() { return thirst; }
  inline int getHunger() { return hunger; }
  inline int getSkill(int index) { return skills[index] + skillBonus[index]; }
  inline bool getStateMod(int mod) { return (stateMod & (1 << mod) ? true : false); }  
  inline bool getProtectedStateMod(int mod) { return (protStateMod & (1 << mod) ? true : false); }  

  inline void setName(char *s) { name = s; }
  inline void setCharacter(Character *c) { character = c; }  
  inline void setLevel(int n) { level = n; }
  void setExp();
  inline void setExp(int n) { exp = n; }
  inline void setMoney(int n) { money = n; }
  inline void setHp(int n) { hp = n; }
  inline void setMp(int n) { mp = n; }
  inline void setThirst(int n)  { if(n<0)n=0; if(n>10)n=10; thirst = n; }
  inline void setHunger(int n)  { if(n<0)n=0; if(n>10)n=10; hunger = n; } 
  void setHp();
  void setMp();

  bool incSkillMod(int index);
  bool decSkillMod(int index);
  void applySkillMod();
  inline int getSkillMod(int index) { return skillMod[index]; }
  void setSkill(int index, int value);
  inline void setSkillBonus(int index, int value) { skillBonus[index] = value; }
  inline int getSkillBonus(int index) { return skillBonus[index]; }
  void setStateMod(int mod, bool setting);
  void setProtectedStateMod(int mod, bool setting);

  // return the initiative for a battle round (0-10), the lower the faster the attack
  // the method can return negative numbers if the weapon skill is very high (-10 to 10)
  int getInitiative(Item *weapon, Spell *spell=NULL);

  // roll the die for the toHit number. returns a value between 0(total miss) - 100(best hit)
  int getToHit(Item *weapon, int *maxToHit=NULL, int *rolledToHit=NULL);

  // get the armor value of the creature (0-100) (this is the max armor)
  inline int getArmor() { return armor; }

  // get the armor as modified by each item's assc. skill
  int getSkillModifiedArmor();
  
  // return the damage as:
  // rand(weapon + power + (skill - 50 % weapon))
  int getDamage(Item *weapon, int *maxDamage=NULL, int *rolledDamage=NULL);

  // take damage
  // return true if the creature dies
  bool takeDamage( int damage, int effect_type = Constants::EFFECT_GLOW, GLuint delay=0 );

  // returns exp gained
  int addExperience(Creature *creature_killed);

  int addExperience(int exp);

  /**
   * Add experience and show message in map window. Also shows
   * message if creature leveled up. Use generally for party
   * characters only.
   */
  int addExperienceWithMessage( int exp );

  // get angle to target creature
  float getTargetAngle();

  // returns coins gained
  int addMoney(Creature *creature_killed);

  void getDetailedDescription(char *s);

  inline int getAvailableSkillPoints() { return availableSkillPoints; }
  inline void setAvailableSkillPoints(int n) { availableSkillPoints = n; }
  inline int getUsedSkillPoints() { return usedSkillPoints; }
  inline void setUsedSkillPoints( int n ) { usedSkillPoints = n; }
  
  int getMaxProjectileCount(Item *item);

  void usePotion(Item *item);

  inline void setBonusArmor(int n) { bonusArmor = n; if(bonusArmor < 0) bonusArmor = 0; recalcAggregateValues(); }
  inline int getBonusArmor() { return bonusArmor; }

  // FIXME: O(n) but there aren't that many spells...
  // return true if spell was added, false if creature already had this spell
  bool addSpell(Spell *spell);
  // FIXME: O(n) but there aren't that many spells...
  bool isSpellMemorized(Spell *spell);
  inline int getSpellCount() { return (int)spells.size(); }
  inline Spell *getSpell(int index) { return spells[index]; }

  void setAction(int action, Item *item=NULL, Spell *spell=NULL, SpecialSkill *skill=NULL);
  inline int getAction() { return action; }
  inline Item *getActionItem() { return actionItem; }
  inline Spell *getActionSpell() { return actionSpell; }
  inline SpecialSkill *getActionSkill() { return actionSkill; }



  // handling battle targets (which in the future may be more than targetCreature)
  inline bool hasTarget() { return targetCreature || targetItem || targetX || targetY || targetZ; }
  bool isTargetValid();
  bool canAttack(RenderedCreature *creature);
  void cancelTarget();
  void followTarget();
  //void makeTargetRetaliate();
  void decideMonsterAction();

  /**
   * Try to heal someone; returns true if someone was found.
   */
  bool castHealingSpell();

  float getDistanceToTarget();
  bool isWithPrereq( Spell *spell );
  Creature *findClosestTargetWithPrereq( Spell *spell );

  inline void setStateModEvent(int mod, Event *event) { stateModEventMap[mod] = event; }
  inline Event *getStateModEvent(int mod) { return(stateModEventMap.find(mod) == stateModEventMap.end() ? NULL : stateModEventMap[mod]); }


  // new Christie-style battle system
  float getACPercent( float *totalP=NULL, float *skillP=NULL );
  void calcArmor( float *armorP, 
                  float *avgArmorLevelP );
  float getAttackPercent( Item *weapon, 
                          float *totalP=NULL, 
                          float *skillP=NULL,
                          float *itemLevelP=NULL,
                          float *levelDiffP=NULL );

 protected:

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

  /**
   * How big of a step to take.
   * Use fps, speed and gamespeed to figure this out.
   */
  GLfloat getStep();
};


#endif

