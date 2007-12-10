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

using namespace std;

#define LIST_TEXT_Y_OFFSET 4

/**
  *@author Gabor Torok
  */
ScrollingList::ScrollingList(int x, int y, int w, int h,
                             GLuint highlight, 
                             DragAndDropHandler *dragAndDropHandler, 
                             int lineHeight ) : Widget(x, y, w, h) {
  value = 0;
  count = 0;
  scrollerWidth = 15;
  listHeight = 0;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  inside = false;
  scrollerY = 0;
  this->dragging = false;
  this->dragX = this->dragY = 0;
  selectedLine = NULL;
  selectedLineCount = 0;
  scrollerHeight = h;
  this->dragAndDropHandler = dragAndDropHandler;
  this->lineHeight = lineHeight;
  this->innerDrag = false;
  this->list = NULL;
  this->colors = NULL;
  this->icons = NULL;
  this->highlight = highlight;
	this->linewrap = false;
	this->iconBorder = false;
  highlightBorders = false;
  debug = false;
  canGetFocusVar = Widget::canGetFocus();
  allowMultipleSelection = false;
  tooltipLine = -1;
}

ScrollingList::~ScrollingList() {
  if( selectedLine ) free( selectedLine );
}

void ScrollingList::setLines(int count, const char *s[], const Color *colors, const GLuint *icons) { 
	textWidthCache.clear();
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
//  selectedLine = (list && count ? 0 : -1);
  selectedLineCount = 0;
  if( selectedLine ) {
    free( selectedLine );
    selectedLine = NULL;
  }
  if( count > 0 ) {
    selectedLine = (int*)malloc( count * sizeof( int ) );
    selectedLine[ 0 ] = 0;
// ***********************
    selectedLineCount = 1;
// ***********************
  }
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
              ((Window*)parent)->getScourgeGui()->getScreenHeight() - 
              (((Window*)parent)->getY() + ((Window*)parent)->getGutter() + y + getHeight()), 
              w, getHeight());  
    glEnable( GL_SCISSOR_TEST );

    // highlight the selected line
    //if(selectedLine > -1) {
    if( selectedLine ) {
      if( theme->getSelectionBackground() ) {
		if( theme->getSelectionBackground()->color.a < 1 ) {
		  glEnable( GL_BLEND );
		  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
        glColor4f( theme->getSelectionBackground()->color.r,
                   theme->getSelectionBackground()->color.g,
                   theme->getSelectionBackground()->color.b,
                   theme->getSelectionBackground()->color.a );
      } else {
        applySelectionColor();
      }
      for( int i = 0; i < selectedLineCount; i++ ) {
        glBegin( GL_QUADS );
        glVertex2d(scrollerWidth, textPos + (selectedLine[i] * lineHeight) + 5);
        glVertex2d(scrollerWidth, textPos + ((selectedLine[i] + 1) * lineHeight + 5));
        glVertex2d(w, textPos + ((selectedLine[i] + 1) * lineHeight + 5));
        glVertex2d(w, textPos + (selectedLine[i] * lineHeight) + 5);
        glEnd();
      }
	  glDisable( GL_BLEND );
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
        if(icons) drawIcon( scrollerWidth + 5, ypos - (lineHeight - 5), icons[i], parent );
        if(colors) glColor4f( (colors + i)->r, (colors + i)->g, (colors + i)->b, 1 );
        else if( isSelected( i ) && theme->getSelectionText() ) {
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

				int startYPos = ypos - ( lineHeight > 15 ? ( lineHeight - 15 ) : 0 );
				int startXPos = scrollerWidth + (icons ? (lineHeight + 5) : 5);
				char *p = (char*)list[i];
				if( linewrap ) {
					while( p && *p ) {
						p = printLine( parent, startXPos, startYPos, p );    
						startYPos += 15;
					}
				} else {
				  ((Window*)parent)->getScourgeGui()->texPrint( startXPos, startYPos + LIST_TEXT_Y_OFFSET, p );
				}
      }
    }

    //if(selectedLine > -1) {
    if( selectedLine ) {
      if( theme->getButtonBorder() ) {
        glColor4f( theme->getButtonBorder()->color.r,
                   theme->getButtonBorder()->color.g,
                   theme->getButtonBorder()->color.b,
                   theme->getButtonBorder()->color.a );
      } else {
        applyBorderColor();
      }
      for( int i = 0; i < selectedLineCount; i++ ) {
        glBegin(GL_LINES);
        glVertex2d(scrollerWidth, textPos + (selectedLine[i] * lineHeight) + 5);
        glVertex2d(w, textPos + (selectedLine[i] * lineHeight) + 5);
        glVertex2d(scrollerWidth, textPos + ((selectedLine[i] + 1) * lineHeight + 5));
        glVertex2d(w, textPos + ((selectedLine[i] + 1) * lineHeight + 5));
        glEnd();
      }
    }

    glDisable( GL_SCISSOR_TEST );
  }

	glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColor4f( 0, 0, 0, 0.4f );
	glBegin( GL_QUADS );
  glVertex2d(0, 0);
  glVertex2d(0, h);
  glVertex2d(scrollerWidth, h);
  glVertex2d(scrollerWidth, 0);  
  glEnd();
	glDisable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );

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

