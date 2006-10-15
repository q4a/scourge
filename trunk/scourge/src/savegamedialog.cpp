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
#include "gui/window.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/textfield.h"
#include "gui/label.h"
#include "gui/confirmdialog.h"

#define DEFAULT_SAVE_FILE "savegame.dat"

SavegameDialog::SavegameDialog( Scourge *scourge ) {
  this->scourge = scourge;
  int w = 350;
  int h = 300;
  win = 
    scourge->createWindow( scourge->getScreenWidth() / 2 - w / 2, 
													 scourge->getScreenHeight() / 2 - h / 2, 
                           w, h, 
                           "Saved games" );
	win->createLabel( 10, 10, "Name of saved game:" );
	name = new TextField( 10, 20, 35 );
	win->addWidget( name );

	win->createLabel( 10, 50, "Current saved games:" );
  files = new ScrollingList( 10, 60, w - 130, h - 100, 
														 scourge->getHighlightTexture() );
  win->addWidget( files );

	save = win->createButton( w - 100, 10, w - 10, 30, "Save" );
	load = win->createButton( w - 100, 10, w - 10, 30, "Load" );
	load->setVisible( false );
	cancel = win->createButton( w - 100, 40, w - 10, 60, "Cancel" );

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
		setName();
		win->setVisible( false );
		scourge->getMainMenu()->setSavegameSelected();
	} else if( widget == confirm->cancelButton ) {
		confirm->setVisible( false );
	} else if( widget == cancel || widget == win->closeButton ) {
		strcpy( selectedName, "" );
    win->setVisible( false );
		scourge->getMainMenu()->setSavegameSelected();
  } else if( widget == save || widget == load ) {
		setName();
		if( widget == save && checkIfFileExists( name->getText() ) ) {
			confirm->setText( "Are you sure you want to overwrite this file?" );
			confirm->setVisible( true );
		} else {
			win->setVisible( false );
			scourge->getMainMenu()->setSavegameSelected();
		}
  } else if( widget == files ) {
    int line = files->getSelectedLine();
    if( line > -1 ) {
			name->setText( filenames[ line ] );
		}
	}
}

void SavegameDialog::setName() {
	char s[255];
	strcpy( s, name->getText() );
	if( strlen( s ) ) {
		// add .dat to end if needed
		char *p = strstr( s, ".dat" );
		if( !p || *( p + 4) ) {
			sprintf( selectedName, "%s.dat", s );
		} else {
			strcpy( selectedName, s ); 
		}
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
	findFiles();
	win->setVisible( true );
}

// will this work on windows?
void SavegameDialog::findFiles() {
	char path[300];
	if( get_config_dir_name( path, 300 ) != 0 ) {
		cerr << "*** Error getting config dir name." << endl;
		return;
	}
	DIR *dir = opendir( path );
	if( !dir ) {
		cerr << "*** Error can't open config dir." << endl;
		return;
	}
	int count = 0;
	struct dirent *entry;
	while( ( entry = readdir( dir ) ) ) {
		if( strstr( entry->d_name, ".dat" ) &&
				!strstr( entry->d_name, "values.dat" ) ) {
			strcpy( filenames[ count++ ], entry->d_name );
		}
	}
	closedir( dir );

	if( count != filenameCount ) {
		filenameCount = count;
		files->setLines( filenameCount, (const char**)filenames );
		if( filenameCount ) name->setText( filenames[ 0 ] );
		else name->setText( DEFAULT_SAVE_FILE );
	}
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

