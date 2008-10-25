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

  This class is both the UI representation (shape) and state (backpack, etc.) of a character or monster.
  All creatures of the same type (character or monster) share the same instance of the prototype class (Character of Monster)

  */

// how many times to attempt to move to range
#define MAX_FAILED_MOVE_ATTEMPTS 10

/// Contains the location of an item in the backpack or where it is equipped.

class BackpackInfo {
public:
	int backpackIndex;
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

	// backpack
	Item *backpack;
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
	bool backpackSorted;
	std::map<Item*, BackpackInfo*> invInfos;
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

	/// The session object used to create this class instance.
	inline Session* getSession() {
		return session;
	}

	/// The current battle turn.
	inline Battle *getBattle() {
		return battle;
	}

	inline PathManager* getPathManager() {
		return pathManager;
	}

	// #######################
	// #### CREATURE TYPE ####
	// #######################
	
	inline Item *getBackpack() { return backpack; }	

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

	/// Is it controlled by the computer?
	inline bool isAutoControlled() {
		return( !isPartyMember() || getStateMod( StateMod::possessed ) );
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

	// ####################
	// #### APPEARANCE ####
	// ####################

	/// The name of the 3D model used for the creature.
	inline char *getModelName() {
		return model_name;
	}

	/// The 3D shape of the creature.
	inline GLShape *getShape() {
		return shape;
	}

	inline char *getSkinName() {
		return skin_name;
	}

	inline int getCharacterModelInfoIndex() {
		return character_model_info_index;
	}

	void draw();

	void drawMoviePortrait( int width, int height );
	void drawPortrait( int width, int height, bool inFrame = false );

	inline void setPortraitTextureIndex( int n ) {
		this->portraitTextureIndex = n;
	}

	inline int getPortraitTextureIndex() {
		return portraitTextureIndex;
	}

	inline void setSex( int n ) {
		this->sex = n;
	}

	/// Male/female?
	inline int getSex() {
		return this->sex;
	}

	// ###################
	// #### ANIMATION ####
	// ###################

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

	/// Currently moving?
	inline bool isMoving() {
		return moving;
	}

	inline void setMoving( bool b ) {
		moving = b;
	}

	inline GLfloat getAngle() {
		return angle;
	}

	inline void setFacingDirection( int direction ) {
		this->facingDirection = direction;
	}

	/// The direction the creature faces (U, D, L, R).
	inline int getFacingDirection() {
		return this->facingDirection;
	}

	inline Uint16 getDir() {
		return dir;
	}

	/// Direction of movement (U, D, L, R).
	inline void setDir( Uint16 dir ) {
		this->dir = dir;
	}

	// ########################
	// #### MAP NAVIGATION ####
	// ########################

	/**
	The movement functions return true if movement has occured, false if it has not.
	 */

	bool move( Uint16 dir );
	void switchDirection( bool force );
	bool follow( Creature *leader );

	/// Makes the creature move out of another creature's path.
	void moveAway( Creature *other );
	Location *moveToLocator();

	void stopMoving();
	bool anyMovesLeft();

	void setMotion( int motion );

	/// The current mode of motion (stand, run, loiter around...)
	inline int getMotion() {
		return this->motion;
	}

	inline void setFormation( int formation ) {
		this->formation = formation;
	}

	/// Formations are currently unused.
	inline int getFormation() {
		return formation;
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

	bool setSelCreature( Creature* creature, float range, bool cancelIfNotPossible = false );

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

	bool setSelXY( int x, int y, bool cancelIfNotPossible = false );

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

	float getDistanceToTarget( RenderedCreature *creature = NULL );
	float getDistanceToSel();
	float getDistance( RenderedCreature *other );

	// get angle to target creature
	float getTargetAngle();

	/// Unused.
	inline void setMapChanged() {
		mapChanged = true;
	}

	// ###############
	// #### AUDIO ####
	// ###############

	void playCharacterSound( int soundType, int panning );
	void playFootstep();

	// ##########################
	// #### BASIC ATTRIBUTES ####
	// ##########################

	/// The creature's UNLOCALIZED name.
	inline char *getName() {
		return name;
	}

	inline void setName( char *s ) {
		strncpy( name, s, 254 ); name[254] = '\0';
	}

	char *getType();
	void getDetailedDescription( std::string& s );

	/// The creature's level.
	inline int getLevel() {
		return level;
	}

	inline void setLevel( int n ) {
		level = ( n < 0 ? 0 : n ); evalSpecialSkills();
	}

	/// Experience points needed for next level.
	inline int getExpOfNextLevel() {
		return expOfNextLevel;
	}

	/// Experience points.
	inline int getExp() {
		return experience;
	}

	void setExp();

	/// Amount of experience points.
	inline void setExp( int n ) {
		experience = ( n < 0 ? 0 : n ); evalSpecialSkills();
	}

	int addExperience( int exp );
	int addExperienceWithMessage( int exp );

	void changeProfession( Character *character );

	/// Amount of money the creature holds.
	inline int getMoney() {
		return money;
	}

	inline void setMoney( int n ) {
		money = n; evalSpecialSkills();
	}

	/// Current hit points.
	inline int getHp() {
		return hp;
	}

	void setHp();

	inline void setHp( int n ) {
		hp = ( n < 0 ? 0 : n ); evalSpecialSkills();
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

	void setMp();

	inline void setMp( int n ) {
		mp = ( n < 0 ? 0 : n ); evalSpecialSkills();
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

	inline void setThirst( int n )  {
		if ( n < 0 )n = 0; if ( n > 10 )n = 10; thirst = n; evalSpecialSkills();
	}

	/// Current hunger level.
	inline int getHunger() {
		return hunger;
	}

	inline void setHunger( int n )  {
		if ( n < 0 )n = 0; if ( n > 10 )n = 10; hunger = n; evalSpecialSkills();
	}

	//#### INTERNAL creature types ####

	void setNpcInfo( NpcInfo *npcInfo );

	/// Extra info for edited NPCs.
	inline NpcInfo *getNpcInfo() {
		return npcInfo;
	}

	/// Gets character info.
	inline Character *getCharacter() {
		return character;
	}

	void setCharacter( Character *c );

	/// Gets monster/NPC info.
	inline Monster *getMonster() {
		return monster;
	}

	// ################
	// #### SKILLS ####
	// ################

	/// Returns the value of the specified skill.
	inline int getSkill( int index, bool includeMod = true ) {
		return skills[index] + skillBonus[index] + ( includeMod ? skillMod[index] : 0 );
	}

	void setSkill( int index, int value );

	/// Additional skill bonus.
	inline int getSkillBonus( int index ) {
		return skillBonus[index];
	}

	void setSkillBonus( int index, int value );

	/// Skill modification (after levelup) that has not yet been applied.
	inline int getSkillMod( int index ) {
		return skillMod[index];
	}

	void setSkillMod( int index, int value );

	inline int getAvailableSkillMod() {
		return availableSkillMod;
	}

	inline void setAvailableSkillMod( int n ) {
		availableSkillMod = n;
		if ( !hasAvailableSkillPoints && n > 0 ) hasAvailableSkillPoints = true;
	}

	void applySkillMods();

	/// Still skill points left?
	bool getHasAvailableSkillPoints() {
		return hasAvailableSkillPoints;
	}

	bool rollSkill( int skill, float luckDiv = 0.0f );
	void rollPerception();

	bool rollSecretDoor( Location *pos );
	void resetSecretDoorAttempts();

	bool rollTrapFind( Trap *trap );
	void resetTrapFindAttempts();
	void disableTrap( Trap *trap );

	// ##############################
	// #### SPECIAL CAPABILITIES ####
	// ##############################

	/// Does it have this special capability?
	inline bool hasCapability( char *name ) {
		std::string skillName = name;
		return specialSkillNames.find( skillName ) != specialSkillNames.end();
	}

	/// Does it have this special capability?
	inline bool hasSpecialSkill( SpecialSkill *ss ) {
		return specialSkills.find( ss ) != specialSkills.end();
	}

	char *useSpecialSkill( SpecialSkill *specialSkill, bool manualOnly );
	void evalSpecialSkills();

	void applyRecurringSpecialSkills();
	float applyAutomaticSpecialSkills( int event, char *varName, float value );

	// ####################
	// #### STATE MODS ####
	// ####################

	/// Returns the active state of a state mod.
	inline bool getStateMod( int mod ) {
		return ( stateMod & ( 1 << mod ) ? true : false );
	}

	void setStateMod( int mod, bool setting );

	/// Returns whether the specified state mod is protected (by an item etc.)
	inline bool getProtectedStateMod( int mod ) {
		return ( protStateMod & ( 1 << mod ) ? true : false );
	}

	void setProtectedStateMod( int mod, bool setting );

	/// Schedules state mod effects.
	inline void setStateModEvent( int mod, Event *event ) {
		stateModEventMap[mod] = event;
	}

	inline Event *getStateModEvent( int mod ) {
		return( stateModEventMap.find( mod ) == stateModEventMap.end() ? NULL : stateModEventMap[mod] );
	}

	// ###############
	// #### MAGIC ####
	// ###############

	bool addSpell( Spell *spell );
	bool isSpellMemorized( Spell *spell );

	/// Number of spells the creature knows.
	inline int getSpellCount() {
		return static_cast<int>( spells.size() );
	}

	inline Spell *getSpell( int index ) {
		return spells[index];
	}

	inline void setDeityIndex( int n ) {
		deityIndex = n;
	}
	/// Deity of the creature.
	inline int getDeityIndex() {
		return deityIndex;
	}

	inline void setLastEnchantDate( Date date ) {
		lastEnchantDate = date;
	}

	/// Last point in time when the creature tried to enchant something.
	inline Date getLastEnchantDate() {
		return lastEnchantDate;
	}

	// ##################
	// #### BACKPACK ####
	// ##################

	float backpackWeight;

	/// Total weight of the backpack.
	inline float getBackpackWeight() {
		return backpackWeight;
	}

	float getMaxBackpackWeight();
	/// Number of items carried in backpack.
	int getBackpackContentsCount();

	/// The item at a specified backpack index.
	Item *getBackpackItem( int backpackIndex );

	bool addToBackpack( Item *item, bool force = false );
	Item *removeFromBackpack( int backpackIndex );
	void debugBackpack();
	int findInBackpack( Item *item );
	bool isItemInBackpack( Item *item );

	Item *getEquippedItem( int equipIndex );
	void equipFromBackpack( int backpackIndex, int equipIndexHint = -1 );
	int doff( int backpackIndex );
	bool isEquippedWeapon( int equipIndex );

	bool isEquipped( Item *item );
	bool isEquipped( int backpackIndex );
	int getEquippedIndex( int backpackIndex );

	char *canEquipItem( Item *item, bool interactive = true );

	inline bool isBackpackSorted() {
		return backpackSorted;
	}

	inline void setBackpackSorted( bool b ) {
		backpackSorted = b;
	}

	void pickUpOnMap( RenderedItem *item );

	bool eatDrink( int backpackIndex );
	bool eatDrink( Item *item );
	void usePotion( Item *item );

	bool removeCursedItems();

	BackpackInfo *getBackpackInfo( Item *item, bool createIfMissing = false );

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

	// ################
	// #### BATTLE ####
	// ################

	inline void setLastTurn( int n ) {
		lastTurn = n;
	}

	inline int getLastTurn() {
		return lastTurn;
	}

	// speed/initiative related stuff.

	float getAttacksPerRound( Item *item = NULL );
	float getMaxAP();
	// return the initiative for a battle round, the lower the faster the attack
	int getInitiative( int *max = NULL );
	// FIXME: should be modified by equipped items (boots of speed, etc.)

	inline int getSpeed() {
		return speed;
	}

	float getWeaponAPCost( Item *item, bool showDebug = true );

	// return the best equipped weapon that works on this distance,
	// or NULL if none are available
	Item *getBestWeapon( float dist, bool callScript = false );

	// Attack related.

	/// The active weapon.
	inline int getPreferredWeapon() {
		return preferredWeapon;
	}

	inline void setPreferredWeapon( int n ) {
		preferredWeapon = n;
	}

	bool nextPreferredWeapon();

	bool canAttack( RenderedCreature *creature, int *cursor = NULL );
	float getAttack( Item *weapon, float *maxP = NULL, float *minP = NULL, bool callScript = false );
	void getCth( Item *weapon, float *cth, float *skill, bool showDebug = true );
	float getAttackerStateModPercent();
	float rollMagicDamagePercent( Item *item );

	std::vector<RenderedProjectile*> *getProjectiles();
	int getMaxProjectileCount( Item *item );

	// Defense related.

	float getParry( Item **parryItem );
	float getDodge( Creature *attacker, Item *weapon = NULL );

	float getArmor( float *armor, float *dodgePenalty, int damageType, Item *vsWeapon = NULL );
	inline int getBonusArmor() {
		return bonusArmor;
	}
	inline void setBonusArmor( int n ) {
		bonusArmor = n; if ( bonusArmor < 0 ) bonusArmor = 0; recalcAggregateValues();
	}

	float getDefenderStateModPercent( bool magical );

	// take damage
	// return true if the creature dies
	bool takeDamage( float damage, int effect_type = Constants::EFFECT_GLOW, GLuint delay = 0 );

	// Action scheduler.

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

	// Target handling.

	bool isTargetValid();

	inline bool hasTarget() {
		return targetCreature || targetItem || targetX || targetY || targetZ;
	}

	void cancelTarget();

	/// Are we doing something (not being idle?)
	inline bool isBusy() {
		return hasTarget() || getAction() > Constants::ACTION_NO_ACTION || getActionItem() || getActionSkill() || getActionSpell();
	}

	// The descriptions of why you died.

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

	// Misc. methods.

	// returns exp gained
	int addExperience( Creature *creature_killed );

	// returns coins gained
	int addMoney( Creature *creature_killed );

	// ###################
	// #### AI BATTLE ####
	// ###################

	void decideAction();

	Creature *getClosestTarget();
	bool attackClosestTarget();

	Creature *getRandomTarget();
	bool attackRandomTarget();

	bool isWithPrereq( Item *item );
	bool isWithPrereq( Spell *spell );
	Creature *findClosestTargetWithPrereq( Spell *spell );

	bool castHealingSpell();
	bool castOffensiveSpell();
	bool useMagicItem();

	// ##############
	// #### MISC ####
	// ##############

	CreatureInfo *save();
	static Creature *load( Session *session, CreatureInfo *info );

	void resurrect( int rx, int ry );

	/// Unused.
	inline void setLastTick( GLint n ) {
		this->lastTick = n;
	}
	/// Unused.
	inline GLint getLastTick() {
		return lastTick;
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

