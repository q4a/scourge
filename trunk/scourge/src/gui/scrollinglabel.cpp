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

using namespace std;

/**
  *@author Gabor Torok
  */
ScrollingLabel::ScrollingLabel(int x, int y, int w, int h, char *text) : Widget(x, y, w, h) {
  value = 0;
  //  count = 0;
  scrollerWidth = 15;
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
  this->handler = NULL;
  canGetFocusVar = Widget::canGetFocus();
  interactive = true;
  setText( text );
}

ScrollingLabel::~ScrollingLabel() {
  coloring.clear();
}

void ScrollingLabel::setText( char *s ) {
  textWidthCache.clear();

  wordPosCount = 0;

  strncpy( text, ( s ? s : "" ), TEXT_SIZE ); 
  text[ TEXT_SIZE - 1 ] = '\0';
  
  lineWidth = ( w - scrollerWidth - 20 ) / 8;

  willScrollToBottom = false;
  willSetScrollerHeight = true;
  listHeight = 0;
  scrollerHeight = 20;

  // reset the scroller
  value = scrollerY = 0;
}

void ScrollingLabel::appendText( const char *s ) {
  int len = strlen( text );
  int slen = strlen( s );
  if( len + slen >= TEXT_SIZE - 1 ) {
    int extra = len + slen - ( TEXT_SIZE - 1 );
    for( int i = extra; i < len; i++ ) text[ i - extra ] = text[ i ];
    text[ len - extra ] = '\0';
  }
  strcat( text, s );

  text[ TEXT_SIZE - 1 ] = '\0';
  
  lineWidth = ( w - scrollerWidth - 20 ) / 8;

  willSetScrollerHeight = true;
  willScrollToBottom = true;
  listHeight = 0;
  scrollerHeight = 20;

  // reset the scroller
  value = scrollerY = 0;
}

