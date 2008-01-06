/***************************************************************************
                          spell.cpp  -  description
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
#include "spell.h"
#include "rpg.h"

using namespace std;

MagicSchool *MagicSchool::schools[10];
int MagicSchool::schoolCount = 0;
map<string, Spell*> Spell::spellMap;
map<string, MagicSchool*> MagicSchool::schoolMap;


// FIXME: move this to some other location if needed elsewhere (rpg/dice.cpp?)
Dice::Dice(char *s) {
  this->s = s;
  frees = false;

  char tmp[80];
  strcpy(tmp, s);
  if(strchr(tmp, 'd')) {
    count = atoi(strtok(tmp, "d"));
    sides = atoi(strtok(NULL, "+"));
    char *p = strtok(NULL, "+");
    if(p) mod = atoi(p);
    else mod = 0;
  } else {
    mod = atoi(s);
  }

  //  cerr << "DICE: count=" << count << " sides=" << sides << " mod=" << mod << " str=" << this->s << endl;
  //  cerr << "\troll 1:" << roll() << endl;
  //  cerr << "\troll 2:" << roll() << endl;
  //  cerr << "\troll 3:" << roll() << endl;
}

Dice::Dice(int count, int sides, int mod) {
  this->count = count;
  this->sides = sides;
  this->mod = mod;
  s = (char*)malloc(80);
  sprintf(s, "%dd%d", count, sides);
  if(mod) {
    // FIXME: handle negative numbers
    sprintf(s, "+%d", mod);
  }
  frees = true;
}

Dice::~Dice() {
  if(frees) free(s);
}




MagicSchool::MagicSchool(char *name, char *displayName, char *deity, int skill, int resistSkill, float red, float green, float blue, char *symbol) {
  this->name = name;
  this->displayName = displayName;
  this->shortName = strdup(name);
  shortName = strtok(shortName, " ");
  this->deity = deity;
  strcpy( this->deityDescription, "" );
  this->skill = skill;
  this->resistSkill = resistSkill;
  this->red = red;
  this->green = green;
  this->blue = blue;
  this->symbol = symbol;
}

MagicSchool::~MagicSchool() {
  free(shortName);
  shortName = NULL;
}

#define UPDATE_MESSAGE N_("Loading Spells")

void MagicSchool::initMagic() {
	ConfigLang *config = ConfigLang::load( "config/spell.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "magic_school" );

	MagicSchool *current = NULL;
  Spell *currentSpell = NULL;
  char name[255], displayName[255], notes[255], dice[255];
  char line[2000], symbol[255], targetTypeStr[255] /*, prereqName[255]*/;
	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( _(UPDATE_MESSAGE), i, v->size() );

		strcpy( name, node->getValueAsString( "name" ) );
    strcpy( displayName, node->getValueAsString( "display_name" ) );
		strcpy( notes, node->getValueAsString( "deity" ) );
		int skill = Skill::getSkillIndexByName( (char*)node->getValueAsString( "skill" ) );
		int resistSkill = Skill::getSkillIndexByName( (char*)node->getValueAsString( "resist_skill" ) );
		strcpy( line, node->getValueAsString( "rgb" ) );
		float red = (float)strtod( strtok( line, "," ), NULL );
		float green = (float)strtod( strtok( NULL, "," ), NULL );
		float blue = (float)strtod( strtok( NULL, "," ), NULL );
		strcpy( symbol, node->getValueAsString( "symbol" ) );
		current = new MagicSchool( strdup(name), 
                               strdup(displayName), 
															 strdup(notes), 
															 skill, 
															 resistSkill, 
															 red, green, blue, 
															 strdup( symbol ) );
		
		strcpy( line, node->getValueAsString( "deity_description" ) );
		if( strlen( line ) ) current->addToDeityDescription( line );

		schools[schoolCount++] = current;
		string nameStr = name;
		schoolMap[nameStr] = current;

		vector<ConfigNode*> *vv = node->getChildrenByName( "low_donate" );
		for( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = (*vv)[i];
			current->lowDonate.push_back( node2->getValueAsString( "text" ) );
		}
		vv = node->getChildrenByName( "neutral_donate" );
		for( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = (*vv)[i];
			current->neutralDonate.push_back( node2->getValueAsString( "text" ) );
		}
		vv = node->getChildrenByName( "high_donate" );
		for( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = (*vv)[i];
			current->highDonate.push_back( node2->getValueAsString( "text" ) );
		}

		vv = node->getChildrenByName( "spell" );
		for( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *node2 = (*vv)[i];
			
			strcpy( name, node2->getValueAsString( "name" ) );
      strcpy( displayName, node2->getValueAsString( "display_name" ) );
			strcpy( symbol, node2->getValueAsString( "symbol" ) );
			int level = node2->getValueAsInt( "level" );
			int mp = node2->getValueAsInt( "mp" );
			int exp = node2->getValueAsInt( "mp" );
			int failureRate = node2->getValueAsInt( "failureRate" );
			strcpy( dice, node2->getValueAsString( "action" ) );
			Dice *action = new Dice(strdup(dice));
			

			int distance = node2->getValueAsInt( "distance" );
			if( distance < (int)MIN_DISTANCE ) distance = (int)MIN_DISTANCE;
			int targetType = ( !strcmp( node2->getValueAsString( "targetType" ), "single") ? 
												 SINGLE_TARGET : GROUP_TARGET);
			int speed = node2->getValueAsInt( "speed" );
			int effect = Constants::getEffectByName( (char*)node2->getValueAsString( "effect" ) );
			strcpy( targetTypeStr, node2->getValueAsString( "target" ) );
			bool creatureTarget = ( strchr( targetTypeStr, 'C' ) != NULL );
			bool locationTarget = ( strchr( targetTypeStr, 'L' ) != NULL );
			bool itemTarget = ( strchr( targetTypeStr, 'I' ) != NULL );
			bool partyTarget = ( strchr( targetTypeStr, 'P' ) != NULL );
			bool doorTarget = ( strchr( targetTypeStr, 'D' ) != NULL );
			strcpy( line, node2->getValueAsString( "icon" ) );
			int iconTileX = atoi( strtok( line, "," ) ) - 1;
			int iconTileY = atoi( strtok( NULL, "," ) ) - 1;
	
			// Friendly/Hostile marker
			bool friendly = node2->getValueAsBool( "friendly" );
			
			int stateModPrereq = -1;
			strcpy( line, node2->getValueAsString( "prerequisite" ) );
			if( strlen( line ) ) {
				// is it a potion state mod?
				int n = Constants::getPotionSkillByName( line );
				if( n == -1 ) {
          StateMod *mod = StateMod::getStateModByName( line );
          if( !mod ) {
            cerr << "Can't find state mod: >" << line << "<" << endl;
            exit( 1 );
          }
					n = mod->getIndex();
				}
				stateModPrereq = n;
				if( stateModPrereq == -1 ) {
					cerr << "Error: spell=" << name << endl;
					cerr << "\tCan't understand prereq for spell: " << line << endl;
				}
			}
			
			currentSpell = new Spell( strdup(name), strdup( displayName ), strdup( symbol ), level, mp, exp, failureRate, 
																action, distance, targetType, speed, effect, 
																creatureTarget, locationTarget, itemTarget, partyTarget, doorTarget,
																current, iconTileX, iconTileY, 
																friendly, stateModPrereq );
			current->addSpell( currentSpell );

			strcpy( line, node2->getValueAsString( "sound" ) );
			if( strlen( line ) ) currentSpell->setSound( strdup(line) );

			strcpy( line, node2->getValueAsString( "notes" ) );
			if( strlen( notes ) ) currentSpell->addNotes( line );
		}
	}

	delete config;
}