// FIXME: this is nearly the same method as in ScrollingLabel... needs to be refactored
char *ScrollingList::printLine( Widget *parent, int x, int y, char *s ) {
  GuiTheme *theme = ((Window*)parent)->getTheme();
  int xp = x;

  int space = getTextWidth( parent, " " );
  char *wordEnd = strpbrk( s, " |" );  
  char *p = s;
  char *word;
  char tmp;
  while( p && *p ) {

    // create word starting at p
    if( wordEnd ) {
      tmp = *wordEnd;
      *wordEnd = 0;
    } else {
      tmp = 0;
    }

    int wordWidth = getTextWidth( parent, p );

		if( theme->getWindowText() ) {
			glColor4f( theme->getWindowText()->r,
								 theme->getWindowText()->g,
								 theme->getWindowText()->b,
								 theme->getWindowText()->a );
		} else {
			applyColor();
		}

		word = p;

    //cerr << "wordWidth=" << wordWidth << " xp=" << xp << " p=" << p << " width=" << getWidth() << " tmp=" << ( tmp ? tmp : '*' ) << endl;    

    if( xp + wordWidth > getWidth() ) {
      if( tmp ) *wordEnd = tmp;
      return p;
    }
    ((Window*)parent)->getScourgeGui()->texPrint( xp, y + LIST_TEXT_Y_OFFSET, word );

    // move caret
    xp += wordWidth;
    xp += space;

    // move p past word end
    if( !tmp ) return NULL;
    *wordEnd = tmp;
    p = wordEnd;

    // end of line?
    if( *p == '|' ) return p + 1;

    // skip space
    p++;

    // find end of new word
    wordEnd = strpbrk( p, " |" );
  }
  return NULL;
}

void ScrollingList::drawIcon( int x, int y, GLuint icon, Widget *parent ) {
  float n = lineHeight - 3;

  glEnable( GL_ALPHA_TEST );
  //glAlphaFunc( GL_EQUAL, 0xff );
	glAlphaFunc( GL_NOTEQUAL, 0 );
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

  glDisable( GL_ALPHA_TEST );
  glDisable(GL_TEXTURE_2D);

	if( iconBorder ) {
		GuiTheme *theme = ((Window*)parent)->getTheme();
		if( theme->getButtonBorder() ) {
			glColor4f( theme->getButtonBorder()->color.r,
								 theme->getButtonBorder()->color.g,
								 theme->getButtonBorder()->color.b,
								 theme->getButtonBorder()->color.a );
		} else {
			applyBorderColor();
		}
		glBegin( GL_LINE_LOOP );
		glVertex2f( 0, 0 );
		glVertex2f( 0, n );
		glVertex2f( n, n );
		glVertex2f( n, 0 );
		glEnd();
	}
  glPopMatrix();
}

int ScrollingList::getLineAtPoint( int x, int y ) {
  int textPos = -(int)(((listHeight - getHeight()) / 100.0f) * (float)value);
  int n = (int)((float)(y - (getY() + textPos)) / (float)lineHeight);
  if( list && count && n >= 0 && n < count ) {
    return n;
  } else {
    return -1;
  }
}

