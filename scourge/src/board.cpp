/***************************************************************************
                          board.cpp  -  description
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

#include "board.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "session.h"
#include "gameadapter.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "configlang.h"
#include "sound.h"
#include "persist.h"

using namespace std;

vector<string> Mission::intros;
vector<string> Mission::unknownPhrases;
map<string, int> Mission::conversations;
map<string, string> Mission::firstKeyPhrase;
vector<string> Mission::answers;

map<string, NpcConversation*> Mission::npcConversations;
map<string, NpcInfo*> Mission::npcInfos;

//#define DEBUG_MODE 1
	
/**
  *@author Gabor Torok
  */

Board::Board(Session *session) 
		:missionText(NULL) {
  this->session = session;
  this->storylineIndex = 0;
	missionListCount = 0;

  char type;
	char name[255], displayName[255], line[255], description[2000], replayDisplayName[255], replayDescription[2000], music[255], success[2000], failure[2000], mapName[80], introDescription[2000], location[20];
	string ambientSoundName;

  ConfigLang *config = ConfigLang::load( string("config/mission.cfg") );
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "sound" );
	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( _("Loading Sounds"), i, v->size() );

		string ambientName = node->getValueAsString( "name" );
		string ambientAmbient = node->getValueAsString( "ambient" );
		string ambientFootsteps = node->getValueAsString( "footsteps" );
		string afterFirstLevel = node->getValueAsString( "after_first_level" );
		session->getSound()->addAmbientSound( ambientName, ambientAmbient, ambientFootsteps, afterFirstLevel );
	}

	v = config->getDocument()->getChildrenByName( "template" );
	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( _("Loading Missions"), i, v->size() );

    type = node->getValueAsString( "type" )[0] + ( 'A' - 'a' );
    strcpy( name, node->getValueAsString( "name" ) );
    strcpy( displayName, node->getValueAsString( "display_name" ) );
    strcpy( description, node->getValueAsString( "description" ) );
    strcpy( music, node->getValueAsString( "music" ) );
    strcpy( success, node->getValueAsString( "success" ) );
    strcpy( failure, node->getValueAsString( "failure" ) );
		ambientSoundName = node->getValueAsString( "sound" );
    templates.push_back( new MissionTemplate( this, name, displayName, type, description, music, success, failure, ambientSoundName ) );
  }

  v = config->getDocument()->getChildrenByName( "mission" );
	for( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		config->setUpdate( _("Loading Missions"), i, v->size() );

    // read the level and depth
		strcpy( name, node->getValueAsString( "name" ) );
    strcpy( displayName, node->getValueAsString( "display_name" ) );
    strcpy( replayDisplayName, node->getValueAsString( "replay_display_name" ) );
    strcpy( replayDescription, node->getValueAsString( "replay_description" ) );
    bool replayable = ( strlen( replayDisplayName ) && strlen( replayDescription ) );
    int level = node->getValueAsInt( "level" );
    int depth = node->getValueAsInt( "depth" );
    strcpy( mapName, node->getValueAsString( "map" ) );
    strcpy( description, node->getValueAsString( "description" ) );
    strcpy( introDescription, node->getValueAsString( "intro" ) );
    strcpy( music, node->getValueAsString( "music" ) );
    strcpy( success, node->getValueAsString( "success" ) );
    strcpy( failure, node->getValueAsString( "failure" ) );
		int chapter = node->getValueAsInt( "chapter" );
		strcpy( location, node->getValueAsString( "position" ) );
		ambientSoundName = node->getValueAsString( "sound" );

    Mission *current_mission = new Mission( this, level, depth, replayable, name, displayName, description, replayDisplayName, replayDescription, introDescription, music, success, failure, mapName);
    current_mission->setStoryLine( true );
		current_mission->setChapter( chapter );
		current_mission->setAmbientSoundName( ambientSoundName );
		if( strlen( location ) ) {
			int x, y;
			sscanf( location, "%d,%d", &x, &y );
			current_mission->setLocation( x, y );
		}
    storylineMissions.push_back( current_mission );


    vector<ConfigNode*> *vv = node->getChildrenByName( "item" );
    for( unsigned int i = 0; vv && i < vv->size(); i++ ) {
      ConfigNode *itemNode = (*vv)[i];
      RpgItem *item = RpgItem::getItemByName( itemNode->getValueAsString( "name" ) );
      current_mission->addItem( item );
    }
    vv = node->getChildrenByName( "creature" );
    for( unsigned int i = 0; vv && i < vv->size(); i++ ) {
      ConfigNode *monsterNode = (*vv)[i];
      Monster *monster = Monster::getMonsterByName( monsterNode->getValueAsString( "name" ) );
      if( !monster ) {
        cerr << "*** Error: can't find mission monster \"" << line << "\"." << endl;
        exit( 1 );
      }
      current_mission->addCreature( monster );
    }
    
    if( strlen( node->getValueAsString( "special" ) ) )
        current_mission->setSpecial( node->getValueAsString( "special" ) );
  }
  delete( config );
}

Board::~Board() {
  freeListText();
}

void Board::freeListText() {
  if( missionListCount ) {
    delete[] missionText;
    free( missionColor );
  }
}

void Board::reset() {
  for( int i = 0; i < static_cast<int>(storylineMissions.size()); i++ ) {
    Mission *mission = storylineMissions[i];
    mission->reset();
  }
  storylineIndex = 0;
  for( int i = 0; i < static_cast<int>(availableMissions.size()); i++ ) {
    Mission *mission = availableMissions[i];
    if( !mission->isStoryLine() ) {
      delete mission;
    }
  }
  availableMissions.clear();
}

void Board::removeCompletedMissionsAndItems() {
  // remove completed missions
  for (vector<Mission*>::iterator e=availableMissions.begin(); e!=availableMissions.end(); ++e) {
    Mission *mission = *e;
		if( mission->isCompleted() ) {
			
			// remove mission items from the party's inventory
			mission->removeMissionItems();

			// delete mission if not storyline
			if( !mission->isStoryLine() ) {
				delete mission;
				availableMissions.erase(e);
				e = availableMissions.begin();
			}
		}
  }
}

