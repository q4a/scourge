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
                           "Saved games" );
	win->createLabel( 10, 20, "Current saved games:" );
  files = new ScrollingList( 10, 30, w - 130, h - 70, 
														 scourge->getHighlightTexture(), NULL, 70 );
	files->setTextLinewrap( true );
	files->setIconBorder( true );
  win->addWidget( files );

	newSave = win->createButton( w - 105, 30, w - 15, 50, "New Save" );
	save = win->createButton( w - 105, 60, w - 15, 80, "Save" );
	load = win->createButton( w - 105, 90, w - 15, 110, "Load" );
	cancel = win->createButton( w - 105, 120, w - 15, 140, "Cancel" );

	filenameCount = 0;
  filenames = (char**)malloc( MAX_SAVEGAME_COUNT * sizeof(char*) );
  for( int i = 0; i < MAX_SAVEGAME_COUNT; i++ ) {
    filenames[i] = (char*)malloc( 120 * sizeof(char) );
  }
	this->screens = (GLuint*)malloc( MAX_SAVEGAME_COUNT * sizeof( GLuint ) );

	confirm = new ConfirmDialog( scourge->getSDLHandler(), 
															 "Overwrite existing file?" );
}

SavegameDialog::~SavegameDialog() {
	delete win;
}

void SavegameDialog::handleEvent( Widget *widget, SDL_Event *event ) {
	if( widget == confirm->okButton ) {
		confirm->setVisible( false );
		//setName();
		win->setVisible( false );
		//scourge->getMainMenu()->setSavegameSelected();
	} else if( widget == confirm->cancelButton ) {
		confirm->setVisible( false );
	} else if( widget == cancel || widget == win->closeButton ) {
		//strcpy( selectedName, "" );
    win->setVisible( false );
		//scourge->getMainMenu()->setSavegameSelected();
  } else if( widget == save ) {
		int n = files->getSelectedLine();
		if( n > -1 && createSaveGame( fileInfos[n] ) ) {
			scourge->showMessageDialog( "Game saved successfully." );
		} else {
			scourge->showMessageDialog( "Error saving game." );
		}
	} else if( widget == newSave ) {
		if( createNewSaveGame() ) {
			scourge->showMessageDialog( "Game saved successfully." );
		} else {
			scourge->showMessageDialog( "Error saving game." );
		}
	} else if( widget == load ) {
		int n = files->getSelectedLine();
		if( n > -1 ) {
			getWindow()->setVisible( false );
			scourge->getSession()->setLoadgameName( fileInfos[n]->path );
			scourge->getSDLHandler()->endMainLoop();
		}

		/*
		setName();
		bool exists = checkIfFileExists( name->getText() );
		if( widget == save && exists ) {
			confirm->setText( "Are you sure you want to overwrite this file?" );
			confirm->setVisible( true );
		} else if( widget == load && !exists ) {
			scourge->showMessageDialog( "Please select an existing file." );
		} else {
			win->setVisible( false );
			scourge->getMainMenu()->setSavegameSelected();
		}
		*/
  } else if( widget == files ) {
		/*
    int line = files->getSelectedLine();
    if( line > -1 ) {
			name->setText( filenames[ line ] );
		}
		*/
	}
}

void SavegameDialog::show( bool inSaveMode ) {
	if( inSaveMode ) {
		save->setEnabled( true );
		newSave->setEnabled( true );
		win->setTitle( "Create a new saved game or load an existing one" );
	} else {
		save->setEnabled( false );
		newSave->setEnabled( false );
		win->setTitle( "Open an existing saved game file" );
	}
	if( savegamesChanged ) findFiles();
	win->setVisible( true );
}

