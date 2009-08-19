/***************************************************************************
                  board.cpp  -  Mission/Mission board classes
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

#include "common/constants.h"
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
#include "conversation.h"

using namespace std;
// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

map<string, NpcInfo*> Mission::npcInfos;  

//#define DEBUG_MODE 1

/**
  *@author Gabor Torok
  */

/// Sets up the mission board object.

/// This object contains all missions that are displayed on the board,
/// as well as all information required for the game to start them.

Board::Board( Session *session ) {
	this->session = session;
	this->storylineIndex = 0;
	
	initLocations();
	initMissions();
}

void Board::initMissions() {
	char name[255], displayName[255], line[255], description[2000], replayDisplayName[255], replayDescription[2000], music[255], success[2000], failure[2000], mapName[80], introDescription[2000], location[20];
	string ambientSoundName;

	ConfigLang *config = ConfigLang::load( string( "config/mission.cfg" ) );
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "sound" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Sounds" ), i, v->size() );

		string ambientName = node->getValueAsString( "name" );
		string ambientAmbient = node->getValueAsString( "ambient" );
		string ambientFootsteps = node->getValueAsString( "footsteps" );
		string afterFirstLevel = node->getValueAsString( "after_first_level" );
		session->getSound()->addAmbientSound( ambientName, ambientAmbient, ambientFootsteps, afterFirstLevel );
	}

	v = config->getDocument()->getChildrenByName( "mission" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Missions" ), i, v->size() );

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

		Mission *current_mission = new Mission( this, level, depth, replayable, name, displayName, description, replayDisplayName, replayDescription, introDescription, music, success, failure, mapName );
		current_mission->setStoryLine( true );
		current_mission->setChapter( chapter );
		current_mission->setAmbientSoundName( ambientSoundName );
		if ( strlen( location ) ) {
			int x, y;
			sscanf( location, "%d,%d", &x, &y );
			current_mission->setLocation( x, y );
		}
		storylineMissions.push_back( current_mission );


		vector<ConfigNode*> *vv = node->getChildrenByName( "item" );
		for ( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *itemNode = ( *vv )[i];
			RpgItem *item = RpgItem::getItemByName( itemNode->getValueAsString( "name" ) );
			current_mission->addItem( item );
		}
		vv = node->getChildrenByName( "creature" );
		for ( unsigned int i = 0; vv && i < vv->size(); i++ ) {
			ConfigNode *monsterNode = ( *vv )[i];
			Monster *monster = Monster::getMonsterByName( monsterNode->getValueAsString( "name" ) );
			if ( !monster ) {
				cerr << "*** Error: can't find mission monster \"" << line << "\"." << endl;
				exit( 1 );
			}
			current_mission->addCreature( monster );
		}

		if ( strlen( node->getValueAsString( "special" ) ) )
			current_mission->setSpecial( node->getValueAsString( "special" ) );
	}
	delete( config );	
}

void Board::walk( Road *road, int rx, int ry, int x, int y, bool walksX ) {
	//cerr << "+++ storing road=" << road->name << " region=" << rx << "," << ry << " pos=" << x << "," << y << " angle=" << angle << endl;
	char tmp[200];
	sprintf( tmp, "%d,%d", rx, ry );
	string key = tmp;
	set<Road*> *v;
	if( roads.find( key ) == roads.end() ) {
		v = new set<Road*>();
		roads[key] = v;
	} else {
		v = roads[key];
	}
	v->insert( road );
	allRoads.insert( road );
}