void Board::initMissions() {
  // free ui
  freeListText();

  // find the highest and lowest levels in the party
  int highest = 0;
  int lowest = -1;
  int sum = 0;
  for(int i = 0; i < session->getParty()->getPartySize(); i++) {
    int n = session->getParty()->getParty(i)->getLevel();
    if(n < 1) n = 1;
    if(highest < n) {
      highest = n;
    } else if(lowest == -1 || lowest > n) {
      lowest = n;
    }
    sum += n;
  }
  float ave = ( sum == 0 ? 1 : (static_cast<float>(sum) / static_cast<float>(session->getParty()->getPartySize()) / 1.0f) );

  // remove the storyline missions
  // remove missions whose level is too low
  for( int i = 0; i < static_cast<int>(availableMissions.size()); i++ ) {
    Mission *mission = availableMissions[ i ];
    if( mission->isStoryLine() ) {
      // move the last element over the current storyline element
      availableMissions[ i ] = availableMissions[ availableMissions.size() - 1 ];
      availableMissions.pop_back();
      // re-examine the current element
      i--;
    }
  }

  // maintain a set of missions
	for( int counter = 0; availableMissions.size() < 8 && counter < 8; counter++ ) {
		int level;
		if( counter % 2 == 0 ) {
			// allow for low level mission
			level = static_cast<int>(Util::roll( 0.0f, ave + 2 ));
		} else {
			// allow for current level missions
			level = static_cast<int>(Util::roll( ave, ave + 4.0f )) - 2;
		}
    if( level < 1 ) level = 1;
    int depth =  static_cast<int>(static_cast<float>(level) / static_cast<float>(MAX_MISSION_DEPTH - 3) ) + 1 + Util::dice( 3 );
    if( depth > MAX_MISSION_DEPTH ) depth = MAX_MISSION_DEPTH;
    int templateIndex = Util::dice( templates.size() );
    
    // only outdoor maps can be 1 depth
    if( depth ==1 && tolower( templates[ templateIndex ]->getMapType() ) != 'o' ) {
    	depth++;
    }
		// Create a new mission but only keep it if there isn't another mission with this name already.
		// This is because missions are 'found' on load via name, so names have to be unique.
		Mission *mission = templates[ templateIndex ]->createMission( session, level, depth );
		bool found = false;
		for( unsigned int i = 0; i < availableMissions.size(); i++ ) {
			if( !strcmp( mission->getName(), availableMissions[i]->getName() ) ) {
				found = true;
				break;
			}
		}
		if( !found ) {
			availableMissions.push_back( mission );
		}
  }  
  

  // add the current storyline mission at the top of the board
  if( storylineIndex >= 0 && storylineIndex < static_cast<int>(storylineMissions.size()) ) {
    availableMissions.insert( availableMissions.begin(), storylineMissions[ storylineIndex ] );
  }

  // append replayable maps at the bottom of the board
  for( int i = 0; i < storylineIndex; i++ ) {
    if ( storylineMissions[ i ]->isReplayable() ) {
      availableMissions.insert( availableMissions.end(), storylineMissions[ i ] );
      availableMissions[ availableMissions.size() -1 ]->setDisplayName( storylineMissions[i]->getReplayDisplayName() ) ;
      availableMissions[ availableMissions.size() -1 ]->setDescription( storylineMissions[i]->getReplayDescription() ) ;
      availableMissions[ availableMissions.size() -1 ]->setChapter( -1 );
    }
  }



#ifdef DEBUG_MODE
  // debug missions
  for( int i = 1; i <= highest; i++ ) {
    int templateIndex = Util::dice(  templates.size() );
    Mission *mission = templates[ templateIndex ]->createMission( session, i, 1 );
    availableMissions.push_back( mission );
  }
#endif

  // init ui
  if( !availableMissions.empty() ) {
		missionListCount = availableMissions.size();
    missionText = new string[availableMissions.size()];
    missionColor = (Color*)malloc(availableMissions.size() * sizeof(Color));
    for(int i = 0; i < static_cast<int>(availableMissions.size()); i++) {
		char str[20];
		snprintf( str, 20, _("L:%d, "), availableMissions[i]->getLevel() );
		missionText[i] = str;
		snprintf( str, 20, _("S:%d, "), availableMissions[i]->getDepth() );
		missionText[i] += str;
		missionText[i] += ( availableMissions[i]->isReplay() ) ? _( "(EXCURSION)" ) : availableMissions[i]->isStoryLine() ? _( "(STORY)" ) : strstr( availableMissions[i]->getMapName(), "caves" ) ? _( "(CAVE)" ) : ""; 
//		missionText[i] += ( availableMissions[i]->isStoryLine() && availableMissions[i]->isCompleted() && availableMissions[i]->isReplayable() ) ? _( "(EXCURSION)" ) : availableMissions[i]->isStoryLine() ? _( "(STORY)" ) : strstr( availableMissions[i]->getMapName(), "caves" ) ? _( "(CAVE)" ) : strstr( availableMissions[i]->getMapName(), "outdoors" ) ? _( "(OUTDOORS)" ) : ""; 
		missionText[i] += " ";
		missionText[i] += availableMissions[i]->getDisplayName();
		if ( availableMissions[i]->isCompleted() && !availableMissions[i]->isReplayable() ) missionText[i] += _( "(completed)" );

      missionColor[i].r = 1.0f;
      missionColor[i].g = 1.0f;
      missionColor[i].b = 0.0f;
      if(availableMissions[i]->isReplay()) {
        missionColor[i].r = 0.0f;
        missionColor[i].g = 0.75f;
        missionColor[i].b = 1.0f;
      } else if(availableMissions[i]->isCompleted()) {
        missionColor[i].r = 0.5f;
        missionColor[i].g = 0.5f;
        missionColor[i].b = 0.5f;
      } else if( availableMissions[i]->isStoryLine() ) {
        missionColor[i].r = 1.0f;
        missionColor[i].g = 0.0f;
        missionColor[i].b = 1.0f;
      } else if(availableMissions[i]->getLevel() < ave) {
        missionColor[i].r = 1.0f;
        missionColor[i].g = 1.0f;
        missionColor[i].b = 1.0f;
      } else if(availableMissions[i]->getLevel() > ave) {
        missionColor[i].r = 1.0f;
        missionColor[i].g = 0.0f;
        missionColor[i].b = 0.0f;
      }
      if(i == 0) {
        if(!session->getGameAdapter()->isHeadless()) 
          session->getGameAdapter()->setMissionDescriptionUI( availableMissions[i]->getDescription(),
                                                             availableMissions[i]->getMapX(),
                                                             availableMissions[i]->getMapY());
      }
    }

    if(!session->getGameAdapter()->isHeadless()) 
      session->getGameAdapter()->updateBoardUI(availableMissions.size(), 
                                               missionText, 
                                               missionColor);
  }
}

