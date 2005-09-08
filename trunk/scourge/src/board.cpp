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

vector<string> Mission::intros;
vector<string> Mission::unknownPhrases;
map<string, string> Mission::conversations;
map<Monster*,NpcConversation*> Mission::npcConversations;
map<string, NpcInfo*> Mission::npcInfos;

//#define DEBUG_MODE 1
	
/**
  *@author Gabor Torok
  */

Board::Board(Session *session) {
  this->session = session;
  this->storylineIndex = 0;

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
      n = Constants::readLine( name, fp );
      type = name[0];
      strcpy( name, name + 2 );
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
    
    } else if(n == 'M' && current_mission) {
      fgetc(fp);
      n = Constants::readLine(line, fp);
      Monster *monster = Monster::getMonsterByName(line);
      current_mission->addCreature( monster );
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

int Mission::readConversationLine( FILE *fp, char *line,
                                   char *keyphrase, char *answer,
                                   int n ) {
  // find the first comma
  char last = line[ strlen( line ) - 1 ];
  char *p = strchr( line, ',' );
  strcpy( answer, p + 1 );
  strcpy( keyphrase, strtok( line, "," ) );     

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
  return n;
}

Board::~Board() {
  freeListText();
}

void Board::freeListText() {
  if( availableMissions.size() ) {
    for( int i = 0; i < (int)availableMissions.size(); i++ ) {
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
    if( mission->isStoryLine() || 
        mission->getLevel() < (int)( ave - 2.0f ) ) {
      // move the last element over the current storyline element
      availableMissions[ i ] = availableMissions[ availableMissions.size() - 1 ];
      availableMissions.pop_back();
      // re-examine the current element
      i--;
    }
  }

  // add the current storyline mission
  if( storylineIndex >= 0 && storylineIndex <  (int)storylineMissions.size() &&
     storylineMissions[ storylineIndex ]->getLevel() <= highest )
    availableMissions.push_back( storylineMissions[ storylineIndex ] );

  // maintain a set of missions
  while( availableMissions.size() < 5 ) {
    int level = (int)( ave + ( 4.0f * rand()/RAND_MAX ) ) - 2;
    if( level < 1 ) level = 1;
    int depth =  (int)((float)level / (float)(MAX_MISSION_DEPTH - 3) ) + 1 + (int)( 3.0f * rand()/RAND_MAX );
    if( depth > 9 ) depth = 9;
    int templateIndex = (int)( (float)( templates.size() ) * rand()/RAND_MAX );
    Mission *mission = templates[ templateIndex ]->createMission( session, level, depth );
    availableMissions.push_back( mission );
  }  

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
    missionText = (char**)malloc(availableMissions.size() * sizeof(char*));
    missionColor = (Color*)malloc(availableMissions.size() * sizeof(Color));
    for(int i = 0; i < (int)availableMissions.size(); i++) {
      missionText[i] = (char*)malloc(120 * sizeof(char));
      sprintf(missionText[i], "L:%d, S:%d, %s%s", 
              availableMissions[i]->getLevel(), 
              availableMissions[i]->getDepth(), 
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

Mission *MissionTemplate::createMission( Session *session, int level, int depth ) {

//  cerr << "*** Creating level " << level << " mission, using template: " << this->name << endl;

  map<string, RpgItem*> items;
  map<string, Monster*> creatures;

  char parsedName[80];
  char parsedDescription[2000];
  char parsedSuccess[2000];
  char parsedFailure[2000];
  char s[2000];
  strcpy( s, name );
  parseText( session, level, depth, s, parsedName, &items, &creatures );
  strcpy( s, description );
  parseText( session, level, depth, s, parsedDescription, &items, &creatures );
  strcpy( s, success );
  parseText( session, level, depth, s, parsedSuccess, &items, &creatures );
  strcpy( s, failure );
  parseText( session, level, depth, s, parsedFailure, &items, &creatures );
  
  Mission *mission = new Mission( board, 
                                  level, depth, parsedName, 
                                  parsedDescription, parsedSuccess, 
                                  parsedFailure, NULL, mapType );
  for(map<string, RpgItem*>::iterator i=items.begin(); i!=items.end(); ++i) {
    RpgItem *item = i->second;
    mission->addItem( item );
  }
  for(map<string, Monster*>::iterator i=creatures.begin(); i!=creatures.end(); ++i) {
    Monster *monster = i->second;
    mission->addCreature( monster );
  }

  return mission;
}                                               

void MissionTemplate::parseText( Session *session, int level, int depth,
                                 char *text, char *parsedText,
                                 map<string, RpgItem*> *items, 
                                 map<string, Monster*> *creatures ) {
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
          item = RpgItem::getRandomItem( 1 );
          //item->setBlocking(true); // don't let monsters pick this up
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
            monster = Monster::getRandomMonster( monsterLevel );
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

  // assign the map grid location
  if( mapName && strlen( mapName ) ) {
    char result[255];
    board->getSession()->getMap()->loadMapLocation( mapName, result, &mapX, &mapY );
  } else {
    board->getSession()->getShapePalette()->getRandomMapLocation( mapType, NULL, &mapX, &mapY );
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
  //cerr << "checkMissionCompleted, items" << endl;
  for(map<RpgItem*, bool >::iterator i=items.begin(); i!=items.end(); ++i) {
    //cerr << "\titem" << i->first->getName() << endl;
    bool b = i->second;
    //cerr << "\t\tb=" << b << endl;
    if( !b ) {
      completed = false;
      return;
    }
  }
  //cerr << "checkMissionCompleted, monster" << endl;
  for(map<Monster*, bool >::iterator i=creatures.begin(); i!=creatures.end(); ++i) {
    //cerr << "\tmonster" << i->first->getType() << endl;
    bool b = i->second;
    //cerr << "\t\tb=" << b << endl;
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
    return (char*)(conversations[ ks ].c_str());
  } else {
    cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
    return (char*)(unknownPhrases[ (int)( (float)( unknownPhrases.size() ) * rand()/RAND_MAX ) ].c_str());
  }
}

char *Mission::getIntro( Monster *npc ) {
  if( npcConversations.find( npc ) == npcConversations.end() ) {
    //cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
    return NULL;
  }
  NpcConversation *nc = npcConversations[ npc ];
  return (char*)(nc->npc_intros[ (int)( (float)( nc->npc_intros.size() ) * rand()/RAND_MAX ) ].c_str());
}

char *Mission::getAnswer( Monster *npc, char *keyphrase ) {
  if( npcConversations.find( npc ) == npcConversations.end() ) {
    cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
    return NULL;
  }
  NpcConversation *nc = npcConversations[ npc ];

  string ks = keyphrase;
  if( nc->npc_conversations.find( ks ) != nc->npc_conversations.end() ) {
    return (char*)(nc->npc_conversations[ ks ].c_str());
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
  for (map<Monster*,NpcConversation*>::iterator i=npcConversations.begin(); i!=npcConversations.end(); ++i) {
    NpcConversation *npcConversation = i->second;
    delete npcConversation;
  }
  npcConversations.clear();


  FILE *fp = openMapDataFile( filename, "r" );
  if( !fp ) return;

  char line[255], keyphrase[80],answer[4000];
  int x, y, level;
  char npcName[255], npcType[255], npcSubType[255];
  Monster *currentNpc = NULL;
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'G') {
      fgetc(fp);
      n = Constants::readLine(line, fp);

      n = readConversationLine( fp, line, keyphrase, answer, n );

      string ks = keyphrase;
      string as = answer;

      if( !strcmp( keyphrase, INTRO_PHRASE ) ) {
        Mission::intros.push_back( as );
      } else if( !strcmp( keyphrase, UNKNOWN_PHRASE ) ) {
        Mission::unknownPhrases.push_back( as );
      } else {
        Mission::conversations[ ks ] = as;
      }

    } else if( n == 'P' ) {    
      fgetc(fp);
      n = Constants::readLine(line, fp);
      currentNpc = Monster::getMonsterByName( line );
    } else if( n == 'V' && currentNpc ) {    
      fgetc(fp);
      n = Constants::readLine(line, fp);

      n = readConversationLine( fp, line, keyphrase, answer, n );

      string ks = keyphrase;
      string as = answer;

      NpcConversation *npcConv;
      if( Mission::npcConversations.find( currentNpc ) == Mission::npcConversations.end() ) {
        npcConv = new NpcConversation();
        Mission::npcConversations[ currentNpc ] = npcConv;
      } else {
        npcConv = Mission::npcConversations[ currentNpc ];
      }

      if( !strcmp( keyphrase, INTRO_PHRASE ) ) {
        npcConv->npc_intros.push_back( as );
      } else if( !strcmp( keyphrase, UNKNOWN_PHRASE ) ) {
        npcConv->npc_unknownPhrases.push_back( as );
      } else {
        npcConv->npc_conversations[ ks ] = as;
      }
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
        cerr << "Error: npc definition in " << filename << 
          " doesn't point to an npc. Line: " << line << endl;
      } else {
        ((Creature*)(pos->creature))->setNpcInfo( npcInfo );
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
    if( creature->getMonster()->isNpc() ) {
      total++;
      if( !getNpcInfo( toint( creature->getX() ), toint( creature->getY() ) ) ) {
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
      fprintf( fp, "G:_INTRO_,Welcome weary adventurer!.\n" );
    if( Mission::unknownPhrases.size() == 0 ) 
      fprintf( fp, "G:_UNKNOWN_,Uh, I don't know anything about that...\n" );

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
    char s[255];
    strcpy( s, subtype );
    char *p = strtok( s, ";" );
    while( p ) {
      if( this->type == Constants::NPC_TYPE_MERCHANT ) {
        // subtype is an RpgItem type
        this->subtype.insert( RpgItem::getTypeByName( p ) );
      } else if( this->type == Constants::NPC_TYPE_TRAINER ) {
        // subtype is a skill
        this->subtype.insert( Constants::getSkillByName( p ) );
      } else {
        break;
      }
      p = strtok( NULL, ";" );
    }
    delete subtype;
  }
}

NpcInfo::~NpcInfo() {
  delete name;
}