void Board::initLocations() {
	char tmp[200];
	ConfigLang *config = ConfigLang::load( "config/location.cfg" );
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "dungeon" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Locations" ), i, v->size() );
		
		MapPlace *place = new MapPlace();
		strcpy( place->name, node->getValueAsString( "name" ) );
		strcpy( place->display_name, node->getValueAsString( "display_name" ) );
		strcpy( place->map_name, node->getValueAsString( "map_name" ) );
		strcpy( place->short_name, node->getValueAsString( "short_name" ) );
		strcpy( tmp, node->getValueAsString( "region" ) );
		place->rx = atoi( strtok( tmp, "," ) );
		place->ry = atoi( strtok( NULL, "," ) );
		strcpy( tmp, node->getValueAsString( "location" ) );
		place->x = atoi( strtok( tmp, "," ) );
		place->y = atoi( strtok( NULL, "," ) );
		place->type = ( !strcmp( "dungeon", node->getValueAsString( "type" ) ) ? MapPlace::TYPE_DUNGEON : MapPlace::TYPE_CAVE );
		place->objective = 
			( !strcmp( "item", node->getValueAsString( "objective" ) ) ? MapPlace::OBJECTIVE_ITEM : 
				( !strcmp( "creature", node->getValueAsString( "objective" ) ) ? MapPlace::OBJECTIVE_CREATURE : 
					MapPlace::OBJECTIVE_NONE ) );
		place->objective_count = node->getValueAsInt( "objective_count" );
		place->level = node->getValueAsInt( "level" );
		place->depth = node->getValueAsInt( "depth" );
		strcpy( place->ambient, node->getValueAsString( "ambient" ) );
		strcpy( place->music, node->getValueAsString( "music" ) );
		strcpy( place->footsteps, node->getValueAsString( "footsteps" ) );
		strcpy( place->description, node->getValueAsString( "description" ) );
		
		sprintf( tmp, "%d,%d", place->rx, place->ry );
		string key = tmp;
		vector<MapPlace*> *v;
		if( places.find( key ) == places.end() ) {
			v = new vector<MapPlace*>();
			places[key] = v;
		} else {
			v = places[key];
		}
		v->push_back( place );
		
		string s = place->short_name;
		placesByShortName[ s ] = place;
	}
	
	v = config->getDocument()->getChildrenByName( "city" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Locations" ), i, v->size() );
		
		MapCity *city = new MapCity();
		strcpy( city->name, node->getValueAsString( "name" ) );
		strcpy( tmp, node->getValueAsString( "region" ) );
		city->rx = atoi( strtok( tmp, "," ) );
		city->ry = atoi( strtok( NULL, "," ) );
		strcpy( tmp, node->getValueAsString( "location" ) );
		city->x = atoi( strtok( tmp, "," ) );
		city->y = atoi( strtok( NULL, "," ) );
		strcpy( tmp, node->getValueAsString( "dimensions" ) );
		city->w = atoi( strtok( tmp, "," ) );
		city->h = atoi( strtok( NULL, "," ) );		
		city->level = node->getValueAsInt( "level" );
		
		// sanity check
		if( city->x + city->w * 4 * MAP_UNIT > MAP_WIDTH / 2 || 
				city->y + city->h * 4 * MAP_UNIT > MAP_DEPTH / 2 ||
				city->x < 0 || city->x > MAP_WIDTH / 2 ||
				city->y < 0 || city->y > MAP_DEPTH / 2 ) {
			cerr << "*** Error: city \"" << city->name << "\" won't fit in a region." << endl;
			delete city;
			continue;
		}
		
		sprintf( tmp, "%d,%d", city->rx, city->ry );
		string key = tmp;
		vector<MapCity*> *v;
		if( cities.find( key ) == cities.end() ) {
			v = new vector<MapCity*>();
			cities[key] = v;
		} else {
			v = cities[key];
		}
		v->push_back( city );
	}
	
	v = config->getDocument()->getChildrenByName( "generator" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Locations" ), i, v->size() );
		
		CreatureGenerator *generator = new CreatureGenerator();
		strcpy( generator->monster, node->getValueAsString( "monster" ) );
		strcpy( tmp, node->getValueAsString( "region" ) );
		generator->rx = atoi( strtok( tmp, "," ) );
		generator->ry = atoi( strtok( NULL, "," ) );
		strcpy( tmp, node->getValueAsString( "location" ) );
		generator->x = atoi( strtok( tmp, "," ) );
		generator->y = atoi( strtok( NULL, "," ) );
		generator->count = node->getValueAsInt( "count" );
		
		sprintf( tmp, "%d,%d", generator->rx, generator->ry );
		string key = tmp;
		vector<CreatureGenerator*> *v;
		if( generators.find( key ) == generators.end() ) {
			v = new vector<CreatureGenerator*>();
			generators[key] = v;
		} else {
			v = generators[key];
		}
		v->push_back( generator );
	}
	
	v = config->getDocument()->getChildrenByName( "road" );
	for ( unsigned int i = 0; i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		config->setUpdate( _( "Loading Locations" ), i, v->size() );
		
		Road *road = new Road();
		strcpy( road->name, node->getValueAsString( "name" ) );
		strcpy( road->display_name, node->getValueAsString( "display_name" ) );
		strcpy( tmp, node->getValueAsString( "start_region" ) );
		road->start_rx = atoi( strtok( tmp, "," ) );
		road->start_ry = atoi( strtok( NULL, "," ) );
		strcpy( tmp, node->getValueAsString( "start_location" ) );
		road->start_x = atoi( strtok( tmp, "," ) );
		road->start_y = atoi( strtok( NULL, "," ) );
		strcpy( tmp, node->getValueAsString( "end_region" ) );
		road->end_rx = atoi( strtok( tmp, "," ) );
		road->end_ry = atoi( strtok( NULL, "," ) );
		strcpy( tmp, node->getValueAsString( "end_location" ) );
		road->end_x = atoi( strtok( tmp, "," ) );
		road->end_y = atoi( strtok( NULL, "," ) );
		road->straight = node->getValueAsBool( "straight" );

		road->walk( this );
	}
	delete config;
}