void Board::setStorylineIndex( int n ) {
	if( n < 0 )
		n = 0;
	if( n > static_cast<int>(storylineMissions.size()) )
		n = static_cast<int>(storylineMissions.size());

	storylineIndex = n;
	for( int i = 0; i < static_cast<int>(storylineMissions.size()); i++ ) {
		storylineMissions[i]->setCompleted( i < storylineIndex ? true : false );
	}
}

void Board::storylineMissionCompleted( Mission *mission ) {
	for( int i = 0; i < static_cast<int>(storylineMissions.size()); i++ ) {
		if( storylineMissions[i] == mission && storylineIndex < ( i + 1 )) {
			storylineIndex = i + 1;
			break;
		}
	}
}

MissionTemplate::MissionTemplate( Board *board, char *name, char *displayName, char type, char *description, char *music, char *success, char *failure, string& ambientSoundName ) {
  this->board = board;
  strcpy( this->name, name );
  strcpy( this->displayName, displayName );
  this->mapType = type;
  strcpy( this->description, description );
  strcpy( this->music, music);
  strcpy( this->success, success );
  strcpy( this->failure, failure );
	this->ambientSoundName = ambientSoundName;
}

MissionTemplate::~MissionTemplate() {
}

Mission *MissionTemplate::createMission( Session *session, int level, int depth, MissionInfo *info ) {

//  cerr << "*** Creating level " << level << " mission, using template: " << this->name << " type=" << mapType << endl;

  map<string, RpgItem*> items;
  map<string, Monster*> creatures;

  char parsedName[80];
  char parsedDescription[2000];
  char parsedSuccess[2000];
  char parsedFailure[2000];
  char s[2000];
  strcpy( s, _( name ) );
  parseText( session, level, depth, s, parsedName, &items, &creatures, info );
  strcpy( s, description );
  parseText( session, level, depth, s, parsedDescription, &items, &creatures, info );
  strcpy( s, success );
  parseText( session, level, depth, s, parsedSuccess, &items, &creatures, info );
  strcpy( s, failure );
  parseText( session, level, depth, s, parsedFailure, &items, &creatures, info );
  
  //TODO VF: select music from multi-tracks missions
  
  Mission *mission = new Mission( board, 
                                  level, depth, false, parsedName, parsedName, 
                                  parsedDescription, NULL, NULL, "", music, parsedSuccess, 
                                  parsedFailure, NULL, mapType );
  for(map<string, RpgItem*>::iterator i=items.begin(); i!=items.end(); ++i) {
    RpgItem *item = i->second;
		bool found = false;
		bool value = false;
		if( info ) {
			for( int i = 0; i < info->itemCount; i++ ) {
				if( !strcmp( (char*)info->itemName[ i ], item->getName() ) ) {
					found = true;
					value = (info->itemDone[ i ] != 0);
					break;
				}
			}
			if( !found ) {
				cerr << "*** Error: can't find rpgItem in saved mission: " << item->getName() << endl;
			}
		}
    mission->addItem( item, value );
  }
  for(map<string, Monster*>::iterator i=creatures.begin(); i!=creatures.end(); ++i) {
    Monster *monster = i->second;
		bool found = false;
		bool value = false;
		if( info ) {
			for( int i = 0; i < info->monsterCount; i++ ) {
				if( !strcmp( (char*)info->monsterName[ i ], monster->getType() ) ) {
					found = true;
					value = (info->monsterDone[ i ] != 0);
					break;
				}
			}
			if( !found ) {
				cerr << "*** Error: can't find Monster in saved mission: " << monster->getType() << endl;
			}
		}
    mission->addCreature( monster, value );
  }
	mission->setTemplateName( this->name );
	mission->setAmbientSoundName( this->ambientSoundName );
	if( info ) {
		mission->setSavedMapName( (char*)info->mapName );
		//cerr << "\tmap name=" << (char*)info->mapName << endl;
		mission->setCompleted( info->completed == 1 ? true : false );
	}

  return mission;
}                                               

void MissionTemplate::parseText( Session *session, int level, int depth,
                                 char *text, char *parsedText,
                                 map<string, RpgItem*> *items, 
                                 map<string, Monster*> *creatures,
																 MissionInfo *info ) {
	int itemCount = 0;
	int monsterCount = 0;
  //cerr << "parsing text: " << text << endl;
  strcpy( parsedText, "" );
  char *p = strtok( text, " " );
  while( p ) {
    if( strlen( parsedText ) ) strcat( parsedText, " " );
    char *start = strchr( p, '{' );
    char *end = strrchr( p, '}' );
    if( start && end && end - start < 255 ) {
      char varName[255];
      strncpy( varName, start, (size_t)( end - start ) );
      *(varName + ( end - start )) = '\0';
      
      if( strstr( varName, "item" ) ) {
        string s = varName;
        RpgItem *item;
        if( items->find( s ) == items->end() ) {
					if( info ) {
						if( itemCount >= info->itemCount ) {
								cerr << "*** error: itemCount out of range!" << endl;
								item = RpgItem::getRandomItem( 1 );
						} else {
							item = RpgItem::getItemByName( (char*)info->itemName[ itemCount++ ] );
						}
					} else {
						item = RpgItem::getRandomItem( 1 );
					}
          (*items)[ s ] = item;
        } else {
          item = (*items)[s];
        }
        // FIXME: also copy text before and after the variable
        strcat( parsedText, item->getDisplayName() );
      } else if( strstr( varName, "creature" ) ) {
        string s = varName;
        Monster *monster = NULL;
        if( creatures->find( s ) == creatures->end() ) {
          
          // find a monster
          int monsterLevel = level;
          while( monsterLevel > 0 && !monster ) {
						if( info ) {
							if( monsterCount >= info->monsterCount ) {
								cerr << "*** error: monsterCount out of range!" << endl;
								monster = Monster::getRandomMonster( monsterLevel );
							} else {
								monster = Monster::getMonsterByName( (char*)info->monsterName[ monsterCount++ ] );
							}
						} else {
							monster = Monster::getRandomMonster( monsterLevel );
						}
            if(!monster) {
              cerr << "+ Warning: no monsters defined for level: " << level << endl;
              monsterLevel--;
            }
          }
          if( !monster ) {
            cerr << "Error: could not find any monsters." << endl;
            exit( 1 );
          }
          (*creatures)[ s ] = monster;
        } else {
          monster = (*creatures)[s];
        }
        // FIXME: also copy text before and after the variable
        strcat( parsedText, monster->getDisplayName() );
      }
    } else {
      strcat( parsedText, p );
    }
    p = strtok( NULL, " " );
  }
}


