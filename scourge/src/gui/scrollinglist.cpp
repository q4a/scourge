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
                             GLuint highlight, 
                             DragAndDropHandler *dragAndDropHandler, 
                             int lineHeight ) : Widget(x, y, w, h) {
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
  this->lineHeight = lineHeight;
  this->innerDrag = false;
  this->list = NULL;
  this->colors = NULL;
  this->icons = NULL;
  this->highlight = highlight;
  highlightBorders = false;
  debug = false;
  canGetFocusVar = Widget::canGetFocus();
}

ScrollingList::~ScrollingList() {
}

void ScrollingList::setLines(int count, const char *s[], const Color *colors, const GLuint *icons) { 
  list = s; 
  this->colors = colors;
  this->icons = icons;
  this->count = count;
  listHeight = count * lineHeight + 5;
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
  GuiTheme *theme = ((Window*)parent)->getTheme();

  // draw the text
  if(debug) {
    cerr << "**********************************************" << endl;
    cerr << "SCROLLING LIST: count=" << count << endl;
    for(int i = 0; i < count; i++) {
      cerr << "i=" << i << " " << list[i] << endl;
    }
    cerr << "**********************************************" << endl;
  }
  int textPos = -(int)(((listHeight - getHeight()) / 100.0f) * (float)value);
  if(!((Window*)parent)->isOpening()) {
    glScissor(((Window*)parent)->getX() + x, 
              ((Window*)parent)->getSDLHandler()->getScreen()->h - 
              (((Window*)parent)->getY() + Window::TOP_HEIGHT + y + getHeight()), 
              w, getHeight());  
    glEnable( GL_SCISSOR_TEST );

    // highlight the selected line
    if(selectedLine > -1) {
      if( theme->getSelectionBackground() ) {
        glColor4f( theme->getSelectionBackground()->color.r,
                   theme->getSelectionBackground()->color.g,
                   theme->getSelectionBackground()->color.b,
                   theme->getSelectionBackground()->color.a );
      } else {
        applySelectionColor();
      }
      glBegin( GL_QUADS );
      glVertex2d(scrollerWidth, textPos + (selectedLine * lineHeight) + 3);
      glVertex2d(scrollerWidth, textPos + ((selectedLine + 1) * lineHeight + 5));
      glVertex2d(w, textPos + ((selectedLine + 1) * lineHeight + 5));
      glVertex2d(w, textPos + (selectedLine * lineHeight) + 3);
      glEnd();
    }

    // draw the contents
    if(!colors) {

      if( theme->getWindowText() ) {
        glColor4f( theme->getWindowText()->r,
                   theme->getWindowText()->g,
                   theme->getWindowText()->b,
                   theme->getWindowText()->a );
      } else {
        applyColor();
      }
    }
    int ypos;
    for(int i = 0; i < count; i++) {
      ypos = textPos + (i + 1) * lineHeight;
      // writing text is expensive, only print what's visible
      if( ypos >= 0 && ypos < getHeight() + lineHeight ) {
        if(icons) drawIcon( scrollerWidth + 5, ypos - (lineHeight - 5), icons[i] );
        if(colors) glColor4f( (colors + i)->r, (colors + i)->g, (colors + i)->b, 1 );
        else if( i == selectedLine && theme->getSelectionText() ) {
          glColor4f( theme->getSelectionText()->r,
                     theme->getSelectionText()->g,
                     theme->getSelectionText()->b,
                     theme->getSelectionText()->a );
        } else {
          if( theme->getWindowText() ) {
            glColor4f( theme->getWindowText()->r,
                       theme->getWindowText()->g,
                       theme->getWindowText()->b,
                       theme->getWindowText()->a );
          } else {
            applyColor();
          }
        }
        ((Window*)parent)->getSDLHandler()->texPrint(scrollerWidth + (icons ? (lineHeight + 5) : 5), ypos, list[i]);
      }
    }

    if(selectedLine > -1) {
      if( theme->getButtonBorder() ) {
        glColor4f( theme->getButtonBorder()->color.r,
                   theme->getButtonBorder()->color.g,
                   theme->getButtonBorder()->color.b,
                   theme->getButtonBorder()->color.a );
      } else {
        applyBorderColor();
      }
      glBegin(GL_LINES);
      glVertex2d(scrollerWidth, textPos + (selectedLine * lineHeight) + 3);
      glVertex2d(w, textPos + (selectedLine * lineHeight) + 3);
      glVertex2d(scrollerWidth, textPos + ((selectedLine + 1) * lineHeight + 5));
      glVertex2d(w, textPos + ((selectedLine + 1) * lineHeight + 5));
      glEnd();
    }

    glDisable( GL_SCISSOR_TEST );
  }
  drawButton( parent, 0, scrollerY, scrollerWidth, scrollerY + scrollerHeight,
              false, false, false, false, inside );

  // draw the outline
  glDisable( GL_TEXTURE_2D );
  if(highlightBorders) {
    glLineWidth( 3.0f );
  }
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

void ScrollingList::drawIcon( int x, int y, GLuint icon ) {
  float n = lineHeight - 3;

  glEnable( GL_ALPHA_TEST );
  glAlphaFunc( GL_EQUAL, 0xff );
  glEnable(GL_TEXTURE_2D);
  glPushMatrix();
  glTranslatef( x, y, 0 );
  if(icon) glBindTexture( GL_TEXTURE_2D, icon );
  glColor4f(1, 1, 1, 1);
  
  glBegin( GL_QUADS );
  glNormal3f( 0, 0, 1 );
  if(icon) glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, 0 );
  if(icon) glTexCoord2f( 0, 1 );
  glVertex3f( 0, n, 0 );
  if(icon) glTexCoord2f( 1, 1 );
  glVertex3f( n, n, 0 );
  if(icon) glTexCoord2f( 1, 0 );
  glVertex3f( n, 0, 0 );
  glEnd();
  glPopMatrix();

  glDisable( GL_ALPHA_TEST );
  glDisable(GL_TEXTURE_2D);
}