Board::~Board() {
	reset();
	for ( map<string, vector<MapPlace*>* >::iterator e = places.begin(); e != places.end(); ++e ) {
		vector<MapPlace*> *v = e->second;
		for( unsigned int i = 0; i < v->size(); i++ ) {
			MapPlace *place = v->at( i );
			delete( place );
		}
		delete( v );
	}
	places.clear();
	placesByShortName.clear();
	for ( map<string, vector<MapCity*>* >::iterator e = cities.begin(); e != cities.end(); ++e ) {
		vector<MapCity*> *v = e->second;
		for( unsigned int i = 0; i < v->size(); i++ ) {
			MapCity *city = v->at( i );
			delete( city );
		}
		delete( v );
	}
	cities.clear();
	for( set<Road*>::iterator e = allRoads.begin(); e != allRoads.end(); ++e ) {
		Road *road = *e;
		delete( road );
	}
	roads.clear();	
	allRoads.clear();
	for ( size_t i = 0; i < storylineMissions.size(); ++i ) {
		delete storylineMissions[i];
	}
//	for ( size_t i = 0; i < templates.size(); ++i ) {
//		delete templates[i];
//	}
}

/// Resets the board, and the story line index.

void Board::reset() {
	for ( int i = 0; i < static_cast<int>( storylineMissions.size() ); i++ ) {
		Mission *mission = storylineMissions[i];
		mission->reset();
	}
	storylineIndex = 0;
}

/// Sets the storyline index.

void Board::setStorylineIndex( int n ) {
	if ( n < 0 )
		n = 0;
	if ( n > static_cast<int>( storylineMissions.size() ) )
		n = static_cast<int>( storylineMissions.size() );

	storylineIndex = n;
	for ( int i = 0; i < static_cast<int>( storylineMissions.size() ); i++ ) {
		storylineMissions[i]->setCompleted( i < storylineIndex ? true : false );
	}
}

/// Called when a storyline mission has been completed.

void Board::storylineMissionCompleted( Mission *mission ) {
	for ( int i = 0; i < static_cast<int>( storylineMissions.size() ); i++ ) {
		if ( storylineMissions[i] == mission && storylineIndex < ( i + 1 ) ) {
			storylineIndex = i + 1;
			break;
		}
	}
}

std::vector<CreatureGenerator*> *Board::getGeneratorsForRegion( int rx, int ry ) {
	std::vector<CreatureGenerator*> *v = NULL;
	char tmp[80];
	sprintf( tmp, "%d,%d", rx, ry );
	std::string key = tmp;
	if( generators.find( key ) != generators.end() ) {
		v = generators[key];
	} else {
		// if no generators were given (and there is no city) create some now
		v = new vector<CreatureGenerator*>();
		generators[key] = v;
		
		if( getCitiesForRegion( rx, ry ) == NULL ) {
			int count = Util::dice( 5 ); 
			for( int i = 0; i < count; i++ ) {
				CreatureGenerator *generator = new CreatureGenerator();
				generator->rx = rx;
				generator->ry = ry;
				generator->x = Util::pickOne( MAP_UNIT * 2, MAP_WIDTH / 2 - MAP_UNIT * 2 );
				generator->y = Util::pickOne( MAP_UNIT * 2, MAP_WIDTH / 2 - MAP_UNIT * 2 );
				generator->count = Util::pickOne( 1, 10 );
				strcpy( generator->monster, Monster::getRandomMonsterType( 1 ) );
				v->push_back( generator );
			}
		}
	}
	return v;
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

	// assign the map grid location
	if ( mapName && strlen( mapName ) ) {
		edited = true;
		string result;
		board->getSession()->getMap()->loadMapLocation( string( mapName ), result, &mapX, &mapY );
	} else {
		strcpy( this->mapName, "" );
	}
}

Mission::~Mission() {
	items.clear();
	itemList.clear();
	creatures.clear();
	creatureList.clear();
}

void Mission::setCompleted( bool b ) {
	completed = b;
	if( completed ) {
		// show mission completed in the ui
		board->getSession()->getGameAdapter()->refreshBackpackUI();
	}
}

