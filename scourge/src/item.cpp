/***************************************************************************
                          item.cpp  -  description
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

#include "item.h"

Item::Item(RpgItem *rpgItem) {
  this->rpgItem = rpgItem;
  this->shapeIndex = this->rpgItem->getShapeIndex();
  this->color = NULL;
  this->shape = ShapePalette::getInstance()->getShape(shapeIndex);
  // for now objects larger than 1 height will block (we can change this later)
  // this is so the player is not blocked by swords and axes on the ground
  this->blocking = (shape->getHeight() > 1);
  this->containedItemCount = 0;
  currentCharges = rpgItem->getMaxCharges();
  weight = rpgItem->getWeight();
}

Item::~Item(){
}

bool Item::addContainedItem(Item *item) { 
  if(containedItemCount < MAX_CONTAINED_ITEMS) {
	containedItems[containedItemCount++] = item; 
	return true;
  } else return false;
}
 
Item *Item::removeContainedItem(int index) {
  Item *item = NULL;
  if(index >= 0 && index < containedItemCount) {
	item = containedItems[index];
	containedItemCount--;
	for(int i = index; i < containedItemCount; i++) {
	  containedItems[i] = containedItems[i + 1];
	}
  }
  return item;
}

Item *Item::getContainedItem(int index) {
  return ((index >= 0 && index < containedItemCount) ? containedItems[index] : NULL);
}

bool Item::isContainedItem(Item *item) {
	for(int i = 0; i < containedItemCount; i++) {
		if(containedItems[i] == item || 
			 (containedItems[i]->getRpgItem()->getType() == RpgItem::CONTAINER &&
				containedItems[i]->isContainedItem(item))) return true;
	}
	return false;
}

void Item::getDetailedDescription(char *s, bool precise){
    int type;
    RpgItem * rpgItem;
    
    rpgItem  = getRpgItem();
    type = rpgItem->getType();
    if(type == RpgItem::DRINK || type == RpgItem::POTION || type == RpgItem::FOOD){
        sprintf(s, "(Q:%d,W:%2.2f, N:%d/%d) %s", 
            rpgItem->getQuality(), 
			rpgItem->getWeight(),
			getCurrentCharges(),
			rpgItem->getMaxCharges(),
			(precise ? rpgItem->getName() : rpgItem->getShortDesc()));
    }
    else{   
	   sprintf(s, "(A:%d,S:%d,Q:%d,W:%2.2f) %s", 
			rpgItem->getAction(), 
			rpgItem->getSpeed(), 
			rpgItem->getQuality(), 
			rpgItem->getWeight(),
			(precise ? rpgItem->getName() : rpgItem->getShortDesc()));
    }
}

// this should really be in RpgItem but that class can't reference ShapePalette and shapes.
void Item::initItems(ShapePalette *shapePal) {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/items.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
	sprintf(errMessage, "Unable to find the file: %s!", s);
	cerr << errMessage << endl;
	exit(1);
  }

  int itemCount = 0, potionTime;
  char name[255], type[255], shape[255], skill[255], potionSkill[255];
  char long_description[500], short_description[120];
  char line[255];
  int n = fgetc(fp);
  while(n != EOF) {
	if(n == 'I') {
	  // skip ':'
	  fgetc(fp);
	  // read the rest of the line
	  n = Constants::readLine(name, fp);
	  n = Constants::readLine(line, fp);
	  int level = atoi(strtok(line + 1, ","));
	  char *p = strtok(NULL, ",");
	  int action = 0;
	  int speed = 0;
	  int distance = 0;
	  int maxCharges = 0;
	  if(p) {
		action = atoi(p);
		speed = atoi(strtok(NULL, ","));
		distance = atoi(strtok(NULL, ","));
		maxCharges = atoi(strtok(NULL, ","));
		p = strtok(NULL, ",");
		if(p) strcpy(potionSkill, p);
		else strcpy(potionSkill, "");
		p = strtok(NULL, ",");
		potionTime = (p ? atoi(p) : 0);
	  }
	  n = Constants::readLine(line, fp);
	  strcpy(type, strtok(line + 1, ","));
	  float weight = strtod(strtok(NULL, ","), NULL);
	  int price = atoi(strtok(NULL, ","));

	  int inventory_location = 0;
	  int twohanded = 0;
	  strcpy(shape, "");
	  strcpy(skill, "");
	  p = strtok(NULL, ",");	  
	  if(p) {
		strcpy(shape, p);
		p = strtok(NULL, ",");
		if(p) {
		  inventory_location = atoi(p);
		  twohanded = atoi(strtok(NULL, ","));
		  strcpy(skill, strtok(NULL, ","));
		}		
	  }
	  
	  n = Constants::readLine(line, fp);
	  strcpy(long_description, line + 1);
	  
	  n = Constants::readLine(line, fp);
	  strcpy(short_description, line + 1);
	  
	  // resolve strings
	  int type_index = RpgItem::getTypeByName(type);	  
	  cerr << "item: looking for shape: " << shape << endl;
	  int shape_index = shapePal->findShapeIndexByName(shape);
	  cerr << "\tindex=" << shape_index << endl;
	  int skill_index = Constants::getSkillByName(skill);
	  if(skill_index < 0) {
		if(strlen(skill)) cerr << "*** WARNING: cannot find skill: " << skill << endl;
		skill_index = 0;
	  }
	  int potion_skill = -1;
	  if(potionSkill != NULL && strlen(potionSkill)) {
		potion_skill = Constants::getSkillByName(potionSkill);
		if(potion_skill < 0) {
		  // try special potion 'skills' like HP, AC boosts
		  potion_skill = Constants::getPotionSkillByName(potionSkill);
		  if(potion_skill == -1) {
			cerr << "*** WARNING: cannot find potion_skill: " << potionSkill << endl;
		  }
		}
		cerr << "**** potionSkill=" << potionSkill << " potion_skill=" << potion_skill << endl;
	  }
	  RpgItem::addItem(new RpgItem(itemCount++, strdup(name), level, type_index, 
								   weight, price, 100, 
								   action, speed, strdup(long_description), 
								   strdup(short_description), 
								   inventory_location, shape_index, 
								   twohanded, 
								   (distance < (int)Constants::MIN_DISTANCE ? 
									(int)Constants::MIN_DISTANCE : distance), 
								   skill_index, maxCharges, potion_skill, potionTime));	  
	} else {
	  n = Constants::readLine(line, fp);
	}
  }
  fclose(fp);
}

// return true if the item is used up
bool Item::decrementCharges(){
    float f1;
    int oldCharges;
    
    oldCharges = getCurrentCharges();            
    if(oldCharges <= 1){
        // The object is totally consummed
        return true;    
    }            
    setCurrentCharges(oldCharges - 1);
            
    // Compute initial weight to be able to compute new weight
    // (without increasing error each time)
    
    f1 = getWeight();
    f1 *= (float) (getRpgItem()->getMaxCharges());
    f1 /= (float) oldCharges;
    f1 *= (((float)oldCharges - 1.0f) / (float)(getRpgItem()->getMaxCharges()));            
    setWeight(f1);
    return false;      
}
