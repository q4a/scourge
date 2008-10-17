/***************************************************************************
               creature.h  -  A class describing any creature
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
#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
//#include <conio.h>   // for getch
#include <vector>    // STL for Vector
#include <map>
#include <set>
#include <algorithm> // STL for Heap

#include "persist.h"
#include "date.h"
#include "render/renderedcreature.h"
#include "storable.h"
#include "rpg/rpglib.h"
#include "session.h"
#include "render/texture.h"

class Map;
class Effect;
class Location;
class Battle;
class PathManager;
class GLShape;
class Item;
class Event;
class RenderedItem;
class NpcInfo;
class RenderedProjectile;
class Trap;


/**
  *@author Gabor Torok

  This class is both the UI representation (shape) and state (inventory, etc.) of a character or monster.
  All creatures of the same type (character or monster) share the same instance of the prototype class (Character of Monster)

  */

// how many times to attempt to move to range
#define MAX_FAILED_MOVE_ATTEMPTS 10

/// Contains the location of an item in the inventory or where it is equipped.

class InventoryInfo {
public:
	int inventoryIndex;
	int equipIndex;
};

/// An individual char/NPC/monster (look and state).

class Creature : public RenderedCreature {

private:
	// gui information
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
	int cantMoveCounter;
	PathManager* pathManager;
	Creature *targetCreature;
	int targetX, targetY, targetZ;
	Item *targetItem;
	bool arrived; // true if no particular destination set for this creature
	std::map<int, Event*> stateModEventMap;
	int portraitTextureIndex;
	GLfloat angle, wantedAngle, angleStep;

	// inventory
	Item *inventory[MAX_INVENTORY_SIZE];
	int inventory_count;
  int equipped[Constants::EQUIP_LOCATION_COUNT];
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
	std::map<Trap*, Uint32> trapFindAttempts;
	bool moving;

	char causeOfDeath[255], pendingCauseOfDeath[255];
	bool inventoryArranged;
	std::map<Item*, InventoryInfo*> invInfos;
	Uint32 lastPerceptionCheck;
	bool boss, savedMissionObjective, scripted;
	int scriptedAnim;

	Texture portrait;
	Uint32 lastDecision;

public:
	static const int DIAMOND_FORMATION = 0;
	static const int STAGGERED_FORMATION = 1;
	static const int SQUARE_FORMATION = 2;
	static const int ROW_FORMATION = 3;
	static const int SCOUT_FORMATION = 4;
	static const int CROSS_FORMATION = 5;
	static const int FORMATION_COUNT = 6;

	Creature( Session *session, Character *character, char *name, int sex, int character_model_info_index );
	Creature( Session *session, Monster *monster, GLShape *shape, bool initMonster = true );
	~Creature();

	void drawMoviePortrait( int width, int height );
	void drawPortrait( int width, int height, bool inFrame = false );
	void playFootstep();

	inline void setScriptedAnimation( int n ) {
		this->scriptedAnim = n;
	}
	void setScripted( bool b );
	inline int getScriptedAnimation() {
		return this->scriptedAnim;
	}
	/// Will the creature stay visible in movie mode?
	inline bool isScripted() {
		return this->scripted;
	}
	inline void setBoss( bool b ) {
		this->boss = b;
	}
	/// Is it a level boss?
	virtual inline bool isBoss() {
		return this->boss;
	}
	inline bool isSavedMissionObjective() {
		return savedMissionObjective;
	}
	inline void setSavedMissionObjective( bool b ) {
		savedMissionObjective = b;
	}

	inline void setCauseOfDeath( char *s ) {
		strncpy( this->causeOfDeath, s, 254 ); this->causeOfDeath[254] = 0;
	}
	inline char *getCauseOfDeath() {
		return this->causeOfDeath;
	}
	inline void setPendingCauseOfDeath( char *s ) {
		strncpy( this->pendingCauseOfDeath, s, 254 ); this->pendingCauseOfDeath[254] = 0;
	}
	inline char *getPendingCauseOfDeath() {
		return this->pendingCauseOfDeath;
	}

	inline void setSex( int n ) {
		this->sex = n;
	}
	/// Male/female?
	inline int getSex() {
		return this->sex;
	}

	/// Currently moving?
	inline bool isMoving() {
		return moving;
	}
	inline void setMoving( bool b ) {
		moving = b;
	}

