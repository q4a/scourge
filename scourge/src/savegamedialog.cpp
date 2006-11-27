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

#define DEFAULT_SAVE_FILE "savegame.dat"

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
  files = new ScrollingList( 10, 30, w - 130, h - 100, 
														 scourge->getHighlightTexture() );
  win->addWidget( files );

	newSave = win->createButton( w - 105, h - 120, w - 15, h - 100, "New Save" );
	save = win->createButton( w - 105, h - 90, w - 15, h - 70, "Save" );
	load = win->createButton( w - 105, h - 90, w - 15, h - 70, "Load" );
	load->setVisible( false );
	cancel = win->createButton( w - 105, h - 60, w - 15, h - 40, "Cancel" );

	filenameCount = 0;
  filenames = (char**)malloc( MAX_SAVEGAME_COUNT * sizeof(char*) );
  for( int i = 0; i < MAX_SAVEGAME_COUNT; i++ ) {
    filenames[i] = (char*)malloc( 120 * sizeof(char) );
  }

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

	} else if( widget == newSave ) {
		createNewSaveGame();
	} else if( widget == load ) {


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
		save->setVisible( true );
		load->setVisible( false );
		win->setTitle( "Create a new saved game file" );
	} else {
		save->setVisible( false );
		load->setVisible( true );
		win->setTitle( "Open an existing saved game file" );
	}
	newSave->setVisible( save->isVisible() );
	findFiles();
	win->setVisible( true );
}

void SavegameDialog::findFiles() {
	for( unsigned int i = 0; i < fileInfos.size(); i++ ) {
		free( fileInfos[i] );
	}
	fileInfos.clear();

	filenameCount = 0;
	char path[300];
	get_file_name( path, 300, "index.txt" );
	FILE *fp = fopen( path, "r" );
	if( fp ) {
		char line[300];
		int lineWidth = 0;
		int n;
		while( ( n = fgetc( fp ) ) != EOF ) {
			if( n == '\n' || n == '\r' ) {
				if( lineWidth > 0 ) {
					line[lineWidth] = '\0';
					lineWidth = 0;
					if( readFileDetails( line ) ) {
						strcpy( filenames[filenameCount++], fileInfos[fileInfos.size() - 1]->title );
					}
				}
			} else {
				line[lineWidth++] = (char)n;
			}
		}
		fclose( fp );
	}
	files->setLines( filenameCount, (const char**)filenames );
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
	fileInfos.push_back( info );

	return true;
}

void SavegameDialog::createNewSaveGame() {
	// create a new save game
	SavegameInfo *info = (SavegameInfo*)malloc( sizeof( SavegameInfo ) );
	sprintf( (char*)info->path, "save_%x", (unsigned int)fileInfos.size() );
	sprintf( (char*)info->title, "%s %s", 
					 scourge->getSession()->getParty()->getCalendar()->getCurrentDate().getDateString(),
					 scourge->getSession()->getBoard()->getStorylineTitle() );
	fileInfos.push_back( info );

	// make its directory
	char path[300];
	get_file_name( path, 300, info->path );
	makeDirectory( path );

	// set it to be the current savegame
	scourge->getSession()->setSavegameName( info->path );

	// save the game here
	bool b = scourge->saveGame( scourge->getSession(), info->title );
	if( b ) {

		getWindow()->setVisible( false );
		saveScreenshot();

		// save the index file
		if( saveIndexFile() ) {

			// reload the list
			findFiles();
		
			scourge->showMessageDialog( "Game saved successfully." );
		} else {
			scourge->showMessageDialog( "Error saving index file." );
		}
	} else {
		scourge->showMessageDialog( "Error saving game." );
	}
}

//#define SCREEN_SHOT_WIDTH 320
//#define SCREEN_SHOT_HEIGHT 200

// fixme: make this faster
void SavegameDialog::saveScreenshot() {

	char tmp[300];
	sprintf( tmp, "%s/screen.bmp", scourge->getSession()->getSavegameName() );
	char path[300];
	get_file_name( path, 300, tmp );
	cerr << "Saving: " << path << endl;
	scourge->getSDLHandler()->saveScreen( path );
/*
	// glReadBuffer( GL_FRONT );
	GLubyte data[ SCREEN_SHOT_WIDTH * SCREEN_SHOT_HEIGHT * 3 ];
	glPushClientAttrib( GL_CLIENT_PIXEL_STORE_BIT );
	glPixelStorei( GL_PACK_ROW_LENGTH, 0 ) ;
	glPixelStorei( GL_PACK_ALIGNMENT, 1 ) ;
	glReadPixels( 0, 0, SCREEN_SHOT_WIDTH, SCREEN_SHOT_HEIGHT, 
								GL_RGB, GL_UNSIGNED_BYTE, data );
	glPopClientAttrib();
	
	Uint32 rmask, gmask, bmask, amask;

	// SDL interprets each pixel as a 32-bit number, so our masks must depend
	// on the endianness (byte order) of the machine
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif
	
	SDL_Surface *surface = 
		SDL_CreateRGBSurfaceFrom( data, 
															SCREEN_SHOT_WIDTH, 
															SCREEN_SHOT_HEIGHT, 
															24, 
															3 * SCREEN_SHOT_WIDTH, 
															rmask,
															gmask,
															bmask,
															amask );
	if( surface ) {
		char tmp[300];
		sprintf( tmp, "%s/screen.bmp", scourge->getSession()->getSavegameName() );
		char path[300];
    get_file_name( path, 300, tmp );
		cerr << "Saving: " << path << endl;
		SDL_SaveBMP( surface, path );
		SDL_FreeSurface( surface );
	} else {
		cerr << "*** Error: Couldn't create surface." << endl;
	}
	*/
}

bool SavegameDialog::saveIndexFile() {
	char path[300];
	get_file_name( path, 300, "index.txt" );
	FILE *fp = fopen( path, "w" );
	if( fp ) {
		for( unsigned int i = 0; i < fileInfos.size(); i++ ) {
			fprintf( fp, "%s\n", fileInfos[i]->path );
		}
		fclose( fp );
		return true;
	}
	return false;
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