bool Mission::itemFound( Item *item ) {
	if ( !completed ) {
		if ( item->getMissionId() == getMissionId() ) {
			RpgItem *rpgItem = itemList[ item->getMissionObjectiveIndex() ];
			items[ rpgItem ] = true;
			checkMissionCompleted();
			board->getSession()->getGameAdapter()->refreshBackpackUI();
			return isCompleted();
		}
	}
	return false;
}

bool Mission::creatureSlain( Creature *creature ) {
	if ( !completed ) {
		if ( creature->getMissionId() == getMissionId() ) {
			Monster *monster = creatureList[ creature->getMissionObjectiveIndex() ];
			creatures[ monster ] = true;
			checkMissionCompleted();
			board->getSession()->getGameAdapter()->refreshBackpackUI();
			return isCompleted();
		}
	}
	return false;
}

void Mission::checkMissionCompleted() {
	// special missions aren't completed when an item is found.
	if ( isSpecial() ) {
		completed = false;
		return;
	}
	if ( isReplay() ) {
		completed = false;
		return;
	}
	completed = true;
	for ( map<RpgItem*, bool >::iterator i = items.begin(); i != items.end(); ++i ) {
		bool b = i->second;
		if ( !b ) {
			completed = false;
			return;
		}
	}
	for ( map<Monster*, bool >::iterator i = creatures.begin(); i != creatures.end(); ++i ) {
		bool b = i->second;
		if ( !b ) {
			completed = false;
			return;
		}
	}
	board->getSession()->getGameAdapter()->refreshBackpackUI();
	if ( storyLine ) board->storylineMissionCompleted( this );
}

void Mission::reset() {
	completed = false;
	// is this ok? (below: to modify the table while iterating it...)
	for ( map<RpgItem*, bool >::iterator i = items.begin(); i != items.end(); ++i ) {
		RpgItem *item = i->first;
		items[ item ] = false;
	}
	for ( map<Monster*, bool >::iterator i = creatures.begin(); i != creatures.end(); ++i ) {
		Monster *monster = i->first;
		creatures[ monster ] = false;
	}
}

void Mission::removeMissionItems() {
	for ( int i = 0; i < board->getSession()->getParty()->getPartySize(); i++ ) {
		Creature *c = board->getSession()->getParty()->getParty( i );
		for ( int t = 0; t < c->getBackpackContentsCount(); t++ ) {
			if ( c->getBackpackItem( t )->getMissionId() == getMissionId() ) {
				c->removeFromBackpack( t );
			}
		}
	}
}

