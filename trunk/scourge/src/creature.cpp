/***************************************************************************
                          creature.cpp  -  description
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

#include "creature.h"
#include "item.h"
#include "render/renderlib.h"
#include "session.h"
#include "shapepalette.h"
#include "events/event.h"
#include "events/potionexpirationevent.h"
#include "events/statemodexpirationevent.h"
#include "events/thirsthungerevent.h"
#include "sqbinding/sqbinding.h"
#include "debug.h"

using namespace std;

bool loading = false;

//#define DEBUG_CAPABILITIES

#define FAST_SPEED 1

#define MOVE_DELAY 7

// at this fps, the players step 1 square                     
#define FPS_ONE 10.0f

// how fast to turn                        
#define TURN_STEP_COUNT 5

// how far to move away when in the player's way
#define AWAY_DISTANCE 8

// how close to stay to the player
#define CLOSE_DISTANCE 8

/**
 * Formations are defined by 4 set of coordinates in 2d space.
 * These starting positions assume dir=Constants::MOVE_UP
 */
static const Sint16 layout[][4][2] = {
  { {0, 0}, {-1, 1}, {1, 1}, {0, 2}},  // DIAMOND_FORMATION
  { {0, 0}, {-1, 1}, {0, 1}, {-1, 2}},  // STAGGERED_FORMATION
  { {0, 0}, {1, 0}, {1, 1}, {0, 1}},   // SQUARE_FORMATION
  { {0, 0}, {0, 1}, {0, 2}, {0, 3}},   // ROW_FORMATION
  { {0, 0}, {-2, 2}, {0, 2}, {2, 2}},  // SCOUT_FORMATION
  { {0, 0}, {-1, 1}, {1, 1}, {0, 3}}   // CROSS_FORMATION
};

// Describes monster toughness in general
typedef struct _MonsterToughness {
  float minSkillBase, maxSkillBase;
  float minHpMpBase, maxHpMpBase;
  float armorMisfuction;
} MonsterToughness;

// goes from not very tough to tough 
MonsterToughness monsterToughness[] = {
  {  .5f,  0.8f,     .7f,      1,  .33f },
  { .75f,  0.9f,    .75f,      1,  .15f },
  {  .8f,     1,    .75f,  1.25f,   .5f }
};

#define roll(min, max) ( ( ( max - min ) * rand() / RAND_MAX ) + min )

Creature::Creature(Session *session, Character *character, char *name, int sex, int character_model_info_index) : RenderedCreature( session->getPreferences(), session->getShapePalette(), session->getMap() ) {
  this->session = session;
  this->character = character;
  this->monster = NULL;
  setName( name );
  this->character_model_info_index = character_model_info_index;
  this->model_name = session->getShapePalette()->getCharacterModelInfo( sex, character_model_info_index )->model_name;
  this->skin_name = session->getShapePalette()->getCharacterModelInfo( sex, character_model_info_index )->skin_name;
  this->originalSpeed = this->speed = 5; // start neutral speed
  this->motion = Constants::MOTION_MOVE_TOWARDS;  
  this->armor=0;
  this->armorChanged = true;
  this->bonusArmor=0;
  this->thirst=10;
  this->hunger=10;  
  this->shape = session->getShapePalette()->getCreatureShape(model_name, skin_name, session->getShapePalette()->getCharacterModelInfo( sex, character_model_info_index )->scale);
	this->sex = sex;
  commonInit();  
}

Creature::Creature(Session *session, Monster *monster, GLShape *shape, bool initMonster ) : RenderedCreature( session->getPreferences(), session->getShapePalette(), session->getMap() ) {
  this->session = session;
  this->character = NULL;
  this->monster = monster;
  setName( monster->getDisplayName() );
  this->model_name = monster->getModelName();
  this->skin_name = monster->getSkinName();
  this->originalSpeed = this->speed = monster->getSpeed();
  this->motion = Constants::MOTION_LOITER;
  this->armor = monster->getBaseArmor();
  this->armorChanged = true;
  this->bonusArmor=0;
  this->shape = shape;  
	this->sex = Constants::SEX_MALE;
  commonInit();
	this->level = monster->getLevel();
	if( initMonster ) monsterInit();
}

void Creature::commonInit() {

	this->causeOfDeath[0] = 0;
  ((AnimatedShape*)shape)->setCreatureSpeed( speed );

	for( int i = 0; i < RpgItem::DAMAGE_TYPE_COUNT; i++ ) {
		lastArmor[i] = lastArmorSkill[i] = lastDodgePenalty[i] = 0;
	}
  for( int i = 0; i < 12; i++ ) quickSpell[ i ] = NULL;
  this->lastMove = 0;
  this->moveCount = 0;
  this->x = this->y = this->z = 0;
  this->dir = Constants::MOVE_UP;
  this->next = NULL;
  this->formation = DIAMOND_FORMATION;
  this->index = 0;
  this->tx = this->ty = -1;  
  this->selX = this->selY = -1;
	this->cantMoveCounter = 0;
  this->bestPathPos = 0;
  this->inventory_count = 0;
  this->preferredWeapon = -1;	
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    equipped[i] = MAX_INVENTORY_SIZE;
  }
  for(int i = 0; i < Skill::SKILL_COUNT; i++) {
    skillBonus[i] = skillsUsed[i] = skillMod[i]= 0;
  }
  this->stateMod = 0;
  this->protStateMod = 0;
  this->level = 1;
  this->experience = 0;
  this->hp = 0;
  this->mp = 0;
  this->startingHp = 0;
  this->startingMp = 0;
  this->ac = 0;
  this->targetCreature = NULL;
  this->targetX = this->targetY = this->targetZ = 0;
  this->targetItem = NULL;
  this->lastTick = 0;
  this->lastTurn = 0;
  this->facingDirection = Constants::MOVE_UP; // good init ?
  this->failedToMoveWithinRangeAttemptCount = 0;
  this->action = Constants::ACTION_NO_ACTION;
  this->actionItem = NULL;
  this->actionSpell = NULL;
  this->actionSkill = NULL;
  this->preActionTargetCreature = NULL;
  this->angle = this->wantedAngle = this->angleStep = 0;
  this->portraitTextureIndex = 0;
  this->deityIndex = -1;
	this->availableSkillMod = 0;

  // Yes, monsters have inventory weight issues too
  inventoryWeight =  0.0f;  
  for(int i = 0; i < inventory_count; i++) {
    inventoryWeight += inventory[i]->getWeight();
  }  
  this->money = this->level * (int)(10.0f * rand()/RAND_MAX);
  calculateExpOfNextLevel();
  this->battle = new Battle(session, this);
  lastEnchantDate.setDate(-1, -1, -1, -1, -1, -1);

  this->npcInfo = NULL;
  this->mapChanged = false;
	this->moving = false;

  evalSpecialSkills();
}

Creature::~Creature(){
	// cancel this creature's events
	session->getParty()->getCalendar()->cancelEventsForCreature( this );

	// now delete the creature
  session->getGameAdapter()->removeBattle(battle);
  delete battle;
	// delete the md2/3 shape
  session->getShapePalette()->
		decrementSkinRefCountAndDeleteShape( model_name, 
																				 skin_name, 
																				 shape,
																				 monster );
}

void Creature::changeProfession( Character *c ) {
	char message[120];

	// boost skills
	for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
		int maxValue = c->getSkill( i );
		if( maxValue > 0 ) {
			int oldValue = character->getSkill( i );
			int newValue = getSkill( i ) + ( oldValue > 0 ? maxValue - oldValue : maxValue );
			setSkill( i, newValue );
			
			sprintf( message, _( "%1$s's skill in %2$s has increased." ), getName(), Skill::skills[ i ]->getDisplayName() );
			session->getMap()->addDescription( message );
		}
	}

	// remove forbidden items
	for( int i = 0; i < Constants::INVENTORY_COUNT; i++ ) {
		if( equipped[i] < MAX_INVENTORY_SIZE ) {
			Item *item = inventory[ equipped[i] ];
			if( !c->canEquip( item->getRpgItem() ) ) {
				doff( equipped[i] );
				sprintf( message, _( "%1$s is not allowed to equip %2$s." ), getName(), item->getName() );
				session->getMap()->addDescription( message );				 
			}
		}
	}

	// add capabilities?


	this->character = c;
	setHp();
	setMp();
}

CreatureInfo *Creature::save() {
  CreatureInfo *info = (CreatureInfo*)malloc(sizeof(CreatureInfo));
  info->version = PERSIST_VERSION;
  strncpy((char*)info->name, getName(), 254);
  info->name[254] = 0;
  if(isMonster()) {
    strcpy((char*)info->character_name, "");
    strcpy((char*)info->monster_name, monster->getType());
    info->character_model_info_index = 0;
  } else {
    strcpy((char*)info->character_name, character->getName());
    strcpy((char*)info->monster_name, "");    
    info->character_model_info_index = character_model_info_index;
  }
  info->deityIndex = deityIndex;
  info->hp = hp;
  info->mp = mp;
  info->exp = experience;
  info->level = level;
  info->money = money;
  info->stateMod = stateMod;
  info->protStateMod = protStateMod;
  info->x = toint(x);
  info->y = toint(y);
  info->z = toint(z);
  info->dir = dir;
  info->speed = originalSpeed;
  info->motion = motion;
  info->sex = sex;
  info->armor = 0;
  info->bonusArmor = bonusArmor;
  //info->bonusArmor = 0;
  info->thirst = thirst;
  info->hunger = hunger;
	info->availableSkillPoints = availableSkillMod;
	for(int i = 0; i < Skill::SKILL_COUNT; i++) {
		info->skills[i] = skills[i];
		info->skillBonus[i] = skillBonus[i];
		info->skillsUsed[i] = skillsUsed[i];
		info->skillMod[i] = skillMod[i];
  }
  info->portraitTextureIndex = portraitTextureIndex;

  // inventory
  info->inventory_count = inventory_count;
  for(int i = 0; i < inventory_count; i++) {
    info->inventory[i] = inventory[i]->save();
    //info->containedItems[i] = inventory[i]->saveContainedItems();
  }
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    info->equipped[i] = equipped[i];
  }

  // spells
  int count = 0;
  for(int i = 0; i < MagicSchool::getMagicSchoolCount(); i++) {
    MagicSchool *school = MagicSchool::getMagicSchool(i);
    for(int t = 0; t < school->getSpellCount(); t++) {
      Spell *spell = school->getSpell(t);
      if(isSpellMemorized(spell)) {
        strcpy((char*)info->spell_name[count++], spell->getName());
      }
    }
  }
  info->spell_count = count;

  for( int i = 0; i < 12; i++ ) {
    strcpy( (char*)info->quick_spell[ i ], 
            ( getQuickSpell( i ) ? getQuickSpell( i )->getName() : "" ) );
  }

  return info;
}

