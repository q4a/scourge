/***************************************************************************
                          label.h  -  description
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

#ifndef LABEL_H
#define LABEL_H

#include "../constants.h"
#include "widget.h"

/**
  *@author Gabor Torok
  */

class SDLHandler;

class Label : public Widget {
 private:
	 char *text;
	 int lineWidth;

 public: 
  /**
	 Create a label with text. The user is responsible for freeing text after label is freed.
	 With constants, you can create a label by calling:
	 new Label(x, y, strdup("xyz"));
   */
  Label(int x, int y, char *text=NULL, int lineWidth=0);
  ~Label();
  inline char *getText() { return text; }
  inline void setText(char *s) { text = s; }
  inline void setTextCopy(char *s) { strcpy(text, s); }
  void drawWidget(Widget *parent);
  inline bool canGetFocus() { return false; }
};

#endif

