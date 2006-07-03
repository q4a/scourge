/***************************************************************************
                          confirmdialog.h  -  description
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

#ifndef CONFIRM_DIALOG_H
#define CONFIRM_DIALOG_H

#include "gui.h"
#include "widget.h"
#include "button.h"
#include "label.h"
#include "checkbox.h"
#include "textfield.h"
#include "guitheme.h"

/**
  *@author Gabor Torok
  */

class Button;
class Label;
class Window;

class ConfirmDialog {
private:
	int mode;
	Label *label;
	void *object;

public:
	Window *win;
	Button *okButton;
	Button *cancelButton;

	ConfirmDialog( ScourgeGui *scourgeGui, char *title=NULL );
	~ConfirmDialog();
	
	void setText( char *text );
	void setVisible( bool b );
	bool isVisible();
	inline void setMode( int n ) { mode = n; }
	inline int getMode() { return mode; }
	inline void setObject( void *p ) { object = p; }
	inline void *getObject() { return object; }

};

#endif

