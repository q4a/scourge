/***************************************************************************
  savegamedialog.h  -  description
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

#ifndef SAVEGAME_DIALOG_H
#define SAVEGAME_DIALOG_H

#include "common/constants.h"
#include <map>
#include <vector>

class Scourge;
class Window;
class Label;
class ScrollingList;
class ScrollingLabel;
class Widget;
class Button;
class TextField;
class ConfirmDialog;

using namespace std;

#define MAX_SAVEGAME_COUNT 100

typedef struct _SavegameInfo {
	char path[300];
	char title[3000];
} SavegameInfo;

class SavegameDialog {
private:
  Scourge *scourge;
  Window *win;
  ScrollingList *files;
  Button *cancel, *save, *load, *newSave, *deleteSave;
	std::vector<SavegameInfo*> fileInfos;
	int filenameCount;
	char **filenames;
	GLuint *screens;
	ConfirmDialog *confirm;

public:
  SavegameDialog( Scourge *scourge );
  ~SavegameDialog();
  inline Window *getWindow() { return win; }
  void handleEvent( Widget *widget, SDL_Event *event );
	void show( bool inSaveMode = true );

	bool createNewSaveGame();

protected:
	bool findFiles();
	bool readFileDetails( char *path );
	void makeDirectory( char *path );
	void findFilesInDir( char *path, vector<string> *fileNameList );
	bool checkIfFileExists( char *filename );
	void saveScreenshot( char *dirName );
	bool copyMaps( char *fromDirName, char *toDirName );
	bool copyFile( char *fromDirName, char *toDirName, char *fileName );
	GLuint loadScreenshot( char *path );
	bool saveGameInternal( SavegameInfo *info );
	bool createSaveGame( SavegameInfo *info );
	void deleteUnreferencedMaps( char *dirName );
	void loadGame( int n );
	void setSavegameInfoTitle( SavegameInfo *info );
	bool deleteDirectory( char *path );
};

#endif