	/// Is it an NPC? (this excludes wandering heroes, which are in fact auto controlled characters)
	inline bool isNpc() {
		return( monster ? monster->isNpc() : false );
	}
	/// Sets whether the creature is an NPC.
	inline void setNpc( bool b ) {
		if ( monster ) monster->setNpc( b );
		if ( !b ) npcInfo = NULL;
	}
	/// Is it a wandering hero (recruitable character)?
	inline bool isWanderingHero() {
		return( !session->getParty()->isPartyMember( this ) && ( character != NULL ) );
	}

	/// Is it a monster?
	inline bool isMonster() {
		return( ( monster != NULL ) && !monster->isNpc() );
	}

	/// Is it harmless critter?
	inline bool isHarmlessAnimal() {
		return( ( monster != NULL ) && monster->isHarmless() );
	}

	/// Is it a party member?
	inline bool isPartyMember() {
		return( session->getParty()->isPartyMember( this ) );
	}

	/// Is it the currently active party member?
	inline bool isPlayer() {
		return( this == session->getParty()->getPlayer() );
	}

	inline GLfloat getAngle() {
		return angle;
	}

	void applyRecurringSpecialSkills();
	void evalSpecialSkills();
	/// Does it have this special capability?
	inline bool hasCapability( char *name ) {
		std::string skillName = name;
		return specialSkillNames.find( skillName ) != specialSkillNames.end();
	}
	/// Does it have this special capability?
	inline bool hasSpecialSkill( SpecialSkill *ss ) {
		return specialSkills.find( ss ) != specialSkills.end();
	}
	char *useSpecialSkill( SpecialSkill *specialSkill,
	                       bool manualOnly );
	float applyAutomaticSpecialSkills( int event,
	                                   char *varName,
	                                   float value );

	/// Stores an item/spell etc. in a specified quickspell slot.
	inline void setQuickSpell( int index, Storable *storable ) {
		for ( int i = 0; storable && i < 12; i++ ) {
			if ( quickSpell[ i ] == storable ) {
				// already stored
				return;
			}
		}
		quickSpell[ index ] = storable;
	}
	/// Returns the storable in a given quickspell slot.
	inline Storable *getQuickSpell( int index ) {
		return quickSpell[ index ];
	}
	void playCharacterSound( int soundType, int panning );

	inline void setDeityIndex( int n ) {
		deityIndex = n;
	}
	/// Deity of the creature.
	inline int getDeityIndex() {
		return deityIndex;
	}

	inline void setPortraitTextureIndex( int n ) {
		this->portraitTextureIndex = n;
	}
	inline int getPortraitTextureIndex() {
		return portraitTextureIndex;
	}

// inline std::vector<Location> *getPath() { return &bestPath; }
// inline int getPathIndex() { return bestPathPos; }

	inline int getCharacterModelInfoIndex() {
		return character_model_info_index;
	}

	inline void setLastEnchantDate( Date date ) {
		lastEnchantDate = date;
	}
	/// Last point in time when the creature tried to enchant something.
	inline Date getLastEnchantDate() {
		return lastEnchantDate;
	}

	/// The current battle turn.
	inline Battle *getBattle() {
		return battle;
	}
	inline PathManager* getPathManager() {
		return pathManager;
	}
	/// The session object used to create this class instance.
	inline Session* getSession() {
		return session;
	}

	void changeProfession( Character *character );

	CreatureInfo *save();
	static Creature *load( Session *session, CreatureInfo *info );

	inline void setLastTurn( int n ) {
		lastTurn = n;
	}
	inline int getLastTurn() {
		return lastTurn;
	}

	/// Map X coordinate of currently selected target.
	inline int getTargetX() {
		if ( targetCreature ) return toint( targetCreature->getX() ); else return targetX;
	}
	/// Map Y coordinate of currently selected target.
	inline int getTargetY() {
		if ( targetCreature ) return toint( targetCreature->getY() ); else return targetY;
	}
	/// Map Z coordinate of currently selected target.
	inline int getTargetZ() {
		if ( targetCreature ) return toint( targetCreature->getZ() ); else return targetZ;
	}