Creature *Creature::load(Session *session, CreatureInfo *info) {
  Creature *creature = NULL;

  if(!strlen((char*)info->character_name)) {
    
    Monster *monster = Monster::getMonsterByName( (char*)info->monster_name );
    if( !monster ) {
      cerr << "Error: can't find monster: " << (char*)info->monster_name << endl;
      return NULL;
    }
    GLShape *shape = session->getShapePalette()->
      getCreatureShape( monster->getModelName(), 
                        monster->getSkinName(), 
                        monster->getScale(),
                        monster );
    creature = session->newCreature( monster, shape, true );
		creature->setName( (char*)info->name ); // important for npc-s
		
		// fixme: throw away this code when saving stats_mods and calendar events is implemented
		// for now, set a monsters stat mods as declared in creatures.txt
		creature->stateMod = monster->getStartingStateMod();
  } else {
    creature = new Creature( session, 
                             Character::getCharacterByName((char*)info->character_name), 
                             strdup((char*)info->name),
														 info->sex,
                             info->character_model_info_index );
  }
  
  // don't recalculate skills
  // NOTE: don't call return until loading=false.
  loading = true;
  
//  cerr << "*** LOAD: creature=" << info->name << endl;
  creature->setDeityIndex( info->deityIndex );
  creature->setHp( info->hp );
  creature->setMp( info->mp );
  creature->setExp( info->exp );
  creature->setLevel( info->level );
  creature->setMoney( info->money );
  creature->moveTo( info->x, info->y, info->z );
  creature->setDir( info->dir );
  //creature->setSpeed( info->speed );
  //creature->setMotion( info->motion );
  //creature->setArmor( info->armor );
  
  // info->bonusArmor: can't be used until calendar is also persisted
  //creature->setBonusArmor( info->bonusArmor );

  creature->setThirst( info->thirst );
  creature->setHunger( info->hunger );
	creature->setAvailableSkillMod( info->availableSkillPoints );	
	
	for(int i = 0; i < Skill::SKILL_COUNT; i++) {
		creature->skills[i] = info->skills[i];
		// Don't set skillBonus: it's reconstructed via the inventory.
		//creature->skillBonus[i] = info->skillBonus[i];
		// Don't set skillUsed: it's not used.
		//creature->skillsUsed[i] = info->skillsUsed[i];
		creature->skillMod[i] = info->skillMod[i];
  }	
  
  // stateMod and protStateMod not useful until calendar is also persisted
  //creature->stateMod = info->stateMod;
  //creature->protStateMod = info->protStateMod;
  // these two don't req. events:
  if(info->stateMod & (1 << StateMod::dead)) creature->setStateMod(StateMod::dead, true);
  //if(info->stateMod & (1 << Constants::leveled)) creature->setStateMod(Constants::leveled, true);

  // inventory
  //creature->inventory_count = info->inventory_count;
  for(int i = 0; i < (int)info->inventory_count; i++) {
    Item *item = Item::load( session, info->inventory[i] );
    if(item) creature->addInventory( item, true );
  }
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if(info->equipped[i] < MAX_INVENTORY_SIZE) { 
      creature->equipInventory(info->equipped[i]);
    } else {
      creature->equipped[i] = info->equipped[i];
    }
  }

  creature->portraitTextureIndex = info->portraitTextureIndex;
  if( creature->portraitTextureIndex >= session->getShapePalette()->getPortraitCount( creature->getSex() ) ) 
    creature->portraitTextureIndex = session->getShapePalette()->getPortraitCount( creature->getSex() ) - 1;

  // spells
  for(int i = 0; i < (int)info->spell_count; i++) {
    Spell *spell = Spell::getSpellByName( (char*)info->spell_name[i] );
    creature->addSpell( spell );
  }

  for( int i = 0; i < 12; i++ ) {
    if( strlen( (char*)info->quick_spell[ i ] ) ) {
      Spell *spell = Spell::getSpellByName( (char*)info->quick_spell[ i ] );
      if( spell ) creature->setQuickSpell( i, spell );
      else {
        SpecialSkill *special = SpecialSkill::findByName( (char*)info->quick_spell[ i ], false );
        if( special ) creature->setQuickSpell( i, special );
        else {
          // it's an item. Find it          
          for( int t = 0; t < creature->getInventoryCount(); t++ ) {
            Item *item = creature->getInventory( t );            
            if( !strcmp( item->getName(), (char*)info->quick_spell[ i ] ) ) {
              creature->setQuickSpell( i, (Storable*)item );
              break;
            }
          }
        }
      }
    }
  }

  creature->calculateExpOfNextLevel();

  creature->evalSpecialSkills();
  
  // recalculate skills from now on
  loading = false;  

  return creature;
}

char *Creature::getType() {
	return( monster ? monster->getType() : character->getName() );
}

void Creature::calculateExpOfNextLevel() {
  if(isMonster()) return;
  expOfNextLevel = 0;
  for(int i = 0; i < level; i++) {
    expOfNextLevel += ((i + 1) * character->getLevelProgression());
  }
}

void Creature::switchDirection(bool force) {
  int n = (int)(10.0f * rand()/RAND_MAX);
  if(n == 0 || force) {
    int dir = (int)(4.0f * rand()/RAND_MAX);
    switch(dir) {
    case 0: setDir(Constants::MOVE_UP); break;
    case 1: setDir(Constants::MOVE_DOWN); break;
    case 2: setDir(Constants::MOVE_LEFT); break;
    case 3: setDir(Constants::MOVE_RIGHT); break;
    }
  }
}

// moving monsters only
bool Creature::move(Uint16 dir) {
  //if(character) return false;

  // is monster (or npc) doing something else?
  if( ((AnimatedShape*)getShape())->getCurrentAnimation() != MD2_RUN ) return false;

  switchDirection(false);

  // a hack for runaway creatures
  if(!(x > 10 && x < MAP_WIDTH - 10 &&
       y > 10 && y < MAP_DEPTH - 10)) {
    if(monster) cerr << "hack for " << getName() << endl;
    switchDirection(true);
    return false;
  }

  GLfloat nx = x;
  GLfloat ny = y;
  GLfloat nz = z;
  GLfloat step = getStep();
  switch(dir) {
  case Constants::MOVE_UP:    
    ny = y - step;
    break;
  case Constants::MOVE_DOWN:    
    ny = y + step;
    break;
  case Constants::MOVE_LEFT:
    nx = x - step;
    break;
  case Constants::MOVE_RIGHT:    
    nx = x + step;
    break;
  }
  setFacingDirection(dir);
  
  if(!session->getMap()->moveCreature(toint(x), toint(y), toint(z), 
                                      toint(nx), toint(ny), toint(nz), this)) {
    ((AnimatedShape*)shape)->setDir(dir);
    if( session->getMap()->getHasWater() &&
        !( toint(x) == toint(nx) &&
           toint(y) == toint(ny) ) ) {
      session->getMap()->startEffect( toint(getX() + getShape()->getWidth() / 2), 
                                      toint(getY() - getShape()->getDepth() / 2), 0,
                                      Constants::EFFECT_RIPPLE, (Constants::DAMAGE_DURATION * 4), 
                                      getShape()->getWidth(), getShape()->getDepth() );
    }
    moveTo(nx, ny, nz);
    setDir(dir);        
    return true;
  } else {
    switchDirection(true);
    return false;
  }
}

void Creature::setTargetCreature( Creature *c, bool findPath ) { 
  targetCreature = c; 
  if( findPath ) {
    if( !setSelXY( toint( c->getX() + ( (float)c->getShape()->getWidth() / 2.0f ) ), 
                   toint( c->getY() - ( (float)c->getShape()->getDepth() / 2.0f ) ),
                   false ) ) {
      // FIXME: should mark target somehow. Path alg. cannot reach it; blocked by something.
      // Keep the target creature anyway.
      if( session->getPreferences()->isBattleTurnBased() ) {
        //cerr << "Can't find path to target creature, checking bounds." << endl;
        for( int xx = 0; xx < targetCreature->getShape()->getWidth(); xx++ ) {
          for( int yy = 0; yy < targetCreature->getShape()->getDepth(); yy++ ) {
            if( setSelXY( toint( c->getX() + xx ), toint( c->getY() - yy ) ) ) {
              //cerr << "...found a way!" << endl;
              return;
            }
          }
        }
        //cerr << "...no path was found." << endl;
        session->getMap()->addDescription( _( "Can't find path to target. Sorry!" ) );
				session->getGameAdapter()->setCursorMode( Constants::CURSOR_FORBIDDEN );
      }
    }
  }
}

bool Creature::isNpc() {
	return( monster ? monster->isNpc() : false );
}

void Creature::moveAway( Creature *other ) {
	bool tryHard = false;
	// already moving away
	if( getMotion() == Constants::MOTION_MOVE_AWAY ) {
		if( cantMoveCounter > 5 ) {
			cantMoveCounter = 0;
			// pick a diff. path
			tryHard = true;
//			cerr << "*** DIFF PATH " << getName() << " when moving away from " << other->getName() << "." << endl;			
		} else {
//			cerr << "*** " << getName() << " is not moving away from " << other->getName() << "." << endl;
			return;
		}
	}

	setMotion( Constants::MOTION_MOVE_AWAY );
	
	// find the farthest possible location and move there.
	bestPathPos = 1;
	vector<Location> path;
	bool pathSelected = false;
	bool pathFound = false;
	int minIntersect = 0;
	Creature *player = session->getParty()->getPlayer();
	//cerr << "+++ " << getName() << " trying to move away: " << endl;
	for( int x = 0; !pathSelected && x < AWAY_DISTANCE; x++ ) {
		for( int y = 0; !pathSelected && y < AWAY_DISTANCE; y++ ) {
			int destX = toint( getX() ) - AWAY_DISTANCE / 2 + x;
			int destY = toint( getY() ) - AWAY_DISTANCE / 2 - y;
			Util::findPath( toint( getX() ), toint( getY() ), toint( getZ() ), 
											destX, destY, toint( getZ() ), 
											&path, 
											session->getMap(), 
											this, 
											player,
											AWAY_DISTANCE * 2,
											tryHard,
											false ); // true );
			if( !path.empty() && 
					!Util::isOutOfTheWay( this, &path, 0, 
																player, 
																player->getPath(), 
																player->getPathIndex() + 1 ) ) {
				int intersectCount = 0;
				for( int i = 0; i < session->getCreatureCount(); i++ ) {
					Creature *c = session->getCreature( i ); 
					if( c != this && 
							c->isNpc() && 
							Util::isOutOfTheWay( this, &path, 0, 
																	 c, 
																	 c->getPath(), 
																	 c->getPathIndex() + 1 ) ) {
						intersectCount++;
					}
				}
								
				for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
					Creature *c = session->getParty()->getParty( i );
					if( tryHard ) {
						if( c != this && c != player && 
							!Util::isOutOfTheWay( this, &path, 0, 
																		 c, 
																		 c->getPath(), 
																		 c->getPathIndex() + 1 ) ) {
							// c->moveAway( this );
							c->moveAway( other );
						}
					} else {
						if( c != this && 
								Util::isOutOfTheWay( this, &path, 0, 
																		 c, 
																		 c->getPath(), 
																		 c->getPathIndex() + 1 ) ) {
							intersectCount++;
						}
					}
				}
				
				// if this puts a pc out of range; that's bad
				if( !this->isMonster() ) {
					float dist = getDistance( session->getParty()->getPlayer() );
					if( dist >= CLOSE_DISTANCE ) {
						intersectCount += 2;
					}
				}

				if( minIntersect == 0 || intersectCount < minIntersect ) {
					speed = FAST_SPEED;
					selX = path[ path.size() - 1 ].x;
					selY = path[ path.size() - 1 ].y;
					bestPath.clear();
					for( int i = 0; i < (int)path.size(); i++ ) bestPath.push_back( path[ i ] );
					//pathSelected = true;
					minIntersect = intersectCount;
					if( minIntersect == 0 ) pathSelected = true;
					pathFound = true;
				}
			}
		}
	}

	if( !pathFound ) {
		stopMoving();
		setMotion( Constants::MOTION_MOVE_TOWARDS );
		other->stopMoving();
		other->setMotion( Constants::MOTION_MOVE_TOWARDS );
	}
}

void Creature::cancelMoveAway() {
	if( getMotion() == Constants::MOTION_MOVE_AWAY ) {
		stopMoving();
		setMotion( Constants::MOTION_MOVE_TOWARDS );
	}
}

bool Creature::follow( Creature *leader ) {
	float dist = getDistance( leader );
	if( dist < CLOSE_DISTANCE ) {
		if( cantMoveCounter > 5 ) {
			cantMoveCounter = 0;
		} else {
			return true;
		}
	}
	speed = FAST_SPEED;
	return findPath( toint( leader->getX() ), toint( leader->getY() ), false, 50, true );
}

bool Creature::setSelXY( int x, int y, bool cancelIfNotPossible, int maxNodes ) { 
  return findPath( x, y, cancelIfNotPossible, maxNodes,
									 ( session->getParty()->getPlayer() == this &&
										 !session->getGameAdapter()->inTurnBasedCombat() ) );
}

bool Creature::findPath( int x, int y, bool cancelIfNotPossible, int maxNodes, bool ignoreParty ) { 
  int oldSelX = selX;
  int oldSelY = selY;
  int oldtx = tx;
  int oldty = ty;

  selX = x; 
  selY = y; 
  setMotion(Constants::MOTION_MOVE_TOWARDS);   
  tx = ty = -1;

  // find the path
  tx = selX;
  ty = selY;
  bestPathPos = 1; // skip 0th position; it's the starting location
  //cerr << getName() << " findPath" << endl;
  Util::findPath( toint(getX()), toint(getY()), toint(getZ()), 
                  selX, selY, 0, 
                  &bestPath, 
                  session->getMap(), 
                  this, 
									session->getParty()->getPlayer(),
                  maxNodes,
                  ignoreParty,
									!cancelIfNotPossible );

  // Does the path lead to the destination?
  bool ret = false;
  if( bestPath.size() > 1 ) {
    Location last = bestPath[ bestPath.size() - 1 ];
    ret = ( last.x == selX &&
            last.y == selY );

    /**
     * For pc-s cancel the move.
     */
    if( !ret && character && cancelIfNotPossible ) {
      bestPathPos = 1;
      bestPath.clear();

      selX = oldSelX;
      selY = oldSelY;
      tx = oldtx;
      ty = oldty;
    }
  }

  // FIXME: when to play sound?
  if( ret && session->getParty()->getPlayer() == this ) {
    // play command sound
    if(x > -1 && 
       0 == (int)((float)(session->getPreferences()->getSoundFreq()) * rand()/RAND_MAX) &&
       !getStateMod(StateMod::dead)) {
      //session->playSound(getCharacter()->getRandomSound(Constants::SOUND_TYPE_COMMAND));
      playCharacterSound( GameAdapter::COMMAND_SOUND );
    }
  }
  
  return ret;
}

