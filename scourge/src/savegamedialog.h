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

class SavegameDialog {
private:
  Scourge *scourge;
  Window *win;
  ScrollingList *files;
  TextField *name;
  Button *cancel, *save, *load;
	int filenameCount;
	char **filenames;
	char selectedName[255];
	ConfirmDialog *confirm;

public:
  SavegameDialog( Scourge *scourge );
  ~SavegameDialog();
  inline Window *getWindow() { return win; }
  void handleEvent( Widget *widget, SDL_Event *event );
	inline char *getSelectedName() { return selectedName; }
	void show( bool inSaveMode = true );

protected:
	void findFiles();
	void setName();
	bool checkIfFileExists( char *filename );
};

#endif