Mission::Mission( Board *board, int level, int depth, bool replayable,
                  char *name, char *displayName, char *description, char *replayDisplayName, char *replayDescription, char *introDescription,
                  char *music,
                  char *success, char *failure,
                  char *mapName, char mapType ) {
	this->missionId = Constants::getNextMissionId();
  this->board = board;
  this->level = level;
  this->depth = depth;
	this->locationX = this->locationY = -1;
  strcpy( this->name, name );
  strcpy( this->displayName, displayName );
  strcpy( this->description, description );
	strcpy( this->introDescription, introDescription );
  strcpy( this->replayDisplayName, ( replayDisplayName ? replayDisplayName : "" ) );
  strcpy( this->replayDescription, ( replayDescription ? replayDescription : "" ) );
  strcpy( this->music, music );
  strcpy( this->success, success );
  strcpy( this->failure, failure );
  strcpy( this->mapName, ( mapName ? mapName : "" ) );
  this->completed = false;
  this->replayable = replayable;
  this->storyLine = false;
	this->chapter = 0;
  this->mapX = this->mapY = 0;
  this->special[0] = '\0';
	this->templateName[0] = '\0';
	this->savedMapName == "";

  // assign the map grid location
  if( mapName && strlen( mapName ) ) {
    edited = true;
    string result;
    board->getSession()->getMap()->loadMapLocation( string(mapName), result, &mapX, &mapY );
  } else {
    edited = false;
    char *s;
    assert( board->getSession()->getShapePalette()->getRandomMapLocation( mapType, &s, &mapX, &mapY ) );
    strcpy( this->mapName, s );
  }
  
//   cerr << "*** Created mission: " << getName() << endl;
//   cerr << "\tmap name=" << ( strlen( this->mapName ) ? mapName : "<random>" ) << endl;
//   cerr << getDescription() << endl;
//   cerr << "-----------------------" << endl;
}

Mission::~Mission() {
  items.clear();
  itemList.clear();
  creatures.clear();
  creatureList.clear();
  monsterInstanceMap.clear();
}

bool Mission::itemFound(Item *item) {
  if( !completed ) {
		if( item->getMissionId() == getMissionId() ) {
			RpgItem *rpgItem = itemList[ item->getMissionObjectiveIndex() ];
			items[ rpgItem ] = true;
			checkMissionCompleted();
			return isCompleted();
		}
	}
  return false;
}

bool Mission::creatureSlain(Creature *creature) {
  if( !completed ) {
    if( monsterInstanceMap.find( creature ) != monsterInstanceMap.end() ) {
      Monster *monster = monsterInstanceMap[ creature ];			
      if( creatures.find( monster ) != creatures.end() ) {
        creatures[ monster ] = true;
        checkMissionCompleted();
      }
    }
    return isCompleted();
  }
  return false;
}

void Mission::checkMissionCompleted() {
  // special missions aren't completed when an item is found.
  if( isSpecial() ) {
    completed = false;
    return;
  }
  if( isReplay() ) {
    completed = false;
    return;
  }
  completed = true;
  for(map<RpgItem*, bool >::iterator i=items.begin(); i!=items.end(); ++i) {
    bool b = i->second;
    if( !b ) {
      completed = false;
      return;
    }
  }
  for(map<Monster*, bool >::iterator i=creatures.begin(); i!=creatures.end(); ++i) {
    bool b = i->second;
    if( !b ) {
      completed = false;
      return;
    }
  }
	board->getSession()->getGameAdapter()->refreshInventoryUI();
  if( storyLine ) board->storylineMissionCompleted( this );
}

void Mission::reset() {
  completed = false;
  // is this ok? (below: to modify the table while iterating it...)
  for(map<RpgItem*, bool >::iterator i=items.begin(); i!=items.end(); ++i) {
    RpgItem *item = i->first;
    items[ item ] = false;
  }
  for(map<Monster*, bool >::iterator i=creatures.begin(); i!=creatures.end(); ++i) {
    Monster *monster = i->first;
    creatures[ monster ] = false;
  }
  deleteMonsterInstances();
}

void Mission::deleteMonsterInstances() {
	monsterInstanceMap.clear();
}

void Mission::removeMissionItems() {
	for(int i = 0; i < board->getSession()->getParty()->getPartySize(); i++) {
		Creature *c = board->getSession()->getParty()->getParty(i);
		for( int t = 0; t < c->getInventoryCount(); t++ ) {
			if( c->getInventory( t )->getMissionId() == getMissionId() ) {
				c->removeInventory( t );
			}
		}
	}
}

char const* Mission::getIntro() {
	if ( !intros.empty() ) {
		return intros[ Util::dice( intros.size() ) ].c_str();
	} else {
		cerr << "Error: Mission has 0 intros" << endl;
		return "";
	}
}

char const* Mission::getAnswer( char const* keyphrase ) {
  string ks = keyphrase;
  if( conversations.find( ks ) != conversations.end() ) {
    return  answers[ conversations[ ks ] ].c_str();
  } else {
    cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
    return unknownPhrases[ Util::dice( unknownPhrases.size() ) ].c_str();
  }
}

char const* Mission::getFirstKeyPhrase( char const* keyphrase ) {
  string ks = keyphrase;
  if( firstKeyPhrase.find( ks ) != firstKeyPhrase.end() ) {
    return firstKeyPhrase[ ks ].c_str();
  } else {
    cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
    return unknownPhrases[ Util::dice( unknownPhrases.size() ) ].c_str();
  }
}

