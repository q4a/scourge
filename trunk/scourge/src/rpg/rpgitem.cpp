/***************************************************************************
                          rpgitem.cpp  -  description
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
#include "rpgitem.h"

/**
   These basic objects are enhanced by adding magical capabilities.
 */
RpgItem *RpgItem::items[] =  {

  // SWORDS:
  new RpgItem(SHORT_SWORD, "Short sword", 1, SWORD, 1.5f, 150, 100, 4, 10, 
			  "A dwarven shortsword of average workmanship",
			  "A stubby short sword", 
			  Character::INVENTORY_LEFT_HAND | Character::INVENTORY_RIGHT_HAND,
			  Constants::SWORD_INDEX, NOT_TWO_HANDED, 1, Constants::SWORD_WEAPON),
  new RpgItem(DAGGER, "Dagger", 1, SWORD, 0.2f, 80, 100, 3, 7, 
			  "There's nothing special about this dagger",
			  "A small dagger", 
			  Character::INVENTORY_LEFT_HAND | Character::INVENTORY_RIGHT_HAND,
			  Constants::SWORD_INDEX, NOT_TWO_HANDED, 1, Constants::SWORD_WEAPON),
  new RpgItem(BASTARD_SWORD, "Bastard sword", 1, SWORD, 2.0f, 200, 100, 8, 15,
			  "A bastard sword can be wielded either by one or both hands... (If you still have both hands)",
			  "A rusty bastard sword", 
			  Character::INVENTORY_LEFT_HAND | Character::INVENTORY_RIGHT_HAND,
			  Constants::SWORD_INDEX, OPTIONAL_TWO_HANDED, 1, Constants::SWORD_WEAPON),
  new RpgItem(LONG_SWORD, "Long sword", 2, SWORD, 4.5f, 250, 100, 10, 25,
			  "The longsword is a knight's standard weapon",
			  "A shiny, sharp longsword", 
			  Character::INVENTORY_LEFT_HAND | Character::INVENTORY_RIGHT_HAND,
			  Constants::SWORD_INDEX, NOT_TWO_HANDED, 2, Constants::SWORD_WEAPON),
  new RpgItem(GREAT_SWORD, "Great sword", 2, SWORD, 6.0f, 350, 100, 16, 35,
			  "The two handed great sword can deliver a lot of damage",
			  "A two-handed greatsword", 
			  Character::INVENTORY_LEFT_HAND | Character::INVENTORY_RIGHT_HAND,
			  Constants::SWORD_INDEX, ONLY_TWO_HANDED, 3, Constants::SWORD_WEAPON),
  
  
  // AXES
  new RpgItem(BATTLE_AXE, "Battleaxe", 1, AXE, 2.0f, 150, 100, 6, 12,
			  "A battle axe of average workmanship",
			  "A well made battle axe", 
			  Character::INVENTORY_LEFT_HAND | Character::INVENTORY_RIGHT_HAND,
			  Constants::AXE_INDEX, NOT_TWO_HANDED, 2, Constants::AXE_WEAPON),
  new RpgItem(THROWING_AXE, "Throwing axe", 1, AXE, 1.0f, 100, 100, 4, 8,
			  "A dull-looking axe for chucking",
			  "A quick throwing axe", 
			  Character::INVENTORY_LEFT_HAND | Character::INVENTORY_RIGHT_HAND,
			  Constants::AXE_INDEX, NOT_TWO_HANDED, 12, Constants::AXE_WEAPON),

  // ARMOR
  new RpgItem(HORNED_HELMET, "Horned helmet", 1, ARMOR, 12.0f, 100, 100, 4, 0,
			  "Your basic head-gear; protects against damage by dull, bludgeoning objects",
			  "Your basic head-gear, adorned with horns for increased potency", 
			  Character::INVENTORY_HEAD,
			  Constants::AXE_INDEX, NOT_TWO_HANDED, 0, Constants::ARMOR_DEFEND),

  
  // CONTAINERS:
  new RpgItem(CHEST, "Chest", 1, CONTAINER, 60.0f, CONTAINER, 100, 0, 0,
			  "A wooden chest with metal re-inforced edges",
			  "An ancient chest", 0,
			  Constants::CHEST_INDEX),
  new RpgItem(BOOKSHELF, "Bookshelf", 1, CONTAINER, 120.0f, CONTAINER, 100, 0, 0,
			  "A bookshelf containing tomes of old",
			  "A large bookself", 0,
			  Constants::BOOKSHELF_INDEX),
  new RpgItem(CHEST2, "Chest", 1, CONTAINER, 60.0f, CONTAINER, 100, 0, 0,
			  "A wooden chest with metal re-inforced edges",
			  "An ancient chest", 0,
			  Constants::CHEST2_INDEX),
  new RpgItem(BOOKSHELF2, "Bookshelf", 1, CONTAINER, 120.0f, CONTAINER, 100, 0, 0,
			  "A bookshelf containing tomes of old",
			  "A large bookself", 0,
			  Constants::BOOKSHELF2_INDEX),
  new RpgItem(CORPSE, "Corpse", 1, CONTAINER, 100.0f, CONTAINER, 100, 0, 0,
			  "The decomposing corpse of one once strong and able",
			  "A decomposing corpse", 0,
			  Constants::CORPSE_INDEX),
  new RpgItem(TABLE, "Table", 1, OTHER, 40.0f, OTHER, 100, 0, 0,
			  "A large featureless wooden table",
			  "A large table", 0,
			  Constants::TABLE_INDEX),
  new RpgItem(CHAIR, "Chair", 1, OTHER, 15.0f, OTHER, 100, 0, 0,
			  "A simple wooden chair",
			  "A simple chair", 0,
			  Constants::CHAIR_INDEX),			  
			  
  // FOOD & DRINK
  new RpgItem(APPLE, "Apple", 1, FOOD, 0.1f, 5, 100, 0, 0,
			  "A tiny red apple",
			  "An apple", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1),			  
 new RpgItem(BANANA, "Banana", 1, FOOD, 0.1f, 5, 100, 0, 0,
			  "A simple banana",
			  "A banana", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1),  
  new RpgItem(BREAD, "Bread", 1, FOOD, 0.1f, 5, 100, 0, 0,
			  "A portion of bread",
			  "Bread", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1), 			  
  new RpgItem(CHOCOLATE_BAR, "Chocolate", 1, FOOD, 0.2f, 50, 100, 0, 0,
			  "A chocolate bar",
			  "Chocolate", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1), 			  
  new RpgItem(MUSHROOM, "Mushroom", 1, FOOD, 0.4f, 25, 100, 0, 0,
			  "A big mushroom",
			  "A mushroom", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1),    			                   
  new RpgItem(TINY_EGG, "Tiny egg", 1, FOOD, 0.1f, 5, 100, 0, 0,
			  "A tiny egg",
			  "An egg", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1),    
  new RpgItem(BIG_EGG, "Big egg", 2, FOOD, 0.5f, 50, 100, 0, 0,
			  "A big egg",
			  "An egg", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1),   
  new RpgItem(MUTTON_MEAT, "Mutton meat", 3, FOOD, 1.0f, 80, 100, 0, 0,
			  "A big slice of mutton meat",
			  "A slice of mutton meat", 0,
			  Constants::SWORD_INDEX, 0,0,0, 2, 2),   
  new RpgItem(WATER_POTION, "Water_potion", 1, DRINK, 0.2f, 5, 100, 0, 0,
			  "A potion full of water",
			  "Water", 0,
			  Constants::SWORD_INDEX, 0,0,0, 1, 1),   
  new RpgItem(WATER_BOTTLE, "Water bottle", 1, DRINK, 0.5f, 30, 100, 0, 0,
			  "A bottle full of water",
			  "Water", 0,
			  Constants::SWORD_INDEX, 0,0,0, 3, 3),   
  new RpgItem(MILK_BOTTLE, "Milk", 2, DRINK, 0.5f, 50, 100, 0, 0,
			  "A bottle full of milk",
			  "Milk", 0,
			  Constants::SWORD_INDEX, 0,0,0, 3, 3),   
  new RpgItem(WINE_BOTTLE, "Wine bottle", 2, DRINK, 0.5f, 75, 100, 0, 0,
			  "A wine bottle",
			  "Wine", 0,
			  Constants::SWORD_INDEX, 0,0,0, 3, 3),   
  new RpgItem(FINE_WINE_BOTTLE, "Fine wine bottle", 2, DRINK, 0.5f, 300, 100, 0, 0,
			  "A fine wine bottle",
			  "Fine wine", 0,
			  Constants::SWORD_INDEX, 0,0,0, 5, 5), 
  new RpgItem(WATER_BARREL, "Water barrel", 3, DRINK, 4.0f, 200, 100, 0, 0,
			  "A barrel full of water",
			  "Water", 0,
			  Constants::SWORD_INDEX, 0,0,0, 8, 8), 
  new RpgItem(WINE_BARREL, "Wine barrel", 3, DRINK, 4.0f, 400, 100, 0, 0,
			  "A barrel full of wine",
			  "Wine", 0,
			  Constants::SWORD_INDEX, 0,0,0, 8, 8), 
  new RpgItem(BEER_BARREL, "Beer barrel", 3, DRINK, 4.0f, 300, 100, 0, 0,
			  "A barrel full of beer",
			  "Beer", 0,
			  Constants::SWORD_INDEX, 0,0,0, 8, 8)

};

