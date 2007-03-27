/***************************************************************************
                          item.h  -  description
                             -------------------
    begin                : Sun Sep 28 2003
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

#ifndef ITEM_H
#define ITEM_H

#include "common/constants.h"
#include "persist.h"
#include "render/rendereditem.h"
#include "storable.h"
#include "rpg/rpg.h"
#include <vector>

class RpgItem;
class Color;
class GLShape;
class Spell;
class MagicSchool;
class Dice;
class Session;
class ShapePalette;
class ConfigLang;
class ConfigNode;
class Scourge;

/**
  *@author Gabor Torok

  This class is both the UI representation (shape) of the rpgItem and it's state (for example, wear).
  All instances of the RpgItem point to the same RpgItem, but a new Item is created for each.

  */

class Item : public RenderedItem, Storable {
 private:
  RpgItem *rpgItem;
  int shapeIndex;
  Color *color;
  GLShape *shape;
  bool blocking;
  Item *containedItems[MAX_CONTAINED_ITEMS];
  int containedItemCount;
  int currentCharges;
  Spell *spell;
  char itemName[255];
  bool containsMagicItem;
  bool showCursed;



  // Things that change with item level (override rpgitem values)
  int level;
  float weight; 
  int price;
  int quality;

  // former magic attrib stuff
  int magicLevel;
  int bonus; // e.g.: sword +2
  int damageMultiplier; // 2=double damage, 3=triple, etc.
  char *monsterType; // if not NULL, damageMultiplier only for this type of monster.
  MagicSchool *school; // magic damage by a school (or NULL if N/A)
  Dice *magicDamage; 
  bool cursed;
  int stateMod[StateMod::STATE_MOD_COUNT]; // 0=nothing, 1=sets, 2=clears/protects against state mod when worn
  bool stateModSet;
  std::map<int, int> skillBonus;
  Session *session;	
	Uint32 identifiedBits;

	int inventoryX, inventoryY;

public:

	enum {
		ID_BONUS=0,
		ID_DAMAGE_MUL,
		ID_MAGIC_DAMAGE,
		ID_STATE_MOD,
		ID_PROT_STATE_MOD,
		ID_SKILL_BONUS,
		ID_CURSED,

		ID_COUNT
	};

  Item(Session *session, RpgItem *rpgItem, int level=1, bool loading=false );
  ~Item();

	void renderIcon( Scourge *scourge, int x, int y, int w, int h );
	void getTooltip( char *tooltip );

	inline void setInventoryLocation( int x, int y ) {
		inventoryX = x;
		inventoryY = y;
	}
	inline int getInventoryX() { return inventoryX; }
	inline int getInventoryY() { return inventoryY; }
	int getInventoryWidth();
	int getInventoryHeight();

	inline void setIdentifiedBit( int bit, bool value ) {
		if( value ) identifiedBits |= ( 1 << bit );  
		else identifiedBits &= ( (Uint32)0xffff - (Uint32)( 1 << bit ) ); 
	}
	inline bool getIdentifiedBit( int bit ) { return( identifiedBits & ( 1 << bit ) ? true : false ); }  
	void identify( int infoDetailLevel );
	inline bool isIdentified() { return( isMagicItem() && identifiedBits >= ( 1 << ID_COUNT ) - 1 ); }


  ItemInfo *save();
  //ContainedItemInfo saveContainedItems();
  static Item *load(Session *session, ItemInfo *info);

  static std::map<int, std::vector<std::string> *> soundMap;
  
  inline Color *getColor() { return color; }
  inline void setColor(Color *c) { color = c; }
  inline void setShape(GLShape *s) { shape = s; }
  inline GLShape *getShape() { return shape; }
  inline RpgItem *getRpgItem() { return rpgItem; }
  inline bool isBlocking() { return blocking; }
  inline void setBlocking(bool b) { blocking = b; }
  inline int getCurrentCharges() { return currentCharges; }
  void setCurrentCharges(int n); 
  inline void setWeight(float f) { if(f < 0.0f)f=0.1f; weight=f; }
  void setSpell(Spell *spell);
  inline Spell *getSpell() { return spell; }

  inline void setShowCursed( bool b ) { showCursed = b; }
  inline bool getShowCursed() { return showCursed; }

  void getDetailedDescription(char *s, bool precise=true);
  inline char *getItemName() { return itemName; }

  inline int getContainedItemCount() { return containedItemCount; }
  // return true if successful
  bool addContainedItem(Item *item, bool force=false);
  // return removed item, or NULL
  Item *removeContainedItem(int index);
  Item *getContainedItem(int index);
  bool isContainedItem(Item *item);
  inline bool getContainsMagicItem() { return containsMagicItem; }

  // return true if the item is used up
  // this method also adjusts weight
  bool decrementCharges();

  const char *getRandomSound();
  
  static void initItems(ShapePalette *shapePal);

  void enchant(int level);

	char *getType();


  // level-based attributes
  inline int getLevel() { return level; }
  inline float getWeight() { return weight; }
  inline int getPrice() { return price; }
  inline int getQuality() { return quality; }

  inline bool isMagicItem() { return ( magicLevel > -1 ); }
  bool isSpecial();
  inline std::map<int,int> *getSkillBonusMap() { return &skillBonus; }
  inline int getSkillBonus(int skill) { return (skillBonus.find(skill) == skillBonus.end() ? 0 : skillBonus[skill]); }
  inline int getMagicLevel() { return magicLevel; }
  inline int getBonus() { return bonus; }
  inline int getDamageMultiplier() { return damageMultiplier; }
  inline char *getMonsterType() { return monsterType; }
  inline MagicSchool *getSchool() { return school; }
  int rollMagicDamage();
  int getMagicResistance();
  char *describeMagicDamage();
  inline bool isCursed() { return cursed; }
  inline bool isStateModSet(int mod) { return(stateMod[mod] == 1); }
  inline bool isStateModProtected(int mod) { return(stateMod[mod] == 2); }
	int getRange();

  void debugMagic(char *s);

  // storable interface
  const char *getName();
  int getIconTileX();
  int getIconTileY();
  int getStorableType();
  const char *isStorable();

 protected:
  void commonInit( bool loading );
  void describeMagic(char *s, char *itemName);
  
  DiceInfo *saveDice( Dice *dice );
  static DiceInfo *saveEmptyDice();
  static Dice *loadDice( Session *session, DiceInfo *info );

	static void initItemTypes( ConfigLang *config );
	static void initItemEntries( ConfigLang *config, ShapePalette *shapePal );
	static void initSounds( ConfigLang *config );
	static void initTags( ConfigLang *config );
	static void decodeInfluenceBlock( RpgItem *item, 
																		int skill, 
																		std::vector<ConfigNode*> *nodes, 
																		int influenceType );	
};

#endif