void ScrollingList::selectLine( int x, int y, bool addToSelection, bool mouseDown ) {
  int n = getLineAtPoint( x, y );
  if( n > -1 ) {
    if( addToSelection && allowMultipleSelection ) {
      // is it already selected?
      for( int i = 0; i < selectedLineCount; i++ ) {
        if( selectedLine[ i ] == n ) {
          
//          if( mouseDown ) {
            for( int t = i; t < selectedLineCount - 1; t++ ) {
              selectedLine[ t ] = selectedLine[ t + 1 ];
            }
            selectedLineCount--;
//          }

          return;
        }        
      }
      // add to selection
      selectedLine[ selectedLineCount++ ] = n;
    } else {
      // set as selection
      selectedLineCount = 0;
      selectedLine[ selectedLineCount++ ] = n;
    }
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
      moveSelectionUp();
      return true;
    } else if(event->key.keysym.sym == SDLK_DOWN) {
      moveSelectionDown();
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
			
		tooltipLine = getLineAtPoint( x, y );
		if( tooltipLine > -1 && 
				!linewrap &&
				((Window*)parent)->getScourgeGui()->textWidth( list[ tooltipLine ] ) > getWidth() - scrollerWidth ) {
			setTooltip( (char*)(list[ tooltipLine ]) );
		} else {
			setTooltip( "" );
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if(!dragging && isInside(x, y)) {
			/*
			selectLine( x, y, 
									( ( SDL_GetModState() & KMOD_SHIFT ) ||
										( SDL_GetModState() & KMOD_CTRL ) ),
									false );
			*/                  
			if(dragAndDropHandler) dragAndDropHandler->receive(this);
		}
		eventType = ( dragging ? EVENT_DRAG : EVENT_ACTION );
		innerDrag = false;
		dragging = false;
		//((Window*)parent)->getScourgeGui()->unlockMouse();
		return isInside(x, y);
	case SDL_MOUSEBUTTONDOWN:
		if( event->button.button ) {
			if( event->button.button == SDL_BUTTON_WHEELUP ) {
				if( isInside(x, y) ) {
					moveSelectionUp();
					return true;
				}
			} if( event->button.button == SDL_BUTTON_WHEELDOWN ) {
				if( isInside(x, y) ) {
					moveSelectionDown();
					return true;
				}
			} else if( event->button.button == SDL_BUTTON_LEFT ||
								 event->button.button == SDL_BUTTON_RIGHT ) {
				if(scrollerHeight < getHeight() && x - getX() < scrollerWidth) {
					innerDrag = false;
					dragging = inside;
					dragX = x - getX();
					dragY = y - (scrollerY + getY());
					//((Window*)parent)->getScourgeGui()->lockMouse( this );

					if((y - getY()) < scrollerY) { // we clicked above the scroller
						moveSelectionUp();
					}
					else if((y -getY()) > (scrollerY + scrollerHeight)) { // we clicked below the scroller
						moveSelectionDown();
					} 
				} else if(isInside(x, y)) {
					dragging = false;
					selectLine( x, y, 
											( ( SDL_GetModState() & KMOD_SHIFT ) ||
												( SDL_GetModState() & KMOD_CTRL ) ), 
											true );
					innerDrag = ( selectedLine );					
					innerDragX = x;
					innerDragY = y;
				}
			}
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
  if( !selectedLine ) return;
  selectedLine[ 0 ] = (line < count ? line : count - 1);
  selectedLineCount = 1;

  // fixme: should check if line is already visible
  if(listHeight > getHeight()) {
    value = (int)(((float)(selectedLine[0] + 1) / (float)count) * 100.0f);
    if(value < 0)	value = 0;
    if(value > 100)	value = 100;
    scrollerY = (int)(((float)(getHeight() - scrollerHeight) / 100.0f) * (float)value);
		// on 0, align to the top of the control
		if( selectedLine[ 0 ] == 0 ) {
			scrollerY = value = 0;
		}
  }
}

void ScrollingList::moveSelectionUp() {
	if( !selectedLine ) setSelectedLine( 0 );
	else if( selectedLine[0] > 0 )
		setSelectedLine( selectedLine[0] - 1 );
}

void ScrollingList::moveSelectionDown() {
	if( !selectedLine ) setSelectedLine( 0 );
	else if( selectedLine[0] < count - 1 )
		setSelectedLine( selectedLine[0] + 1 );
}
