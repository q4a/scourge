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

#include "gui.h"
#include "widget.h"
#include <vector>

/**
  *@author Gabor Torok
  */

class SDLHandler;

class Label : public Widget {
 private:
	 char text[3000];
	 int lineWidth;
     int fontType;
     int lineHeight;
     std::vector<std::string> lines;
     bool specialColor;

 public: 
     Label(int x, int y, char const* text=NULL, int lineWidth=0, int fontType=0, int lineHeight=15);
     ~Label();
     inline int getFontType() { return fontType; }
     inline void setFontType( int n ) { fontType = n; }
     inline int getLineHeight() { return lineHeight; }
     inline void setLineHeight( int n ) { lineHeight = n; }
     inline char *getText() { return text; }
     void setText(char const* s);
     void drawWidget(Widget *parent);
     inline bool canGetFocus() { return false; }
		 inline void setSpecialColor() { specialColor = true; }
};

#endif