char const* Mission::getIntro( char const* npc ) {
	string s = npc;
  if( npcConversations.find( s ) == npcConversations.end() ) {
    //cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
    return NULL;
  }
  NpcConversation *nc = npcConversations[ s ];
  return nc->npc_intros[ Util::dice( nc->npc_intros.size() ) ].c_str();
}

bool Mission::setIntro( Creature *s, char const* keyphrase ) {
	NpcConversation *nc = NULL;
	string npc = s->getMonster()->getType();
	if( npcConversations.find( npc ) != npcConversations.end() ) {
		nc = npcConversations[ npc ];
  }
	if( !nc ) {
		npc = npc = s->getName();
		if( npcConversations.find( npc ) != npcConversations.end() ) {
			nc = npcConversations[ npc ];
		}
	}
	if( !nc ) {
		cerr << "*** Error: can't find conversation with: " << s << endl;
		return false;
	}

	string ks = keyphrase;
	Util::toLowerCase( ks );
	if( nc->npc_conversations.find( ks ) != nc->npc_conversations.end() ) {
		nc->npc_intros.clear();
    nc->npc_intros.push_back( nc->npc_answers[ nc->npc_conversations[ ks ] ] );
	} else {
		cerr << "------------------------------------" << endl;
		for( map<string,int>::iterator i=nc->npc_conversations.begin(); i!=nc->npc_conversations.end(); ++i) {
			cerr << i->first << "=" << i->second << endl;
		}
		cerr << "------------------------------------" << endl;
		cerr << "Can't find " << keyphrase << " in npc conversation for creature: " << s->getName() << endl;
		return false;
	}
	return true;
}

char const* Mission::getAnswer( char const* npc, char const* keyphrase ) {
	string s( npc );
  if( npcConversations.find( s ) == npcConversations.end() ) {
    //cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
    return NULL;
  }
  NpcConversation *nc = npcConversations[ s ];

  string ks = keyphrase;
  if( nc->npc_conversations.find( ks ) != nc->npc_conversations.end() ) {
    return  nc->npc_answers[ nc->npc_conversations[ ks ] ].c_str();
  } else {
    cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
    return nc->npc_unknownPhrases[ Util::dice( nc->npc_unknownPhrases.size() ) ].c_str();
  }
}

char const* Mission::getFirstKeyPhrase( char const* npc, char const* keyphrase ) {
  string s( npc );
  if( npcConversations.find( s ) == npcConversations.end() ) {
    cerr << "Can't find npc conversation for creature: " << npc << endl;
    return NULL;
  }
  NpcConversation *nc = npcConversations[ s ];

  string ks = keyphrase;
  if( nc->npc_firstKeyPhrase.find( ks ) != nc->npc_firstKeyPhrase.end() ) {
    return nc->npc_firstKeyPhrase[ ks ].c_str();
  } else {
    cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
    return NULL;
  }
}

void Mission::clearConversations() {
  // clean up
  for( map<string,NpcInfo*>::iterator i=npcInfos.begin(); i!=npcInfos.end(); ++i) {
    NpcInfo *npcInfo = i->second;
    delete npcInfo;
  }
  npcInfos.clear();
  intros.clear();
  unknownPhrases.clear();
  conversations.clear();
  firstKeyPhrase.clear();
  answers.clear();
  for (map<string,NpcConversation*>::iterator i=npcConversations.begin(); i!=npcConversations.end(); ++i) {
    NpcConversation *npcConversation = i->second;
    delete npcConversation;
  }
  npcConversations.clear();
}

//TODO: this will segfault if filename starts with a .
void Mission::loadMapData( GameAdapter *adapter, const string& filename ) {
	clearConversations();

  // read general conversation from level 0.
  char dup[300];
  strcpy( dup, filename.c_str() );
  char *p = strrchr( dup, '.' );
  if( p ) {
    char c = *( p - 1 );
    if( c >= '1' && c <= '9' ) {
      char *q = p - 1;
      //cerr << "..." << endl;
      while( true ) {
        //cerr << "\tq=" << q << endl;
        if( q <= dup ) {
          q = NULL;
          break;
        } else if( !( *q >= '1' && *q <= '9' ) ) {
          break;
        }
        q--;
      }
      if( q ) {
        *( q + 1 ) = 0;
        strcat( dup, ".map" );
				string dup2(dup);
        loadMapDataFile( adapter, dup2, true );
      }
    }
  }

  // read the level's data
  loadMapDataFile( adapter, filename );
}

/**
 * Called by squirrel code to assign a different cfg file to be used with the
 * (possibly generated) map.
 */
void Mission::loadMapConfig( GameAdapter *adapter, const string& filename ) {
	clearConversations();
	string path = rootDir + filename;
  loadMapDataFile( adapter, path );
}

string getKeyValue( string key ) {
	string::size_type n = key.find( "." );
	if( n == string::npos ) {
		cerr << "Bad answer string name: " << key << endl;
		return "";
	} else {
		string k;
		string::size_type m = key.find( ".", n + 1 );
		return( m == string::npos ? key.substr( n + 1 ) : key.substr( n + 1, m - n - 1 ) );
	}
}

