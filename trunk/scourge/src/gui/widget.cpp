/***************************************************************************
                          widget.cpp  -  description
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

#include "widget.h"
#include "window.h"
#include "guitheme.h"

using namespace std;

#define TOOLTIP_DELAY 500

/**
  *@author Gabor Torok
  */

Widget::Widget(int x, int y, int w, int h) {
  move(x, y);
  this->w = w;
  this->h = h;
  setColor( 0.05f, 0.05f, 0.05f, 1 );
  setBackground( 1, 0.75f, 0.45f );
  //  setSelectionColor( 1, 0.5f, 0.45f );
  setSelectionColor( 0.75f, 0.75f, 0.8f );
  //setBorderColor( 0.8f, 0.5f, 0.2f );
  setBorderColor( 0.6f, 0.3f, 0.1f );
  setHighlightedBorderColor( 1.0f, 0.8f, 0.4f );
  setFocusColor( 0.8f, 0.5f, 0.2f, 1 );
  visible = true;
  enabled = true;
  debug = false;
  focus = false;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  strcpy( tooltip, "" );
  tooltipTicks = 0;
  tooltipShowing = false;
  invalid = true;
  displayList = glGenLists( 1 );
  if( !displayList ) {
    cerr << "*** Error: couldn't generate display list for gui." << endl;
    exit(1);
  }
}

Widget::~Widget() {
  glDeleteLists( displayList, 1 );
}

void Widget::draw(Widget *parent) {
  glTranslated( x, y, 0 );
  if(hasFocus()) {
    GuiTheme *theme = ((Window*)parent)->getTheme();
    if( theme->getSelectedBorder() ) {
      glColor4f( theme->getSelectedBorder()->color.r,
                 theme->getSelectedBorder()->color.g,
                 theme->getSelectedBorder()->color.b,
                 theme->getSelectedBorder()->color.a );
    } else {
      applyFocusColor();
    }
    glLineWidth( 2.0f );
    glBegin(GL_LINES);
    glVertex2f( -1, -1 );
    glVertex2f( -1, getHeight() + 1 );
    
    glVertex2f( -1, -1 );
    glVertex2f( getWidth() + 2, -1 );
    
    glVertex2f( -1, getHeight() + 1 );
    glVertex2f( getWidth() + 2, getHeight() + 1 );
    
    glVertex2f( getWidth() + 2, -1 );
    glVertex2f( getWidth() + 2, getHeight() + 1 );
    glEnd();
    glLineWidth( 1.0f );
  }
  //if( invalid ) {
    //glNewList( displayList, GL_COMPILE );
    drawWidget(parent);
    //glEndList();
    //invalid = false;
  //}
  //glCallList( displayList );
}

bool Widget::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  // do nothing by default
  return false;
}

void Widget::removeEffects(Widget *parent) {
}

bool Widget::isInside(int x, int y) {
  return(x >= getX() && x < getX() + w &&
  		  y >= getY() && y < getY() + h);
}