Location *Creature::moveToLocator() {
  Location *pos = NULL;
  if(selX > -1) {
    // take a step
    pos = takeAStepOnPath();

    // if we've no more steps
    if( (int)bestPath.size() <= bestPathPos ) {
			stopMoving();
			setMotion( Constants::MOTION_MOVE_TOWARDS );			
			
			// stop move-away-s
			if( this == session->getParty()->getPlayer() ) {
				for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
					session->getParty()->getParty( i )->cancelMoveAway();
				}
				for( int i = 0; i < session->getCreatureCount(); i++ ) {
					if( session->getCreature( i )->isNpc() ) {
						session->getCreature( i )->cancelMoveAway();
					}
				}
			}
    } else if( pos ) {
			// if blocked by a creature (non-monster), tell them to move out of the way
			if( pos && pos->creature &&
					pos->creature != session->getParty()->getPlayer() &&
					( !pos->creature->isMonster() || pos->creature->isNpc() ) ) {				
				((Creature*)pos->creature)->moveAway( this );
			}
			cantMoveCounter++;
		} else if( !pos ) {
			cantMoveCounter = 0;
			setMoving( true );
		}
  }
  return pos;
}

/**
 * Returns the blocking shape or NULL if move is possible.
 */
Location *Creature::takeAStepOnPath() {
	Location *position = NULL;
  int a = ((AnimatedShape*)getShape())->getCurrentAnimation();
  if((int)bestPath.size() > bestPathPos && a != MD2_TAUNT ) {

    // take a step on the bestPath
    Location location = bestPath[bestPathPos];

    GLfloat newX = getX();
    GLfloat newY = getY();
    
    GLfloat step = getStep();
    float lx = (float)(location.x);
    float ly = (float)(location.y);
    float mx = lx - getX();
    float my = ly - getY();
    //if( !strcmp(getName(),"Alamont") ) 
//      cerr << "taking step! step=" << step << " mx=" << mx << " my=" << my << endl;

    // Tolerance is 0.5 because toint will round to nearest int on map from there.
    GLfloat tolerance = 0.5f;
    if( my > tolerance ) newY += step;
    else if( my < -tolerance ) newY -= step;
    if( mx > tolerance ) newX += step;
    else if( mx < -tolerance ) newX -= step;

    //if( !strcmp(getName(), "Alamont") ) 
//      cerr << "x=" << x << "," << y << " bestPathPos=" << bestPathPos << " location=" << location.x << "," << location.y << endl;

    position = session->getMap()->
      moveCreature(toint(getX()), toint(getY()), toint(getZ()),
                   toint(newX), toint(newY), toint(getZ()),
                   this);


    /**
     * Sending newX,newY as new coordinates could be a wall.
     * location.x,location.y are the only known non-wall coordinates.
     * This is what's causing the creatures to "hang" not
     * want to move around obsticles.
     * To fix this, we need to test for walls and undo one of the
     * directional steps (newX change or newY change) and try again.
     * 
     * Situation:
     * (X-wall, s-start, e-end
     * 
     * XXXXXXXs
     *       e
     * 
     * The path using toint(float-s) would go through the wall.
     */
    if(position && ( newX != getX() && newY != getY() )) {
      position = session->getMap()->
        moveCreature(toint(getX()), toint(getY()), toint(getZ()),
                     toint(getX()), toint(newY), toint(getZ()),
                     this);
      if( !position ) {
        newX = getX();
      } else {
        position = session->getMap()->
          moveCreature(toint(getX()), toint(getY()), toint(getZ()),
                       toint(newX), toint(getY()), toint(getZ()),
                       this);
        if( !position ) {
          newY = getY();
        }
      }
    }


    if( !position ) {
      computeAngle( newX, newY );
      showWaterEffect( newX, newY );
      moveTo( newX, newY, getZ() );
      if( toint(newX) == toint(lx) && toint(newY) == toint(ly) ) {
        bestPathPos++;
      }
    } else {
      // if we can't get to the destination, stop trying
      // do this so the animation switches to "stand"
      //stopMoving();
    }
  }
  return position;
}

void Creature::computeAngle( GLfloat newX, GLfloat newY ) {
  GLfloat a = Util::getAngle( newX, newY, 1, 1,
                              getX(), getY(), 1, 1 );

  if( bestPathPos == 1 || a != wantedAngle ) {
    wantedAngle = a;
    GLfloat diff = Util::diffAngle( a, angle );
    angleStep = diff / (float)TURN_STEP_COUNT;
  }
  
  if( fabs(angle - wantedAngle) > 2.0f ) {
    GLfloat diff = Util::diffAngle( wantedAngle, angle );
    if( fabs( diff ) < angleStep ) {
      angle = wantedAngle;
    } else {
      angle += angleStep;
    }
    if( angle < 0.0f ) angle = 360.0f + angle;
    if( angle >= 360.0f ) angle -= 360.0f;
  } else {
    angle = wantedAngle;
  }
  
  ((AnimatedShape*)shape)->setAngle( angle + 180.0f );
}

void Creature::showWaterEffect( GLfloat newX, GLfloat newY ) {
  if( session->getMap()->getHasWater() &&
      !( toint(getX()) == toint(newX) &&
         toint(getY()) == toint(newY) ) ) {
    session->getMap()->startEffect( toint(getX() + getShape()->getWidth() / 2), 
                                    toint(getY() - getShape()->getDepth() / 2), 0, 
                                    Constants::EFFECT_RIPPLE, 
                                    (Constants::DAMAGE_DURATION * 4), 
                                    getShape()->getWidth(), getShape()->getDepth() );
  }
}                                             

void Creature::stopMoving() {
	cantMoveCounter = 0;
  bestPathPos = (int)bestPath.size();
  selX = selY = -1;
	speed = originalSpeed;
	getShape()->setCurrentAnimation( MD2_STAND );
}

bool Creature::anyMovesLeft() {
  return(selX > -1 && (int)bestPath.size() > bestPathPos); 
}

void Creature::getFormationPosition( Sint16 *px, Sint16 *py, Sint16 *pz, int x, int y ) {
  Sint16 dx = layout[formation][index][0];
  Sint16 dy = -layout[formation][index][1];

  // get the angle
  float angle = 0;
  if(next->getDir() == Constants::MOVE_RIGHT) angle = 270.0;
  else if(next->getDir() == Constants::MOVE_DOWN) angle = 180.0;
  else if(next->getDir() == Constants::MOVE_LEFT) angle = 90.0;

  // rotate points
  if(angle != 0) { 
    Util::rotate(dx, dy, px, py, angle);
  } else {
    *px = dx;
    *py = dy;
  }

  // translate
  if( x > -1 ) {
    *px = (*(px) * getShape()->getWidth()) + x;
    *py = (-(*(py)) * getShape()->getDepth()) + y;
  } else if(next->getSelX() > -1) {
    *px = (*(px) * getShape()->getWidth()) + next->getSelX();
    *py = (-(*(py)) * getShape()->getDepth()) + next->getSelY();
  } else {
    *px = (*(px) * getShape()->getWidth()) + toint(next->getX());
    *py = (-(*(py)) * getShape()->getDepth()) + toint(next->getY());
  }
  *pz = toint(next->getZ());
}

void Creature::setNext(Creature *next, int index) { 
  this->next = next; 
  this->index = index;
  // stand in formation
  Sint16 px, py, pz;
  getFormationPosition(&px, &py, &pz);
  if(px > -1) moveTo(px, py, pz);
}

void Creature::setNextDontMove(Creature *next, int index) {
  this->next = next; 
  this->index = index;
}

float Creature::getMaxInventoryWeight() { 
  return (float) getSkill(Skill::POWER) * 2.5f;
}  

void Creature::pickUpOnMap( RenderedItem *item ) {
  addInventory( (Item*)item );
}

bool Creature::addInventory(Item *item, bool force) { 
  if(inventory_count < MAX_INVENTORY_SIZE &&
     (force || !item->isBlocking() || 
      item->getRpgItem()->getEquip() ||
      getShape()->fitsInside( item->getShape(), true ) ) ) {
    inventory[inventory_count++] = item;
    inventoryWeight += item->getWeight(); 

    if(inventoryWeight > getMaxInventoryWeight()) {
      if( !isMonster() ) {
        char msg[80];
        sprintf(msg, _( "%s is overloaded." ), getName());
        session->getMap()->addDescription(msg);
      }
      setStateMod(StateMod::overloaded, true);
    }

    // check if the mission is over
    if(!isMonster() && 
       session->getCurrentMission() &&
       session->getCurrentMission()->itemFound(item)) {
      session->getGameAdapter()->missionCompleted();
    }

    return true;
  } else{
    return false;
  }
}

int Creature::findInInventory(Item *item) {
  for(int i = 0; i < inventory_count; i++) {
    Item *invItem = inventory[i];
    if(item == invItem) return i;
  }
  return -1;
}

Item *Creature::removeInventory(int index) { 
  Item *item = NULL;
  if(index < inventory_count) {
    // drop item if carrying it
    doff(index);
    // drop from inventory
    item = inventory[index];
    inventoryWeight -= item->getWeight();
    if(getStateMod(StateMod::overloaded) && inventoryWeight < getMaxInventoryWeight()) {
      if( !isMonster() ) {
        char msg[80];
        sprintf(msg, _( "%s is not overloaded anymore." ), getName());
        session->getMap()->addDescription(msg);
      }
      setStateMod(StateMod::overloaded, false);
    }
    for(int i = index; i < inventory_count - 1; i++) {
      inventory[i] = inventory[i + 1];
    }
    inventory_count--;
    // adjust equipped indexes too
    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
      if(equipped[i] > index && equipped[i] < MAX_INVENTORY_SIZE) {
        equipped[i]--;
      }
    }
    recalcAggregateValues();
  }
  return item;
}

bool Creature::eatDrink(int index){
  return eatDrink(getInventory(index));
}

bool Creature::eatDrink(Item *item) {
  char msg[500];
  char buff[200];
  RpgItem * rpgItem = item->getRpgItem();

  int type = rpgItem->getType();    
  //weight = item->getWeight();
  int level = item->getLevel();
  if(type == RpgItem::FOOD){
    if(getHunger() == 10){                
      sprintf(msg, _( "%s is not hungry at the moment." ), getName()); 
      session->getMap()->addDescription(msg); 
      return false;
    }

    // TODO : the quality member of rpgItem should indicate if the
    // food is totally healthy or roten or partially roten etc...
    // We eat the item and it gives us "level" hunger points back
    setHunger(getHunger() + level);            
    strcpy(buff, rpgItem->getShortDesc());
    buff[0] = tolower(buff[0]);
    sprintf(msg, _( "%1$s eats %2$s." ), getName(), buff);
    session->getMap()->addDescription(msg);
    bool b = item->decrementCharges();
    if(b) {
      sprintf(msg, _( "%s is used up." ), item->getItemName());
      session->getMap()->addDescription(msg);
    }
    return b;
  } else if(type == RpgItem::DRINK){
    if(getThirst() == 10){                
      sprintf(msg, _( "%s is not thirsty at the moment." ), getName()); 
      session->getMap()->addDescription(msg); 
      return false;
    }
    setThirst(getThirst() + level);
    strcpy(buff, rpgItem->getShortDesc());
    buff[0] = tolower(buff[0]);
    sprintf(msg, _( "%1$s drinks %2$s." ), getName(), buff);
    session->getMap()->addDescription(msg); 
    // TODO : according to the alcool rate set drunk state or not            
    bool b = item->decrementCharges();
    if(b) {
      sprintf(msg, _( "%s is used up." ), item->getItemName());
      session->getMap()->addDescription(msg);
    }
    return b;
  } else if(type == RpgItem::POTION) {
    // It's a potion            
    // Even if not thirsty, character will always drink a potion
    strcpy(buff, rpgItem->getShortDesc());
    buff[0] = tolower(buff[0]);
    setThirst(getThirst() + level);
    sprintf(msg, _( "%1$s drinks from %2$s." ), getName(), buff);
    session->getMap()->addDescription(msg); 
    usePotion(item);
    bool b = item->decrementCharges();
    if(b) {
      sprintf(msg, _( "%s is used up." ), item->getItemName());
      session->getMap()->addDescription(msg);
    }
    return b;
  } else {
    session->getMap()->addDescription( _( "You cannot eat or drink that!" ), 1, 0.2f, 0.2f);
    return false;
  }
}

