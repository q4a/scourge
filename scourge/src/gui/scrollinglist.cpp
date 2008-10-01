/***************************************************************************
                 scrollinglist.cpp  -  Scrollable list widget
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
#include "../util.h"

using namespace std;

#define LIST_TEXT_Y_OFFSET 4

/**
  *@author Gabor Torok
  */
ScrollingList::ScrollingList(int x, int y, int w, int h, GLuint highlight, DragAndDropHandler *dragAndDropHandler, int lineHeight ) 
        : Widget(x, y, w, h) {
  value = 0;
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
  delete [] selectedLine;
}

void ScrollingList::setLines(int count, string const s[], const Color *colors, const GLuint *icons) { 
	textWidthCache.clear();
	list.clear();
	for(int i = 0; i < count; i++)
		list.push_back(s[i]);
  this->colors = colors;
  this->icons = icons;
	setupHeight();
}

void ScrollingList::setLines(const vector<string>::iterator begin, const vector<string>::iterator end, const Color *colors, const GLuint *icons) {
	textWidthCache.clear();
	list.clear();
	list.insert(list.end(), begin, end);
	this->colors = colors;
	this->icons = icons;
	setupHeight();
}

void ScrollingList::setLine(const string& toPush) {
	list.push_back(toPush);
	setupHeight();
}

void ScrollingList::setLine(size_t pos, const string& toPush) {
	if( pos >= 0 && pos < list.size()) {
		list[pos] = toPush;
		setupHeight();
	}
}

void ScrollingList::setupHeight() {
	listHeight = list.size() * lineHeight + 5;
	scrollerHeight = (listHeight <= getHeight() ? getHeight() : (getHeight() * getHeight()) / listHeight);
  // set a min. height for scrollerHeight
  if(scrollerHeight < 20) scrollerHeight = 20;
  // reset the scroller
  value = scrollerY = 0;
  selectedLineCount = 0;
  delete [] selectedLine;
  selectedLine = NULL;

  if( !list.empty() ) {
    selectedLine = new int[ list.size() ];
      if ( !allowMultipleSelection ) {
        selectedLine[ 0 ] = 0;
        selectedLineCount = 1;
      }
  }
}