void Widget::drawButton( Widget *parent, int x, int y, int x2, int y2, 
                         bool toggle, bool selected, bool inverse, 
                         bool glowing, bool inside ) {
  GuiTheme *theme = ((Window*)parent)->getTheme();

  glPushMatrix();
  glTranslatef( x, y, 0 );

  if(isTranslucent()) {
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
    glEnable( GL_BLEND );
  }

  int n = 0;
  GLuint tex;
	if(toggle && selected) {
    if( theme->getButtonSelectionBackground() ) {
      glEnable( GL_TEXTURE_2D );
      tex = theme->getButtonSelectionBackground()->texture;
      glBindTexture( GL_TEXTURE_2D, theme->getButtonSelectionBackground()->texture );
      if( isEnabled() ) {
	      glColor4f( theme->getButtonSelectionBackground()->color.r,
	                 theme->getButtonSelectionBackground()->color.g,
	                 theme->getButtonSelectionBackground()->color.b,
	                 theme->getButtonSelectionBackground()->color.a );
			} else {
	      glColor4f( theme->getButtonSelectionBackground()->color.r / 2,
	                 theme->getButtonSelectionBackground()->color.g / 2,
	                 theme->getButtonSelectionBackground()->color.b / 2,
	                 theme->getButtonSelectionBackground()->color.a / 2.0f );			
			}
      n = theme->getButtonSelectionBackground()->width;
    } else {
      applySelectionColor();
    }
  } else if( theme->getButtonBackground() ) {
    glEnable( GL_TEXTURE_2D );
    tex = theme->getButtonBackground()->texture;
    glBindTexture( GL_TEXTURE_2D, theme->getButtonBackground()->texture );
    if( isEnabled() ) {
	    glColor4f( theme->getButtonBackground()->color.r, 
	               theme->getButtonBackground()->color.g, 
	               theme->getButtonBackground()->color.b, 
	               theme->getButtonBackground()->color.a );
		} else {
			    glColor4f( theme->getButtonBackground()->color.r / 2, 
	               theme->getButtonBackground()->color.g / 2, 
	               theme->getButtonBackground()->color.b / 2, 
	               theme->getButtonBackground()->color.a / 2 );
		}
    n = theme->getButtonBackground()->width;
  } else {
    applyBackgroundColor(true);
  }

  if( inverse ) {
    glBegin(GL_QUADS);
    glTexCoord2f(1, 1);
    glVertex2d(n, n);
    glTexCoord2f(1, 0);
    glVertex2d(n, y2 - y - n);    
    glTexCoord2f(0, 0);
    glVertex2d(x2 - x - n, y2 - y - n);
    glTexCoord2f(0, 1);
    glVertex2d(x2 - x - n, n);
    glEnd();
  } else {
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2d(n, n);
    glTexCoord2f(0, 1);
    glVertex2d(n, y2 - y - n);
    glTexCoord2f(1, 1);
    glVertex2d(x2 - x - n, y2 - y - n);
    glTexCoord2f(1, 0);
    glVertex2d(x2 - x - n, n);
    glEnd();
  }

  if( n ) {
    glPushMatrix();
    if( toggle && selected ) {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonSelectionBackground()->tex_south :
                       theme->getButtonSelectionBackground()->tex_north ) );
    } else {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonBackground()->tex_south :
                       theme->getButtonBackground()->tex_north ) );
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2d(0, 0);
    glTexCoord2f(0, 1);
    glVertex2d(0, n);
    glTexCoord2f(1, 1);
    glVertex2d(x2 - x, n);
    glTexCoord2f(1, 0);
    glVertex2d(x2 - x, 0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef( 0, y2 - y - n, 0 );
    if( toggle && selected ) {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonSelectionBackground()->tex_north :
                       theme->getButtonSelectionBackground()->tex_south ) );
    } else {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonBackground()->tex_north : 
                       theme->getButtonBackground()->tex_south ) );
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2d(0, 0);
    glTexCoord2f(0, 1);
    glVertex2d(0, n);
    glTexCoord2f(1, 1);
    glVertex2d(x2 - x, n);
    glTexCoord2f(1, 0);
    glVertex2d(x2 - x, 0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef( 0, n, 0 );
    if( toggle && selected ) {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonSelectionBackground()->tex_east :
                       theme->getButtonSelectionBackground()->tex_west ) );
    } else {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonBackground()->tex_east : 
                       theme->getButtonBackground()->tex_west ) );
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2d(0, 0);
    glTexCoord2f(0, 1);
    glVertex2d(0, y2 - y - ( 2 * n ));
    glTexCoord2f(1, 1);
    glVertex2d(n, y2 - y - ( 2 * n ));
    glTexCoord2f(1, 0);
    glVertex2d(n, 0);
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glTranslatef( x2 - x - n, n, 0 );
    if( toggle && selected ) {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonSelectionBackground()->tex_west :
                       theme->getButtonSelectionBackground()->tex_east ) );
    } else {
      glBindTexture( GL_TEXTURE_2D, 
                     ( inverse ? theme->getButtonBackground()->tex_west :
                       theme->getButtonBackground()->tex_east ) );
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2d(0, 0);
    glTexCoord2f(0, 1);
    glVertex2d(0, y2 - y - ( 2 * n ));
    glTexCoord2f(1, 1);
    glVertex2d(n, y2 - y - ( 2 * n ));
    glTexCoord2f(1, 0);
    glVertex2d(n, 0);
    glEnd();
    glPopMatrix();
  }

  glDisable( GL_TEXTURE_2D );
  if(isTranslucent()) {
    glDisable( GL_BLEND );
  }

  GLint t = SDL_GetTicks();
  if(lastTick == 0 || t - lastTick > 50) {
    lastTick = t;
    alpha += alphaInc;
    if(alpha >= 0.7f || alpha < 0.4f) alphaInc *= -1.0f;
  }


  // glowing red
  if(glowing) {
    if( theme->getButtonHighlight() ) {
      glEnable( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, theme->getButtonHighlight()->texture );
    }
    // FIXME: use theme
    glColor4f( 1, 0.15f, 0.15f, alpha );    
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d(n, n);
    glTexCoord2f( 0, 1 );
    glVertex2d(n, y2 - y - n);
    glTexCoord2f( 1, 1 );
    glVertex2d(x2 - x - n, y2 - y - n);
    glTexCoord2f( 1, 0 );
    glVertex2d(x2 - x - n, n);
    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
  }


  if( inside && isEnabled() ) {
    if( theme->getButtonHighlight() ) {
      glEnable( GL_TEXTURE_2D );
      glColor4f( theme->getButtonHighlight()->color.r, 
                 theme->getButtonHighlight()->color.g, 
                 theme->getButtonHighlight()->color.b, 
                 alpha );
      glBindTexture( GL_TEXTURE_2D, theme->getButtonHighlight()->texture );
    }
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2d(n, n);
    glTexCoord2f( 0, 1 );
    glVertex2d(n, y2 - y - n);
    glTexCoord2f( 1, 1 );
    glVertex2d(x2 - x - n, y2 - y - n);
    glTexCoord2f( 1, 0 );
    glVertex2d(x2 - x - n, n);
    glEnd();
    glDisable( GL_BLEND );
    glDisable( GL_TEXTURE_2D );
  }

  if( theme->getButtonBorder() ) {
    glColor4f( theme->getButtonBorder()->color.r,
               theme->getButtonBorder()->color.g,
               theme->getButtonBorder()->color.b,
               theme->getButtonBorder()->color.a );
  }
  glBegin( GL_LINE_LOOP );
  glVertex2d(0, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, y2 - y);
  glVertex2d(x2 - x, 0);

  /*
  glVertex2d(0, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, 0);
  glVertex2d(x2 - x, y2 - y);
  glVertex2d(0, 0);
  glVertex2d(x2 - x, 0);
  glVertex2d(0, y2 - y);
  glVertex2d(x2 - x, y2 - y);
  */

  glEnd();

  glPopMatrix();
}