void ScrollingList::selectLine(int x, int y) {
  int textPos = -(int)(((listHeight - getHeight()) / 100.0f) * (float)value);
  int oldLine = selectedLine;
  selectedLine = (int)((float)(y - (getY() + textPos)) / (float)lineHeight);
  if(!list || count == 0 || selectedLine < 0 || selectedLine >= count) {
    selectedLine = oldLine;
  }
}

bool ScrollingList::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  eventType = EVENT_ACTION;
	inside = (x >= getX() && x < getX() + scrollerWidth &&
						y >= getY() + scrollerY && y < getY() + scrollerY + scrollerHeight);
	switch(event->type) {
  case SDL_KEYDOWN:
  if(hasFocus()) {
    if(event->key.keysym.sym == SDLK_UP || 
       event->key.keysym.sym == SDLK_DOWN) {
      return true;
    }
  }
  break;
  case SDL_KEYUP:
  if(hasFocus()) {
    if(event->key.keysym.sym == SDLK_UP) {
      if(selectedLine) {
        setSelectedLine(selectedLine - 1);
      }
      return true;
    } else if(event->key.keysym.sym == SDLK_DOWN) {
      if(selectedLine < count - 1) {
        setSelectedLine(selectedLine + 1);
      }
      return true;
    }
  }
  break;
	case SDL_MOUSEMOTION:
		if(innerDrag && 
			 (abs(innerDragX - x) > DragAndDropHandler::DRAG_START_DISTANCE ||
				abs(innerDragY - y) > DragAndDropHandler::DRAG_START_DISTANCE) &&
			 dragAndDropHandler) {
			innerDrag = false;
			dragAndDropHandler->startDrag(this);
		}
    highlightBorders = (isInside(x, y) && dragAndDropHandler);
		break;
	case SDL_MOUSEBUTTONUP:
		if(!dragging && isInside(x, y)) {
			selectLine(x, y);
			if(dragAndDropHandler) dragAndDropHandler->receive(this);
		}
    eventType = ( x - getX() < scrollerWidth ? EVENT_DRAG : EVENT_ACTION );
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

void ScrollingList::removeEffects(Widget *parent) {
  highlightBorders = false;
  inside = false;
}

void ScrollingList::setSelectedLine(int line) { 
  selectedLine = (line < count ? line : count - 1);

  // fixme: should check if line is already visible
  if(listHeight > getHeight()) {
    value = (int)(((float)(selectedLine + 1) / (float)count) * 100.0f);
    if(value < 0)	value = 0;
    if(value > 100)	value = 100;
    scrollerY = (int)(((float)(getHeight() - scrollerHeight) / 100.0f) * (float)value);
  }
}

