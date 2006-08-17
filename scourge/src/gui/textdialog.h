/***************************************************************************
                          textdialog.h  -  description
                             -------------------
    begin                : Thu Aug 28 2003
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

#ifndef TEXT_DIALOG_H
#define TEXT_DIALOG_H

#include "gui.h"
#include "widget.h"
#include "button.h"
#include "scrollinglabel.h"
#include "checkbox.h"
#include "textfield.h"
#include "guitheme.h"

/**
  *@author Gabor Torok
  */

class Button;
class ScrollingLabel;
class Window;

class TextDialog {
private:
	int mode;
	ScrollingLabel *label;
	void *object;

public:
	Window *win;
	Button *okButton;

	TextDialog( ScourgeGui *scourgeGui, char *title=NULL );
	~TextDialog();
	
	void setText( char *text );
	void setVisible( bool b );
	bool isVisible();

};

#endif