void SavegameDialog::findFiles() {
	for( unsigned int i = 0; i < fileInfos.size(); i++ ) {
		free( fileInfos[i] );
	}
	fileInfos.clear();

	for( int i = 0; i < filenameCount; i++ ) {
		if( screens[ i ] ) glDeleteTextures( 1, &(screens[ i ]) );
	}

	char path[300];
	get_file_name( path, 300, "" );
	vector<string> fileNameList;
	findFilesInDir( path, &fileNameList );

	maxFileSuffix = 0;
	filenameCount = 0;
	for( unsigned int i = 0; i < fileNameList.size(); i++ ) {
		string s = fileNameList[ i ];
		if( s.substr( 0, 5 ) == "save_" && readFileDetails( (char*)s.c_str() ) ) {
			// add in reverse order
			if( filenameCount > 0 ) {
				for( int i = filenameCount; i > 0; i-- ) {
					strcpy( filenames[ i ], filenames[ i - 1 ] );
					screens[ i ] = screens[ i - 1 ];
				}
			}
			filenameCount++;
			strcpy( filenames[ 0 ], fileInfos[ 0 ]->title );
			screens[ 0 ] = loadScreenshot( fileInfos[ 0 ]->path );						
			int n = (int)strtol( s.c_str() + 5, (char**)NULL, 16 );
			if( n > maxFileSuffix ) maxFileSuffix = n;
		}
	}
	files->setLines( filenameCount, (const char**)filenames, NULL, screens );
	savegamesChanged = false;
}

GLuint SavegameDialog::loadScreenshot( char *dirName ) {
	char tmp[300];
	sprintf( tmp, "%s/screen.bmp", dirName );
	char path[300];
	get_file_name( path, 300, tmp );
	//cerr << "Loading: " << path << endl;
	GLuint n = Shapes::loadTextureWithAlpha( path, -1, -1, -1, true, false ); // -1 no alpha
	//cerr << "\tid=" << n << endl;
	return n;
}

bool SavegameDialog::readFileDetails( char *dirname ) {
	char tmp[300];
	sprintf( tmp, "%s/savegame.dat", dirname );
	char path[300];
	get_file_name( path, 300, tmp );
	cerr << "Loading: " << path << endl;
	FILE *fp = fopen( path, "rb" );
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
	
	Uint8 title[255];
	file->read( title, 255 );
  
	delete file;

	SavegameInfo *info = (SavegameInfo*)malloc( sizeof( SavegameInfo ) );
	strcpy( info->path, dirname );
	strcpy( info->title, (char*)title );
	fileInfos.insert( fileInfos.begin(), info );

	return true;
}

bool SavegameDialog::createNewSaveGame() {

	// in case we're called from the outside
	if( savegamesChanged ) findFiles();

	// create a new save game
	SavegameInfo info;
	sprintf( (char*)info.path, "save_%x", ++maxFileSuffix ); // incr. maxFileSuffix in case we crash and the file is created
	sprintf( (char*)info.title, "%s %s", 
					 scourge->getSession()->getParty()->getCalendar()->getCurrentDate().getDateString(),
					 scourge->getSession()->getBoard()->getStorylineTitle() );

	// make its directory
	char path[300];
	get_file_name( path, 300, info.path );
	makeDirectory( path );

	return saveGameInternal( &info );
}

bool SavegameDialog::createSaveGame( SavegameInfo *info ) {
	// create a new save game title
	sprintf( (char*)info->title, "%s %s", 
					 scourge->getSession()->getParty()->getCalendar()->getCurrentDate().getDateString(),
					 scourge->getSession()->getBoard()->getStorylineTitle() );

	// its directory
	char path[300];
	get_file_name( path, 300, info->path );

	return saveGameInternal( info );
}

bool SavegameDialog::saveGameInternal( SavegameInfo *info ) {
	// save the game here
	bool b = scourge->saveGame( scourge->getSession(), info->path, info->title );
	if( b ) {

		// if there is a current game and it's diff. than the one we're saving, copy the maps over
		if( strcmp( scourge->getSession()->getSavegameName(), info->path ) && 
				strlen( scourge->getSession()->getSavegameName() ) ) {
			b = copyMaps( scourge->getSession()->getSavegameName(), info->path );
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
			
		}
	}
	return b;
}

