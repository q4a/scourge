/***************************************************************************
                          scrollinglist.cpp  -  description
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
#include "scrollinglist.h"

/**
  *@author Gabor Torok
  */
ScrollingList::ScrollingList(int x, int y, int w, int h, 
							 DragAndDropHandler *dragAndDropHandler) : Widget(x, y, w, h) {
  value = 0;
  count = 0;
  scrollerWidth = 20;
  listHeight = 0;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  inside = false;
  scrollerY = 0;
  this->dragging = false;
  this->dragX = this->dragY = 0;
  selectedLine = -1;
  scrollerHeight = h;
  this->dragAndDropHandler = dragAndDropHandler;
  this->innerDrag = false;
}

ScrollingList::~ScrollingList() {
}

void ScrollingList::setLines(int count, const char *s[], const Color *colors) { 
  list = s; 
  this->colors = colors;
  this->count = count;
  listHeight = count * 15 + 5;
  scrollerHeight = (listHeight <= getHeight() ? 
					getHeight() : 
					(getHeight() * getHeight()) / listHeight);
  // set a min. height for scrollerHeight
  if(scrollerHeight < 20) scrollerHeight = 20;
  // reset the scroller
  value = scrollerY = 0;
  selectedLine = (list && count ? 0 : -1);
}

void ScrollingList::drawWidget(Widget *parent) {
  // draw the button
  applyBackgroundColor(true);
  if(isTranslucent()) {
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );
	glEnable( GL_BLEND );
  }
  glBegin(GL_QUADS);
  glVertex2d(0, scrollerY);
  glVertex2d(0, scrollerY + scrollerHeight);
  glVertex2d(scrollerWidth, scrollerY + scrollerHeight);
  glVertex2d(scrollerWidth, scrollerY);
  glEnd();
  if(isTranslucent()) {
	glDisable( GL_BLEND );
  }

  if(inside) {
    GLint t = SDL_GetTicks();
    if(lastTick == 0 || t - lastTick > 50) {
      lastTick = t;
      alpha += alphaInc;
      if(alpha >= 0.7f || alpha < 0.4f) alphaInc *= -1.0f;
    }
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glEnable( GL_BLEND );
    glBegin( GL_QUADS );
    glColor4f( 1, 0, 0, alpha );
    glVertex2d(0, scrollerY);
    glColor4f( 0, 1, 0, alpha );
    glVertex2d(0, scrollerY + scrollerHeight);
    glColor4f( 0, 0, 1, alpha );
    glVertex2d(scrollerWidth, scrollerY + scrollerHeight);
    glColor4f( 1, 0, 1, alpha );
    glVertex2d(scrollerWidth, scrollerY);
    glEnd();
    glDisable( GL_BLEND );
  }

  // draw the text
  int textPos = -(int)(((listHeight - getHeight()) / 100.0f) * (float)value);
  if(!((Window*)parent)->isOpening()) {
    glScissor(((Window*)parent)->getX() + x, 
	      ((Window*)parent)->getSDLHandler()->getScreen()->h - 
	      (((Window*)parent)->getY() + Window::TOP_HEIGHT + y + getHeight()), 
	      w, getHeight());  
    glEnable( GL_SCISSOR_TEST );
   
    // highlight the selected line
    if(selectedLine > -1) {
      applySelectionColor();
      glBegin( GL_QUADS );
      glVertex2d(scrollerWidth, textPos + (selectedLine * 15) + 3);
      glVertex2d(scrollerWidth, textPos + ((selectedLine + 1) * 15 + 5));
      glVertex2d(w, textPos + ((selectedLine + 1) * 15 + 5));
      glVertex2d(w, textPos + (selectedLine * 15) + 3);
      glEnd();
    }
    
    // draw the contents
    if(!colors) {
	  applyColor();
	}
    int ypos;
    for(int i = 0; i < count; i++) {
      ypos = textPos + (i + 1) * 15;
      // writing text is expensive, only print what's visible
      if(ypos >= 0 && ypos < getHeight()) {
	if(colors) glColor4f( (colors + i)->r, (colors + i)->g, (colors + i)->b, 1 );
	((Window*)parent)->getSDLHandler()->
	  texPrint(scrollerWidth + 5, ypos, list[i]);
      }
    }
    
    if(selectedLine > -1) {
      applyBorderColor();
      glBegin(GL_LINES);
      glVertex2d(scrollerWidth, textPos + (selectedLine * 15) + 3);
      glVertex2d(w, textPos + (selectedLine * 15) + 3);
      glVertex2d(scrollerWidth, textPos + ((selectedLine + 1) * 15 + 5));
      glVertex2d(w, textPos + ((selectedLine + 1) * 15 + 5));
      glEnd();
    }
    
    glDisable( GL_SCISSOR_TEST );
  }
  
  // draw the outline
  applyBorderColor();
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
}

void ScrollingList::selectLine(int x, int y) {
  int textPos = -(int)(((listHeight - getHeight()) / 100.0f) * (float)value);
  int oldLine = selectedLine;
  selectedLine = (int)((float)(y - (getY() + textPos)) / 15.0);
  if(!list || count == 0 || selectedLine < 0 || selectedLine >= count) {
	selectedLine = oldLine;
  }
}

bool ScrollingList::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
	inside = (x >= getX() && x < getX() + scrollerWidth &&
						y >= getY() + scrollerY && y < getY() + scrollerY + scrollerHeight);
	switch(event->type) {
	case SDL_MOUSEMOTION:
		if(innerDrag && 
			 (abs(innerDragX - x) > DragAndDropHandler::DRAG_START_DISTANCE ||
				abs(innerDragY - y) > DragAndDropHandler::DRAG_START_DISTANCE) &&
			 dragAndDropHandler) {
			innerDrag = false;
			dragAndDropHandler->startDrag(this);
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if(!dragging && isInside(x, y)) {
			selectLine(x, y);
			if(dragAndDropHandler) dragAndDropHandler->receive(this);
		}
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
			selectLine(x, y);
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