//TODO: this will segfault if filename starts with a .
void Mission::loadMapData( GameAdapter *adapter, const string& filename ) {
	// read general conversation from level 0.
	char dup[300];
	strcpy( dup, filename.c_str() );
	char *p = strrchr( dup, '.' );
	if ( p ) {
		char c = *( p - 1 );
		if ( c >= '1' && c <= '9' ) {
			char *q = p - 1;
			//cerr << "..." << endl;
			while ( true ) {
				//cerr << "\tq=" << q << endl;
				if ( q <= dup ) {
					q = NULL;
					break;
				} else if ( !( *q >= '1' && *q <= '9' ) ) {
					break;
				}
				q--;
			}
			if ( q ) {
				*( q + 1 ) = 0;
				strcat( dup, ".map" );
				string dup2( dup );
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
	//clearConversations();
	string path = rootDir + filename;
	loadMapDataFile( adapter, path );
}

void Mission::initNpcs( ConfigLang *config, GameAdapter *adapter ) {
	char line[1000];
	int x, y, level;
	char npcName[255], npcType[255], npcSubType[1000];

	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "npc" );
	for ( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

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
		if ( !( pos &&
		        pos->creature &&
		        ( Creature* )( pos->creature )->isNpc() ) ) {

			// the creature moved, try to find it by name
			bool found = false;
			for ( int i = 0; i < adapter->getSession()->getCreatureCount(); i++ ) {
				if ( !strcmp( npcInfo->name, adapter->getSession()->getCreature( i )->getName() ) ) {
					found = true;
					adapter->getSession()->getCreature( i )->setNpcInfo( npcInfo );
					break;
				}
			}
			if ( !found ) {
				cerr << "Error: npc definition." <<
				"Line: " << line << " npc=" << npcInfo->name <<
				" doesn't point to an npc." << endl;
				//} else {
				//cerr << "* found npc by name: " << npcInfo->name << endl;
			}
		} else {
			( ( Creature* )( pos->creature ) )->setNpcInfo( npcInfo );
		}
	}
}

void Mission::loadMapDataFile( GameAdapter *adapter, const string& filename, bool generalOnly ) {
	string tmp = getMapConfigFile( filename );
	ConfigLang *config = ConfigLang::load( tmp, true );
	if ( config ) {
		if ( !generalOnly ) initNpcs( config, adapter );
		//initConversations( config, adapter, generalOnly );
		delete config;
	}
}

NpcInfo *Mission::getNpcInfo( int x, int y ) {
	string key = getNpcInfoKey( x, y );
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

	if ( ".map" == s.substr( s.length() - 4, 4 ) ) {
		s = s.substr( 0, s.length() - 4 ) + ".cfg";
	} else {
		s.append( ".cfg" );
	}
	return s;
}

void Mission::saveMapData( GameAdapter *adapter, const string& filename ) {
	// append to txt file the new npc info
	string path = getMapConfigFile( filename );
	ConfigLang *config = ConfigLang::load( path, true );
	if ( !config ) {
		config = ConfigLang::fromString( "[map]\n[/map]\n" );
	}

	// Are there default conversation elements?
	int maxKey = 0;
	ConfigNode *general = NULL;
	vector<ConfigNode*> *v = config->getDocument()->
	                         getChildrenByName( "conversation" );
	for ( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];
		if ( !strcmp( node->getValueAsString( "name" ), "general" ) ) {
			general = node;
			break;
		}
		for ( map<std::string, ConfigValue*>::iterator e = node->getValues()->begin();
		        e != node->getValues()->end(); ++e ) {
			string key = e->first;
			// ConfigValue *value = e->second;
			if ( key == "name" ) {
			} else {
				string k = Conversation::decodeKeyValue( key );
				int n = atoi( k.c_str() );
				if ( n > maxKey ) maxKey = n;
			}
		}
	}
	enum { TMP_SIZE = 300 };
	char tmp[ TMP_SIZE ];
	if ( !general ) {
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
	for ( int i = 0; i < adapter->getSession()->getCreatureCount(); i++ ) {
		Creature *creature = adapter->getSession()->getCreature( i );
		if ( creature->getMonster() &&
		        creature->getMonster()->isNpc() ) {
			snprintf( tmp, TMP_SIZE, "%d,%d", toint( creature->getX() ),  toint( creature->getY() ) );
			string position = tmp;
			bool foundNpc = false;
			vector<ConfigNode*> *v = config->getDocument()->
			                         getChildrenByName( "npc" );
			for ( unsigned int t = 0; v && t < v->size(); t++ ) {
				ConfigNode *node = ( *v )[t];
				if ( node->getValueAsString( "position" ) == position ) {
					foundNpc = true;
					break;
				}
			}
			if ( !foundNpc ) {
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
	MissionInfo *info = new  MissionInfo;
	info->version = PERSIST_VERSION;
	strncpy( ( char* )info->templateName, ( isStoryLine() ? "storyline" : "template" ), 79 );
	info->templateName[79] = 0;
//	strcpy( ( char* )info->mapName, savedMapName.c_str() );
	strcpy( ( char* )info->mapName, getMapName() );
	info->level = getLevel();
	info->depth = getDepth();
	info->completed = ( completed ? 1 : 0 );
	info->itemCount = itemList.size();
	for ( int i = 0; i < static_cast<int>( itemList.size() ); i++ ) {
		strncpy( ( char* )info->itemName[i], itemList[i]->getName(), 254 );
		info->itemName[i][254] = '\0';
		info->itemDone[i] = ( items[ itemList[i] ] ? 1 : 0 );
	}
	info->monsterCount = creatureList.size();
	for ( int i = 0; i < static_cast<int>( creatureList.size() ); i++ ) {
		strncpy( ( char* )info->monsterName[i], creatureList[i]->getType(), 254 );
		info->monsterName[i][254] = '\0';
		info->monsterDone[i] = ( creatures[ creatureList[i] ] ? 1 : 0 );
	}
	info->missionId = getMissionId();
	return info;
}

Mission *Mission::load( Session *session, MissionInfo *info ) {
	// find the template
	Mission *mission;
	if ( !strcmp( ( char* )info->templateName, "storyline" ) ) {
		mission = session->getBoard()->getCurrentStorylineMission();
		cerr << "Loading storyling mission. chapter index=" << session->getBoard()->getStorylineIndex() << " - " << session->getBoard()->getStorylineTitle() << endl;
		mission->loadStorylineMission( info );
	} else {
		cerr << "loading mission for place " << info->mapName << endl;
		MapPlace *place = session->getBoard()->getMapPlaceByShortName( (char*)info->mapName );
		if( !place ) {
			cerr << "*** Error: cannot find map place by short name: " << (char*)(info->mapName) << endl;
			return NULL;
		}
		place->mission = NULL;
		mission = place->findOrCreateMission( session->getBoard(), info );
	}
	mission->setMissionId( info->missionId );
	return mission;
}

void Mission::loadStorylineMission( MissionInfo *info ) {
	cerr << "Loading storyline mission!" << endl;
	for ( int i = 0; i < static_cast<int>( info->itemCount ); i++ ) {
		char *p = ( char* )info->itemName[i];
		for ( unsigned int t = 0; t < itemList.size(); t++ ) {
			RpgItem *rpgItem = itemList[t];
			if ( !strcmp( rpgItem->getName(), p ) ) {
				items[rpgItem] = ( info->itemDone[i] != 0 );
				break;
			}
		}
	}
	for ( int i = 0; i < static_cast<int>( info->monsterCount ); i++ ) {
		char *p = ( char* )info->monsterName[i];
		for ( unsigned int t = 0; t < creatureList.size(); t++ ) {
			Monster *monster = creatureList[t];
			if ( !strcmp( monster->getType(), p ) ) {
				creatures[monster] = ( info->monsterDone[i] ? true : false );
				break;
			}
		}
	}
	//setSavedMapName( ( char* )info->mapName );
	setCompleted( info->completed ? true : false );
}

string outdoors_ambient_sound = "outdoors";
string& Mission::getAmbientSoundName() {
	return( !isStoryLine() && board->getSession()->getGameAdapter()->getCurrentDepth() == 0 ? outdoors_ambient_sound : ambientSoundName );
}

NpcInfo *Mission::addNpcInfo( int x, int y, char *npcName, int level, char *npcType, char *npcSubType ) {
	string key = getNpcInfoKey( x, y );
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
	if ( npcType == Constants::NPC_TYPE_MERCHANT ) {
		// fixme: trade-able should be an attribute to itemType in item.cfg
		int n = Util::dice( 5 );
		switch ( n ) {
		case 0: strcpy( npcSubType, "POTION;WAND;RING;AMULET;STAFF" ); strcpy( npcTypeName, _( "Magic Merchant" ) ); break;
		case 1: strcpy( npcSubType, "ARMOR" ); strcpy( npcTypeName, _( "Armor Merchant" ) ); break;
		case 2: strcpy( npcSubType, "FOOD;DRINK" ); strcpy( npcTypeName, _( "Rations Merchant" ) ); break;
		case 3: strcpy( npcSubType, "SCROLL" ); strcpy( npcTypeName, _( "Scrolls Merchant" ) ); break;
		case 4: strcpy( npcSubType, "SWORD;AXE;BOW;MACE;POLE" ); strcpy( npcTypeName, _( "Weapons Merchant" ) ); break;
		}
	} else if ( npcType == Constants::NPC_TYPE_TRAINER ) {
		Character *character = Character::getRandomCharacter();
		strcpy( npcSubType, character->getDisplayName() );
		snprintf( npcTypeName, NAME_LEN, _( "Trainer for %s" ), character->getDisplayName() );
	} else if ( npcType == Constants::NPC_TYPE_HEALER ) {
		strcpy( npcSubType, MagicSchool::getRandomSchool()->getName() );
	}
	char name[NAME_LEN+1] = {0};
	snprintf( name, NAME_LEN, _( "%s the %s" ), Rpg::createName().c_str(), npcTypeName );
	// don't add it to the mission, just create the objects. They're added later in scourge.cpp.
	NpcInfo *npcInfo = new NpcInfo( fx, fy, name, level, ( char* )Constants::npcTypeName[ npcType ], npcSubType );
	creature->setNpcInfo( npcInfo );
}

NpcInfo::NpcInfo( int x, int y, char *name, int level, char *type, char *subtype ) {
	this->x = x;
	this->y = y;
	strcpy( this->name, name );
	this->level = level;
	this->type = -1;
	for ( int i = 0; i < Constants::NPC_TYPE_COUNT; i++ ) {
		if ( !strcmp( type, Constants::npcTypeName[ i ] ) ) {
			this->type = i;
			break;
		}
	}
	if ( this->type == -1 ) {
		cerr << "Error: npc type " << type << " is not known. Setting it to commoner." << endl;
		this->type = 0;
	}

	if ( subtype ) {
		// store as a string
		strcpy( this->subtypeStr, subtype );

		// parse for some npc types
		char s[255];
		strcpy( s, subtype );
		char *p = strtok( s, ";" );
		while ( p ) {
			if ( this->type == Constants::NPC_TYPE_MERCHANT ) {
				// subtype is an RpgItem type
				this->subtype.insert( RpgItem::getTypeByName( p ) );
			} else if ( this->type == Constants::NPC_TYPE_TRAINER ) {
				// subtype is a root profession
				this->subtype.insert( Characters::getRootIndexByName( p ) );
			} else {
				break;
			}
			p = strtok( NULL, ";" );
		}
	} else {
		strcpy( this->subtypeStr, "" );
	}
}

NpcInfo::~NpcInfo() {
}

NpcInfoInfo *NpcInfo::save() {
	NpcInfoInfo *info = new NpcInfoInfo;
	info->x = x;
	info->y = y;
	strcpy( ( char* )info->name, name );
	info->level = level;
	info->type = type;
	strcpy( ( char* )info->subtype, ( subtypeStr ? subtypeStr : "" ) );
	return info;
}

NpcInfo *NpcInfo::load( NpcInfoInfo *info ) {
	char *s = ( char* )info->subtype;
	return new NpcInfo( info->x, info->y, ( char* )info->name, info->level, ( char* )Constants::npcTypeName[info->type], strlen( s ) > 0 ? s : NULL );
}

MapPlace::MapPlace() {
	mission = NULL;
}

MapPlace::~MapPlace() {
	if( mission ) {
		delete( mission );
	}
}

Mission *MapPlace::findOrCreateMission( Board *board, MissionInfo *info ) {
	cerr << "\tMapPlace::findOrCreateMission for place=" << name << endl;
	if( !mission ) {
		cerr << "\t\tcreating mission: level=" << level << " depth=" << depth << endl;
		// add one to depth b/c now the outdoors is not part of the depth
		mission = new Mission( board, level, depth + 1, false, name, display_name, description, NULL, NULL, "", music, "", "", short_name );
		
		// set objectives
		RpgItem *item;
		Monster *monster;
		bool found;
		for( int i = 0; i < objective_count; i++ ) {
			if( objective == MapPlace::OBJECTIVE_ITEM ) {
				if( info ) {
					item = RpgItem::getItemByName( (char*)info->itemName[ i ] );
					found = info->itemDone[ i ] == 1;
					cerr << "\t\tadding item " << (char*)info->itemName[ i ] << " item=" << item->getName() << endl;
				} else {
					item = RpgItem::getRandomItem( 1 );
					found = false;
					cerr << "\t\tadding random item " << item->getName() << endl;
				}
				mission->addItem( item, found );
			} else if( objective == MapPlace::OBJECTIVE_CREATURE ) {
				if( info ) {
					monster = Monster::getMonsterByName( (char*)info->monsterName[ i ] );
					found = info->monsterDone[ i ] == 1;
					cerr << "\t\tadding monster " << (char*)info->monsterName[ i ] << " monster=" << monster->getType() << endl;
				} else {
					monster = Monster::getRandomMonster( level );
					found = false;
					cerr << "\t\tadding random monster " << monster->getType() << endl;
				}
				mission->addCreature( monster, found );
			}
		}
		
		if( info ) {
			mission->setCompleted( info->completed );
			mission->setMissionId( info->missionId );
		}
		
		//mission->setAmbientSoundName( this->ambientSoundName );
	} else {
		cerr << "\t\tmission already exists. map=" << mission->getMapName() << endl;
	}
	cerr << "\t\tmission id=" << mission->getMissionId() << endl;
	return mission;
}

void CreatureGenerator::removeDead() {
	// remove the dead creatures
	for( unsigned int i = 0; i < creatures.size(); i++ ) {
		if ( creatures[i]->getStateMod( StateMod::dead ) ) {
			creatures.erase( creatures.begin() + i );
			i--;
		}
	}
}

void CreatureGenerator::generate( Session *session ) {
	removeDead();
	
	// generate new ones
	int n = this->count - creatures.size();
	for( int i = 0; i < n; i++ ) {
		// add monsters with generator to clone themselves upon their demise
		Monster *monster = Monster::getMonsterByName( this->monster );
		if ( !monster ) {
			cerr << "Warning: can't find monster for generator: " << monster << endl;
			break;
		}
		GLShape *shape = session->getShapePalette()->getCreatureShape( monster->getModelName(), 
		                                                               monster->getSkinName(), 
		                                                               monster->getScale(),
		                                                               monster );
		Creature *creature = session->newCreature( monster, shape );
		creature->findPlaceBoundedRadial( this->x, this->y, MAP_UNIT );
		creature->startEffect( Constants::EFFECT_TELEPORT, Constants::DAMAGE_DURATION * 4 );
		session->registerWithSquirrel( creature );
		creatures.push_back( creature );
	}
}

void Road::walk( RoadWalker *walker ) {
	cerr << "+++ Road: " << name << 
		" START region: " << start_rx << "," << start_ry << " pos: " << start_x << "," << start_y <<
		" END region: " << end_rx << "," << end_ry << " pos: " << end_x << "," << end_y << endl;
	
	int region_width_in_units = MAP_WIDTH / 2 / MAP_UNIT;
	int region_depth_in_units = MAP_DEPTH / 2 / MAP_UNIT;
	int abs_start_x = start_rx * region_width_in_units + start_x / MAP_UNIT;
	int abs_start_y = start_ry * region_depth_in_units + start_y / MAP_UNIT;
	int abs_end_x = end_rx * region_width_in_units + end_x / MAP_UNIT;
	int abs_end_y = end_ry * region_depth_in_units + end_y / MAP_UNIT;
	
	cerr << "+++ abs START=" << abs_start_x << "," << abs_start_y <<
	" END=" << abs_end_x << "," << abs_end_y << endl;
	
	// store road sections for each region
	float ydiff = (float)( abs_start_y - abs_end_y );
	float xdiff = (float)( abs_start_x - abs_end_x );
	cerr << "+++ xdiff=" << xdiff << " ydiff=" << ydiff << endl;
	if( ydiff == 0 ) {
		cerr << "+++ moving horizontally on x axis" << endl;
		if( abs_start_x > abs_end_x ) {
			int tmp_n = abs_start_x;
			abs_start_x = abs_end_x;
			abs_end_x = tmp_n;
		}
		for( int x = abs_start_x; x <= abs_end_x; x++ ) {
			walker->walk( this, 
			             (int)(x / (float)region_width_in_units), (int)(abs_start_y / (float)region_depth_in_units),
			             (int)(x % region_width_in_units) * MAP_UNIT, (int)(abs_start_y % region_depth_in_units) * MAP_UNIT, 
			             true );
		}
	} else if( xdiff == 0 ) {
		cerr << "+++ moving vertically on y axis" << endl;
		if( abs_start_y > abs_end_y ) {
			int tmp_n = abs_start_y;
			abs_start_y = abs_end_y;
			abs_end_y = tmp_n;
		}
		for( int y = abs_start_y; y <= abs_end_y; y++ ) {
			walker->walk( this, 
			             (int)(abs_start_x / (float)region_width_in_units), (int)(y / (float)region_depth_in_units), 
			             (int)(abs_start_x % region_width_in_units) * MAP_UNIT, (int)(y % region_depth_in_units) * MAP_UNIT,
			             false );
		}
	} else {
		if( fabs( ydiff ) < fabs( xdiff ) ) {
			float m = ydiff / xdiff;
			float ang = atan( m ) * ( 180.0 / PI );
			if( ang < 0 ) ang += 360;
			if( ang >= 360 ) ang -= 360;
			cerr << "+++ moving on x axis delta y is " << m << endl;
			if( abs_start_x > abs_end_x ) {
				int tmp_n = abs_start_x;
				abs_start_x = abs_end_x;
				abs_end_x = tmp_n;
				tmp_n = abs_start_y;
				abs_start_y = abs_end_y;
				abs_end_y = tmp_n;
			}
			
			float y = abs_start_y;
			for( int x = abs_start_x; x <= abs_end_x; x++ ) {
				walker->walk( this, 
				           (int)(x / (float)region_width_in_units), (int)(y / (float)region_depth_in_units),
				           ( x % region_width_in_units )  * MAP_UNIT, ( (int)y % region_depth_in_units ) * MAP_UNIT,
				           true );
				y += m; 
			}
		} else {
			float m = xdiff / ydiff;
			float ang = atan( m ) * ( 180.0 / PI );
			if( ang < 0 ) ang += 360;
			if( ang >= 360 ) ang -= 360;
			cerr << "+++ moving on y axis delta x is " << m << endl;
			if( abs_start_y > abs_end_y ) {
				int tmp_n = abs_start_x;
				abs_start_x = abs_end_x;
				abs_end_x = tmp_n;
				tmp_n = abs_start_y;
				abs_start_y = abs_end_y;
				abs_end_y = tmp_n;
			}
			float x = abs_start_x;
			for( int y = abs_start_y; y <= abs_end_y; y++ ) {
				walker->walk( this, 
				           (int)(x / (float)region_width_in_units), (int)(y / (float)region_depth_in_units),
				           ( (int)x % region_width_in_units ) * MAP_UNIT, ( y % region_depth_in_units ) * MAP_UNIT,
				           false );
				x += m; 
			}				
		}
	}
}
