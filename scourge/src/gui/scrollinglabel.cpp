/***************************************************************************
                          scrollinglabel.cpp  -  description
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
#include "scrollinglabel.h"

/**
  *@author Gabor Torok
  */
ScrollingLabel::ScrollingLabel(int x, int y, int w, int h, char *text) : Widget(x, y, w, h) {
  value = 0;
  //  count = 0;
  scrollerWidth = 20;
  listHeight = 0;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  inside = false;
  scrollerY = 0;
  this->dragging = false;
  this->dragX = this->dragY = 0;
  //selectedLine = -1;
  scrollerHeight = h;
  //  this->dragAndDropHandler = dragAndDropHandler;
  this->innerDrag = false;
  //  this->list = NULL;
  //  this->colors = NULL;
  //  this->icons = NULL;
  //  this->highlight = highlight;
  // highlightBorders = false;
  canGetFocusVar = Widget::canGetFocus();
  setText( text );
}

ScrollingLabel::~ScrollingLabel() {
}

void ScrollingLabel::setText( char *s ) {

  strncpy(text, ( s ? s : "" ), 3000); 
  text[2999] = '\0';

  lineWidth = ( w - scrollerWidth - 20 ) / 6;

  // break text into a list
  lines.clear();
  breakText( text, lineWidth, &lines );

  listHeight = lines.size() * 15 + 5;
  scrollerHeight = (listHeight <= getHeight() ? 
					getHeight() : 
					(getHeight() * getHeight()) / listHeight);
  // set a min. height for scrollerHeight
  if(scrollerHeight < 20) scrollerHeight = 20;
  // reset the scroller
  value = scrollerY = 0;
}

void ScrollingLabel::drawWidget(Widget *parent) {
  GuiTheme *theme = ((Window*)parent)->getTheme();

  // draw the text
  int textPos = -(int)(((listHeight - getHeight()) / 100.0f) * (float)value);
  if(!((Window*)parent)->isOpening()) {
    glScissor(((Window*)parent)->getX() + x, 
              ((Window*)parent)->getSDLHandler()->getScreen()->h - 
              (((Window*)parent)->getY() + Window::TOP_HEIGHT + y + getHeight()), 
              w, getHeight());  
    glEnable( GL_SCISSOR_TEST );
   
    // draw the contents
    if( theme->getWindowText() ) {
      glColor4f( theme->getWindowText()->r,
                 theme->getWindowText()->g,
                 theme->getWindowText()->b,
                 theme->getWindowText()->a );
    } else {
      applyColor();
    }

    //    int ypos = ypos = textPos + (i + 1) * 15;
    //    ((Window*)parent)->getSDLHandler()->texPrint( scrollerWidth + 5, ypos, text, lineWidth );

    ((Window*)parent)->getSDLHandler()->setFontType( SDLHandler::MONO_FONT );
    int ypos;
    for(int i = 0; i < (int)lines.size(); i++) {
      ypos = textPos + (i + 1) * 15;
      // writing text is expensive, only print what's visible
      if(ypos >= 0 && ypos < getHeight()) {
		((Window*)parent)->getSDLHandler()->texPrint( scrollerWidth + 5, ypos, lines[i].c_str() );
      }
    }
    ((Window*)parent)->getSDLHandler()->setFontType( SDLHandler::DEFAULT_FONT );
        
    glDisable( GL_SCISSOR_TEST );
  }
  drawButton( parent, 0, scrollerY, scrollerWidth, scrollerY + scrollerHeight,
              false, false, false, false, inside );

  // draw the outline
  glDisable( GL_TEXTURE_2D );
  if( theme->getButtonBorder() ) {
    glColor4f( theme->getButtonBorder()->color.r,
               theme->getButtonBorder()->color.g,
               theme->getButtonBorder()->color.b,
               theme->getButtonBorder()->color.a );
  } else {
    applyBorderColor();
  }  

  glBegin(GL_LINES);
  glVertex2d(0, 0);
  glVertex2d(0, h);
  glVertex2d(w, 0);
  glVertex2d(w, h);
  glVertex2d(0, 0);
  glVertex2d(w, 0);
  glVertex2d(0, h);
  glVertex2d(w, h);
  glVertex2d(scrollerWidth, 0);
  glVertex2d(scrollerWidth, h);
  glVertex2d(0, scrollerY);
  glVertex2d(scrollerWidth, scrollerY);
  glVertex2d(0, scrollerY + scrollerHeight);
  glVertex2d(scrollerWidth, scrollerY + scrollerHeight);
  glEnd();
  glLineWidth( 1.0f );
}

bool ScrollingLabel::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inside = (x >= getX() && x < getX() + scrollerWidth &&
            y >= getY() + scrollerY && y < getY() + scrollerY + scrollerHeight);
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    innerDrag = false;
    dragging = false;
    return isInside(x, y);
  case SDL_MOUSEBUTTONDOWN:
    if(scrollerHeight < getHeight() && x - getX() < scrollerWidth) {
      innerDrag = false;
      dragging = inside;
      dragX = x - getX();
      dragY = y - (scrollerY + getY());
    } else if(isInside(x, y)) {
      dragging = false;
      innerDrag = (selectedLine != -1);
      innerDragX = x;
      innerDragY = y;
    }
    break;
  }
  if(dragging) {
    value = (int)((float)((y - dragY) - getY()) / 
                  ((float)(getHeight() - scrollerHeight) / 100.0f));
    if(value < 0)	value = 0;
    if(value > 100)	value = 100;
    scrollerY = (int)(((float)(getHeight() - scrollerHeight) / 100.0f) * (float)value);
  }
  return false;
}

void ScrollingLabel::removeEffects(Widget *parent) {
  inside = false;
}