void Mission::initConversations( ConfigLang *config, GameAdapter *adapter, bool generalOnly ) {
	map<string,string> keyphrases;
	map<string,map<string,vector<string>*>*> answers;

	char *currentNpc;
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "conversation" );
	for( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		char const* name = node->getValueAsString( "name" );
		
		// if it's NOT general
		currentNpc = NULL;
		if( strcmp( name, "general" ) ) {
			
			if( generalOnly ) continue;

			Monster *m = Monster::getMonsterByName( name );
			if( m ) {
				currentNpc = m->getType();
			} else {
				Creature *c = adapter->getSession()->getCreatureByName( name );
				if( c ) currentNpc = c->getName();
				else {
					cerr << "*** Error: can't find creature named: " << name << endl;
				}
			}
		}
				
		for( map<string,ConfigValue*>::iterator e = node->getValues()->begin();
				 e != node->getValues()->end(); ++e ) {
			string key = e->first;
			ConfigValue *value = e->second;
			
			if( key == "name" ) {
				// do nothing
			} else if( key.substr( 0, 3 ) == "key" ) {
				string k = getKeyValue( key );
				if( k != "" ) {
					keyphrases[k] = value->getAsString();
				}
			}	else {
				string k = getKeyValue( key );
				if( k != "" ) {
					string currentName = ( currentNpc == NULL ? "general" : currentNpc );
					map<string,vector<string>*> *m;
					if( answers.find( currentName ) == answers.end() ) {
						m = new map<string,vector<string>*>();
						answers[currentName] = m;
					} else {
						m = answers[currentName];
					}
					vector<string>* v;
					if( m->find( k ) == m->end() ) {
						v = new vector<string>();
						(*m)[k] = v;
					} else {
						v = (*m)[k];
					}
					v->push_back( value->getAsString() );
				}
			}
		}
	}

	for( map<string,map<string,vector<string>*>*>::iterator e = answers.begin();
			 e != answers.end(); ++e ) {
		string phase = e->first;
		map<string,vector<string>*>* m = e->second;

		for( map<string,vector<string>*>::iterator e2 = m->begin();
				 e2 != m->end(); ++e2 ) {
			string key = e2->first;
			vector<string> *v = e2->second;
			if( keyphrases.find( key ) == keyphrases.end() ) {
				cerr << "*** Error: can't find conversation keyphrase id=" << key << endl;
			} else {
				string keyphrase = keyphrases[ key ];
				for( unsigned int i = 0; i < v->size(); i++ ) {
					string answer = (*v)[i];
					if( phase == "general" ) {
						setGeneralConversationLine( keyphrase, answer );
					} else {
						setConversationLine( phase, keyphrase, answer );
					}
				}
			}
		}
	}

	// cleanup
	for( map<string,map<string,vector<string>*>*>::iterator e = answers.begin();
			 e != answers.end(); ++e ) {
		//string phase = e->first;
		map<string,vector<string>*>* m = e->second;
		for( map<string,vector<string>*>::iterator e2 = m->begin();
				 e2 != m->end(); ++e2 ) {
			//string key = e2->first;
			vector<string> *v = e2->second;
			free( v );
		}
		free( m );
	}
}

void Mission::setGeneralConversationLine( string keyphrase, string answer ) {
	storeConversationLine( keyphrase, 
						   answer,
						   &Mission::intros, 
						   &Mission::unknownPhrases,
						   &Mission::conversations,
						   &Mission::firstKeyPhrase,
						   &Mission::answers );
}

void Mission::setConversationLine( string npc, string keyphrase, string answer ) {
	NpcConversation *npcConv;
	if( Mission::npcConversations.find( npc ) == Mission::npcConversations.end() ) {
		npcConv = new NpcConversation();
		Mission::npcConversations[ npc ] = npcConv;
	} else {
		npcConv = Mission::npcConversations[ npc ];
	}
	
	storeConversationLine( keyphrase, 
						   answer, 
						   &npcConv->npc_intros, 
						   &npcConv->npc_unknownPhrases,
						   &npcConv->npc_conversations,
						   &npcConv->npc_firstKeyPhrase,
						   &npcConv->npc_answers );
}												 	

void Mission::storeConversationLine( string keyphrase, 
									 string answer,
									 vector<string> *intros,
									 vector<string> *unknownPhrases,
									 map<string, int> *conversations,
									 map<string, string> *firstKeyPhrase,
									 vector<string> *answers ) {
  if( keyphrase == INTRO_PHRASE ) {
    intros->push_back( answer );
  } else if( keyphrase == UNKNOWN_PHRASE ) {
    unknownPhrases->push_back( answer );
  } else {
	char line[300];
	strcpy( line, keyphrase.c_str() );
	
    char tmp[80];
    char *p = strtok( line, "," );
	string first = p;
    while( p ) {
      strcpy( tmp, p );
      string lower = Util::toLowerCase( tmp );
	  (*firstKeyPhrase)[ lower ] = first;
      (*conversations)[ lower ] = answers->size();
      p = strtok( NULL, "," );
    }
    answers->push_back( answer );
  }
}

void Mission::initNpcs( ConfigLang *config, GameAdapter *adapter ) {
	char line[1000];
  int x, y, level;
  char npcName[255], npcType[255], npcSubType[1000];

	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "npc" );
	for( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];

		strcpy( line, node->getValueAsString( "position" ) );
    x = atoi( strtok( line, "," ) );
		y = atoi( strtok( NULL, "," ) );
		strcpy( npcName, node->getValueAsString( "display_name" ) );
		level = node->getValueAsInt( "level" );
		strcpy( npcType, node->getValueAsString( "type" ) );
		strcpy( npcSubType, node->getValueAsString( "subtype" ) );
  
		// store npc info
		NpcInfo *npcInfo = addNpcInfo( x, y, npcName, level, npcType, npcSubType );
  
		// Assign to creature
		Location *pos = adapter->getSession()->getMap()->getLocation( x, y, 0 );
		if( !( pos && 
					 pos->creature && 
					 pos->creature->isMonster() && 
					 ((Creature*)(pos->creature))->getMonster()->isNpc() ) ) {
			
			// the creature moved, try to find it by name
			bool found = false;
			for( int i = 0; i < adapter->getSession()->getCreatureCount(); i++ ) {
				if( !strcmp( npcInfo->name, adapter->getSession()->getCreature(i)->getName() ) ) {
					found = true;
					adapter->getSession()->getCreature(i)->setNpcInfo( npcInfo );
					break;
				}
			}
			if( !found ) {
				cerr << "Error: npc definition." << 
					"Line: " << line << " npc=" << npcInfo->name << 
					" doesn't point to an npc." << endl;
				//} else {
				//cerr << "* found npc by name: " << npcInfo->name << endl;
			}
		} else {
			((Creature*)(pos->creature))->setNpcInfo( npcInfo );
		}
	}
}

void Mission::loadMapDataFile( GameAdapter *adapter, const string& filename, bool generalOnly ) {
	string tmp = getMapConfigFile( filename );
	ConfigLang *config = ConfigLang::load( tmp, true );
	if( config ) {
		initConversations( config, adapter, generalOnly );
		if( !generalOnly ) initNpcs( config, adapter );
		delete config;
	}
}

NpcInfo *Mission::getNpcInfo( int x, int y ) {
  string key = getNpcInfoKey( x,y );
  return( npcInfos.find( key ) == npcInfos.end() ? NULL : npcInfos[ key ] );
}