void Widget::breakText( char *text, int lineWidth, vector<string> *lines ) {
  char *p =  new char[ lineWidth + 3 ];
  int len = strlen(text);
  int start = 0;
  //cerr << "len=" << len << endl;
  //cerr << "text=" << text << endl;
  while(start < len) {
    int end = start + lineWidth;
    if(end > len) end = len;
    else {
      // find a space (if any)
      int n = end;
      while(n > start && *(text + n) != ' ') n--;
      if(n > start) end = n + 1;
    }
    
    // search for hard breaks
    int n = start;
    while(n < end && *(text + n) != '|') n++;
    bool hardbreak = false;
    if( n < end) {
      end = n + 1;
      hardbreak = true;
    }
    
    //cerr << "\tstart=" << start << endl;
    //cerr << "\tend=" << end << endl;
    //cerr << "\tend-start=" << (end-start) << endl;
    
    strncpy(p, text + start, end - start);
    *(p + end - (hardbreak ? 1 : 0) - start) = 0;
    //cerr << "p=" << p << endl;

    string s = p;
    lines->push_back( s );
    start = end;
  }
  delete [] p;
}

void Widget::drawTooltip( Widget *parent ) {
  int xpos = ((Window*)parent)->getScourgeGui()->getMouseX() -  parent->getX();
  int ypos = ((Window*)parent)->getScourgeGui()->getMouseY() -  parent->getY() - ((Window*)parent)->getGutter();
  bool b = isInside( xpos, ypos );

  if( !( tooltipShowing && b ) ) {
    if( !b ) {
      tooltipShowing = b;
      tooltipTicks = 0;
    } else {
      if( !tooltipTicks ) tooltipTicks = SDL_GetTicks();
      else if( SDL_GetTicks() - tooltipTicks > TOOLTIP_DELAY ) {
        tooltipShowing = b;
      }
    }
  }
  if( !tooltipShowing || !strlen( tooltip ) ) return;
  ((Window*)parent)->getScourgeGui()->drawTooltip( xpos, ypos, 450, 0, 0, tooltip ); 
}