void ScrollingLabel::drawWidget(Widget *parent) {
  wordPosCount = 0;
  GuiTheme *theme = ((Window*)parent)->getTheme();

  // draw the text
  int textPos = -(int)(((listHeight - getHeight()) / 100.0f) * (float)value);
  if(!((Window*)parent)->isOpening()) {
    glScissor(((Window*)parent)->getX() + x, 
              ((Window*)parent)->getScourgeGui()->getScreenHeight() - 
              (((Window*)parent)->getY() + ((Window*)parent)->getGutter() + y + getHeight()), 
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

    //((Window*)parent)->getSDLHandler()->setFontType( SDLHandler::SCOURGE_MONO_FONT );
    int ypos;

    int lineCount = 0;
    ypos = textPos + 15;
    char *p = text;
    while( p && *p ) {
      p = printLine( parent, scrollerWidth + 5, ypos, p );    
      ypos += 15;
      lineCount++;
    }

    if( willSetScrollerHeight ) {
      willSetScrollerHeight = 0;
      listHeight = lineCount * 15 + 5;
      scrollerHeight = (listHeight <= getHeight() ? 
                        getHeight() : 
                        (getHeight() * getHeight()) / listHeight);
      // set a min. height for scrollerHeight
      if(scrollerHeight < 20) scrollerHeight = 20;

      if( willScrollToBottom ) {
        willScrollToBottom = false;
        value = 100;
        scrollerY = (int)(((float)(getHeight() - scrollerHeight) / 100.0f) * (float)value);
      }
    }
    //((Window*)parent)->getSDLHandler()->setFontType( SDLHandler::SCOURGE_DEFAULT_FONT );
        
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

char *ScrollingLabel::printLine( Widget *parent, int x, int y, char *s ) {
  GuiTheme *theme = ((Window*)parent)->getTheme();
  int xp = x;
  
//  char *tmp = strdup( s );
//  char *p = strtok( tmp, " " );


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

    int wordWidth;
    if( coloring.find( *p ) != coloring.end() ) {      

      wordWidth = getTextWidth( parent, p + 1 );

      // store word pos for lookup on click
      wordPos[ wordPosCount ].x = xp;
      wordPos[ wordPosCount ].y = y - 10;
      wordPos[ wordPosCount ].w = wordWidth;
      wordPos[ wordPosCount ].h = 10;
      //strcpy( wordPos[ wordPosCount ].word, p );
      // copy only valid characters
      int c = 0;
      for( int i = 0; i < (int)strlen( p ); i++ ) {
        if( ( p[i] >= 'a' && p[i] <= 'z' ) ||
            ( p[i] >= 'A' && p[i] <= 'Z' ) ||
            ( p[i] >= '0' && p[i] <= '9' ) ||
            p[i] == '-' || p[i] == '\'' ) wordPos[ wordPosCount ].word[ c++ ] = p[ i ];
      }
      wordPos[ wordPosCount ].word[ c++ ] = 0;

      // Is the mouse over this word?
      int tx = ((Window*)parent)->getScourgeGui()->getMouseX() - getX() - parent->getX();
      int ty = ((Window*)parent)->getScourgeGui()->getMouseY() - getY() - parent->getY() - ((Window*)parent)->getGutter();
      if( interactive &&
          wordPos[ wordPosCount ].x <= tx && 
          wordPos[ wordPosCount ].x + wordPos[ wordPosCount ].w > tx &&
          wordPos[ wordPosCount ].y <= ty && 
          wordPos[ wordPosCount ].y + wordPos[ wordPosCount ].h > ty ) {
        // FIXME: shouldn't be hard-coded
        glColor4f( 1, 0, 1, 1 );
      } else {
        Color c = coloring[ *p ];
        glColor4f( c.r, c.g, c.b, c.a );
      }
      if( handler ) handler->showingWord( wordPos[ wordPosCount ].word );
      wordPosCount++;
      word = p + 1;
    } else {

      wordWidth = getTextWidth( parent, p );

      if( theme->getWindowText() ) {
        glColor4f( theme->getWindowText()->r,
                   theme->getWindowText()->g,
                   theme->getWindowText()->b,
                   theme->getWindowText()->a );
      } else {
        applyColor();
      }

      word = p;
    }

    //cerr << "wordWidth=" << wordWidth << " xp=" << xp << " p=" << p << " width=" << getWidth() << " tmp=" << ( tmp ? tmp : '*' ) << endl;    

    if( xp + wordWidth > getWidth() ) {
      if( tmp ) *wordEnd = tmp;
      return p;
    }
    ((Window*)parent)->getScourgeGui()->texPrint( xp, y, word );

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
//  free( tmp );
  return NULL;
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

      if( handler ) {
        int index = getWordPos( x, y );
        if( index > -1 ) handler->wordClicked( wordPos[ index ].word );
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

int ScrollingLabel::getWordPos( int x, int y ) {
  int tx = x - getX();
  int ty = y - getY();
  //cerr << "wordPosCount=" << wordPosCount << " x=" << tx << " y=" << ty << endl;
  for( int i = 0; i < wordPosCount; i++ ) {
    //cerr << "\ti=" << i << " x=" << wordPos[ i ].x << " y=" << wordPos[ i ].y << endl;
    if( wordPos[ i ].x <= tx && wordPos[ i ].x + wordPos[ i ].w > tx &&
        wordPos[ i ].y <= ty && wordPos[ i ].y + wordPos[ i ].h > ty ) {
      return i;
    }
  }
  return -1;
}

void ScrollingLabel::removeEffects(Widget *parent) {
  inside = false;
}

int ScrollingLabel::getTextWidth( Widget *parent, const char *s ) {
  string str = s;
  int n;
  if( textWidthCache.find( str ) == textWidthCache.end() ) {
    n = ((Window*)parent)->getScourgeGui()->textWidth( s );
    textWidthCache[ str ] = n;
  } else {
    n = textWidthCache[ str ];
  }
  return n;
}