void Creature::usePotion(Item *item) {
  // nothing to do?
  if(item->getRpgItem()->getPotionSkill() == -1) return;

  int n;
  char msg[255];

  int skill = item->getRpgItem()->getPotionSkill();
  if(skill < 0) {
    switch(-skill - 2) {
    case Constants::HP:
      n = item->getRpgItem()->getPotionPower() + item->getLevel();
      if(n + getHp() > getMaxHp())
        n = getMaxHp() - getHp();
      setHp(getHp() + n);
      sprintf(msg, _( "%s heals %d points." ), getName(), n);
      session->getMap()->addDescription(msg, 0.2f, 1, 1);
      startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
      return;
    case Constants::MP:
      n = item->getRpgItem()->getPotionPower() + item->getLevel();
      if(n + getMp() > getMaxMp())
        n = getMaxMp() - getMp();
      setMp(getMp() + n);
      sprintf(msg, _( "%s receives %d magic points." ), getName(), n);
      session->getMap()->addDescription(msg, 0.2f, 1, 1);
      startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));
      return;
    case Constants::AC:
      {
        bonusArmor += item->getRpgItem()->getPotionPower();
        recalcAggregateValues();
        sprintf(msg, _( "%s feels impervious to damage!" ), getName());
        session->getMap()->addDescription(msg, 0.2f, 1, 1);
        startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));

        // add calendar event to remove armor bonus
        // (format : sec, min, hours, days, months, years)
        Date d(0, item->getRpgItem()->getPotionTime() + item->getLevel(), 0, 0, 0, 0); 
        Event *e = 
        new PotionExpirationEvent(session->getParty()->getCalendar()->getCurrentDate(), 
                                  d, this, item, session, 1);
        session->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!		
      }
      return;
    default:
      cerr << "Implement me! (other potion skill boost)" << endl;
      return;
    }
  } else {
    skillBonus[skill] += item->getRpgItem()->getPotionPower();
    //	recalcAggregateValues();
    sprintf(msg, _( "%s feels at peace." ), getName());
    session->getMap()->addDescription(msg, 0.2f, 1, 1);
    startEffect(Constants::EFFECT_SWIRL, (Constants::DAMAGE_DURATION * 4));

    // add calendar event to remove armor bonus
    // (format : sec, min, hours, days, months, years)
    Date d(0, item->getRpgItem()->getPotionTime() + item->getLevel(), 0, 0, 0, 0); 
    Event *e = 
    new PotionExpirationEvent(session->getParty()->getCalendar()->getCurrentDate(), 
                              d, this, item, session, 1);
    session->getParty()->getCalendar()->scheduleEvent((Event*)e);   // It's important to cast!!
  }
}

void Creature::setAction(int action, 
                         Item *item, 
                         Spell *spell,
                         SpecialSkill *skill) {
  this->action = action;
  this->actionItem = item;
  this->actionSpell = spell;
  this->actionSkill = skill;
  preActionTargetCreature = getTargetCreature();  
  // zero the clock
  setLastTurn(0);

  char msg[80];
  switch(action) {
  case Constants::ACTION_EAT_DRINK:
    this->battle->invalidate();
    sprintf(msg, _( "%1$s will consume %2$s." ), getName(), item->getItemName());
    break;
  case Constants::ACTION_CAST_SPELL:
    this->battle->invalidate();
    sprintf(msg, _( "%1$s will cast %2$s." ), getName(), spell->getDisplayName());
    break;
  case Constants::ACTION_SPECIAL:
    this->battle->invalidate();
    sprintf(msg, _( "%1$s will use capability %2$s." ), getName(), skill->getDisplayName());
    break;
  case Constants::ACTION_NO_ACTION:
    // no-op
    preActionTargetCreature = NULL;
    strcpy(msg, "");
    break;
  default:
    cerr << "*** Error: unknown action " << action << endl;
    return;
  }
  if(strlen(msg)) session->getMap()->addDescription(msg, 1, 1, 0.5f);
}

void Creature::equipInventory(int index) {
  this->battle->invalidate();
  // doff
  if(doff(index)) return;
  // don
  // FIXME: take into account: two-handed weapons, min skill req-s., etc.
  Item *item = getInventory(index);
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    // if the slot is empty and the item can be worn here
    if(item->getRpgItem()->getEquip() & ( 1 << i ) && 
       equipped[i] == MAX_INVENTORY_SIZE) {
      equipped[i] = index;

      // once worn, show if it's cursed
      item->setShowCursed( true );

      // handle magic attrib settings
      if(item->isMagicItem()) {

        //item->debugMagic("Equip: ");

        // set the good attributes
        for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
          if(item->isStateModSet(i)) {
            this->setStateMod(i, true);
          }
        }
        // set the protected attributes
        for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
          if(item->isStateModProtected(i)) {
            this->setProtectedStateMod(i, true);
          }
        }
        // skill bonuses
        map<int,int> *m = item->getSkillBonusMap();
        for(map<int,int>::iterator e=m->begin(); e!=m->end(); ++e) {
          int skill = e->first;
          int bonus = e->second;
          setSkillBonus(skill, getSkillBonus(skill) + bonus);
        }
        // if armor, enhance magic resistance
        if(!item->getRpgItem()->isWeapon() && 
           item->getSchool()) {
          int skill = item->getSchool()->getResistSkill();
          setSkillBonus(skill, getSkillBonus(skill) + item->getMagicResistance());
        }
        // refresh map for invisibility, etc.
        session->getMap()->refresh();
      }

      // call script
      if( !monster ) session->getSquirrel()->callItemEvent( this, item, "equipItem" );

      // recalc current weapon
      recalcAggregateValues();
      return;
    }
  }
}

int Creature::doff(int index) {
  // doff
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if(equipped[i] == index) {
      Item *item = getInventory(index);
      equipped[i] = MAX_INVENTORY_SIZE;

      // handle magic attrib settings
      if(item->isMagicItem()) {

        //item->debugMagic("Doff: ");

        // set the good attributes
        for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
          if(item->isStateModSet(i)) {
            this->setStateMod(i, false);
          }
        }
        // set the protected attributes
        for(int i = 0; i < StateMod::STATE_MOD_COUNT; i++) {
          if(item->isStateModProtected(i)) {
            this->setProtectedStateMod(i, false);
          }
        }
        // skill bonus
        map<int,int> *m = item->getSkillBonusMap();
        for(map<int,int>::iterator e=m->begin(); e!=m->end(); ++e) {
          int skill = e->first;
          int bonus = e->second;
          setSkillBonus(skill, getSkillBonus(skill) - bonus);
        }
        // if armor, enhance magic resistance
        if(!item->getRpgItem()->isWeapon() && 
           item->getSchool()) {
          int skill = item->getSchool()->getResistSkill();
          setSkillBonus(skill, getSkillBonus(skill) - item->getMagicResistance());
        }

        // refresh map for invisibility, etc.
        session->getMap()->refresh();
      }

      // call script
      if( !monster ) session->getSquirrel()->callItemEvent( this, item, "doffItem" );

      // recalc current weapon
      recalcAggregateValues();
      return 1;
    }
  }
  return 0;
}

/**
   Get item at equipped index. (What is at equipped location?)
 */
Item *Creature::getEquippedInventory(int index) {
  int n = equipped[index];
  if(n < MAX_INVENTORY_SIZE) {
    return getInventory(n);
  }
  return NULL;
}

bool Creature::isEquippedWeapon( int location ) {
  Item *item = getItemAtLocation( location );
  return( item && item->getRpgItem()->isWeapon() );
}


bool Creature::isEquipped( Item *item ) {
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if(equipped[i] < MAX_INVENTORY_SIZE &&
       inventory[ equipped[i] ] == item ) return true;
  }
  return false;
}

bool Creature::isEquipped( int inventoryIndex ) {
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if( equipped[i] == inventoryIndex ) return true;
  }
  return false;

}

bool Creature::removeCursedItems() {
  bool found = false;
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if( equipped[i] < MAX_INVENTORY_SIZE &&
        inventory[ equipped[i] ]->isCursed() ) {
      found = true;
      // not the most efficient way to do this, but it works...
      doff( equipped[i] );
    }
  }
  return found;
}

/**
   Get equipped index of inventory index. (Where is the item worn?)
*/
int Creature::getEquippedIndex(int index) {
  for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if(equipped[i] == index) return i;
  }
  return -1;
}

bool Creature::isItemInInventory(Item *item) {
  for(int i = 0; i < inventory_count; i++) {
    if(inventory[i] == item || 
       (inventory[i]->getRpgItem()->getType() == RpgItem::CONTAINER &&
        inventory[i]->isContainedItem(item)))
      return true;
  }
  return false;
}

Item *Creature::getItemAtLocation(int location) {
  int i;
  for(i = 0; i < Constants::INVENTORY_COUNT; i++) {
    if((1 << i) == location) {
      if(equipped[i] < MAX_INVENTORY_SIZE) {
        return getInventory(equipped[i]);
      } else {
        return NULL;
      }
    }
  }
  return NULL;
}

// calculate the aggregate values based on equipped items
void Creature::recalcAggregateValues() {
  armorChanged = true;

  // try to select a new preferred weapon if needed.
  if( preferredWeapon == -1 || !isEquippedWeapon( preferredWeapon ) ) {
    int values[] = { 
      Constants::INVENTORY_LEFT_HAND, 
      Constants::INVENTORY_RIGHT_HAND, 
      Constants::INVENTORY_WEAPON_RANGED,
      -1
    };
    preferredWeapon = -1;
    for( int i = 0; values[ i ] > -1; i++ ) {
      if( isEquippedWeapon( values[ i ] ) ) {
        preferredWeapon = values[ i ];
        break;
      }
    }
  }
}

bool Creature::nextPreferredWeapon() {
  int pos = preferredWeapon;
  for( int i = 0; i < 4; i++ ) {
    switch( pos ) {
    case Constants::INVENTORY_LEFT_HAND: pos = Constants::INVENTORY_RIGHT_HAND; break;
    case Constants::INVENTORY_RIGHT_HAND: pos = Constants::INVENTORY_WEAPON_RANGED; break;
    case Constants::INVENTORY_WEAPON_RANGED: pos = -1; break;
    case -1: pos = Constants::INVENTORY_LEFT_HAND; break;
    }
    if( pos == -1 || isEquippedWeapon( pos ) ) {
      preferredWeapon = pos;
      return true;
    }
  }
  preferredWeapon = -1;
  return false;
}

Item *Creature::getBestWeapon( float dist, bool callScript ) {

  Item *ret = NULL;

  // for TB combat for players, respect the current weapon
  if( session->getPreferences()->isBattleTurnBased() &&
      !isMonster() ) {
    ret = ( preferredWeapon > -1 ? getItemAtLocation( preferredWeapon ) : NULL );
  } else {
    int location[] = { 
      Constants::INVENTORY_RIGHT_HAND,
      Constants::INVENTORY_LEFT_HAND,
      Constants::INVENTORY_WEAPON_RANGED,
      -1
    };  
    for( int i = 0; location[i] > -1; i++ ) {
      Item *item = getItemAtLocation( location[i] );
      if(item && 
         item->getRpgItem()->isWeapon() && 
         item->getRange() >= dist) {
        ret = item;
        break;
      }
    }
  }
  if( !monster && ret && callScript ) {
    session->getSquirrel()->callItemEvent( this, ret, "startBattleWithItem" );
  }

  return ret;
}

// return the initiative for a battle round, the higher the faster the attack
int Creature::getInitiative( int *max ) {
	float n = ( getSkill( Skill::SPEED ) + ( getSkill( Skill::LUCK ) / 5.0f ) );
	if( max ) *max = toint( n );
  return toint( n * rand() / RAND_MAX );
}

// return number of projectiles that can be launched simultaniously
// it is a function of speed, level, and weapon skill
// this method returns a number from 1-10
int Creature::getMaxProjectileCount(Item *item) {
  int n = (int)((double)(getSkill(Skill::SPEED) + 
                         (getLevel() * 10) + 
                         getSkill(item->getRpgItem()->getDamageSkill())) / 30.0f);
  if(n <= 0) n = 1;
  return n;
}

vector<RenderedProjectile*> *Creature::getProjectiles() {
	map<RenderedCreature*, vector<RenderedProjectile*>*> *m =
		RenderedProjectile::getProjectileMap();
	return( m->find( this ) == m->end() ? NULL : (*m)[ (RenderedCreature*)this ] );
}