/**
 * Draw a rectangle with a texture, such that only the middle of the texture
 * is stretched.
 * 
 * texture the texture to use
 * x,y,w,h the dimensions of the quad
 * left,right the size of the border area
 * textureWidth the size of the texture used
 * 
 * 3 quads will be drawn like this:
 * +--+-----+--+
 * |  |     |  |
 * |A |  B  |C |
 * |  |     |  |
 * +--+-----+--+
 * 
 * The texture is only stretched on quad B. This assumes that 
 * the height changes of the quad is not as important as stretching
 * it horizontally. This is generally true for buttons, progress bars, etc.
 */
void Widget::drawBorderedTexture( GLuint texture, int x, int y, int width, int height, 
                                  int left, int right, int textureWidth, bool inverse ) {
  
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_ALPHA_TEST );    
  glAlphaFunc( GL_GREATER, 0 );
  glBindTexture( GL_TEXTURE_2D, texture );
  
  glPushMatrix();
  glTranslatef( x, y, 0 );
  glBegin( GL_QUADS );
  if( inverse ) {
    // quad A
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 1 );
    glVertex3f( 0, 0, 0 );
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 0 );
    glVertex3f( 0, height, 0);
    glTexCoord2f( 0, 0 );    
    glVertex3f( left, height, 0 );
    glTexCoord2f( 0, 1 );
    glVertex3f( left, 0, 0 );
  
    // quad B
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 1 );
    glVertex3f( left, 0, 0 );
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 0 );
    glVertex3f( left, height, 0);
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 0 );
    glVertex3f( width - right, height, 0 );
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 1 );
    glVertex3f( width - right, 0, 0 );
  
    // quad C
    glTexCoord2f( 1, 1 );
    glVertex3f( width - right, 0, 0 );
    glTexCoord2f( 1, 0 );
    glVertex3f( width - right, height, 0);
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 0 );
    glVertex3f( width, height, 0 );
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 1 );
    glVertex3f( width, 0, 0 );
  } else {
    // quad A
    glTexCoord2f( 0, 0 );
    glVertex3f( 0, 0, 0 );
    glTexCoord2f( 0, 1 );
    glVertex3f( 0, height, 0);
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 1 );
    glVertex3f( left, height, 0 );
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 0 );
    glVertex3f( left, 0, 0 );
  
    // quad B
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 0 );
    glVertex3f( left, 0, 0 );
    glTexCoord2f( static_cast<float>(left) / static_cast<float>(textureWidth), 1 );
    glVertex3f( left, height, 0);
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 1 );
    glVertex3f( width - right, height, 0 );
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 0 );
    glVertex3f( width - right, 0, 0 );
  
    // quad C
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 0 );
    glVertex3f( width - right, 0, 0 );
    glTexCoord2f( 1.0f - (static_cast<float>(right) / static_cast<float>(textureWidth)), 1 );
    glVertex3f( width - right, height, 0);
    glTexCoord2f( 1, 1 );
    glVertex3f( width, height, 0 );
    glTexCoord2f( 1, 0 );
    glVertex3f( width, 0, 0 );
  }
  glEnd();

  /* debugging only
  glDisable( GL_TEXTURE_2D );
  glColor4f( 1, 0, 0, 1 );
  glBegin( GL_LINE_LOOP );
  
  // quad A
  glVertex3f( 0, 0, 0 );
  glVertex3f( 0, height, 0);
  glVertex3f( left, height, 0 );
  glVertex3f( left, 0, 0 );

  // quad B
  glVertex3f( left, 0, 0 );
  glVertex3f( left, height, 0);
  glVertex3f( width - right, height, 0 );
  glVertex3f( width - right, 0, 0 );

  // quad C
  glVertex3f( width - right, 0, 0 );
  glVertex3f( width - right, height, 0);
  glVertex3f( width, height, 0 );
  glVertex3f( width, 0, 0 );
  
  glEnd();
  */

  glPopMatrix();

}

int Widget::getTextWidth( Widget *parent, const char *s ) {
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