const char *MagicSchool::getRandomString( vector<string> *v ) {
  return (*v)[ Util::dice( v->size() ) ].c_str();
}

const char *MagicSchool::getLowDonateMessage() {
  return getRandomString( &lowDonate );
}

const char *MagicSchool::getNeutralDonateMessage() {
  return getRandomString( &neutralDonate );
}

const char *MagicSchool::getHighDonateMessage() {
  return getRandomString( &highDonate );
}

Spell *MagicSchool::getRandomSpell(int level) {
  int n = Util::dice( getMagicSchoolCount() );
  int c = getMagicSchool(n)->getSpellCount();
  if(c > 0) {
    return getMagicSchool(n)->getSpell( Util::dice( c ) );
  } else {
    return NULL;
  }
}



Spell::Spell(char *name, char *displayName, char *symbol, int level, int mp, int exp, int failureRate, Dice *action, 
						 int distance, int targetType, int speed, int effect, bool creatureTarget, 
						 bool locationTarget, bool itemTarget, bool partyTarget, bool doorTarget,
						 MagicSchool *school,
						 int iconTileX, int iconTileY, bool friendly, int stateModPrereq ) {
  this->name = name;
  this->displayName = displayName;
  this->symbol = symbol;
  this->sound = NULL;
  this->level = level;
  this->mp = mp;
  this->exp = exp;
  this->failureRate = failureRate;
  this->action = action;
  this->distance = distance;
  this->targetType = targetType;
  this->speed = speed;
  this->effect = effect;
  this->creatureTarget = creatureTarget; 
  this->locationTarget = locationTarget;
  this->itemTarget = itemTarget;
  this->partyTarget = partyTarget;
	this->doorTarget = doorTarget;
  this->school = school;
  this->iconTileX = iconTileX;
  this->iconTileY = iconTileY;
  this->friendly = friendly;
  this->stateModPrereq = stateModPrereq;

  strcpy(this->notes, "");

  string s = name;
  spellMap[s] = this;
}

Spell::~Spell() {
}

Spell *Spell::getSpellByName(char *name) {
  string s = name;
  if(spellMap.find(s) == spellMap.end()) {
    cerr << "Warning: can't find spell: " << name << endl;
    return NULL;
  }
  return spellMap[s];
}