/**
 take some damage
*/
bool Creature::takeDamage( float damage, 
													 int effect_type, 
													 GLuint delay ) {
	
  int intDamage = toint( damage );
  addRecentDamage( intDamage );

  hp -= intDamage;
  // if creature dies start effect at its location
  if(hp > 0) {
    startEffect(effect_type);
    int pain = (int)(3.0f * rand()/RAND_MAX);
    getShape()->setCurrentAnimation(pain == 0 ? (int)MD2_PAIN1 : (pain == 1 ? (int)MD2_PAIN2 : (int)MD2_PAIN3));
  } else if(effect_type != Constants::EFFECT_GLOW) {
    session->getMap()->startEffect( toint(getX()), toint(getY() - this->getShape()->getDepth() + 1), toint(getZ()), 
                                    effect_type, (Constants::DAMAGE_DURATION * 4), 
                                    getShape()->getWidth(), getShape()->getDepth(), delay );
  }

  // creature death here so it can be used from script
  if( hp <= 0 ) {
		if( ( isMonster() && !MONSTER_IMORTALITY ) || !GOD_MODE ) {
			session->creatureDeath( this );
		}
    return true;
  } else {
    return false;
  }
}

void Creature::resurrect( int rx, int ry ) {
	// remove all state mod effects
	for( int i = 0; i < StateMod::STATE_MOD_COUNT; i++ ) {
		setStateMod( i, false );
	}
	if( getThirst() < 5 ) setThirst( 5 );
	if( getHunger() < 5 ) setHunger( 5 );

  setHp( (int)( 3.0f * rand() / RAND_MAX ) + 1 );
  
  findPlace( rx, ry );

  startEffect( Constants::EFFECT_TELEPORT, ( Constants::DAMAGE_DURATION * 4 ) );

  char msg[120];
  sprintf( msg, _( "%s is raised from the dead!" ), getName() );
  session->getMap()->addDescription( msg, 0, 1, 1 );
}

// add exp after killing a creature
// only called for characters
int Creature::addExperience(Creature *creature_killed) {
	int n = ( creature_killed->level + 1 ) * 25;
	// extra for killing higher level creatures
	int bonus = ( creature_killed->level - getLevel() );
	if( bonus > 0 ) n += bonus * 10;
  return addExperienceWithMessage( n );
}

/**
 * Add n exp points. Only called for characters
 * Note that n can be a negative number. (eg.: failure to steal)
 */
int Creature::addExperience(int delta) {
  int n = delta;
  experience += n;
  if( experience < 0 ) {
    n = experience;
    experience = 0;
  }

  // level up?
  if(experience >= expOfNextLevel) {
    level++;
    hp += character->getStartingHp();
    mp += character->getStartingMp();
    calculateExpOfNextLevel();
    availableSkillMod += character->getSkillBonus();
    char message[255];
    sprintf( message, _( "  %s levels up!" ), getName() );
    session->getGameAdapter()->startTextEffect( message );
		session->getGameAdapter()->refreshInventoryUI();
  }

  evalSpecialSkills();

  return n;
}

int Creature::addExperienceWithMessage( int exp ) {
  int n = 0;
  if( !getStateMod( StateMod::dead ) ) {
    char message[120];
    int oldLevel = level;
    n = addExperience( exp );
    if( n > 0 ) {
      sprintf( message, _( "%s gains %d experience points." ), getName(), n );
      session->getMap()->addDescription( message );
      if( oldLevel != level ) {
        sprintf( message, _( "%s gains a level!" ), getName() );
        session->getMap()->addDescription( message, 1.0f, 0.5f, 0.5f );
      }
    } else if( n < 0 ) {
      sprintf( message, _( "%s looses %d experience points!" ), getName(), -n );
      session->getMap()->addDescription( message, 1.0f, 0.05f, 0.05f );
    }
  }
  return n;
}

// add money after a creature is killed
int Creature::addMoney(Creature *creature_killed) {
  int n = creature_killed->level - getLevel();
  if( n < 1 ) n = 1;
  // fixme: use creature_killed->getMonster()->getMoney() instead of 100.0f
  long delta = (long)n * (int)(50.0f * rand()/RAND_MAX);
  money += delta;
  return money;
}

void Creature::monsterInit() {

  this->level = monster->getLevel();


	//cerr << "------------------------------------" << endl << "Creature: " << monster->getType() << endl;
  for(int i = 0; i < Skill::SKILL_COUNT; i++) {

    //int n = Creature::rollStartingSkill( scourge->getSession(), LEVEL, i );
		int n;
		if( Skill::skills[i]->getGroup()->isStat() ) {
			//n = (int)( 19.0f * rand() / RAND_MAX ) + 1;
			MonsterToughness *mt = &(monsterToughness[ session->getPreferences()->getMonsterToughness() ]);
			n = (int)( 20.0f * roll( mt->minSkillBase, mt->maxSkillBase ) );
		} else {

			// create the starting value as a function of the stats
			n = 0;
			for( int t = 0; t < Skill::skills[i]->getPreReqStatCount(); t++ ) {
				int index = Skill::skills[i]->getPreReqStat( t )->getIndex();
				n += getSkill( index );
			}
			n = (int)( ( n / (float)( Skill::skills[i]->getPreReqStatCount() ) ) * 
								 (float)( Skill::skills[i]->getPreReqMultiplier() ) );
		}

		// special: adjust magic resistance... makes game too hard otherwise
		if( i == Skill::RESIST_AWARENESS_MAGIC ||
				i == Skill::RESIST_CONFRONTATION_MAGIC ||
				i == Skill::RESIST_DECEIT_MAGIC ||
				i == Skill::RESIST_HISTORY_MAGIC ||
				i == Skill::RESIST_LIFE_AND_DEATH_MAGIC ||
				i == Skill::RESIST_NATURE_MAGIC ) {
			n /= 2;
		}

		// apply monster settings
		int minSkill = monster->getSkillLevel( Skill::skills[i]->getName() );
		if( minSkill > n ) n = minSkill;
		
    setSkill( i, n );
		//cerr << "\t" << Skill::skills[ i ]->getName() << "=" << getSkill( i ) << endl;

		stateMod = monster->getStartingStateMod();
  }

  // equip starting inventory
  for(int i = 0; i < getMonster()->getStartingItemCount(); i++) {  
    int itemLevel = getMonster()->getLevel() - (int)( ( 2.0f * rand() / RAND_MAX ) );
    if( itemLevel < 1 ) itemLevel = 1;
    Item *item = session->newItem( getMonster()->getStartingItem(i), itemLevel );
    addInventory( item, true );
    equipInventory(inventory_count - 1);
  }

  // add some loot
  int nn = (int)( 5.0f * rand()/RAND_MAX ) + 3;
  //cerr << "Adding loot:" << nn << endl;
  for( int i = 0; i < nn; i++ ) {
    Item *loot;
    if( 0 == (int)( 10.0f * rand()/RAND_MAX ) ) {
      Spell *spell = MagicSchool::getRandomSpell( getLevel() );
      loot = session->newItem( RpgItem::getItemByName("Scroll"), 
                               getLevel(), 
                               spell );
    } else {
      loot = session->newItem( RpgItem::getRandomItem( session->getGameAdapter()->getCurrentDepth() ), 
                               getLevel() );
    }
    //cerr << "\t" << loot->getRpgItem()->getName() << endl;
    // make it contain all items, no matter what size
    addInventory( loot, true );
  }

  // add spells
  for(int i = 0; i < getMonster()->getStartingSpellCount(); i++) {
    addSpell(getMonster()->getStartingSpell(i));
  }

  // add some hp and mp
  float n = (float)( monster->getHp() * ( level + 2 ) );
  //startingHp = hp = (int)( n * ( ( 0.3f * rand() / RAND_MAX ) + 0.7f ) );
  startingHp = hp = (int)( n * roll( monsterToughness[ session->getPreferences()->getMonsterToughness() ].minHpMpBase,
                                     monsterToughness[ session->getPreferences()->getMonsterToughness() ].maxHpMpBase ) );

  n = (float)( monster->getMp() * ( level + 2 ) );
  //startingMp = mp = (int)( n * ( ( 0.3f * rand() / RAND_MAX ) + 0.7f ) );
  startingMp = mp = (int)( n * roll( monsterToughness[ session->getPreferences()->getMonsterToughness() ].minHpMpBase,
                                     monsterToughness[ session->getPreferences()->getMonsterToughness() ].maxHpMpBase ) );
}

int Creature::getMaxHp() {
  if(isMonster()) {
    return startingHp;
  } else {
    return(character->getStartingHp() * ( getLevel() + 1 ));
  }
}

int Creature::getMaxMp() {
  if(isMonster()) {
    return startingMp;
  } else {
    return(character->getStartingMp() * ( getLevel() + 1 ));
  }
}

float Creature::getTargetAngle() {
  //if(!targetCreature) return -1.0f;
  if(!targetCreature) return angle;
  return Util::getAngle(getX(), getY(), getShape()->getWidth(), getShape()->getDepth(),
                        getTargetCreature()->getX(), getTargetCreature()->getY(), 
                        getTargetCreature()->getShape()->getWidth(), 
                        getTargetCreature()->getShape()->getHeight());
}

// FIXME: O(n) but there aren't that many spells...
bool Creature::isSpellMemorized(Spell *spell) {
  for(int i = 0; i < (int)spells.size(); i++) {
    if(spells[i] == spell) return true;
  }
  return false;
}

// FIXME: O(n) but there aren't that many spells...
// return true if spell was added, false if creature already had this spell
bool Creature::addSpell(Spell *spell) { 
  for(vector<Spell*>::iterator e = spells.begin(); e != spells.end(); ++e) {
    Spell *thisSpell = *e;
    if(thisSpell == spell) return false;
  }
  spells.push_back(spell); 
  return true;
}

// this assumes that hasTarget() was called first.
bool Creature::isTargetValid() {
  // is it a non-creature target? (item or location)
  if(!getTargetCreature()) return true;
  if(getTargetCreature()->getStateMod(StateMod::dead)) return false;
  // when attacking, attack the opposite kind (unless possessed)
  // however, you can cast spells on anyone
  if(getAction() == Constants::ACTION_NO_ACTION && 
     !canAttack(getTargetCreature())) return false;
  return true;
}

bool Creature::canAttack(RenderedCreature *creature, int *cursor) {
  // when attacking, attack the opposite kind (unless possessed)
  bool ret;
  if( isMonster() ) {
    if( !getStateMod( StateMod::possessed ) ) {
      ret = ( ( !creature->isMonster() && !creature->getStateMod(StateMod::possessed) ) ||
              ( creature->isMonster() && creature->getStateMod(StateMod::possessed) ) );
    } else {
      ret = ( ( !creature->isMonster() && creature->getStateMod(StateMod::possessed) ) ||
              ( creature->isMonster() && !creature->getStateMod(StateMod::possessed) ) );
    }
  } else {
    if( !getStateMod( StateMod::possessed ) ) {
      ret = ( ( creature->isMonster() && !creature->getStateMod(StateMod::possessed) ) ||
              ( !creature->isMonster() && creature->getStateMod(StateMod::possessed) ) );
    } else {
      ret = ( ( !creature->isMonster() && !creature->getStateMod(StateMod::possessed) ) ||
              ( creature->isMonster() && creature->getStateMod(StateMod::possessed) ) );
    }
  }
  if( ret && cursor ) {
    float dist = getDistanceToTarget( creature );
    Item *item = getBestWeapon( dist );
    if( dist <= getBattle()->calculateRange( item ) ) {
      *cursor = ( !item ? Constants::CURSOR_NORMAL :
                  ( item->getRpgItem()->isRangedWeapon() ? 
                    Constants::CURSOR_RANGED : 
                    Constants::CURSOR_ATTACK ) );
    } else {
      *cursor = Constants::CURSOR_MOVE;
    }
  }
  return ret;
}

void Creature::cancelTarget() {
  setTargetCreature(NULL);
  setTargetItem(0, 0, 0, NULL);
  if(preActionTargetCreature) setTargetCreature(preActionTargetCreature);
  preActionTargetCreature = NULL;
  setAction(Constants::ACTION_NO_ACTION);
  if(isMonster()) setMotion(Constants::MOTION_LOITER);
}

/**
 * Does the spell's prerequisite apply to this creature?
 */
bool Creature::isWithPrereq( Spell *spell ) {
  if( spell->isStateModPrereqAPotionSkill() ) {
    switch( spell->getStateModPrereq() ) {
    case Constants::HP:
      //cerr << "\tisWithPrereq: " << getName() << " max hp=" << getMaxHp() << " hp=" << getHp() << endl;
      return( getHp() <= (int)( (float)( getMaxHp() ) * 0.25f ) ? 
              true : 
              false );
    case Constants::MP:
      return( getMp() <= (int)( (float)( getMaxMp() ) * 0.25f ) ? 
              true : 
              false );
    case Constants::AC:
      /*
      Even if needed only cast it 1 out of 4 times.
      Really need some AI here to remember if the spell helped or not. (or some way
      to predict if casting a spell will help.) Otherwise the monster keeps casting
      Body of Stone to no effect.
      Also: 10 should not be hard-coded...
			*/
			float armor, dodgePenalty;
		getArmor( &armor, &dodgePenalty, 0 );
		return( armor < 10 ? 
						( (int)( 4.0f * rand() / RAND_MAX ) == 0 ? true : false ) : 
						false ); 
		default: return false;
    }
  } else {
    return getStateMod( spell->getStateModPrereq() );
  }
}

