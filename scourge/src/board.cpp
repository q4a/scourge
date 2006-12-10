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

using namespace std;

vector<string> Mission::intros;
vector<string> Mission::unknownPhrases;
map<string, int> Mission::conversations;
vector<string> Mission::answers;

map<string, NpcConversation*> Mission::npcConversations;
map<string, NpcInfo*> Mission::npcInfos;

//#define DEBUG_MODE 1
	
/**
  *@author Gabor Torok
  */

Board::Board(Session *session) {
  this->session = session;
  this->storylineIndex = 0;
	missionListCount = 0;

  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/missions.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  Mission *current_mission = NULL;
  char type;
  char name[255], line[255], description[2000], 
    success[2000], failure[2000], mapName[80];
  //char keyphrase[80],answer[4000];
  //Monster *currentNpc = NULL;
  int n = fgetc(fp);
  while(n != EOF) {
    if( n == 'M' ) {
      // skip ':'
      fgetc( fp );
      n = Constants::readLine( line, fp );
      type = line[0];
      strcpy( name, line + 2 );
      strcpy( description, "" );
      while( n == 'D' ) {
        n = Constants::readLine( line, fp );
        if( strlen( description ) ) strcat( description, " " );
        strcat( description, line + 1 );
      }

      strcpy( success, "" );
      while( n == 'Y' ) {
        n = Constants::readLine( line, fp );
        if( strlen( success ) ) strcat( success, " " );
        strcat( success, line + 1 );
      }

      strcpy( failure, "" );
      while( n == 'N' ) {
        n = Constants::readLine( line, fp );
        if( strlen( failure ) ) strcat( failure, " " );
        strcat( failure, line + 1 );
      }

      templates.push_back( new MissionTemplate( this, name, type, description, success, failure ) );
    } else if( n == 'T' ) {
      // skip ':'
      fgetc( fp );
      // read the name
      n = Constants::readLine( name, fp );

      // read the level and depth
      n = Constants::readLine( line, fp );
      int level = atoi( strtok( line + 1, "," ) );
      int depth = atoi( strtok( NULL, ",") );
      char *p = strtok( NULL, "," );
      if( p ) strcpy( mapName, p );
      else strcpy( mapName, "" );

      strcpy( description, "" );
      while( n == 'D' ) {
        n = Constants::readLine( line, fp );
        if( strlen( description ) ) strcat( description, " " );
        strcat( description, line + 1 );
      }

      strcpy( success, "" );
      while( n == 'Y' ) {
        n = Constants::readLine( line, fp );
        if( strlen( success ) ) strcat( success, " " );
        strcat( success, line + 1 );
      }

      strcpy( failure, "" );
      while( n == 'N' ) {
        n = Constants::readLine( line, fp );
        if( strlen( failure ) ) strcat( failure, " " );
        strcat( failure, line + 1 );
      }

      current_mission = new Mission( this, level, depth, name, description, success, failure, mapName );
      current_mission->setStoryLine( true );
      storylineMissions.push_back( current_mission );

    } else if(n == 'I' && current_mission) {
      fgetc(fp);
      n = Constants::readLine( line, fp );
      RpgItem *item = RpgItem::getItemByName(line);
      current_mission->addItem( item );
    
    } else if(n == 'C' && current_mission) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      Monster *monster = Monster::getMonsterByName(line);
      if( !monster ) {
        cerr << "*** Error: can't find mission monster \"" << line << "\"." << endl;
        exit( 1 );
      }
      current_mission->addCreature( monster );
    } else if(n == 'S' && current_mission) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      current_mission->setSpecial( line );
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

int Mission::readConversationLine( FILE *fp, char *line, int n,
                                   vector<string> *intros,
                                   vector<string> *unknownPhrases,
                                   map<string, int> *conversations,
                                   vector<string> *answers ) {

  // find the first comma
  char keyphrase[80], answer[4000];
  char last = line[ strlen( line ) - 1 ];
  char *p = strchr( line, ';' );
	/*
	This happens if the line has bad syntax or in 'general-only' mode when skipping
	other lines.
	*/
	if( !p ) return last;
  strcpy( answer, p + 1 );
  strcpy( keyphrase, strtok( line, ";" ) );   
  //cerr << "1: keyphrase=" << keyphrase << endl;

  // Read lines that end with a \.
  int r;
  while( last == '\\' ) {
    r = strlen( answer ) - 1;
    answer[ r ] = ' ';
    answer[ r + 1 ] = n;
    answer[ r + 2 ] = '\0';
    n = Constants::readLine(line, fp);
    strcat( answer, line );
    last = line[ strlen( line ) - 1 ];
  }

  string ks = keyphrase;
  string as = answer;

  if( !strcmp( keyphrase, INTRO_PHRASE ) ) {
    intros->push_back( as );
  } else if( !strcmp( keyphrase, UNKNOWN_PHRASE ) ) {
    unknownPhrases->push_back( as );
  } else {
    char tmp[80];
    p = strtok( keyphrase, "," );
    while( p ) {
      //cerr << "\t2: p=" << p << endl;
      strcpy( tmp, p );
      string lower = Util::toLowerCase( tmp );
      (*conversations)[ lower ] = answers->size();
      p = strtok( NULL, "," );
    }
    answers->push_back( as );
  }

  return n;
}

Board::~Board() {
  freeListText();
}

void Board::freeListText() {
  if( missionListCount ) {
    for( int i = 0; i < missionListCount; i++ ) {
      free( missionText[i] );
    }
    free( missionText );
    free( missionColor );
  }
}

void Board::reset() {
  for( int i = 0; i < (int)storylineMissions.size(); i++ ) {
    Mission *mission = storylineMissions[i];
    mission->reset();
  }
  storylineIndex = 0;
  for( int i = 0; i < (int)availableMissions.size(); i++ ) {
    Mission *mission = availableMissions[i];
    if( !mission->isStoryLine() ) {
      delete mission;
    }
  }
  availableMissions.clear();
}

void Board::initMissions() {
  // free ui
  freeListText();

  // remove completed missions
  for (vector<Mission*>::iterator e=availableMissions.begin(); e!=availableMissions.end(); ++e) {
    Mission *mission = *e;
    if( !mission->isStoryLine() && mission->isCompleted() ) {
      delete mission;
      availableMissions.erase(e);
      e = availableMissions.begin();
    }
  }

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
  float ave = ((float)sum / (float)session->getParty()->getPartySize() / 1.0f);

  // remove the storyline missions
  // remove missions whose level is too low
  for( int i = 0; i < (int)availableMissions.size(); i++ ) {
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
	for( int counter = 0; availableMissions.size() < 8; counter++ ) {
		int level;
		if( counter % 2 == 0 ) {
			// allow for low level mission
			level = (int)( (float)( ave + 2 ) * rand()/RAND_MAX );
		} else {
			// allow for current level missions
			level = (int)( ave + ( 4.0f * rand()/RAND_MAX ) ) - 2;
		}
    if( level < 1 ) level = 1;
    int depth =  (int)((float)level / (float)(MAX_MISSION_DEPTH - 3) ) + 1 + (int)( 3.0f * rand()/RAND_MAX );
    if( depth > MAX_MISSION_DEPTH ) depth = MAX_MISSION_DEPTH;
    int templateIndex = (int)( (float)( templates.size() ) * rand()/RAND_MAX );
    Mission *mission = templates[ templateIndex ]->createMission( session, level, depth );
    availableMissions.push_back( mission );
  }  

  // add the current storyline mission at the top of the board
  if( storylineIndex >= 0 && storylineIndex <  (int)storylineMissions.size() )
    availableMissions.insert( availableMissions.begin(), 
															storylineMissions[ storylineIndex ] );


#ifdef DEBUG_MODE
  // debug missions
  for( int i = 1; i <= highest; i++ ) {
    int templateIndex = (int)( (float)( templates.size() ) * rand()/RAND_MAX );
    Mission *mission = templates[ templateIndex ]->createMission( session, i, 1 );
    availableMissions.push_back( mission );
  }
#endif

  // init ui
  if(availableMissions.size()) {
		missionListCount = availableMissions.size();
    missionText = (char**)malloc(availableMissions.size() * sizeof(char*));
    missionColor = (Color*)malloc(availableMissions.size() * sizeof(Color));
    for(int i = 0; i < (int)availableMissions.size(); i++) {
      missionText[i] = (char*)malloc(120 * sizeof(char));
      sprintf(missionText[i], "L:%d, S:%d, %s %s%s", 
              availableMissions[i]->getLevel(), 
              availableMissions[i]->getDepth(), 
              ( availableMissions[i]->isStoryLine() ? "(STORY)" : 
                ( strstr( availableMissions[i]->getMapName(), "caves" ) ? "(CAVE)" : "" ) ),
              availableMissions[i]->getName(),
              (availableMissions[i]->isCompleted() ? "(completed)" : ""));
      missionColor[i].r = 1.0f;
      missionColor[i].g = 1.0f;
      missionColor[i].b = 0.0f;
      if(availableMissions[i]->isCompleted()) {
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
          session->getGameAdapter()->setMissionDescriptionUI((char*)availableMissions[i]->getDescription(),
                                                             availableMissions[i]->getMapX(),
                                                             availableMissions[i]->getMapY());
      }
    }

    if(!session->getGameAdapter()->isHeadless()) 
      session->getGameAdapter()->updateBoardUI(availableMissions.size(), 
                                               (const char**)missionText, 
                                               missionColor);
  }
}

void Board::setStorylineIndex( int n ) {
  if( n < 0 ) n = 0;
  if( n > (int)storylineMissions.size() ) n = (int)storylineMissions.size();
  storylineIndex = n;
  for( int i = 0; i < (int)storylineMissions.size(); i++ ) {
    storylineMissions[i]->setCompleted( i < storylineIndex ? true : false );
  }
}

void Board::storylineMissionCompleted( Mission *mission ) {
  for( int i = 0; i < (int)storylineMissions.size(); i++ ) {
    if( storylineMissions[i] == mission &&
        storylineIndex < ( i + 1 )) {
      storylineIndex = i + 1;
      break;
    }
  }
}






MissionTemplate::MissionTemplate( Board *board, char *name, char type, char *description, char *success, char *failure ) {
  this->board = board;
  strcpy( this->name, name );
  this->mapType = type;
  strcpy( this->description, description );
  strcpy( this->success, success );
  strcpy( this->failure, failure );
}

MissionTemplate::~MissionTemplate() {
}

Mission *MissionTemplate::createMission( Session *session, int level, int depth, MissionInfo *info ) {

//  cerr << "*** Creating level " << level << " mission, using template: " << this->name << endl;

  map<string, RpgItem*> items;
  map<string, Monster*> creatures;

  char parsedName[80];
  char parsedDescription[2000];
  char parsedSuccess[2000];
  char parsedFailure[2000];
  char s[2000];
  strcpy( s, name );
  parseText( session, level, depth, s, parsedName, &items, &creatures, info );
  strcpy( s, description );
  parseText( session, level, depth, s, parsedDescription, &items, &creatures, info );
  strcpy( s, success );
  parseText( session, level, depth, s, parsedSuccess, &items, &creatures, info );
  strcpy( s, failure );
  parseText( session, level, depth, s, parsedFailure, &items, &creatures, info );
  
  Mission *mission = new Mission( board, 
                                  level, depth, parsedName, 
                                  parsedDescription, parsedSuccess, 
                                  parsedFailure, NULL, mapType );
  for(map<string, RpgItem*>::iterator i=items.begin(); i!=items.end(); ++i) {
    RpgItem *item = i->second;
		bool found = false;
		bool value = false;
		if( info ) {
			for( int i = 0; i < info->itemCount; i++ ) {
				if( !strcmp( (char*)info->itemName[ i ], item->getName() ) ) {
					found = true;
					value = info->itemDone[ i ];
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
					value = info->monsterDone[ i ];
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
	if( info ) {
		mission->setSavedMapName( (char*)info->mapName );
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
        strcat( parsedText, item->getName() );
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
              cerr << "Warning: no monsters defined for level: " << level << endl;
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
        strcat( parsedText, monster->getType() );
      }
    } else {
      strcat( parsedText, p );
    }
    p = strtok( NULL, " " );
  }
}


Mission::Mission( Board *board, int level, int depth, 
                  char *name, char *description, 
                  char *success, char *failure,
                  char *mapName, char mapType ) {
  this->board = board;
  this->level = level;
  this->depth = depth;
  strcpy( this->name, name );
  strcpy( this->description, description );
  strcpy( this->success, success );
  strcpy( this->failure, failure );
  strcpy( this->mapName, ( mapName ? mapName : "" ) );
  this->completed = false;
  this->storyLine = false;
  this->mapX = this->mapY = 0;
  this->special[0] = '\0';
	this->templateName[0] = '\0';
	strcpy( this->savedMapName, "" );

  // assign the map grid location
  if( mapName && strlen( mapName ) ) {
    edited = true;
    char result[255];
    board->getSession()->getMap()->loadMapLocation( mapName, result, &mapX, &mapY );
  } else {
    edited = false;
    char *s;
    board->getSession()->getShapePalette()->getRandomMapLocation( mapType, &s, &mapX, &mapY );
    strcpy( this->mapName, s );
  }
  
//  cerr << "*** Created mission: " << getName() << endl;
//  cerr << "\tmap name=" << ( strlen( this->mapName ) ? mapName : "<random>" ) << endl;
//  cerr << getDescription() << endl;
//  cerr << "-----------------------" << endl;
}

Mission::~Mission() {
  items.clear();
  itemList.clear();
  creatures.clear();
  creatureList.clear();
  itemInstanceMap.clear();
  monsterInstanceMap.clear();
}

bool Mission::itemFound(Item *item) {
//  cerr << "completed=" << completed << endl;
  if( !completed ) {
    //cerr << "\titemInstanceMap.size()=" << itemInstanceMap.size() << endl;
    if( itemInstanceMap.find( item ) != itemInstanceMap.end() ) {
      //cerr << "\t111" << endl;
      RpgItem *rpgItem = itemInstanceMap[ item ];
      if( items.find( rpgItem ) != items.end() ) {
        //cerr << "\t222" << endl;
        items[ rpgItem ] = true;
        checkMissionCompleted();
      }
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
  deleteItemMonsterInstances();
}

void Mission::deleteItemMonsterInstances() {
  // also remove mission objects from party's inventory
  for(map<Item*, RpgItem*>::iterator i=itemInstanceMap.begin(); i!=itemInstanceMap.end(); ++i) {
    Item *item = i->first;
    bool itemRemoved = false;
    for(int i = 0; i < board->getSession()->getParty()->getPartySize(); i++) {
      Creature *c = board->getSession()->getParty()->getParty(i);
      for( int t = 0; t < c->getInventoryCount(); t++ ) {
        if( c->getInventory( t ) == item ) {
          //cerr << "Removing mission item: " << item->getRpgItem()->getName() << " from " << c->getName() << endl;
          c->removeInventory( t );
          itemRemoved = true;
          break;
        }
      }
      if( itemRemoved ) break;
    }
  }
  itemInstanceMap.clear();
  monsterInstanceMap.clear();
}

char *Mission::getIntro() {
  return (char*)(intros[ (int)( (float)( intros.size() ) * rand()/RAND_MAX ) ].c_str());
}

char *Mission::getAnswer( char *keyphrase ) {
  string ks = keyphrase;
  if( conversations.find( ks ) != conversations.end() ) {
    return (char*)( answers[ conversations[ ks ] ].c_str());
  } else {
    cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
    return (char*)(unknownPhrases[ (int)( (float)( unknownPhrases.size() ) * rand()/RAND_MAX ) ].c_str());
  }
}

char *Mission::getIntro( char *s ) {
	string npc = s;
  if( npcConversations.find( s ) == npcConversations.end() ) {
    //cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
    return NULL;
  }
  NpcConversation *nc = npcConversations[ s ];
  return (char*)(nc->npc_intros[ (int)( (float)( nc->npc_intros.size() ) * rand()/RAND_MAX ) ].c_str());
}

bool Mission::setIntro( Creature *s, char *keyphrase ) {
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
	if( !nc ) return false;
	string ks = keyphrase;
	if( nc->npc_conversations.find( ks ) != nc->npc_conversations.end() ) {
		nc->npc_intros.clear();
    nc->npc_intros.push_back( nc->npc_answers[ nc->npc_conversations[ ks ] ] );
	} else {
		cerr << "Can't find " << keyphrase << " in npc conversation for creature: " << s->getName() << endl;
		return false;
	}
	return true;
}

char *Mission::getAnswer( char *s, char *keyphrase ) {
	string npc = s;
  if( npcConversations.find( npc ) == npcConversations.end() ) {
    //cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
    return NULL;
  }
  NpcConversation *nc = npcConversations[ npc ];

  string ks = keyphrase;
  if( nc->npc_conversations.find( ks ) != nc->npc_conversations.end() ) {
    return (char*)( nc->npc_answers[ nc->npc_conversations[ ks ] ].c_str());
  } else {
    cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
    return (char*)(nc->npc_unknownPhrases[ (int)( (float)( nc->npc_unknownPhrases.size() ) * rand()/RAND_MAX ) ].c_str());
  }
}

void Mission::loadMapData( GameAdapter *adapter, const char *filename ) {

  // clean up
  for( map<string,NpcInfo*>::iterator i=npcInfos.begin(); i!=npcInfos.end(); ++i) {
    NpcInfo *npcInfo = i->second;
    delete npcInfo;
  }
  npcInfos.clear();
  intros.clear();
  unknownPhrases.clear();
  conversations.clear();
  answers.clear();
  for (map<string,NpcConversation*>::iterator i=npcConversations.begin(); i!=npcConversations.end(); ++i) {
    NpcConversation *npcConversation = i->second;
    delete npcConversation;
  }
  npcConversations.clear();

  // read general conversation from level 0.
  char dup[300];
  strcpy( dup, filename );
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
        loadMapDataFile( adapter, dup, true );
      }
    }
  }

  // read the level's data
  loadMapDataFile( adapter, filename );

	// if there are npc-s add some wondering heroes
	//if( !npcInfos.empty() ) {
		//addWanderingHeroes( adapter );
	//}
}

/*
void Mission::addWanderingHeroes( GameAdapter *adapter ) {

	if( !adapter->hasParty() ) return;

	int level = adapter->getSession()->getParty()->getAverageLevel();
	int count = (int)( 5.0f * rand() / RAND_MAX ) + 5;
	for( int i = 0; i < count; i++ ) {
		RenderedCreature *creature = adapter->createWanderingHero( level );

		// find a place for the here
		RenderedCreature *c = 
			adapter->getSession()->
			getCreature( (int)( (float)(adapter->getSession()->getCreatureCount()) * 
													rand() / RAND_MAX ) );
		creature->findPlace( toint( c->getX() ), toint( c->getY() ) );
	}
}
*/
void Mission::loadMapDataFile( GameAdapter *adapter, const char *filename, bool generalOnly ) {

	//cerr << "Current creatures:" << endl;
	//for( int i = 0; i < adapter->getSession()->getCreatureCount(); i++ ) {
		//cerr << "\t" << adapter->getSession()->getCreature(i)->getName() << endl;
	//}

  FILE *fp = openMapDataFile( filename, "r" );
  if( !fp ) return;

  char line[1000];
  int x, y, level;
  char npcName[255], npcType[255], npcSubType[1000];
  //Monster *currentNpc = NULL;
	char *currentNpc = NULL;
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'G') {
      fgetc(fp);
      n = Constants::readLine(line, fp);

      n = readConversationLine( fp, line, n, 
                                &Mission::intros, 
                                &Mission::unknownPhrases,
                                &Mission::conversations,
                                &Mission::answers );
    } else if( !generalOnly ) {
      if( n == 'P' ) {
        fgetc(fp);
        n = Constants::readLine(line, fp);
        Monster *m = Monster::getMonsterByName( line );
				if( m ) {
					currentNpc = m->getType();
				} else {
					Creature *c = adapter->getSession()->getCreatureByName( line );
					if( c ) currentNpc = c->getName();
					else {
						cerr << "*** Error: can't find creature named: " << line << endl;
					}
				}
      } else if( n == 'V' && currentNpc ) {    
        fgetc(fp);
        n = Constants::readLine(line, fp);
  
        NpcConversation *npcConv;
        if( Mission::npcConversations.find( currentNpc ) == Mission::npcConversations.end() ) {
          npcConv = new NpcConversation();
          Mission::npcConversations[ currentNpc ] = npcConv;
        } else {
          npcConv = Mission::npcConversations[ currentNpc ];
        }
  
        n = readConversationLine( fp, line, n, 
                                  &npcConv->npc_intros, 
                                  &npcConv->npc_unknownPhrases,
                                  &npcConv->npc_conversations,
                                  &npcConv->npc_answers );
      } else if( n == 'N' ) { 
        fgetc( fp );
        n = Constants::readLine(line, fp);
  
        x = atoi( strtok( line, "," ) );
        y = atoi( strtok( NULL, "," ) );
        strcpy( npcName, strtok( NULL, "," ) );
        level = atoi( strtok( NULL, "," ) );
        strcpy( npcType, strtok( NULL, "," ) );
        char *p = strtok( NULL, "," );
        strcpy( npcSubType, ( p ? p : "" ) );
  
        // store npc info
        string key = getNpcInfoKey( x,y );
        NpcInfo *npcInfo = 
          new NpcInfo( x, y, 
                       strdup( npcName ), 
                       level, 
                       strdup( npcType ), 
                       ( strlen( npcSubType ) ? 
                         strdup( npcSubType ) : 
                         NULL ) );
        npcInfos[ key ] = npcInfo;
  
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
						cerr << "Error: npc definition in " << filename << 
							" doesn't point to an npc. Line: " << line << " npc=" << npcInfo->name << endl;
					//} else {
						//cerr << "* found npc by name: " << npcInfo->name << endl;
					}
        } else {
          ((Creature*)(pos->creature))->setNpcInfo( npcInfo );
        }
      } else {
        n = Constants::readLine(line, fp);
      }
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

NpcInfo *Mission::getNpcInfo( int x, int y ) {
  string key = getNpcInfoKey( x,y );
  return( npcInfos.find( key ) == npcInfos.end() ? NULL : npcInfos[ key ] );
}

string Mission::getNpcInfoKey( int x, int y ) {
  char line[255];
  sprintf( line, "%d,%d", x, y );
  string key = line;
  return key;
}

FILE *Mission::openMapDataFile( const char *filename, const char *mode ) {

  // Create the text file name from the map file name.
  char s[200];
  strncpy( s, filename, 200 );
  char *p = strrchr( s, '.' );
  if( p && 
      strlen( p ) >= 4 && 
      !strncmp( p, ".map", 4 ) ) {
    strcpy( p, ".txt" );
  } else {
    strcat( s, ".txt" );
  }

  // Open the file
  cerr << "*** Opening: " << s << " in mode: " << mode << endl;
  FILE *fp = fopen( s, mode );
  if(!fp) {        
    char errMessage[500];
    sprintf( errMessage, "Unable to find the file: %s!", s );
    cerr << errMessage << endl;
    return NULL;
  }
  return fp;
}

void Mission::saveMapData( GameAdapter *adapter, const char *filename ) {
  // find new npc-s
  int total = 0;
  vector< Creature* > newNpcs;
  for( int i = 0; i < adapter->getSession()->getCreatureCount(); i++ ) {
    Creature *creature = adapter->getSession()->getCreature( i );
    if( creature->getMonster() && 
				creature->getMonster()->isNpc() ) {
      total++;
      if( toint( creature->getX() ) && toint( creature->getY() ) &&
          !getNpcInfo( toint( creature->getX() ), toint( creature->getY() ) ) ) {
        newNpcs.push_back( creature );
      }
    }
  }

  // append to txt file the new npc info
  if( newNpcs.size() > 0 ) {
    cerr << "Saving npcInfos: " << newNpcs.size() << " out of " << total << endl;
    FILE *fp = openMapDataFile( filename, "a" );
    if( !fp ) return;

    // Are there default conversation elements?
    if( Mission::intros.size() == 0 ) 
      fprintf( fp, "G:_INTRO_;Welcome weary adventurer!.\n" );
    if( Mission::unknownPhrases.size() == 0 ) 
      fprintf( fp, "G:_UNKNOWN_;Uh, I don't know anything about that...\n" );

    fprintf( fp, "# Unknown npc-s found:\n" );
    fprintf( fp, "# Key:\n" );
    fprintf( fp, "#   N:x,y,name,level,type[,subtype]\n\n" );
    for( int i = 0; i < (int)newNpcs.size(); i++ ) {
      Creature *creature = newNpcs[ i ];
      fprintf( fp, "N:%d,%d,%s,1,commoner\n", 
               toint( creature->getX() ), 
               toint( creature->getY() ), 
               creature->getMonster()->getType() );
    }
    fclose( fp );
  }
}

MissionInfo *Mission::save() {
	MissionInfo *info = (MissionInfo*)malloc(sizeof(MissionInfo));
  info->version = PERSIST_VERSION;
  strncpy( (char*)info->templateName, getTemplateName(), 79);
  info->templateName[79] = 0;
	strcpy( (char*)info->mapName, savedMapName );
	info->level = getLevel();
	info->depth = getDepth();
	info->completed = ( completed ? 1 : 0 );
	info->itemCount = itemList.size();
	for( int i = 0; i < (int)itemList.size(); i++ ) {
		strncpy( (char*)info->itemName[i], itemList[i]->getName(), 254 );
		info->itemName[i][254] = '\0';
		info->itemDone[i] = ( items[ itemList[i] ] ? 1 : 0 );
	}
	info->monsterCount = creatureList.size();
	for( int i = 0; i < (int)creatureList.size(); i++ ) {
		strncpy( (char*)info->monsterName[i], creatureList[i]->getType(), 254 );
		info->monsterName[i][254] = '\0';
		info->monsterDone[i] = ( creatures[ creatureList[i] ] ? 1 : 0 );
	}
	return info;
}

Mission *Mission::load( Session *session, MissionInfo *info ) {
	// find the template
	MissionTemplate *missionTemplate = session->getBoard()->
		findTemplateByName( (char*)info->templateName );
	if( !missionTemplate ) {
		cerr << "Can't find template for name: " << info->templateName << endl;
		return NULL;
	}
	cerr << "Loading mission with template: " << (char*)(info->templateName) << endl;
	return missionTemplate->createMission( session, info->level, info->depth, info );
}

NpcInfo::NpcInfo( int x, int y, char *name, int level, char *type, char *subtype ) {
  this->x = x;
  this->y = y;
  this->name = name;
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
    this->subtypeStr = subtype;
    
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
  delete name;
}

