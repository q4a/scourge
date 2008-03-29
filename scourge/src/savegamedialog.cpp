/***************************************************************************
  savegamedialog.cpp  -  description
-------------------
    begin                : 9/9/2005
    copyright            : (C) 2005 by Gabor Torok
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

#include "savegamedialog.h"
#include "scourge.h"
#include "shapepalette.h"
#include "persist.h"
#include "creature.h"
#include "rpg/character.h"
#include "io/file.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/label.h"
#include "gui/confirmdialog.h"
#include "gui/canvas.h"

bool savegamesChanged = true;
int maxFileSuffix = 0;

SavegameDialog::SavegameDialog( Scourge *scourge ) {
  this->scourge = scourge;
  int w = 600;
  int h = 400;
  win = 
    scourge->createWindow( scourge->getScreenWidth() / 2 - w / 2, 
													 scourge->getScreenHeight() / 2 - h / 2, 
                           w, h, 
                           _( "Saved games" ) );
	win->createLabel( 10, 20, _( "Current saved games:" ) );
  files = new ScrollingList( 10, 30, w - 130, h - 70, 
														 scourge->getHighlightTexture(), NULL, 70 );
	files->setTextLinewrap( true );
	files->setIconBorder( true );
  win->addWidget( files );

	newSave = win->createButton( w - 105, 30, w - 15, 50, _( "New Save" ) );
	save = win->createButton( w - 105, 60, w - 15, 80, _( "Save" ) );
	load = win->createButton( w - 105, 90, w - 15, 110, _( "Load" ) );
	deleteSave = win->createButton( w - 105, 150, w - 15, 170, _( "Delete" ) );
	cancel = win->createButton( w - 105, 210, w - 15, 230, _( "Cancel" ) );

	confirm = new ConfirmDialog( scourge->getSDLHandler(), _( "Overwrite existing file?" ) );
}

SavegameDialog::~SavegameDialog() {
	for( std::vector<GLuint>::iterator i = screens.begin(); i != screens.end(); i++ )
		glDeleteTextures( 1, &*i );
	delete win;
}

#define SAVE_MODE 1
#define LOAD_MODE 2
#define DELETE_MODE 3

void SavegameDialog::handleEvent( Widget *widget, SDL_Event *event ) {
	static int selectedFile;
	if( widget == confirm->okButton ) {
		confirm->setVisible( false );
		if( confirm->getMode() == SAVE_MODE ) {
			if( createSaveGame( fileInfos[ selectedFile ] ) ) {
				scourge->showMessageDialog( _( "Game saved successfully." ) );
			} else {
				scourge->showMessageDialog( _( "Error saving game." ) );
			}
		} else if( confirm->getMode() == DELETE_MODE ) {
			getWindow()->setVisible( false );
			savegamesChanged = true;
			string tmp = get_file_name( fileInfos[ selectedFile ]->path );
			if( deleteDirectory( tmp ) ) {
				scourge->showMessageDialog( _( "Game was successfully removed." ) );
			} else {
				scourge->showMessageDialog( _( "Could not delete saved game." ) );
			}
		} else {
			loadGame( selectedFile );
		}
	} else if( widget == confirm->cancelButton ) {
		confirm->setVisible( false );
	} else if( widget == cancel || widget == win->closeButton ) {
    win->setVisible( false );
  } else if( widget == save ) {
		int n = files->getSelectedLine();
		if( n > -1 ) {
			selectedFile = n;
			confirm->setText( _( "Are you sure you want to overwrite this file?" ) );
			confirm->setMode( SAVE_MODE );
			confirm->setVisible( true );
		}		
	} else if( widget == newSave ) {
		if( createNewSaveGame() ) {
			scourge->showMessageDialog( _( "Game saved successfully." ) );
		} else {
			scourge->showMessageDialog( _( "Error saving game." ) );
		}
	} else if( widget == load ) {
		int n = files->getSelectedLine();
		if( n > -1 ) {
			if( save->isEnabled() ) {
				selectedFile = n;
				confirm->setText( _( "Are you sure you want to load this file?" ) );
				confirm->setMode( LOAD_MODE );
				confirm->setVisible( true );
			} else {
				loadGame( n );
			}
		}
	} else if( widget == deleteSave ) {
		int n = files->getSelectedLine();
		if( n > -1 ) {
			if( fileInfos[ n ]->path == scourge->getSession()->getSavegameName() ) {
				scourge->showMessageDialog( _( "You can't delete the current game." ) );
			} else {
				selectedFile = n;
				confirm->setText( _( "Are you sure you want to delete this file?" ) );
				confirm->setMode( DELETE_MODE );
				confirm->setVisible( true );
			}
		}
	}
}

void SavegameDialog::loadGame( int n ) {
	getWindow()->setVisible( false );
	scourge->getSession()->setLoadgameName( fileInfos[n]->path );
	scourge->getSession()->setLoadgameTitle( fileInfos[n]->title );
	scourge->getSDLHandler()->endMainLoop();
}

void SavegameDialog::show( bool inSaveMode ) {
	if( inSaveMode ) {
		save->setEnabled( true );
		newSave->setEnabled( true );
		win->setTitle( _( "Create a new saved game or load an existing one" ) );
	} else {
		// check for save games
		save->setEnabled( false );
		newSave->setEnabled( false );
		win->setTitle( _( "Open an existing saved game file" ) );
	}
	if( savegamesChanged ) {
		if( !findFiles() ) {
			scourge->showMessageDialog( _( "No savegames have been created yet." ) );
			savegamesChanged = true; // check again next time
			return;
		}
	}
	win->setVisible( true );
}

bool SavegameDialog::findFiles() {
	fileInfos.clear();
	for( std::vector<GLuint>::iterator i = screens.begin(); i != screens.end(); i++ )
		glDeleteTextures( 1, &*i );
	screens.clear();
	filenames.clear();

	vector<string> fileNameList, realList;
	findFilesInDir( get_file_name( "" ), &fileNameList );

	maxFileSuffix = 0;
	for(vector<string>::reverse_iterator i = fileNameList.rbegin(); i != fileNameList.rend(); i++ ) {
		if( i->substr( 0, 5 ) == "save_" && readFileDetails( *i ) ) {
			filenames.push_back(fileInfos.back()->title);
			screens.push_back(loadScreenshot( fileInfos.back()->path ));
			cout << "last screen: " << screens.back() << endl;

			int n = static_cast<int>(strtol( i->c_str() + 5, (char**)NULL, 16 ));
			if( n > maxFileSuffix )
				maxFileSuffix = n;
		}
	}
	files->setLines( filenames.begin(), filenames.end(), NULL, &screens[0] );
	savegamesChanged = false;
	return( filenames.size() > 0 );
}

GLuint SavegameDialog::loadScreenshot( const string& dirName ) {
	string path = get_file_name( dirName + "/screen.bmp" );
	return Shapes::loadTextureWithAlpha( path, -1, -1, -1, true, false ); // -1 no alpha
}

bool SavegameDialog::readFileDetails( const string& dirname ) {
	string path = get_file_name( dirname + "/savegame.dat" );
	cerr << "Loading: " << path << endl;
	FILE *fp = fopen( path.c_str(), "rb" );
	if(!fp) {
		cerr << "*** error: Can't find file." << endl;
		return false;
	}
	File *file = new File( fp );
	Uint32 version = PERSIST_VERSION;
	file->read( &version );
	if( version < OLDEST_HANDLED_VERSION ) {
		cerr << "*** Error: Savegame file is too old (v" << version <<
			" vs. current v" << PERSIST_VERSION <<
			", vs. last handled v" << OLDEST_HANDLED_VERSION <<
			"): ignoring data in file." << endl;
		delete file;
		//strcpy( error, "Error: Saved game version is too old." );
		return false;
	} else {
		if( version < PERSIST_VERSION ) {
			cerr << "*** Warning: loading older savegame file: v" << version <<
				" vs. v" << PERSIST_VERSION << ". Will try to convert it." << endl;
		}
	}
	
	Uint8 title[3000];
	file->read( title, 3000 );

	delete file;

	SavegameInfo *info = new SavegameInfo();
	info->path = dirname;
	info->title = (char*)title;
	fileInfos.push_back( info );

	return true;
}
namespace { // anonymous local force 

typedef char POS_TXT[10];

void getPosition( int n, POS_TXT& buff ) {
	snprintf( buff, 10, "%d", n );
	switch( buff[ strlen( buff ) - 1 ] ) {
	case '1': strcat( buff, _("st") ); break;
	case '2': strcat( buff, _("nd") ); break;
	case '3': strcat( buff, _("rd") ); break;
	default: strcat( buff, _("th") );
	}
}

} // anonymous namespace

void SavegameDialog::setSavegameInfoTitle( SavegameInfo *info ) {
	Creature *player = scourge->getParty()->getParty( 0 );
	if( player ) {
		char tmp[10];
		getPosition( player->getLevel(), tmp );
		enum {PLACE_SIZE = 255};
		char place[PLACE_SIZE];

		if( scourge->getSession()->getCurrentMission() ) {
			if( strstr( scourge->getSession()->getCurrentMission()->getMapName(), "outdoors" ) ) {
				strcpy( place, _( "Somewhere in the wilderness." ) );
			} else if( strstr( scourge->getSession()->getCurrentMission()->getMapName(), "caves" ) ) {
				snprintf( place, PLACE_SIZE, _( "In a cave on level %d." ), 
								 ( scourge->getCurrentDepth() + 1 ) );
			} else {
				snprintf( place, PLACE_SIZE, _( "Dungeon level %d at %s." ), 
								 ( scourge->getCurrentDepth() + 1 ),
								 scourge->getSession()->getCurrentMission()->getMapName() );
			}
		} else {
			strcpy( place, _("Resting at HQ.") );
		}
		char tmp2[300];
		snprintf( tmp2, 300, _( "Party of %s the %s level %s." ), player->getName(), tmp, player->getCharacter()->getDisplayName() );
		info->title = scourge->getSession()->getParty()->getCalendar()->getCurrentDate().getDateString() + string(" ") + scourge->getSession()->getBoard()->getStorylineTitle() + string(", ") + tmp2 + string(" ") + place;
	} else {
		info->title = scourge->getSession()->getParty()->getCalendar()->getCurrentDate().getDateString() + string(" ") + scourge->getSession()->getBoard()->getStorylineTitle();
	}
}

bool SavegameDialog::createNewSaveGame() {

	// in case we're called from the outside
	if( savegamesChanged ) findFiles();

	// create a new save game
	SavegameInfo info;
	stringstream tmp;
	tmp << "save_" << std::hex << ++maxFileSuffix; // incr. maxFileSuffix in case we crash and the file is created
	info.path = tmp.str();
	setSavegameInfoTitle( &info );

	// make its directory
	makeDirectory( get_file_name( info.path ) );

	return saveGameInternal( &info );
}

void SavegameDialog::quickLoad() {
	scourge->getSession()->setLoadgameName( scourge->getSession()->getSavegameName() );
	scourge->getSession()->setLoadgameTitle( scourge->getSession()->getSavegameTitle() );
	scourge->getSDLHandler()->endMainLoop();
}

bool SavegameDialog::quickSave( const string& dirName, const string& title ) {
	SavegameInfo info;
	info.path = dirName;
	info.title = title;
	// create a new save game title
	setSavegameInfoTitle( &info );

	return saveGameInternal( &info );
}

bool SavegameDialog::createSaveGame( SavegameInfo *info ) {
	// create a new save game title
	setSavegameInfoTitle( info );

	return saveGameInternal( info );
}

bool SavegameDialog::saveGameInternal( SavegameInfo *info ) {
	// save the game here
	bool b = scourge->saveGame( scourge->getSession(), info->path, info->title );
	if( b ) {

		// if there is a current game and it's diff. than the one we're saving, copy the maps over
		string s(scourge->getSession()->getSavegameName());
		if( s == info->path && s != "" ) {
			b = copyMaps( s, info->path );
		}

		// pre-emptively save the current map (just in case it hasn't been done yet and we crash later
		if( scourge->getSession()->getCurrentMission() ) {
			scourge->saveCurrentMap( info->path );
		}

		// delete any unreferenced map files 
		// (these are either left when saving over an old game 
		// or completed and no longer on the board)
		deleteUnreferencedMaps( info->path );

		if( b ) {
	
			getWindow()->setVisible( false );
			saveScreenshot( info->path );
	
			// reload the next time
			savegamesChanged = true;
	
			// set it to be the current savegame
			scourge->getSession()->setSavegameName( info->path );
			scourge->getSession()->setSavegameTitle( info->title );
			
		}
	}
	return b;
}

void SavegameDialog::deleteUnvisitedMaps( const string& dirName, set<string> *visitedMaps ) {

	string path = get_file_name( dirName );
	vector<string> fileNameList;
	findFilesInDir( path, &fileNameList );
	for( vector<string>::iterator i = fileNameList.begin(); i < fileNameList.end(); i++ ) {
		if( (*i)[0] == '_' ) {
			if( visitedMaps->find( *i ) == visitedMaps->end() ) {
				string tmp = path + "/" + *i;
				cerr << "\tDeleting un-visited map file: " << tmp << endl;
				int n = remove( tmp.c_str() );
				cerr << "\t\t" << ( !n ? "success" : "can't delete file" ) << endl;
			}
		}
	}
}

void SavegameDialog::deleteUnreferencedMaps( const string& dirName ) {
	vector<string> referencedMaps;
	for( int i = 0; i < scourge->getSession()->getBoard()->getMissionCount(); i++ ) {
		string s = scourge->getSession()->getBoard()->getMission( i )->getSavedMapName();
		if( s != "" )
			referencedMaps.push_back( s );
	}

	string path = get_file_name( dirName );
	vector<string> fileNameList;
	findFilesInDir( path, &fileNameList );
	for( vector<string>::iterator i = fileNameList.begin(); i != fileNameList.end(); i++ ) {
		if( (*i)[0] == '_' ) {
			bool found = false;
			for( vector<string>::iterator t = referencedMaps.begin(); t != referencedMaps.end(); t++ ) {
				if( i->compare( 0, t->length(), *t ) == 0 ) {
					found = true;
					break;
				}
			}
			
			if( !found ) {
				string tmp = path + "/" + *i;
				cerr << "\tDeleting un-referenced map file: " << tmp << endl;
				int n = remove( tmp.c_str() );
				cerr << "\t\t" << ( !n ? "success" : "can't delete file" ) << endl;
			}
		}
	}
}

bool SavegameDialog::copyMaps( const string& fromDirName, const string& toDirName ) {
	vector<string> fileNameList;
	findFilesInDir( get_file_name( fromDirName ), &fileNameList );
	for( vector<string>::iterator i = fileNameList.begin(); i != fileNameList.end(); i++ ) {
		if( (*i)[0] == '_' )
			if( !copyFile( fromDirName, toDirName, *i ) )
				return false;
	}
	return true;
}

#define BUFFER_SIZE 4096
bool SavegameDialog::copyFile( const string& fromDirName, const string& toDirName, const string& fileName ) {
	string fromPath = get_file_name( fromDirName + "/" + fileName );
	string toPath = get_file_name( toDirName + "/" + fileName );

	cerr << "+++ copying file: " << fromPath << " to " << toPath << endl;

	FILE *from = fopen( fromPath.c_str(), "rb" );
	if( from ) {
		FILE *to = fopen( toPath.c_str(), "wb" );
		if( to ) {
			unsigned char buff[ BUFFER_SIZE ];
			size_t count;
			while( ( count = fread( buff, 1, BUFFER_SIZE, from ) ) ) fwrite( buff, 1, count, to );
			bool result = ( feof( from ) != 0 );
			fclose( to );
			return result;
		}
		fclose( from );
	}
	return false;
}

void SavegameDialog::saveScreenshot( const string& dirName ) {
	string path = get_file_name( dirName + "/screen.bmp");
	cerr << "Saving: " << path << endl;

	scourge->getSDLHandler()->saveScreenNow( path );
}

void SavegameDialog::makeDirectory( const string& path ) {
#ifdef WIN32
    CreateDirectory(path.c_str(), NULL);
#else
	int err = mkdir( path.c_str(), S_IRWXU|S_IRGRP|S_IXGRP );
	if(err) {
		cerr << "Error creating config directory: " << path << endl;
		cerr << "Error: " << err << endl;
		perror( "SavegameDialog::makeDirectory: " );
		exit(1);
	}
#endif
}

void SavegameDialog::findFilesInDir( const string& path, vector<string> *fileNameList ) {
#ifdef WIN32
	string winpath = path + "/*.*";

    WIN32_FIND_DATA FindData;
    HANDLE hFind = FindFirstFile (winpath.c_str(), &FindData);
    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "*** Error: can't open path: " << path << " error: " << GetLastError() << endl;
		return;
    }

    fileNameList->push_back( FindData.cFileName );
    while (FindNextFile (hFind, &FindData)) {
        fileNameList->push_back( FindData.cFileName );
    }
    FindClose (hFind);    
#else
	DIR *dir = opendir( path.c_str() );
	if( !dir ) {
		cerr << "*** Error: can't open path: " << path << " error: " << errno << endl;
		return;
	}
	struct dirent *de;
	while( ( de = readdir( dir ) ) ) {
		string s = de->d_name;
		fileNameList->push_back( s );
	}
	closedir( dir );
#endif
}

bool SavegameDialog::deleteDirectory( const string& path ) {
	vector<string> fileNameList;
	findFilesInDir( path, &fileNameList );
	for( vector<string>::iterator i = fileNameList.begin(); i != fileNameList.end(); i++ ) {
		string toDelete = path + "/" + *i;
		cerr << "\tDeleting file: " << toDelete << endl;
#ifdef WIN32
		int n = !DeleteFile(toDelete.c_str());
#else
		int n = remove( toDelete.c_str() );
#endif
		cerr << "\t\t" << ( !n ? "success" : "can't delete file" ) << endl;
	}
	cerr << "\tDeleting directory: " << path << endl;
#ifdef WIN32
	int n = !RemoveDirectory(path.c_str());
#else
	int n = remove( path.c_str() );
#endif
	cerr << "\t\t" << ( !n ? "success" : "can't delete directory" ) << endl;
	return( !n ? true : false );
}

bool SavegameDialog::checkIfFileExists( const string& filename ) {
	ifstream fin;
	fin.open(get_file_name( filename ).c_str());

	if( !fin ) 
		return false;

	return true;
}