void ScrollingList::drawWidget(Widget *parent) {
  GuiTheme *theme = ((Window*)parent)->getTheme();

  // draw the text
  if(debug) {
    cerr << "**********************************************" << endl;
		cerr << "SCROLLING LIST: count=" << list.size() << endl;
		for(vector<string>::iterator i = list.begin(); i != list.end(); i++) {
			cerr << "i=" << distance(list.begin(), i) << " " << *i << endl;
    }
    cerr << "**********************************************" << endl;
  }
  int textPos = -static_cast<int>(((listHeight - getHeight()) / 100.0f) * static_cast<float>(value));
  if(!((Window*)parent)->isOpening()) {
    glScissor(((Window*)parent)->getX() + x, 
              ((Window*)parent)->getScourgeGui()->getScreenHeight() - 
              (((Window*)parent)->getY() + ((Window*)parent)->getGutter() + y + getHeight()), 
              w, getHeight());  
    glEnable( GL_SCISSOR_TEST );

    // highlight the selected line
    //if(selectedLine > -1) {
    if( selectedLine != NULL ) {
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
        glBegin( GL_TRIANGLE_STRIP );
        glVertex2d(w, textPos + (selectedLine[i] * lineHeight) + 5);
        glVertex2d(scrollerWidth, textPos + (selectedLine[i] * lineHeight) + 5);
        glVertex2d(w, textPos + ((selectedLine[i] + 1) * lineHeight + 5));
        glVertex2d(scrollerWidth, textPos + ((selectedLine[i] + 1) * lineHeight + 5));
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
		for(size_t i = 0; i < list.size(); i++) {
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
				printLine( parent, startXPos, startYPos, list[i] );
      }
    }

    if( selectedLine != NULL ) {
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
	glBegin( GL_TRIANGLE_STRIP );
	glVertex2d(0, 0);
	glVertex2d(scrollerWidth, 0);  
	glVertex2d(0, h);
	glVertex2d(scrollerWidth, h);
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

void ScrollingList::printLine( Widget *parent, int x, int y, const std::string& s ) {
	if(!linewrap) {
		((Window*)parent)->getScourgeGui()->texPrint( x, y + LIST_TEXT_Y_OFFSET, s.c_str() );
		return;
	}

	int currentX = x, currentY = y;
	vector<string> parts = Util::Tokenize<vector<string> >( s, " |" );
  int space = getTextWidth( parent, " " );
	GuiTheme *theme = ((Window*)parent)->getTheme();

	for(vector<string>::iterator i = parts.begin(); i != parts.end(); i++) {
		int wordWidth = getTextWidth( parent, i->c_str() );
		if( currentX + wordWidth > getWidth() ) {
			currentY += 15;
			currentX = x;
    }

		if( theme->getWindowText() ) {
			glColor4f( theme->getWindowText()->r,
								 theme->getWindowText()->g,
								 theme->getWindowText()->b,
								 theme->getWindowText()->a );
		} else {
			applyColor();
		}
		((Window*)parent)->getScourgeGui()->texPrint( currentX, currentY + LIST_TEXT_Y_OFFSET, i->c_str() );

		currentX += space + wordWidth;
  }
}

void ScrollingList::drawIcon( int x, int y, GLuint icon, Widget *parent ) {
  float n = lineHeight - 3;

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glEnable(GL_TEXTURE_2D);

  glPushMatrix();
  glTranslatef( x, y, 0 );
  if(icon) glBindTexture( GL_TEXTURE_2D, icon );
  glColor4f(1, 1, 1, 1);
  
  
  glBegin( GL_TRIANGLE_STRIP );
  if(icon) glTexCoord2f( 0, 0 );
  glVertex3f( 0, 0, 0 );
  if(icon) glTexCoord2f( 1, 0 );
  glVertex3f( n, 0, 0 );
  if(icon) glTexCoord2f( 0, 1 );
  glVertex3f( 0, n, 0 );
  if(icon) glTexCoord2f( 1, 1 );
  glVertex3f( n, n, 0 );
  glEnd();

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

  glDisable(GL_BLEND);
}

int ScrollingList::getLineAtPoint( int x, int y ) {
  int textPos = -static_cast<int>(((listHeight - getHeight()) / 100.0f) * static_cast<float>(value));
  int n = static_cast<int>(static_cast<float>(y - (getY() + textPos)) / static_cast<float>(lineHeight));
	if( !list.empty() && n >= 0 && n < static_cast<int>(list.size()) ) {
		return n;
	} else {
		return -1;
	}
}

void ScrollingList::selectLine( int x, int y, bool addToSelection, bool mouseDown ) {
  int n = getLineAtPoint( x, y );
  if( n > -1 ) {
    if( addToSelection && allowMultipleSelection && selectedLineCount > 0 ) {
      // is it already selected?
      for( int i = 0; i < selectedLineCount; i++ ) {
        if( selectedLine[ i ] == n ) {
          
          if( mouseDown ) {
            for( int t = i; t < selectedLineCount - 1; t++ ) {
              selectedLine[ t ] = selectedLine[ t + 1 ];
            }
            selectedLineCount--;
          }

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
				((Window*)parent)->getScourgeGui()->textWidth( list[ tooltipLine ].c_str() ) > getWidth() - scrollerWidth ) {
			setTooltip( list[tooltipLine].c_str() );
		} else {
			setTooltip( "" );
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if(!dragging && isInside(x, y)) {
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
				eventType = EVENT_DRAG;
				if( isInside(x, y) ) {
					if(listHeight > getHeight()) {
					int scrollDelta = static_cast<int>( 100.0f / static_cast<float>(list.size()) );
					if( scrollDelta == 0 ) scrollDelta = 1;
					value -= scrollDelta;
					if(value < 0)	value = 0;
					scrollerY = static_cast<int>((static_cast<float>(getHeight() - scrollerHeight) / 100.0f) * static_cast<float>(value));
					}
					return true;
				}
			} if( event->button.button == SDL_BUTTON_WHEELDOWN ) {
				eventType = EVENT_DRAG;
				if( isInside(x, y) ) {
					if(listHeight > getHeight()) {
					int scrollDelta = static_cast<int>( 100.0f / static_cast<float>(list.size()) );
					if( scrollDelta == 0 ) scrollDelta = 1;
					value += scrollDelta;
					if(value > 100)	value = 100;
					scrollerY = static_cast<int>((static_cast<float>(getHeight() - scrollerHeight) / 100.0f) * static_cast<float>(value));
					}
					return true;
				}
			} else if( event->button.button == SDL_BUTTON_LEFT ) {
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
					innerDrag = ( selectedLine != NULL );					
					innerDragX = x;
					innerDragY = y;
				}
			}
		}
		break;
	}
	if(dragging) {
		value = static_cast<int>(static_cast<float>((y - dragY) - getY()) / (static_cast<float>(getHeight() - scrollerHeight) / 100.0f));
		if(value < 0)	value = 0;
		if(value > 100)	value = 100;
		scrollerY = static_cast<int>((static_cast<float>(getHeight() - scrollerHeight) / 100.0f) * static_cast<float>(value));
	}
	return false;
}

void ScrollingList::removeEffects(Widget *parent) {
  highlightBorders = false;
  inside = false;
}

void ScrollingList::unselectAllLines() {
	if( allowMultipleSelection ) {
		selectedLineCount = 0;
	} else {
		setSelectedLine( 0 );
	}
}

void ScrollingList::setSelectedLine(size_t line) {

	if( selectedLine == NULL ) return;

	selectedLine[ 0 ] = (line < list.size() ? line : list.size() - 1);
  selectedLineCount = 1;

  // fixme: should check if line is already visible
  if(listHeight > getHeight()) {
		value = static_cast<int>((static_cast<float>(selectedLine[0] + 1) / static_cast<float>(list.size())) * 100.0f);
    if(value < 0)	value = 0;
    if(value > 100)	value = 100;
    scrollerY = static_cast<int>((static_cast<float>(getHeight() - scrollerHeight) / 100.0f) * static_cast<float>(value));
		// on 0, align to the top of the control
		if( selectedLine[ 0 ] == 0 ) {
			scrollerY = value = 0;
		}
  }
}

void ScrollingList::moveSelectionUp() {
	if( selectedLine == NULL )
		setSelectedLine( 0 );
	else if( selectedLine[0] > 0 )
		setSelectedLine( selectedLine[0] - 1 );
}

void ScrollingList::moveSelectionDown() {
	if( selectedLine == NULL )
		setSelectedLine( 0 );
	else if( selectedLine[0] < static_cast<int>(list.size()) - 1 )
		setSelectedLine( selectedLine[0] + 1 );
}