string Mission::getNpcInfoKey( int x, int y ) {
  char line[255];
  snprintf( line, 255, "%d,%d", x, y );
  string key = line;
  return key;
}

//TODO original algorthm seems broken, it might exchange .map in the middle of a string
string Mission::getMapConfigFile( const string& filename ) {
  string s = filename;

	if(".map" == s.substr(s.length() - 4, 4)) {
		s = s.substr(0, s.length() - 4) + ".cfg";
	} else {
		s.append(".cfg");
	}

	return s;
}

void Mission::saveMapData( GameAdapter *adapter, const string& filename ) {
  // append to txt file the new npc info
	string path = getMapConfigFile( filename );
	ConfigLang *config = ConfigLang::load( path, true );
	if( !config ) {
		config = ConfigLang::fromString( "[map]\n[/map]\n" );
	}
	
	// Are there default conversation elements?
	int maxKey = 0;
	ConfigNode *general = NULL;
	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "conversation" );
	for( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = (*v)[i];
		if( !strcmp( node->getValueAsString( "name" ), "general" ) ) {
			general = node;
			break;
		}
		for( map<std::string, ConfigValue*>::iterator e = node->getValues()->begin();
				 e != node->getValues()->end(); ++e ) {
			string key = e->first;
			// ConfigValue *value = e->second;
			if( key == "name" ) {
			} else {
				string k = getKeyValue( key );
				int n = atoi( k.c_str() );
				if( n > maxKey ) maxKey = n;
			}
		}
	}
	enum { TMP_SIZE = 300 };
	char tmp[ TMP_SIZE ];
	if( !general ) {
		general = new ConfigNode( config, "conversation" );
		general->addValue( "name", new ConfigValue( "\"general\"" ) );
		maxKey++;
		snprintf( tmp, TMP_SIZE, "keyphrase.%d", ( maxKey ) );
		general->addValue( tmp, new ConfigValue( "\"_INTRO_\"" ) );
		snprintf( tmp, TMP_SIZE, "answer.%d", ( maxKey ) );
		general->addValue( tmp, new ConfigValue( "_( \"Welcome weary adventurer!\" )" ) );
		maxKey++;
		snprintf( tmp, TMP_SIZE, "keyphrase.%d", ( maxKey ) );
		general->addValue( tmp, new ConfigValue( "\"_UNKNOWN_\"" ) );
		snprintf( tmp, TMP_SIZE, "answer.%d", ( maxKey ) );
		general->addValue( tmp, new ConfigValue( "_( \"Uh, I don't know anything about that...\" )" ) );
		config->getDocument()->addChild( general );
	}

	// find new npc-s
  for( int i = 0; i < adapter->getSession()->getCreatureCount(); i++ ) {
    Creature *creature = adapter->getSession()->getCreature( i );
    if( creature->getMonster() && 
				creature->getMonster()->isNpc() ) {
			snprintf( tmp, TMP_SIZE, "%d,%d", toint( creature->getX() ),  toint( creature->getY() ) );
			string position = tmp;
			bool foundNpc = false;
			vector<ConfigNode*> *v = config->getDocument()->
				getChildrenByName( "npc" );
			for( unsigned int t = 0; v && t < v->size(); t++ ) {
				ConfigNode *node = (*v)[t];
				if( node->getValueAsString( "position" ) == position ) {
					foundNpc = true;
					break;
				}
			}
			if( !foundNpc ) {
				ConfigNode *node = new ConfigNode( config, "npc" );
				snprintf( tmp, TMP_SIZE, "\"%s\"", creature->getMonster()->getType() );
				node->addValue( "name", new ConfigValue( tmp ) );
				snprintf( tmp, TMP_SIZE, "_( \"%s\" )", creature->getMonster()->getType() );
				node->addValue( "display_name", new ConfigValue( tmp ) );
				snprintf( tmp, TMP_SIZE, "\"%d,%d\"", toint( creature->getX() ),  toint( creature->getY() ) );
				node->addValue( "position", new ConfigValue( tmp ) );
				node->addValue( "type", new ConfigValue( "\"commoner\"" ) );
				node->addValue( "level", new ConfigValue( "1" ) );
				config->getDocument()->addChild( node );
			}
		}
	}

	// FIXME: should also delete unused references to npcs (ie. ones removed
	// from the map). But that is not simple to do...
	
	// save out to the file again
	config->save( path, true );

	delete config;
}

MissionInfo *Mission::save() {
	MissionInfo *info = (MissionInfo*)malloc(sizeof(MissionInfo));
  info->version = PERSIST_VERSION;
  strncpy( (char*)info->templateName, ( isStoryLine() ? "storyline" : getTemplateName() ), 79);
  info->templateName[79] = 0;
	strcpy( (char*)info->mapName, savedMapName.c_str() );
	info->level = getLevel();
	info->depth = getDepth();
	info->completed = ( completed ? 1 : 0 );
	info->itemCount = itemList.size();
	for( int i = 0; i < static_cast<int>(itemList.size()); i++ ) {
		strncpy( (char*)info->itemName[i], itemList[i]->getName(), 254 );
		info->itemName[i][254] = '\0';
		info->itemDone[i] = ( items[ itemList[i] ] ? 1 : 0 );
	}
	info->monsterCount = creatureList.size();
	for( int i = 0; i < static_cast<int>(creatureList.size()); i++ ) {
		strncpy( (char*)info->monsterName[i], creatureList[i]->getType(), 254 );
		info->monsterName[i][254] = '\0';
		info->monsterDone[i] = ( creatures[ creatureList[i] ] ? 1 : 0 );
	}
	info->missionId = getMissionId();
	return info;
}

Mission *Mission::load( Session *session, MissionInfo *info ) {
	// find the template
	Mission *mission;
	if( !strcmp( (char*)info->templateName, "storyline" ) ) {
		mission = session->getBoard()->getCurrentStorylineMission();
		//cerr << "Loading storyling mission. chapter index=" << session->getBoard()->getStorylineIndex() << " - " << session->getBoard()->getStorylineTitle() << endl;
		mission->loadStorylineMission( info );
	} else {
		MissionTemplate *missionTemplate = session->getBoard()->
			findTemplateByName( (char*)info->templateName );
		if( !missionTemplate ) {
			cerr << "Can't find template for name: " << info->templateName << endl;
			return NULL;
		}
		//cerr << "Loading mission with template: " << (char*)(info->templateName) << " map: " << info->mapName << endl;
		mission = missionTemplate->createMission( session, info->level, info->depth, info );
	}
	mission->setMissionId( info->missionId );
	return mission;
}

