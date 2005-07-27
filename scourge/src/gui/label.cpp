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
#include "label.h"
#include "window.h"

/**
  *@author Gabor Torok
  */

Label::Label(int x, int y, char *text, int lineWidth, int fontType, int lineHeight) : Widget(x, y, 0, 0) {
  this->lineWidth = lineWidth;
  this->fontType = fontType;
  this->lineHeight = lineHeight;
  setText( text);
}

Label::~Label() {
}

void Label::drawWidget(Widget *parent) {
  if(text) {
    ((Window*)parent)->getScourgeGui()->setFontType( fontType );
    GuiTheme *theme = ((Window*)parent)->getTheme();
    if( theme->getWindowText() ) {
      glColor4f( theme->getWindowText()->r,
                 theme->getWindowText()->g,
                 theme->getWindowText()->b,
                 theme->getWindowText()->a );
    } else {
      applyColor();
    }
    if( lines.size() == 0 ) {
      // draw a single-line label
      ((Window*)parent)->getScourgeGui()->texPrint(0, 0, text);
    } else {
      int y = 0;
      for( int i = 0; i < (int)lines.size(); i++ ) {
        ((Window*)parent)->getScourgeGui()->texPrint( 0, y, lines[i].c_str() );
        y += lineHeight;
      }
    }
    ((Window*)parent)->getScourgeGui()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
  }
}

void Label::setText( char *s ) {
  strncpy(text, ( s ? s : "" ), 3000); 
  text[2999] = '\0';
  lines.clear();
  if(lineWidth > 1 && (int)strlen(text) >= lineWidth) {
    breakText( text, lineWidth, &lines );
  }
}
