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
#include "item.h"
#include "creature.h"
#include "session.h"

vector<string> Mission::intros;
vector<string> Mission::unknownPhrases;
map<string, string> Mission::conversations;
map<Monster*,NpcConversation*> Mission::npcConversations;

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
  char name[255], line[255], description[2000], 
    success[2000], failure[2000], keyphrase[80],answer[4000];
  Monster *currentNpc = NULL;
  int n = fgetc(fp);
  while(n != EOF) {
    if( n == 'M' ) {
      // skip ':'
      fgetc( fp );
      n = Constants::readLine( name, fp );
      strcpy( description, "" );
      while( n == 'D' ) {
        n = Constants::readLine( line, fp );
        if( strlen( description ) ) strcat( description, " " );
        strcat( description, line + 1 );
      }
      templates.push_back( new MissionTemplate( this, name, description ) );
    } else if( n == 'T' ) {
      // skip ':'
      fgetc( fp );
      // read the name
      n = Constants::readLine( name, fp );
//      cerr << "Creating story mission: " << name << endl;

      // read the level and depth
      n = Constants::readLine( line, fp );
      int level = atoi( strtok( line + 1, "," ) );
      int depth = atoi( strtok( NULL, ",") );

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

      current_mission = new Mission( this, level, depth, name, description, success, failure );
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

    } else if(n == 'G') {
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

    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

int Board::readConversationLine( FILE *fp, char *line,
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

  // add the current storyline mission
  if( storylineMissions[ storylineIndex ]->getLevel() <= highest )
    availableMissions.push_back( storylineMissions[ storylineIndex ] );

  // maintain a set of missions
  while( availableMissions.size() < 5 ) {
    int level = (int)( ( ave + 2.0f ) * rand()/RAND_MAX ) - 4;
    if( level < 1 ) level = 1;
    int depth =  (int)((float)level / 7.0f ) + 1 + (int)( 3.0f * rand()/RAND_MAX );
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
        session->getGameAdapter()->setMissionDescriptionUI((char*)availableMissions[i]->getDescription());
      }
    }

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






MissionTemplate::MissionTemplate( Board *board, char *name, char *description ) {
  this->board = board;
  strcpy( this->name, name );
  strcpy( this->description, description );
}

MissionTemplate::~MissionTemplate() {
}

Mission *MissionTemplate::createMission( Session *session, int level, int depth ) {

//  cerr << "*** Creating level " << level << " mission, using template: " << this->name << endl;

  map<string, RpgItem*> items;
  map<string, Monster*> creatures;

  char parsedName[80];
  char parsedDescription[2000];
  char s[2000];
  strcpy( s, name );
  parseText( session, level, s, parsedName, &items, &creatures );
  strcpy( s, description );
  parseText( session, level, s, parsedDescription, &items, &creatures );
  
  Mission *mission = new Mission( board, 
								  level, depth, parsedName, parsedDescription, 
                                  "You have succeeded in your mission!", 
                                  "You have failed to complete your mission" );
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

void MissionTemplate::parseText( Session *session, int level, 
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
				  char *success, char *failure ) {
  this->board = board;
  this->level = level;
  this->depth = depth;
  strcpy( this->name, name );
  strcpy( this->description, description );
  strcpy( this->success, success );
  strcpy( this->failure, failure );
  this->completed = false;
  this->storyLine = false;

//  cerr << "*** Created mission: " << getName() << endl;
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