RpgItem::RpgItem(int index, char *name, int level, int type, float weight, int price, int quality, 
				 int action, int speed, char *desc, char *shortDesc, int equip, int shape_index, 
				 int twohanded, int distance, int skill, int currentCharges, int maxCharges) {
  this->index = index;
  this->name = name;
  this->level = level;
  this->type = type;
  this->weight = weight;
  this->price = price;
  this->quality = quality;
  this->action = action;
  this->speed = speed;
  this->desc = desc;
  this->shortDesc = shortDesc;
  this->shape_index = shape_index;
  this->equip = equip;
  this->twohanded = twohanded;
  this->distance = distance;
  this->skill = skill;
  this->currentCharges = currentCharges;
  this->maxCharges = maxCharges;
}

RpgItem::~RpgItem() {
}

RpgItem *RpgItem::getRandomItem(int level) {
  int n = (int) (5.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return items[RpgItem::SHORT_SWORD];
  case 1: return items[RpgItem::DAGGER];
  case 2: return items[RpgItem::BASTARD_SWORD];
  case 3: return items[RpgItem::BATTLE_AXE];
  case 4: return items[RpgItem::THROWING_AXE];  
  default: return NULL; // won't happen
  }
}

RpgItem *RpgItem::getRandomContainer() {
  int n = (int)(10.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return items[RpgItem::BOOKSHELF];
  case 1: return items[RpgItem::CHEST];
  default: return NULL;
  }
}

RpgItem *RpgItem::getRandomContainerNS() {
  int n = (int)(10.0 * rand()/RAND_MAX);
  switch(n) {
  case 0: return items[RpgItem::BOOKSHELF2];
  case 1: return items[RpgItem::CHEST2];
  default: return NULL;
  }
}