Creature *Creature::findClosestTargetWithPrereq( Spell *spell ) {

  // is it self?
  if( isWithPrereq( spell ) ) return this;

  // who are the possible targets?
  vector<Creature*> possibleTargets;
  if( getStateMod( StateMod::possessed ) ) {
    for( int i = 0; i < session->getParty()->getPartySize(); i++ ) {
      if( !session->getParty()->getParty( i )->getStateMod( StateMod::dead ) &&
          session->getParty()->getParty( i )->isWithPrereq( spell ) )
        possibleTargets.push_back( session->getParty()->getParty( i ) );
    }
  } else {
    for( int i = 0; i < session->getCreatureCount(); i++ ) {
      if( session->getCreature( i )->isMonster() &&
          !session->getCreature( i )->getMonster()->isNpc() &&
          !session->getCreature( i )->getStateMod( StateMod::dead ) &&
          session->getCreature( i )->isWithPrereq( spell ) ) 
        possibleTargets.push_back( session->getCreature( i ) );
    }
  }

  // find the closest one that is closer than 20 spaces away.
  Creature *closest = NULL;
  float closestDist = 0;
  for( int i = 0; i < (int)possibleTargets.size(); i++ ) {
    Creature *p = possibleTargets[ i ];
    float dist = 
      Constants::distance( getX(),  getY(), 
                           getShape()->getWidth(), getShape()->getDepth(),
                           p->getX(), p->getY(),
                           p->getShape()->getWidth(), 
                           p->getShape()->getDepth() );
    if( !closest || dist < closestDist ) {
      closest = p;
      closestDist = dist;
    }
  }
  return( closest && closestDist < 20.0f ? closest : NULL );
}                                             

/**
 * Try to heal someone; returns true if someone was found.
 */
bool Creature::castHealingSpell() {
  if( !isMonster() ) return false;

  // are we in the middle of casting a healing spell already?
  //if( getAction() == Constants::ACTION_CAST_SPELL ) return false;

  // try to heal someone
  for(int i = 0; i < getSpellCount(); i++) {
    Spell *spell = getSpell(i);
    // can it be cast and is it a "friendly" (healing) spell?
    if( spell->getMp() < getMp() && 
        spell->isFriendly() && 
        spell->hasStateModPrereq() ) {
      //cerr << "Looking to heal a creature:" << endl;
      Creature *p = findClosestTargetWithPrereq( spell );
      if( p ) {

        // is this needed?
        // getBattle()->reset();

        //cerr << "\t*** Selected: " << p->getName() << endl;
        setAction(Constants::ACTION_CAST_SPELL, 
                  NULL,
                  spell);
        setMotion(Constants::MOTION_MOVE_TOWARDS);
        setTargetCreature(p);
        return true;
      }
    }
  }
  return false;
}

void Creature::decideMonsterAction() {
  //if( !( isMonster() || getStateMod( Constants::possessed ) ) ) return;

  if( !isMonster() && getStateMod( StateMod::possessed ) ) {
    Creature *p = 
      session->getParty()->getClosestPlayer( toint(getX()), toint(getY()), 
                                             getShape()->getWidth(),
                                             getShape()->getDepth(),
                                             20 );
    // attack with item
    setMotion(Constants::MOTION_MOVE_TOWARDS);
    setTargetCreature(p);
  } else if( !monster || monster->isNpc() ) {
		if( getMotion() == Constants::MOTION_MOVE_AWAY ) return;
    int n = (int)( 10.0f * rand()/RAND_MAX );
    switch( n ) {
    case 0 : getShape()->setCurrentAnimation(MD2_WAVE); break;
    case 1 : getShape()->setCurrentAnimation(MD2_POINT); break;
    case 2 : getShape()->setCurrentAnimation(MD2_SALUTE); break;
    default : getShape()->setCurrentAnimation(MD2_STAND); break;
    }
    setMotion(Constants::MOTION_STAND);
  } else {

    if( castHealingSpell() ) return;
    
    // try to attack someone
    Creature *p;
    if(getStateMod(StateMod::possessed)) {
      p = session->getClosestVisibleMonster(toint(getX()), toint(getY()), 
                                                      getShape()->getWidth(),
                                                      getShape()->getDepth(),
                                                      20);
    } else {
      p = session->getParty()->getClosestPlayer(toint(getX()), toint(getY()), 
                                                getShape()->getWidth(),
                                                getShape()->getDepth(),
                                                20);
    }
    if(p) {
      float dist = 
        Constants::distance(getX(),  getY(), 
                            getShape()->getWidth(), getShape()->getDepth(),
                            p->getX(), p->getY(),
                            p->getShape()->getWidth(), 
                            p->getShape()->getDepth());
      
      // can monster use magic?
      if( getSpellCount() ) {
        for( int i = 0; i < getSpellCount(); i++ ) {
          Spell *spell = getSpell( i );

					if( useOffensiveSpell( spell, dist, p ) ) {
						setAction( Constants::ACTION_CAST_SPELL, 
											 NULL,
											 spell );
						setMotion( Constants::MOTION_MOVE_TOWARDS );
						setTargetCreature( p );
						return;
					}
        }
      }

      // attack with item
      setMotion(Constants::MOTION_MOVE_TOWARDS);
      setTargetCreature(p);
    }
  }
}

bool Creature::useOffensiveSpell( Spell *spell, float dist, Creature *possibleTarget ) {
	if( spell->getMp() < getMp() && !( spell->isFriendly() ) ) {

		// if there is a prereq. and the target already has it, skip this spell.
		if( spell->hasStateModPrereq() &&
				possibleTarget->isWithPrereq( spell ) ) return false;

		if( ( spell->getDistance() == 1 && dist <= MIN_DISTANCE ) ||
				( spell->getDistance() > 1 && dist > MIN_DISTANCE ) ) {
			return true;
		}
	}
	return false;
}

float Creature::getDistanceToSel() {
	if( selX > -1 && selY > -1 ) {
		return Constants::distance( getX(),  getY(), 
																getShape()->getWidth(), getShape()->getDepth(),
																selX, selY,
																getShape()->getWidth(), getShape()->getDepth());
	} else {
		return 0;
	}
}

float Creature::getDistance( RenderedCreature *other ) {
	return Constants::distance( getX(),  getY(), 
															getShape()->getWidth(), getShape()->getDepth(),
															other->getX(), 
															other->getY(),
															other->getShape()->getWidth(), 
															other->getShape()->getDepth());
}

float Creature::getDistanceToTarget( RenderedCreature *creature ) {
  if( creature ) return getDistance( creature );
  
  if(!hasTarget()) return 0.0f;
  if(getTargetCreature()) {
		return getDistance( getTargetCreature() );
  } else if( getTargetX() || getTargetY() ) {
    if(getTargetItem()) {
      return Constants::distance(getX(),  getY(), 
                                 getShape()->getWidth(), getShape()->getDepth(),
                                 getTargetX(), getTargetY(),
                                 getTargetItem()->getShape()->getWidth(), 
                                 getTargetItem()->getShape()->getDepth());
    } else {
      return Constants::distance(getX(),  getY(), 
                                 getShape()->getWidth(), getShape()->getDepth(),
                                 getTargetX(), getTargetY(), 1, 1);
    }
  } else {
    return 0.0f;
  }
}

// sets min exp for current level
void Creature::setExp() {
  if(isMonster()) return;
  expOfNextLevel = 0;
  for(int i = 0; i < level - 1; i++) {
    expOfNextLevel += ((i + 1) * character->getLevelProgression());
  }
}

GLfloat Creature::getStep() {
  GLfloat fps = session->getGameAdapter()->getFps();
  GLfloat div = FPS_ONE + (float)( ( 4 - session->getPreferences()->getGameSpeedLevel() ) * 3.0f);
  if( fps < div ) return 0.8f;
  GLfloat step = 1.0f / ( fps / div  ); 
  if( getSpeed() <= 0 ) {
    step *= 1.0f - ( 1.0f / 10.0f );
  } else if( getSpeed() >= 10 ) {
    step *= 1.0f - ( 9.9f / 10.0f );
  } else {
    step *= ( 1.0f - ((GLfloat)(getSpeed()) / 10.0f ));
  }
  return step;
}

void Creature::getDetailedDescription(char *s) {
  sprintf(s, "%s %s", 
          getName(), 
					( session->getCurrentMission() && 
						session->getCurrentMission()->isMissionCreature( this ) ? 
						_( "*Mission*" ) : "" ) );
}

void Creature::setHp() { 
  hp = ( getLevel() + 1 ) * getCharacter()->getStartingHp(); 
}

void Creature::setMp() { 
  mp = ( getLevel() + 1 ) * getCharacter()->getStartingMp(); 
}

void Creature::draw() { 
  getShape()->draw(); 
}  

void Creature::setNpcInfo( NpcInfo *npcInfo ) { 
  this->npcInfo = npcInfo; 
  setName( npcInfo->name );

  // for merchants, re-create inventory with the correct types
  if( npcInfo->type == Constants::NPC_TYPE_MERCHANT ) {
    // drop everything beyond the basic inventory
    for( int i = getMonster()->getStartingItemCount(); i < this->getInventoryCount(); i++ ) {
      this->removeInventory( getMonster()->getStartingItemCount() );
    }
    
    int types[ npcInfo->getSubtype()->size() ];
    int typeCount = 0;
    for( set<int>::iterator e = npcInfo->getSubtype()->begin(); e != npcInfo->getSubtype()->end(); ++e ) {
      types[ typeCount++ ] = *e;
    }

    // equip merchants at the party's level
    int level = ( session->getParty() ? 
                  toint(((float)(session->getParty()->getTotalLevel())) / ((float)(session->getParty()->getPartySize()))) : 
                  getLevel() );
    if( level < 0 ) level = 1;

    // add some loot
    int nn = (int)( 5.0f * rand()/RAND_MAX ) + 3;
    //cerr << "Adding loot:" << nn << endl;
    for( int i = 0; i < nn; i++ ) {
      Item *loot;
      if( npcInfo->isSubtype( RpgItem::SCROLL ) ) {
        Spell *spell = MagicSchool::getRandomSpell( level );
        loot = session->newItem( RpgItem::getItemByName("Scroll"), 
                                 level, 
                                 spell );
      } else {
        loot = 
          session->newItem( 
            RpgItem::getRandomItemFromTypes( session->getGameAdapter()->
                                             getCurrentDepth(), 
                                             types, typeCount ), 
            level );
      }
      //cerr << "\t" << loot->getRpgItem()->getName() << endl;
      // make it contain all items, no matter what size
      addInventory( loot, true );
    }
  }
}

void Creature::evalSpecialSkills() {
  //if( !isMonster() ) cerr << "In Creature::evalSpecialSkills for " << getName() << endl;
	set<SpecialSkill*> oldSpecialSkills;
	for(int t = 0; t < SpecialSkill::getSpecialSkillCount(); t++) {
		SpecialSkill *ss = SpecialSkill::getSpecialSkill(t);
		if( specialSkills.find( ss ) != specialSkills.end() ) {
			oldSpecialSkills.insert( ss );
		}
	}
	char tmp[120];
  specialSkills.clear();
	specialSkillNames.clear();
  HSQOBJECT *param = session->getSquirrel()->getCreatureRef( this );
  if( param ) {
    bool result;
    for(int t = 0; t < SpecialSkill::getSpecialSkillCount(); t++) {
      SpecialSkill *ss = SpecialSkill::getSpecialSkill(t);
      session->getSquirrel()->
        callBoolMethod( ss->getSquirrelFunctionPrereq(),
                        param,
                        &result );
      if( result ) {
        specialSkills.insert( ss );
				string skillName = ss->getName();
				specialSkillNames.insert( skillName );
				if( oldSpecialSkills.find( ss ) == oldSpecialSkills.end() ) {
					if( session->getParty()->isPartyMember( this ) ) {
						sprintf( tmp, _( "%1$s gains the %2$s special ability!" ), getName(), ss->getDisplayName() );
						session->getMap()->addDescription( tmp, 0.3f, 1.0f, 0.2f );
					}
				}
      }
    }
  }
	for(int t = 0; t < SpecialSkill::getSpecialSkillCount(); t++) {
		SpecialSkill *ss = SpecialSkill::getSpecialSkill(t);
		if( specialSkills.find( ss ) == specialSkills.end() &&
				oldSpecialSkills.find( ss ) != oldSpecialSkills.end() ) {
			if( session->getParty()->isPartyMember( this ) ) {
				sprintf( tmp, _( "%1$s looses the %2$s special ability!" ), getName(), ss->getDisplayName() );
				session->getMap()->addDescription( tmp, 1.0f, 0.3f, 0.2f );
			}
		}
	}
}