	void setTargetCreature( Creature *c, bool findPath = false , float range = static_cast<float>( MIN_DISTANCE ) );
	/// The creature that is currently set as a target.
	inline Creature *getTargetCreature() {
		return targetCreature;
	}
	inline void setTargetLocation( int x, int y, int z ) {
		targetItem = NULL; targetCreature = NULL; targetX = x; targetY = y; targetZ = z;
	}
	/// The map location that is currently set as a target.
	inline void getTargetLocation( int *x, int *y, int *z ) {
		*x = targetX; *y = targetY; *z = targetZ;
	}
	inline void setTargetItem( int x, int y, int z, Item *item ) {
		setTargetLocation( x, y, z ); targetItem = item;
	}
	/// The item that is currently set as a target.
	inline Item *getTargetItem() {
		return targetItem;
	}

	void setMotion( int motion );
	/// The current mode of motion (stand, run, loiter around...)
	inline int getMotion() {
		return this->motion;
	}

//  bool Creature::isInRange();

	inline void setFacingDirection( int direction ) {
		this->facingDirection = direction;
	}
	/// The direction the creature faces (U, D, L, R).
	inline int getFacingDirection() {
		return this->facingDirection;
	}


	inline void setLastTick( GLint n ) {
		this->lastTick = n;
	}
	inline GLint getLastTick() {
		return lastTick;
	}

	// FIXME: should be modified by inventory (boots of speed, etc.)
	inline int getSpeed() {
		return speed;
	}

	/**
	The movement functions return true if movement has occured, false if it has not.
	 */

	bool move( Uint16 dir );
	void switchDirection( bool force );
	bool follow( Creature *leader );
	Location *moveToLocator();
	void stopMoving();

	/// The name of the 3D model used for the creature.
	inline char *getModelName() {
		return model_name;
	}
	inline char *getSkinName() {
		return skin_name;
	}
	/// The 3D shape of the creature.
	inline GLShape *getShape() {
		return shape;
	}
	inline void setFormation( int formation ) {
		this->formation = formation;
	}
	/// Formations are currently unused.
	inline int getFormation() {
		return formation;
	}
	inline Uint16 getDir() {
		return dir;
	}
	/// Direction of movement (U, D, L, R).
	inline void setDir( Uint16 dir ) {
		this->dir = dir;
	}

	void draw();

	bool setSelXY( int x, int y, bool cancelIfNotPossible = false );
	bool setSelCreature( Creature* creature, float range, bool cancelIfNotPossible = false );
	/// Set target without performing pathfinding.
	inline void setSelXYNoPath( int x, int y ) {
		selX = x; selY = y;
	}

	/// Map X coordinate of selection target.
	inline int getSelX() {
		return selX;
	}
	/// Map Y coordinate of selection target.
	inline int getSelY() {
		return selY;
	}
	/// Unused.
	inline void setMapChanged() {
		mapChanged = true;
	}
	bool anyMovesLeft();
	/// Makes the creature move out of another creature's path.
	void moveAway( Creature *other );
	//void cancelMoveAway();

	// inventory
	float inventoryWeight;
	/// Total weight of the inventory.
	inline float getInventoryWeight() {
		return inventoryWeight;
	}
	float getMaxInventoryWeight();
	Item *getEquippedInventory( int index );
	bool isEquippedWeapon( int index );

	/// The item at a specified inventory index.
	inline Item *getInventory( int index ) {
		return inventory[index];
	}
	/// Number of items carried in inventory.
	inline int getInventoryCount() {
		return inventory_count;
	}
	/// Adds an item to the inventory at the specified index.
	inline void setInventory( int index, Item *item ) {
		if ( index < inventory_count ) inventory[index] = item;
	}
	/// The active weapon.
	inline int getPreferredWeapon() {
		return preferredWeapon;
	}
	inline void setPreferredWeapon( int n ) {
		preferredWeapon = n;
	}
	bool nextPreferredWeapon();

	void pickUpOnMap( RenderedItem *item );
	bool addInventory( Item *item, bool force = false );
	Item *removeInventory( int index );
	int findInInventory( Item *item );

	bool eatDrink( int index );
	bool eatDrink( Item *item );

	void equipInventory( int index, int locationHint = -1 );
	int doff( int index );

	int getEquippedIndex( int index );
	bool isItemInInventory( Item *item );

	Item *getItemAtLocation( int location );
	bool isEquipped( Item *item );
	bool isEquipped( int inventoryIndex );
	bool removeCursedItems();
	InventoryInfo *getInventoryInfo( Item *item, bool createIfMissing = false );


	// return the best equipped weapon that works on this distance,
	// or NULL if none are available
	Item *getBestWeapon( float dist, bool callScript = false );

