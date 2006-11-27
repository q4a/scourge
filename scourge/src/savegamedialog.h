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
	char title[300];
} SavegameInfo;

class SavegameDialog {
private:
  Scourge *scourge;
  Window *win;
  ScrollingList *files;
  Button *cancel, *save, *load, *newSave;
	std::vector<SavegameInfo*> fileInfos;
	int filenameCount;
	char **filenames;
	ConfirmDialog *confirm;

public:
  SavegameDialog( Scourge *scourge );
  ~SavegameDialog();
  inline Window *getWindow() { return win; }
  void handleEvent( Widget *widget, SDL_Event *event );
	void show( bool inSaveMode = true );

protected:
	void findFiles();
	bool readFileDetails( char *path );
	void createNewSaveGame();
	bool saveIndexFile();
	void makeDirectory( char *path );
	bool checkIfFileExists( char *filename );
	void saveScreenshot();
};

#endif