void Creature::setSkill(int index, int value) { 
	int oldValue = getSkill( index );
  skills[index] = ( value < 0 ? 0 : value );
	skillChanged( index, oldValue, getSkill( index ) );
  evalSpecialSkills();
  session->getParty()->recomputeMaxSkills();
}

void Creature::setSkillBonus( int index, int value ) { 
	int oldValue = getSkill( index );
  skillBonus[index] = value;
	skillChanged( index, oldValue, getSkill( index ) );
  session->getParty()->recomputeMaxSkills();
}

void Creature::setSkillMod( int index, int value ) {
	int oldValue = getSkill( index );
	skillMod[ index ] = ( value < 0 ? 0 : value );
	skillChanged( index, oldValue, getSkill( index ) );
	session->getParty()->recomputeMaxSkills();
}

/**
 * Recalculate skills when stats change.
 */
void Creature::skillChanged( int index, int oldValue, int newValue ) {
	// while loading don't update skill values.
	if( loading ) return;

	if( Skill::skills[ index ]->getGroup()->isStat() && character ) {
		for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
			int oldPrereq = 0;
			int newPrereq = 0;
			for( int t = 0; t < Skill::skills[i]->getPreReqStatCount(); t++ ) {
				int statIndex = Skill::skills[i]->getPreReqStat( t )->getIndex();
				if( statIndex == index ) {
					oldPrereq += oldValue;
					newPrereq += newValue;
				} else {
					oldPrereq += getSkill( statIndex );
					newPrereq += getSkill( statIndex );
				}
			}
			oldPrereq = (int)( ( oldPrereq / (float)( Skill::skills[i]->getPreReqStatCount() ) ) * 
												 (float)( Skill::skills[i]->getPreReqMultiplier() ) );
			newPrereq = (int)( ( newPrereq / (float)( Skill::skills[i]->getPreReqStatCount() ) ) * 
												 (float)( Skill::skills[i]->getPreReqMultiplier() ) );
			if( oldPrereq != newPrereq ) {
				setSkill( i, getSkill( i ) + ( newPrereq - oldPrereq ) );
				armorChanged = true;
			}
		}
	}
}

void Creature::applySkillMods() {
	for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
		if( skillMod[ i ] > 0 ) {
			setSkill( i, skills[ i ] + skillMod[ i ] );
			skillMod[ i ] = 0;
		}
	}
	availableSkillMod = 0;
}

void Creature::setStateMod(int mod, bool setting) { 
  if(setting) stateMod |= (1 << mod);  
  else stateMod &= ((GLuint)0xffff - (GLuint)(1 << mod)); 
  evalSpecialSkills();
}

void Creature::setProtectedStateMod(int mod, bool setting) { 
  if(setting) protStateMod |= (1 << mod);  
  else protStateMod &= ((GLuint)0xffff - (GLuint)(1 << mod)); 
  evalSpecialSkills();
}

void Creature::applyRecurringSpecialSkills() {
	for(int t = 0; t < SpecialSkill::getSpecialSkillCount(); t++) {
    SpecialSkill *skill = SpecialSkill::getSpecialSkill(t);   
    if( skill->getType() == SpecialSkill::SKILL_TYPE_RECURRING &&
        hasSpecialSkill( skill ) ) {
			useSpecialSkill( skill, false );
		}
	}
}

float Creature::applyAutomaticSpecialSkills( int event, 
                                           char *varName,
                                           float varValue ) {
#ifdef DEBUG_CAPABILITIES
  cerr << "Using automatic capabilities for event type: " << event << endl;
#endif
  for(int t = 0; t < SpecialSkill::getSpecialSkillCount(); t++) {
    SpecialSkill *skill = SpecialSkill::getSpecialSkill(t);   
    if( skill->getEvent() == event &&
        skill->getType() == SpecialSkill::SKILL_TYPE_AUTOMATIC &&
        hasSpecialSkill( skill ) ) {
#ifdef DEBUG_CAPABILITIES
      cerr << "\tusing capability: " << skill->getDisplayName() << 
        " and setting var: " << 
        varName << "=" << varValue << endl;
#endif
      session->getSquirrel()->setGlobalVariable( varName, varValue );
      useSpecialSkill( skill, false );
      varValue = session->getSquirrel()->getGlobalVariable( varName );
#ifdef DEBUG_CAPABILITIES
      cerr << "\t\tgot back " << varValue << endl;
#endif
    }
  }
#ifdef DEBUG_CAPABILITIES
  cerr << "final value=" << varValue << " ===============================" << endl;
#endif
  return varValue;
}

char *Creature::useSpecialSkill( SpecialSkill *specialSkill, 
                                 bool manualOnly ) {
  if( !hasSpecialSkill( specialSkill ) ) {
    return Constants::getMessage( Constants::UNMET_CAPABILITY_PREREQ_ERROR );
  } else if( manualOnly && 
             specialSkill->getType() != 
             SpecialSkill::SKILL_TYPE_MANUAL ) {
    return Constants::getMessage( Constants::CANNOT_USE_AUTO_CAPABILITY_ERROR );
  }
  HSQOBJECT *param = session->getSquirrel()->getCreatureRef( this );
  if( param ) {
    session->getSquirrel()->
      callNoArgMethod( specialSkill->getSquirrelFunctionAction(), 
                       param );
    return NULL;
  } else {
    cerr << "*** Error: can't find squarrel reference for creature: " << getName() << endl;
    return NULL;
  }
}






/**
 * ============================================================
 * ============================================================
 * 
 * New battle system calls
 * 
 * damage=attack_roll - ac
 * attack_roll=(item_action + item_level) % skill
 * ac=(armor_total + avg_armor_level) % skill
 * skill=avg. of ability skill (power,coord, etc.), item skill, luck
 * 
 * ============================================================
 * ============================================================
 * 
 * Fixme: 
 * -currently, extra attacks are not used. (use SPEED skill to eval?)
 * -magic item armor
 * -automatic capabilities
 *
 * move here from battle.cpp:
 * -critical hits (2x,3x,damage,etc.)  
 * -conditions modifiers
 * 
 * ** Look in old methods and battle.cpp for more info
 */

// 1 item level equals this many character levels in combat
#define ITEM_LEVEL_DIVISOR 8.0f

// base weapon damage of an attack with bare hands
#define HAND_ATTACK_DAMAGE Dice(1,4,0)

float Creature::getDodge( Creature *attacker, Item *weapon ) {
	// the target's dodge if affected by angle of attack
	bool inFOV = 
		Util::isInFOV( getX(), getY(), getTargetAngle(),
									 attacker->getX(), attacker->getY() );

	// the target's dodge
	float armor, dodgePenalty;
	getArmor( &armor, &dodgePenalty, 
						weapon ? weapon->getRpgItem()->getDamageType() : 0,
						weapon );
	float dodge = getSkill( Skill::DODGE_ATTACK ) - dodgePenalty;
	if( !inFOV ) {
		dodge /= 2.0f;
		session->getMap()->addDescription( _( "...Attack from blind-spot!" ) );
	}
	return dodge;
}

float Creature::getArmor( float *armorP, float *dodgePenaltyP, 
													int damageType, Item *vsWeapon ) {
	calcArmor( damageType, &armor, dodgePenaltyP, ( vsWeapon ? true : false ) );

  // negative feedback: for monsters only, allow hits now and then
  if( monster && 
      ( rand() / RAND_MAX < 
				monsterToughness[ session->getPreferences()->getMonsterToughness() ].
				armorMisfuction ) ) {
      // 3.0f * rand() / RAND_MAX < 1.0f ) {
    armor = ( armor / 2.0f * rand() / RAND_MAX );
  } else {
		// apply any armor enhancing capabilities
		if( vsWeapon ) {
			session->getSquirrel()->setCurrentWeapon( vsWeapon );
			armor = applyAutomaticSpecialSkills( SpecialSkill::SKILL_EVENT_ARMOR,
																					 "armor", armor );
		}
	}
	
	*armorP = armor;

	return armor;
}

void Creature::calcArmor( int damageType,
													float *armorP, 
                          float *dodgePenaltyP,
                          bool callScript ) {
  if( armorChanged ) {
		for( int t = 0; t < RpgItem::DAMAGE_TYPE_COUNT; t++ ) {
			lastArmor[ t ] = ( monster ? monster->getBaseArmor() : 0 );
			lastDodgePenalty[ t ] = 0;
			int armorCount = 0;
			for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
				if( equipped[i] != MAX_INVENTORY_SIZE ) {
					Item *item = inventory[equipped[i]];
					if( item->getRpgItem()->getType() == RpgItem::ARMOR ||
							( item->isMagicItem() && item->getBonus() > 0 && !item->getRpgItem()->isWeapon() ) ) {

						int n = ( item->getRpgItem()->getType() == RpgItem::ARMOR ?
											item->getRpgItem()->getDefense( t ) :
											item->getBonus() );

						if( callScript && !monster ) {
							session->getSquirrel()->setGlobalVariable( "armor", lastArmor[ t ] );
							session->getSquirrel()->callItemEvent( this, item, "useItemInDefense" );
							lastArmor[ t ] = session->getSquirrel()->getGlobalVariable( "armor" );
						}					
						lastArmor[ t ] += n;
						lastDodgePenalty[ t ] += item->getRpgItem()->getDodgePenalty();

						// item's level has a small influence.
						lastArmor[ t ] += item->getLevel() / 8;

            // apply the armor influence... it uses the first
            // influence slot (AP_INFLUENCE)
						lastArmor[ t ] += 
              getInfluenceBonus( item, AP_INFLUENCE, 
                                 ( callScript ? (char*)"CTH" : NULL ) );

						armorCount++;
					}
				}
			}
			lastArmor[ t ] += bonusArmor;		
			if( lastArmor[ t ] < 0 ) lastArmor[ t ] = 0;
		}
		armorChanged = false;
	}

	*armorP = lastArmor[ damageType ];
	*dodgePenaltyP = lastDodgePenalty[ damageType ];
}

#define MAX_RANDOM_DAMAGE 2.0f

float power( float base, int e ) {
	float n = 1;
	for( int i = 0; i < e; i++ ) {
		n *= base;
	}
	return n;
}

float Creature::getInfluenceBonus( Item *weapon,
																	 int influenceType, 
																	 char *debugMessage ) {

	if( !weapon ) return 0;

	float bonus = 0;
	for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
		WeaponInfluence *minInfluence = weapon->getRpgItem()->getWeaponInfluence( i, influenceType, MIN_INFLUENCE );
		WeaponInfluence *maxInfluence = weapon->getRpgItem()->getWeaponInfluence( i, influenceType, MAX_INFLUENCE );
		
		float n = 0;
		int value = getSkill( i );
		if( minInfluence->limit > -1 && minInfluence->limit > value ) {
			switch( minInfluence->type ) {
			case 'E' :
				// exponential malus
				n = -power( minInfluence->base, minInfluence->limit - value );
				break;
			case 'L' :
				// linear
				n = -( minInfluence->limit - value ) * minInfluence->base;
				break;
			default:
				cerr << "*** Error: unknown influence type for item: " << weapon->getRpgItem()->getDisplayName() << endl;
			}
		} else if( maxInfluence->limit > -1 && maxInfluence->limit < value ) {
			switch( maxInfluence->type ) {
			case 'E' :
				// exponential bonus
				n = power( maxInfluence->base, value - maxInfluence->limit );
			break;																										
			case 'L' :
				// linear bonus
				n = ( value - maxInfluence->limit ) * maxInfluence->base;
				break;
			default:
				cerr << "*** Error: unknown influence type for item: " << weapon->getRpgItem()->getDisplayName() << endl;
			}
		}
		
		if( n != 0 && debugMessage && 
				session->getPreferences()->getCombatInfoDetail() > 0 ) {
			char message[120];
			sprintf( message, "...%s %s:%s %d-%d %s %d, %s=%.2f", 
							 debugMessage, 
							 _( "skill" ),
							 Skill::skills[i]->getDisplayName(), 
							 minInfluence->limit, 
							 maxInfluence->limit, 
							 _( "vs." ),
							 value, 
							 _( "bonus" ),
							 n );
			session->getMap()->addDescription( message );
		}
		
		bonus += n;
	}
	return bonus;
}