void SavegameDialog::deleteUnreferencedMaps( char *dirName ) {
	//cerr << "Deleting unreferenced maps:" << endl;
	vector<string> referencedMaps;
	for( int i = 0; i < scourge->getSession()->getBoard()->getMissionCount(); i++ ) {
		string s = scourge->getSession()->getBoard()->getMission( i )->getSavedMapName();
		if( s != "" ) referencedMaps.push_back( s );
	}
	//cerr << "\tstarting" << endl;

	char path[300];
	get_file_name( path, 300, dirName );
	vector<string> fileNameList;
	findFilesInDir( path, &fileNameList );
	char tmp[300];
	for( unsigned int i = 0; i < fileNameList.size(); i++ ) {
		string s = fileNameList[i];
		//cerr << "\tconsidering: " << s << endl;
		if( s.substr( 0, 1 ) == "_" ) {
			
			bool found = false;
			for( unsigned int t = 0; t < referencedMaps.size(); t++ ) {
				string z = referencedMaps[t];
				if( s.substr( 0, z.length() ) == z ) {
					found = true;
					break;
				}
			}
			//cerr << "\tfound: " << found << endl;
			
			if( !found ) {
				sprintf( tmp, "%s/%s", path, s.c_str() );
				cerr << "\tDeleting un-referenced map file: " << tmp << endl;
				int n = remove( tmp );
				cerr << "\t\t" << ( !n ? "success" : "can't delete file" ) << endl;
			}
		}
	}
	//cerr << "\tDone." << endl;
}

bool SavegameDialog::copyMaps( char *fromDirName, char *toDirName ) {
	char path[300];
	get_file_name( path, 300, fromDirName );
	vector<string> fileNameList;
	findFilesInDir( path, &fileNameList );
	for( unsigned int i = 0; i < fileNameList.size(); i++ ) {
		string s = fileNameList[i];
		if( s.substr( 0, 1 ) == "_" ) {
			if( !copyFile( fromDirName, toDirName, (char*)s.c_str() ) ) return false;
		}
	}
	return true;
}

#define BUFFER_SIZE 4096
bool SavegameDialog::copyFile( char *fromDirName, char *toDirName, char *fileName ) {
	char tmp[300];
	
	sprintf( tmp, "%s/%s", fromDirName, fileName );
	char fromPath[300];
	get_file_name( fromPath, 300, tmp );
	
	sprintf( tmp, "%s/%s", toDirName, fileName );
	char toPath[300];
	get_file_name( toPath, 300, tmp );

	cerr << "+++ copying file: " << fromPath << " to " << toPath << endl;

	FILE *from = fopen( fromPath, "rb" );
	if( from ) {
		FILE *to = fopen( toPath, "wb" );
		if( to ) {
			unsigned char buff[ BUFFER_SIZE ];
			size_t count;
			while( ( count = fread( buff, 1, BUFFER_SIZE, from ) ) ) fwrite( buff, 1, count, to );
			bool result = feof( from );
			fclose( to );
			return result;
		}
		fclose( from );
	}
	return false;
}

void SavegameDialog::saveScreenshot( char *dirName ) {
	char tmp[300];
	sprintf( tmp, "%s/screen.bmp", dirName );
	char path[300];
	get_file_name( path, 300, tmp );
	cerr << "Saving: " << path << endl;

	scourge->getSDLHandler()->saveScreenNow( path );
}

void SavegameDialog::makeDirectory( char *path ) {
#ifdef WIN32
	cerr << "*** IMPLEMENT ME: SavegameDialog::makeDirectory() for windows." << endl;
	exit(1);
#else
	int err = mkdir( path, S_IRWXU|S_IRGRP|S_IXGRP );
	if(err) {
		cerr << "Error creating config directory: " << path << endl;
		cerr << "Error: " << err << endl;
		perror( "SavegameDialog::makeDirectory: " );
		exit(1);
	}
#endif
}

void SavegameDialog::findFilesInDir( char *path, vector<string> *fileNameList ) {
#ifdef WIN32
	cerr << "*** IMPLEMENT ME: SavegameDialog::findFilesInDir() for windows." << endl;
	exit(1);
#else
	DIR *dir = opendir( path );
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

bool SavegameDialog::checkIfFileExists( char *filename ) {
	char path[300];
	get_file_name( path, 300, filename );
	FILE *fp = fopen( path, "r" );
	if( fp ) {
		fclose( fp );
		return true;
	} else {
		return false;
	}
}

