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

#include "common/constants.h"
#include "persist.h"
#include "date.h"
#include "render/renderedcreature.h"
#include "storable.h"
#include "rpg/rpglib.h"

class Map;
class Session;
class Effect;
class Location;
class Battle;
class GLShape;
class Item;
class Event;
class RenderedItem;
class NpcInfo;
class RenderedProjectile;

/**
  *@author Gabor Torok

  This class is both the UI representation (shape) and state (inventory, etc.) of a character or monster.
  All creatures of the same type (character or monster) share the same instance of the prototype class (Character of Monster)

  */

// how many times to attempt to move to range
#define MAX_FAILED_MOVE_ATTEMPTS 10

class InventoryInfo {
public:
	int inventoryIndex;
	int equipIndex;
};

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
  int bestPathPos;
	int cantMoveCounter;
  std::vector<Location> bestPath;
  Creature *targetCreature;
  int targetX, targetY, targetZ;
  Item *targetItem;
  bool arrived; // true if no particular destination set for this creature
  std::map<int, Event*> stateModEventMap;
  GLfloat angle, wantedAngle, angleStep;
  int portraitTextureIndex;
  
  // inventory
  Item *inventory[MAX_INVENTORY_SIZE];
  int inventory_count;
  int equipped[Constants::INVENTORY_COUNT];
  int preferredWeapon;

  // character information
  char name[255];
  int level, experience, hp, mp, startingHp, startingMp, ac, thirst, hunger, money, expOfNextLevel;
  int sex;
  Character *character;
  int skills[Skill::SKILL_COUNT], skillBonus[Skill::SKILL_COUNT], skillMod[Skill::SKILL_COUNT];
	int availableSkillMod;
	bool hasAvailableSkillPoints;
	int skillsUsed[Skill::SKILL_COUNT];
  GLuint stateMod, protStateMod;
  Monster *monster;

  GLint lastTick;
  int speed, originalSpeed;
  float armor;
  int bonusArmor;
  bool armorChanged;
  float lastArmor[ RpgItem::DAMAGE_TYPE_COUNT ];
	float lastArmorSkill[ RpgItem::DAMAGE_TYPE_COUNT ];
	float lastDodgePenalty[ RpgItem::DAMAGE_TYPE_COUNT ];
  
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

  NpcInfo *npcInfo;

  std::set<SpecialSkill*> specialSkills;
	std::set<std::string> specialSkillNames;

  bool mapChanged;

  std::map<Location*, Uint32> secretDoorAttempts;
	bool moving;

	char causeOfDeath[255], pendingCauseOfDeath[255];
	bool inventoryArranged;
	std::map<Item*, InventoryInfo*> invInfos;
  
 public:
  static const int DIAMOND_FORMATION = 0;
  static const int STAGGERED_FORMATION = 1;
  static const int SQUARE_FORMATION = 2;
  static const int ROW_FORMATION = 3;
  static const int SCOUT_FORMATION = 4;
  static const int CROSS_FORMATION = 5;
  static const int FORMATION_COUNT = 6;
  
  Creature(Session *session, Character *character, char *name, int sex, int character_model_info_index);
  Creature(Session *session, Monster *monster, GLShape *shape, bool initMonster=true);
  ~Creature();

	inline void setCauseOfDeath( char *s ) { strncpy( this->causeOfDeath, s, 254 ); this->causeOfDeath[254] = 0; }
	inline char *getCauseOfDeath() { return this->causeOfDeath; }
	inline void setPendingCauseOfDeath( char *s ) { strncpy( this->pendingCauseOfDeath, s, 254 ); this->pendingCauseOfDeath[254] = 0; }
	inline char *getPendingCauseOfDeath() { return this->pendingCauseOfDeath; }
  
  inline void setSex( int n ) { this->sex = n; }
  inline int getSex() { return this->sex; }

	inline bool isMoving() { return moving; }
	inline void setMoving( bool b ) { moving = b; }

	bool isNpc();
  bool isPathToTargetCreature();

  inline GLfloat getAngle() { return angle; }

	void applyRecurringSpecialSkills();
  void evalSpecialSkills();
	inline bool hasCapability( char *name ) {
		std::string skillName = name;
		return specialSkillNames.find( skillName ) != specialSkillNames.end();
	}
  inline bool hasSpecialSkill( SpecialSkill *ss ) { return specialSkills.find(ss) != specialSkills.end(); }
  char *useSpecialSkill( SpecialSkill *specialSkill, 
                         bool manualOnly );
  float applyAutomaticSpecialSkills( int event, 
                                     char *varName,
                                     float value );
  
  inline void setQuickSpell( int index, Storable *storable ) { 
    for( int i = 0; storable && i < 12; i++ ) {
      if( quickSpell[ i ] == storable ) {
        // already stored
        return;
      }
    }
    quickSpell[ index ] = storable; 
  }
  inline Storable *getQuickSpell( int index ) { return quickSpell[ index ]; }
  void playCharacterSound( int soundType );

  inline void setDeityIndex( int n ) { deityIndex = n; }
  inline int getDeityIndex() { return deityIndex; }

  inline void setPortraitTextureIndex( int n ) { this->portraitTextureIndex = n; }
  inline int getPortraitTextureIndex() { return portraitTextureIndex; }

  inline std::vector<Location> *getPath() { return &bestPath; }
  inline int getPathIndex() { return bestPathPos; }

  inline int getCharacterModelInfoIndex() { return character_model_info_index; }

  inline void setLastEnchantDate(Date date) { lastEnchantDate = date; }
  inline Date getLastEnchantDate() { return lastEnchantDate; }

  inline Battle *getBattle() { return battle; }

	void changeProfession( Character *character );

  CreatureInfo *save();
  static Creature *load(Session *session, CreatureInfo *info);

  inline void setLastTurn(int n) { lastTurn = n; }
  inline int getLastTurn() { return lastTurn; }

  inline bool isMonster() { return (monster ? true : false); }
  
  inline int getTargetX() { if(targetCreature) return toint(targetCreature->getX()); else return targetX; }
  inline int getTargetY() { if(targetCreature) return toint(targetCreature->getY()); else return targetY; }
  inline int getTargetZ() { if(targetCreature) return toint(targetCreature->getZ()); else return targetZ; }
  void setTargetCreature( Creature *c, bool findPath=false );
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
  
  
  inline void setLastTick(GLint n) { this->lastTick = n; }
  inline GLint getLastTick() { return lastTick; }

  // FIXME: should be modified by inventory (boots of speed, etc.)
  inline int getSpeed() { return speed; }
  
  /**
	 The movement functions return true if movement has occured, false if it has not.
   */
  bool move(Uint16 dir);
  void switchDirection(bool force);
  bool follow( Creature *leader );
  Location *moveToLocator();
  void stopMoving();
  
  inline char *getModelName() { return model_name; }
  inline char *getSkinName() { return skin_name; }
  inline GLShape *getShape() { return shape; }
  inline void setFormation(int formation) { this->formation = formation; }
  inline int getFormation() { return formation; }
  void setNext(Creature *next, int index);
  void setNextDontMove(Creature *next, int index);
  Creature *getNext() { return next; }
  inline Uint16 getDir() { return dir; }
  inline void setDir(Uint16 dir) { this->dir = dir; }
  
  void draw();
    
  /**
   * Set where to move the creature. 
   * Returns true if the move is possible, false otherwise.
   */
  bool setSelXY( int x, int y, bool cancelIfNotPossible=true, int maxNodes=100 );  
  inline int getSelX() { return selX; }
  inline int getSelY() { return selY; }
  inline void setMapChanged() { mapChanged = true; }
  bool anyMovesLeft();
	void moveAway( Creature *other );
	void cancelMoveAway();
  
  // inventory
  // get the item at the given equip-index (inventory location)
  float inventoryWeight;
  inline float getInventoryWeight() { return inventoryWeight;  }
  float getMaxInventoryWeight();
  Item *getEquippedInventory(int index);
  bool isEquippedWeapon(int index);
  
  inline Item *getInventory(int index) { return inventory[index]; }
  inline int getInventoryCount() { return inventory_count; }
  inline void setInventory(int index, Item *item) { 
    if(index < inventory_count) inventory[index] = item; 
  }
  inline int getPreferredWeapon() { return preferredWeapon; }
  inline void setPreferredWeapon( int n ) { preferredWeapon = n; }
  bool nextPreferredWeapon();
  // returns the index of the last item added
  void pickUpOnMap( RenderedItem *item );
  bool addInventory(Item *item, bool force=false);
  Item *removeInventory(int index);  
  int findInInventory(Item *item);
  // returns true if ate/drank item completely and false else
  bool eatDrink(int index);  
  bool eatDrink(Item *item);
  // equip or doff if already equipped
  void equipInventory( int index, int locationHint=-1 );
  int doff(int index);
  // return the equip index (inventory location) for an inventory index
  int getEquippedIndex(int index);
  bool isItemInInventory(Item *item);
  // return the item at location Character::INVENTORY_LEFT_HAND, etc.
  Item *getItemAtLocation(int location);
  bool isEquipped( Item *item );
  bool isEquipped( int inventoryIndex );
  bool removeCursedItems();
	InventoryInfo *getInventoryInfo( Item *item, bool createIfMissing=false );
	

  // return the best equipped weapon that works on this distance, 
  // or NULL if none are available
  Item *getBestWeapon( float dist, bool callScript=false );  

  float getAttacksPerRound( Item *item = NULL );
	float getWeaponAPCost( Item *item, bool showDebug=true );

  void setNpcInfo( NpcInfo *npcInfo );
  inline NpcInfo *getNpcInfo() { return npcInfo; }


  inline char *getName() { return name; }
  inline Character *getCharacter() { return character; }  
  inline Monster *getMonster() { return monster; }  
  inline int getLevel() { return level; }
  inline int getExpOfNextLevel() { return expOfNextLevel; }
  inline int getExp() { return experience; }
  inline int getMoney() { return money; }
  inline int getHp() { return hp; }
  inline int getStartingHp() { return hp; }
  int getMaxHp();
  inline int getMp() { return mp; }
  inline int getStartingMp() { return mp; }
  int getMaxMp();
  inline int getThirst() { return thirst; }
  inline int getHunger() { return hunger; }
  inline int getSkill(int index, bool includeMod=true) { return skills[index] + skillBonus[index] + ( includeMod ? skillMod[index] : 0 ); }
  inline bool getStateMod(int mod) { return (stateMod & (1 << mod) ? true : false); }  
  inline bool getProtectedStateMod(int mod) { return (protStateMod & (1 << mod) ? true : false); }  

  inline void setName(char *s) { strncpy( name, s, 254 ); name[254]='\0'; }
  void setCharacter(Character *c);
  inline void setLevel(int n) { level = ( n < 0 ? 0 : n ); evalSpecialSkills(); }
  void setExp();
  inline void setExp(int n) { experience = ( n < 0 ? 0 : n ); evalSpecialSkills(); }
  inline void setMoney(int n) { money = n; evalSpecialSkills(); }
  inline void setHp(int n) { hp = ( n < 0 ? 0 : n ); evalSpecialSkills(); }
  inline void setMp(int n) { mp = ( n < 0 ? 0 : n ); evalSpecialSkills(); }
  inline void setThirst(int n)  { if(n<0)n=0; if(n>10)n=10; thirst = n; evalSpecialSkills(); }
  inline void setHunger(int n)  { if(n<0)n=0; if(n>10)n=10; hunger = n; evalSpecialSkills(); } 
  void setHp();
  void setMp();

  void setSkill(int index, int value);
  void setSkillBonus( int index, int value );	
  inline int getSkillBonus(int index) { return skillBonus[index]; }
	void setSkillMod( int index, int value );
	inline int getSkillMod( int index ) { return skillMod[index];}
	void applySkillMods();
	bool getHasAvailableSkillPoints() { return hasAvailableSkillPoints; }		
	inline int getAvailableSkillMod() { return availableSkillMod; }
	inline void setAvailableSkillMod( int n ) { 
 	  availableSkillMod = n;
		 if( !hasAvailableSkillPoints && n > 0 ) hasAvailableSkillPoints = true; 
  }
  void setStateMod(int mod, bool setting);
  void setProtectedStateMod(int mod, bool setting);

  // return the initiative for a battle round, the lower the faster the attack
  int getInitiative( int *max=NULL );
  
  // take damage
  // return true if the creature dies
  bool takeDamage( float damage, int effect_type = Constants::EFFECT_GLOW, GLuint delay=0 );

  void resurrect( int rx, int ry );

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
  
  int getMaxProjectileCount(Item *item);

	std::vector<RenderedProjectile*> *getProjectiles();

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
  bool canAttack(RenderedCreature *creature, int *cursor=NULL);
  void cancelTarget();
  //void makeTargetRetaliate();
  void decideMonsterAction();

  /**
   * Try to heal someone; returns true if someone was found.
   */
  bool castHealingSpell();	

  float getDistanceToTarget( RenderedCreature *creature=NULL );
	float getDistance( RenderedCreature *other );
	float getDistanceToSel();

  bool isWithPrereq( Spell *spell );
  Creature *findClosestTargetWithPrereq( Spell *spell );
	bool useOffensiveSpell( Spell *spell, float dist, Creature *possibleTarget );

  inline void setStateModEvent(int mod, Event *event) { stateModEventMap[mod] = event; }
  inline Event *getStateModEvent(int mod) { return(stateModEventMap.find(mod) == stateModEventMap.end() ? NULL : stateModEventMap[mod]); }


	// ======================================
	// Combat methods
	void getCth( Item *weapon, float *cth, float *skill, bool showDebug=true );

	float getDodge( Creature *attacker, Item *weapon=NULL );
  
	float getAttack( Item *weapon, 
									 float *maxP=NULL, 
									 float *minP=NULL, 
									 bool callScript=false );
	
	float getParry( Item **parryItem );	

	float getArmor( float *armor, float *dodgePenalty, int damageType, Item *vsWeapon = NULL );	

  float getAttackerStateModPercent();
  float getDefenderStateModPercent( bool magical );
  float rollMagicDamagePercent( Item *item );
  float getMaxAP();
  char *canEquipItem( Item *item, bool interactive = true );  
  bool rollSkill( int skill, float luckDiv=0.0f );
  bool rollSecretDoor( Location *pos );
  void resetSecretDoorAttempts();
	char *getType();

	inline bool isInventoryArranged() { return inventoryArranged; }
	inline void setInventoryArranged( bool b ) { inventoryArranged = b; }

 protected:

	/**
	 * Recalculate skills when stats change.
	 */
	void skillChanged( int index, int oldValue, int newValue );


	 void calcArmor( int damageType,
									float *armorP, 
                  float *dodgePenalty,
                  bool callScript=false );

   bool findPath( int x, int y, bool cancelIfNotPossible, int maxNodes, bool ignoreParty );

  /**
   * Get the position of this creature in the formation.
   * returns -1,-1 if the position cannot be set (if the person followed is not moving)
   */
  void getFormationPosition( Sint16 *px, Sint16 *py, Sint16 *pz, int x=-1, int y=-1 );

  void commonInit();
  void calculateExpOfNextLevel();
  void monsterInit();
  void recalcAggregateValues();

  Location *takeAStepOnPath();
  void computeAngle( GLfloat newX, GLfloat newY );
  void showWaterEffect( GLfloat newX, GLfloat newY );

  /**
   * How big of a step to take.
   * Use fps, speed and gamespeed to figure this out.
   */
  GLfloat getStep();

	/**
	 * Apply a weapon influence modifier.
	 * 
	 * @param weapon the item used or null for unarmed attack
	 * @param influenceType an influence type from RpgItem
	 * @param debugMessage a message to print
	 * @return the bonus (malus if negative)
	 */
	float getInfluenceBonus( Item *weapon, 
													 int influenceType, 
													 char *debugMessage );
};


#endif