void Creature::getCth( Item *weapon, float *cth, float *skill, bool showDebug ) {
	// the attacker's skill
	*skill = getSkill( weapon ? 
										 weapon->getRpgItem()->getDamageSkill() : 
										 Skill::HAND_TO_HAND_COMBAT );
	
	// The max cth is closer to the skill to avoid a lot of misses
	// This is ok, since dodge is subtracted from it anyway.
	float maxCth = *skill * 1.5f;
	if( maxCth > 100 ) maxCth = 100;	
	if( maxCth < 40 ) maxCth = 40;

	// roll chance to hit (CTH)
	*cth = maxCth * rand() / RAND_MAX;

	// item's level has a small influence
	if( weapon ) *skill += weapon->getLevel() / 2;

	// Apply COORDINATION influence to skill 
	// (As opposed to subtracting it from cth. This is b/c 
	// skill is shown in the characterInfo as the cth.)
	*skill += getInfluenceBonus( weapon, CTH_INFLUENCE, 
															 ( showDebug ? (char*)"CTH" : NULL ) );
	if( *skill < 0 ) *skill = 0;

	if( showDebug && session->getPreferences()->getCombatInfoDetail() > 0 ) {
		char message[120];
		sprintf( message, "...%s:%.2f (%s:%.2f) %s %s:%.2f", 
						 _( "CTH" ),
						 *cth, 
						 _( "max" ),
						 maxCth,
						 _( "vs." ),
						 _( "skill" ),
						  *skill );
		session->getMap()->addDescription( message );
	}
}

float Creature::getAttack( Item *weapon, 
													 float *maxP,
													 float *minP, 
													 bool callScript ) {

	float power;
	if( weapon && weapon->getRpgItem()->isRangedWeapon() ) {
		power = getSkill( Skill::POWER ) / 2.0f +
			getSkill( Skill::COORDINATION ) / 2.0f;
	} else {
		power = getSkill( Skill::POWER );
	}

	// the min/max power value
	float minPower, maxPower;
	if( power < 10 ) { 
		// 1d10
		minPower = 1; maxPower = 10;
	} else if( power < 15 ) { 
		// 2d10
		minPower = 2; maxPower = 20;
	} else {
		// 3d10
		minPower = 3; maxPower = 30;
	}

	// What percent of power is given by weapon? 
	// (For unarmed combat it's a coordination bonus.)
	float damagePercent = ( weapon ? 
													weapon->getRpgItem()->getDamage() : 
													getSkill( Skill::COORDINATION ) + 
													getSkill( Skill::POWER ) );

	// item's level has a small influence
	if( weapon ) damagePercent += weapon->getLevel() / 2;

	// apply POWER influence
	damagePercent += getInfluenceBonus( weapon, DAM_INFLUENCE, 
																			( callScript ? (char*)"DAM" : NULL ) );
	if( damagePercent < 0 ) damagePercent = 0;

	if( minP ) {
		*minP = ( minPower / 100.0f ) * damagePercent;
	}
	if( maxP ) {
		*maxP = ( maxPower / 100.0f ) * damagePercent;
	}

	// roll the power
	float roll = minPower + ( ( maxPower - minPower ) * rand() / RAND_MAX );

	// take the weapon's skill % of the max power
	roll = ( roll / 100.0f ) * damagePercent;	

	// apply damage enhancing capabilities
	if( callScript ) {
		session->getSquirrel()->setCurrentWeapon( weapon );
		roll = applyAutomaticSpecialSkills( SpecialSkill::SKILL_EVENT_DAMAGE,
																				"damage", roll );
		if( weapon && !monster )
				session->getSquirrel()->setGlobalVariable( "damage", roll );
		session->getSquirrel()->callItemEvent( this, weapon, "useItemInAttack" );
		roll = session->getSquirrel()->getGlobalVariable( "damage" );
	}
	
	return roll;
}

float Creature::getParry( Item **parryItem ) {
	int location[] = {
		Constants::INVENTORY_RIGHT_HAND,
		Constants::INVENTORY_LEFT_HAND,
		Constants::INVENTORY_WEAPON_RANGED,
		-1
	};
	float ret = 0;
	float maxParry = 0;
	for( int i = 0; location[i] > -1; i++ ) {
		Item *item = getItemAtLocation( location[i] );
		if( !item ) continue;
		if( item->getRpgItem()->getDefenseSkill() == Skill::SHIELD_DEFEND ) {
			// parry using a shield: use shield skill to parry
			maxParry = getSkill( item->getRpgItem()->getDefenseSkill() );
		} else if( item->getRpgItem()->getParry() > 0 ) {
			// parry using a weapon: get max parry skill amount (% of weapon skill)
			maxParry = 
				( getSkill( item->getRpgItem()->getDamageSkill() ) / 100.0f ) * 
				item->getRpgItem()->getParry();
			
			// use the item's CTH skill to modify parry also
			maxParry += getInfluenceBonus( item, CTH_INFLUENCE, "PARRY" );
			if( maxParry < 0 ) maxParry = 0;

		} else {
			// no parry with this hand
			continue;
		}
		// roll to parry
		float parry = maxParry * rand() / RAND_MAX;
		// select the highest value
		if( ret == 0 || ret < parry ) {
			ret = parry;
			*parryItem = item;
		}
	}
	return ret;
}

/**
 * Apply this to the damage caused to the defender.
 */
float Creature::getDefenderStateModPercent( bool magical ) {
    /* 
      apply state_mods:
      (Done here so it's used for spells too)
      
      blessed, 
      empowered, 
      enraged, 
      ac_protected, 
      magic_protected, 

      drunk, 

      poisoned, 
      cursed, 
      possessed, 
      blinded, 
      charmed, 
      changed,
      overloaded,
    */
    float delta = 0.0f;
    if(getStateMod(StateMod::blessed)) {
      delta += (10.0f * rand()/RAND_MAX);
    }
    if(getStateMod(StateMod::empowered)) {
      delta += (10.0f * rand()/RAND_MAX) + 5;
    }
    if(getStateMod(StateMod::enraged)) {
      delta += (10.0f * rand()/RAND_MAX) + 8;
    }
    if(getStateMod(StateMod::drunk)) {
      delta += (14.0f * rand()/RAND_MAX) - 7;
    }
    if(getStateMod(StateMod::cursed)) {
      delta -= ((10.0f * rand()/RAND_MAX) + 5);
    }
    if(getStateMod(StateMod::blinded)) {
      delta -= (10.0f * rand()/RAND_MAX);
    }
    if(!magical && getTargetCreature()->getStateMod(StateMod::ac_protected)) {
      delta -= (7.0f * rand()/RAND_MAX);
    }
    if(magical && getTargetCreature()->getStateMod(StateMod::magic_protected)) {
      delta -= (7.0f * rand()/RAND_MAX);
    }
    if(getTargetCreature()->getStateMod(StateMod::blessed)) {
      delta -= (5.0f * rand()/RAND_MAX);
    }
    if(getTargetCreature()->getStateMod(StateMod::cursed)) {
      delta += (5.0f * rand()/RAND_MAX);
    }
    if(getTargetCreature()->getStateMod(StateMod::overloaded)) {
      delta += (2.0f * rand()/RAND_MAX);
    }
    if(getTargetCreature()->getStateMod(StateMod::blinded)) {
      delta += (2.0f * rand()/RAND_MAX);
    }
    if(getTargetCreature()->getStateMod(StateMod::invisible)) {
      delta -= (10.0f * rand()/RAND_MAX);
    }
    return delta;
}

/**
 * Apply this to the attack roll.
 */
float Creature::getAttackerStateModPercent() {
  /* 
    apply state_mods:
    blessed, 
	  empowered, 
  	enraged, 
  	ac_protected, 
  	magic_protected, 
    
  	drunk, 
    
  	poisoned, 
  	cursed, 
  	possessed, 
  	blinded, 
  	charmed, 
  	changed,
  	overloaded,
  */
  float delta = 0.0f;
  if(getStateMod(StateMod::blessed)) {
    delta += (15.0f * rand()/RAND_MAX);
  }
  if(getStateMod(StateMod::empowered)) {
    delta += (15.0f * rand()/RAND_MAX) + 10;
  }
  if(getStateMod(StateMod::enraged)) {
    delta -= (10.0f * rand()/RAND_MAX);
  }
  if(getStateMod(StateMod::drunk)) {
    delta += (30.0f * rand()/RAND_MAX) - 15;
  }
  if(getStateMod(StateMod::cursed)) {
    delta -= ((15.0f * rand()/RAND_MAX) + 10);
  }
  if(getStateMod(StateMod::blinded)) {
    delta -= (15.0f * rand()/RAND_MAX);
  }
  if(getStateMod(StateMod::overloaded)) {
    delta -= (10.0f * rand()/RAND_MAX);
  }
  if(getStateMod(StateMod::invisible)) {
    delta += (5.0f * rand()/RAND_MAX) + 5;
  }
  return delta;
}

float Creature::rollMagicDamagePercent( Item *item ) {
  float itemLevel = ( item->getLevel() - 1 ) / ITEM_LEVEL_DIVISOR;
  return item->rollMagicDamage() + itemLevel;
}

float Creature::getMaxAP( ) { 
	return( (float)( getSkill( Skill::COORDINATION ) ) +
					(float)( getSkill( Skill::SPEED ) ) ) / 2.0f;
}

float Creature::getAttacksPerRound( Item *item ) {
  return( getMaxAP() / getWeaponAPCost( item, false ) );
}

float Creature::getWeaponAPCost( Item *item, bool showDebug ) {
	float baseAP = ( item ? 
								 item->getRpgItem()->getAP() : 
								 Constants::HAND_WEAPON_SPEED );
	// never show debug (called a lot)
	// Apply a max 3pt influence to weapon AP
	baseAP -= getInfluenceBonus( item, AP_INFLUENCE, NULL );
	// can't be free..
	if( baseAP < 1 ) baseAP = 1;
	return baseAP;
}

char *Creature::canEquipItem( Item *item, bool interactive ) {

	// check item tags to see if this item can be equipped.
  if( character ) {		
		if( !character->canEquip( item->getRpgItem() ) ) {
			return Constants::getMessage( Constants::ITEM_ACL_VIOLATION );
		}
  }

	// check the level
	if( getLevel() < item->getLevel() ) {
		return Constants::getMessage( Constants::ITEM_LEVEL_VIOLATION );
	}

  // two handed weapon violations
  if( item->getRpgItem()->getEquip() & Constants::INVENTORY_LEFT_HAND ||
      item->getRpgItem()->getEquip() & Constants::INVENTORY_RIGHT_HAND ) {
    Item *leftHandWeapon = getItemAtLocation( Constants::INVENTORY_LEFT_HAND );
    Item *rightHandWeapon = getItemAtLocation( Constants::INVENTORY_RIGHT_HAND );
    bool bothHandsFree = !( leftHandWeapon || rightHandWeapon );
    bool holdsTwoHandedWeapon =
      ( ( leftHandWeapon && leftHandWeapon->getRpgItem()->getTwoHanded() == RpgItem::ONLY_TWO_HANDED ) ||
        ( rightHandWeapon && rightHandWeapon->getRpgItem()->getTwoHanded() == RpgItem::ONLY_TWO_HANDED ) );

    if( holdsTwoHandedWeapon ||
        ( !bothHandsFree && 
          item->getRpgItem()->getTwoHanded() == RpgItem::ONLY_TWO_HANDED ) ) {
      if( interactive ) {
        return Constants::getMessage(Constants::ITEM_TWO_HANDED_VIOLATION);
      }
    }
  }
  return NULL;
}

void Creature::setCharacter( Character *c ) {
  assert( !isMonster() );  
	character = c;
}

// does the path end in the target creature
bool Creature::isPathToTargetCreature() {
  if( bestPath.size() <= 0 || !getTargetCreature() ) return false;
  Location pos = bestPath[ bestPath.size() - 1 ];
  Location *mapPos = session->getMap()->getLocation( pos.x, pos.y, 0 );
  return( mapPos && mapPos->creature == getTargetCreature() );
}

void Creature::playCharacterSound( int soundType ) {
  if( !monster ) session->getGameAdapter()->playCharacterSound( model_name, soundType );
}

bool Creature::rollSkill( int skill, float luckDiv ) {
  float f = (float)( getSkill( skill ) );
  if( luckDiv > 0 ) f += (float)( getSkill( Skill::LUCK ) ) / luckDiv;
  return( 100.0f * rand() / RAND_MAX <= f ? true : false );
}             

#define SECRET_DOOR_ATTEMPT_INTERVAL 5000
bool Creature::rollSecretDoor( Location *pos ) {
  if( secretDoorAttempts.find( pos ) != secretDoorAttempts.end() ) {
    Uint32 lastTime = secretDoorAttempts[ pos ];
    if( SDL_GetTicks() - lastTime < SECRET_DOOR_ATTEMPT_INTERVAL ) return false;
  }
  bool ret = rollSkill( Skill::FIND_SECRET_DOOR, 4.0f );
  if( !ret ) {
    secretDoorAttempts[ pos ] = SDL_GetTicks();
  }
  return ret;
}

void Creature::resetSecretDoorAttempts() {
  secretDoorAttempts.clear();
}