void Mission::loadStorylineMission( MissionInfo *info ) {
	for( int i = 0; i < static_cast<int>(info->itemCount); i++ ) {
		char *p = (char*)info->itemName[i];
		for( unsigned int t = 0; t < itemList.size(); t++ ) {
			RpgItem *rpgItem = itemList[t];
			if( !strcmp( rpgItem->getName(), p ) ) {
				items[rpgItem] = ( info->itemDone[i] != 0 );
				break;
			}
		}
	}
	for( int i = 0; i < static_cast<int>(info->monsterCount); i++ ) {
		char *p = (char*)info->monsterName[i];
		for( unsigned int t = 0; t < creatureList.size(); t++ ) {
			Monster *monster = creatureList[t];
			if( !strcmp( monster->getType(), p ) ) {
				creatures[monster] = ( info->monsterDone[i] ? true : false );
				break;
			}
		}
	}
	setCompleted( info->completed ? true : false );
}

string outdoors_ambient_sound = "outdoors";
string& Mission::getAmbientSoundName() { 
	return( !isStoryLine() && board->getSession()->getGameAdapter()->getCurrentDepth() == 0 ? outdoors_ambient_sound : ambientSoundName );
}

NpcInfo *Mission::addNpcInfo( int x, int y, char *npcName, int level, char *npcType, char *npcSubType ) {
	string key = getNpcInfoKey( x,y );
	NpcInfo *npcInfo = 
		new NpcInfo( x, y, 
								 npcName, 
								 level, 
								 npcType, 
								 ( strlen( npcSubType ) ? npcSubType : NULL ) ); 
	npcInfos[ key ] = npcInfo;
	return npcInfo;
}

NpcInfo *Mission::addNpcInfo( NpcInfo *info ) {
	string key = getNpcInfoKey( info->x, info->y );
	npcInfos[ key ] = info;
	return info;
}

void Mission::createTypedNpc( Creature *creature, int level, int fx, int fy ) {
	int npcType = 1 + Util::dice( Constants::NPC_TYPE_COUNT - 1 );
	int const NAME_LEN = 254;
	char npcSubType[NAME_LEN+1] = {0};
	char npcTypeName[NAME_LEN+1] = {0};
	strcpy( npcTypeName, _( Constants::npcTypeDisplayName[ npcType ] ) );
	if( npcType == Constants::NPC_TYPE_MERCHANT ) {
		// fixme: trade-able should be an attribute to itemType in item.cfg
		int n = Util::dice( 5 );
		switch( n ) {
		case 0: strcpy( npcSubType, "POTION;WAND;RING;AMULET;STAFF" ); strcpy( npcTypeName, _( "Magic Merchant" ) ); break;
		case 1: strcpy( npcSubType, "ARMOR" ); strcpy( npcTypeName, _( "Armor Merchant" ) ); break;
		case 2: strcpy( npcSubType, "FOOD;DRINK" ); strcpy( npcTypeName, _( "Rations Merchant" ) ); break;
		case 3: strcpy( npcSubType, "SCROLL" ); strcpy( npcTypeName, _( "Scrolls Merchant" ) ); break;
		case 4: strcpy( npcSubType, "SWORD;AXE;BOW;MACE;POLE" ); strcpy( npcTypeName, _( "Weapons Merchant" ) ); break;
		}
	} else if( npcType == Constants::NPC_TYPE_TRAINER ) {
		Character *character = Character::getRandomCharacter();
		strcpy( npcSubType, character->getDisplayName() );
		snprintf( npcTypeName, NAME_LEN, _( "Trainer for %s" ), character->getDisplayName() );
	} else if( npcType == Constants::NPC_TYPE_HEALER ) {
		strcpy( npcSubType, MagicSchool::getRandomSchool()->getDisplayName() );
	}
	char name[NAME_LEN+1] = {0};
	snprintf( name,NAME_LEN, _( "%s the %s" ), Rpg::createName(), npcTypeName );
	NpcInfo *npcInfo = Mission::addNpcInfo( fx, fy, name, level, (char*)Constants::npcTypeName[ npcType ], npcSubType );
	creature->setNpcInfo( npcInfo );
}

NpcInfo::NpcInfo( int x, int y, char *name, int level, char *type, char *subtype ) {
  this->x = x;
  this->y = y;
  strcpy( this->name, name );
  this->level = level;
  this->type = -1;
  for( int i = 0; i < Constants::NPC_TYPE_COUNT; i++ ) {
    if( !strcmp( type, Constants::npcTypeName[ i ] ) ) {
      this->type = i;
      break;
    }
  }
  if( this->type == -1 ) {
    cerr << "Error: npc type " << type << " is not known. Setting it to commoner." << endl;
    this->type = 0;
  }
  
  if( subtype ) {
    // store as a string
    strcpy( this->subtypeStr, subtype );
    
    // parse for some npc types
    char s[255];
    strcpy( s, subtype );
    char *p = strtok( s, ";" );
    while( p ) {
      if( this->type == Constants::NPC_TYPE_MERCHANT ) {
        // subtype is an RpgItem type
        this->subtype.insert( RpgItem::getTypeByName( p ) );
      } else if( this->type == Constants::NPC_TYPE_TRAINER ) {
        // subtype is a root profession
				this->subtype.insert( Character::getRootCharacterIndexByName( p ) );
      } else {
        break;
      }
      p = strtok( NULL, ";" );
    }
  }
}

NpcInfo::~NpcInfo() {
}

NpcInfoInfo *NpcInfo::save() {
	NpcInfoInfo *info = (NpcInfoInfo*)malloc(sizeof(NpcInfoInfo));
	info->x = x;
	info->y = y;
	strcpy( (char*)info->name, name );
	info->level = level;
	info->type = type;
	strcpy( (char*)info->subtype, subtypeStr );	
	return info;	
}

NpcInfo *NpcInfo::load( NpcInfoInfo *info ) {
	return new NpcInfo( info->x, info->y, (char*)info->name, info->level, (char*)Constants::npcTypeName[info->type], (char*)info->subtype );
}