	float getAttacksPerRound( Item *item = NULL );
	float getWeaponAPCost( Item *item, bool showDebug = true );

	void setNpcInfo( NpcInfo *npcInfo );
	/// Extra info for edited NPCs.
	inline NpcInfo *getNpcInfo() {
		return npcInfo;
	}

	/// The creature's UNLOCALIZED name.
	inline char *getName() {
		return name;
	}
	/// Gets character info.
	inline Character *getCharacter() {
		return character;
	}
	/// Gets monster/NPC info.
	inline Monster *getMonster() {
		return monster;
	}
	/// The creature's level.
	inline int getLevel() {
		return level;
	}
	/// Experience points needed for next level.
	inline int getExpOfNextLevel() {
		return expOfNextLevel;
	}
	/// Experience points.
	inline int getExp() {
		return experience;
	}
	/// Amount of money the creature holds.
	inline int getMoney() {
		return money;
	}
	/// Current hit points.
	inline int getHp() {
		return hp;
	}
	/// Starting HP for level 1.
	inline int getStartingHp() {
		return hp;
	}
	int getMaxHp();
	/// Current magic points.
	inline int getMp() {
		return mp;
	}
	/// Starting MP for level 1.
	inline int getStartingMp() {
		return mp;
	}
	int getMaxMp();
	/// Current thirst level.
	inline int getThirst() {
		return thirst;
	}
	/// Current hunger level.
	inline int getHunger() {
		return hunger;
	}
	/// Returns the value of the specified skill.
	inline int getSkill( int index, bool includeMod = true ) {
		return skills[index] + skillBonus[index] + ( includeMod ? skillMod[index] : 0 );
	}
	/// Returns the active state of a state mod.
	inline bool getStateMod( int mod ) {
		return ( stateMod & ( 1 << mod ) ? true : false );
	}
	/// Returns whether the specified state mod is protected (by an item etc.)
	inline bool getProtectedStateMod( int mod ) {
		return ( protStateMod & ( 1 << mod ) ? true : false );
	}

	inline void setName( char *s ) {
		strncpy( name, s, 254 ); name[254] = '\0';
	}
	void setCharacter( Character *c );
	/// Sets the character's level.
	inline void setLevel( int n ) {
		level = ( n < 0 ? 0 : n ); evalSpecialSkills();
	}
	void setExp();
	/// Amount of experience points.
	inline void setExp( int n ) {
		experience = ( n < 0 ? 0 : n ); evalSpecialSkills();
	}
	/// Amount of gold.
	inline void setMoney( int n ) {
		money = n; evalSpecialSkills();
	}
	/// Sets amount of hit points.
	inline void setHp( int n ) {
		hp = ( n < 0 ? 0 : n ); evalSpecialSkills();
	}
	/// Sets amount of magic points.
	inline void setMp( int n ) {
		mp = ( n < 0 ? 0 : n ); evalSpecialSkills();
	}
	/// Sets thirst level.
	inline void setThirst( int n )  {
		if ( n < 0 )n = 0; if ( n > 10 )n = 10; thirst = n; evalSpecialSkills();
	}
	/// Sets hunger level.
	inline void setHunger( int n )  {
		if ( n < 0 )n = 0; if ( n > 10 )n = 10; hunger = n; evalSpecialSkills();
	}
	void setHp();
	void setMp();

	void setSkill( int index, int value );
	void setSkillBonus( int index, int value );
	/// Additional skill bonus.
	inline int getSkillBonus( int index ) {
		return skillBonus[index];
	}
	void setSkillMod( int index, int value );
	/// Skill modification (after levelup) that has not yet been applied.
	inline int getSkillMod( int index ) {
		return skillMod[index];
	}
	void applySkillMods();
	/// Still skill points left?
	bool getHasAvailableSkillPoints() {
		return hasAvailableSkillPoints;
	}

	inline int getAvailableSkillMod() {
		return availableSkillMod;
	}
	inline void setAvailableSkillMod( int n ) {
		availableSkillMod = n;
		if ( !hasAvailableSkillPoints && n > 0 ) hasAvailableSkillPoints = true;
	}
	void setStateMod( int mod, bool setting );
	void setProtectedStateMod( int mod, bool setting );

	// return the initiative for a battle round, the lower the faster the attack
	int getInitiative( int *max = NULL );

	// take damage
	// return true if the creature dies
	bool takeDamage( float damage, int effect_type = Constants::EFFECT_GLOW, GLuint delay = 0 );

	void resurrect( int rx, int ry );

	// returns exp gained
	int addExperience( Creature *creature_killed );

	int addExperience( int exp );

	int addExperienceWithMessage( int exp );

	// get angle to target creature
	float getTargetAngle();

	// returns coins gained
	int addMoney( Creature *creature_killed );

	void getDetailedDescription( std::string& s );

	int getMaxProjectileCount( Item *item );

	std::vector<RenderedProjectile*> *getProjectiles();

	void usePotion( Item *item );

	inline void setBonusArmor( int n ) {
		bonusArmor = n; if ( bonusArmor < 0 ) bonusArmor = 0; recalcAggregateValues();
	}
	inline int getBonusArmor() {
		return bonusArmor;
	}

	bool addSpell( Spell *spell );
	bool isSpellMemorized( Spell *spell );
	/// Number of spells the creature knows.
	inline int getSpellCount() {
		return static_cast<int>( spells.size() );
	}
	inline Spell *getSpell( int index ) {
		return spells[index];
	}

	void setAction( int action, Item *item = NULL, Spell *spell = NULL, SpecialSkill *skill = NULL );
	/// Current creature action (item, spell, capability).
	inline int getAction() {
		return action;
	}
	/// The item used for the current action.
	inline Item *getActionItem() {
		return actionItem;
	}
	/// The spell used for the current action.
	inline Spell *getActionSpell() {
		return actionSpell;
	}
	/// The capability used for the current action.
	inline SpecialSkill *getActionSkill() {
		return actionSkill;
	}

	inline bool isBusy() {
		return hasTarget() || getAction() > Constants::ACTION_NO_ACTION || getActionItem() || getActionSkill() || getActionSpell();
	}
	

	// handling battle targets
	inline bool hasTarget() {
		return targetCreature || targetItem || targetX || targetY || targetZ;
	}
	bool isTargetValid();
	bool canAttack( RenderedCreature *creature, int *cursor = NULL );
	void cancelTarget();
	//void makeTargetRetaliate();
	void decideAction();

	bool attackClosestTarget();
	bool castHealingSpell();

	float getDistanceToTarget( RenderedCreature *creature = NULL );
	float getDistance( RenderedCreature *other );
	float getDistanceToSel();

	bool isWithPrereq( Spell *spell );
	Creature *findClosestTargetWithPrereq( Spell *spell );
	bool useOffensiveSpell( Spell *spell, float dist, Creature *possibleTarget );

	/// Schedules state mod effects.
	inline void setStateModEvent( int mod, Event *event ) {
		stateModEventMap[mod] = event;
	}
	inline Event *getStateModEvent( int mod ) {
		return( stateModEventMap.find( mod ) == stateModEventMap.end() ? NULL : stateModEventMap[mod] );
	}


	// ======================================
	// Combat methods
	void getCth( Item *weapon, float *cth, float *skill, bool showDebug = true );

	float getDodge( Creature *attacker, Item *weapon = NULL );

	float getAttack( Item *weapon,
	                 float *maxP = NULL,
	                 float *minP = NULL,
	                 bool callScript = false );

	float getParry( Item **parryItem );

	float getArmor( float *armor, float *dodgePenalty, int damageType, Item *vsWeapon = NULL );

	float getAttackerStateModPercent();
	float getDefenderStateModPercent( bool magical );
	float rollMagicDamagePercent( Item *item );
	float getMaxAP();
	char *canEquipItem( Item *item, bool interactive = true );
	bool rollSkill( int skill, float luckDiv = 0.0f );
	bool rollSecretDoor( Location *pos );
	void resetSecretDoorAttempts();
	bool rollTrapFind( Trap *trap );
	void resetTrapFindAttempts();
	void rollPerception();
	void disableTrap( Trap *trap );
	char *getType();

	inline bool isInventoryArranged() {
		return inventoryArranged;
	}
	inline void setInventoryArranged( bool b ) {
		inventoryArranged = b;
	}

protected:

	void evalTrap();

	/**
	 * Recalculate skills when stats change.
	 */
	void skillChanged( int index, int oldValue, int newValue );


	void calcArmor( int damageType,
	                float *armorP,
	                float *dodgePenalty,
	                bool callScript = false );


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
	                         char const* debugMessage );
};


#endif

