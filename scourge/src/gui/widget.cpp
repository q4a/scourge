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
#include "../sdlhandler.h"

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
  debug = false;
  focus = false;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  strcpy( tooltip, "" );
  tooltipTicks = 0;
  tooltipShowing = false;
}

Widget::~Widget() {
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
    glVertex2f( -1, getHeight() + 2 );
    
    glVertex2f( -1, -1 );
    glVertex2f( getWidth() + 2, -1 );
    
    glVertex2f( -1, getHeight() + 2 );
    glVertex2f( getWidth() + 2, getHeight() + 2 );
    
    glVertex2f( getWidth() + 2, -1 );
    glVertex2f( getWidth() + 2, getHeight() + 2 );
    glEnd();
    glLineWidth( 1.0f );
  }
  drawWidget(parent);
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
  if(toggle && selected) {
    if( theme->getButtonSelectionBackground() ) {
      glEnable( GL_TEXTURE_2D );
      glBindTexture( GL_TEXTURE_2D, theme->getButtonSelectionBackground()->texture );
      glColor4f( theme->getButtonSelectionBackground()->color.r,
                 theme->getButtonSelectionBackground()->color.g,
                 theme->getButtonSelectionBackground()->color.b,
                 theme->getButtonSelectionBackground()->color.a );
      n = theme->getButtonSelectionBackground()->width;
    } else {
      applySelectionColor();
    }
  } else if( theme->getButtonBackground() ) {
    glEnable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, theme->getButtonBackground()->texture );
    glColor4f( theme->getButtonBackground()->color.r, 
               theme->getButtonBackground()->color.g, 
               theme->getButtonBackground()->color.b, 
               theme->getButtonBackground()->color.a );
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
    glColor4f( 1, 0.15, 0.15, alpha );    
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


  if(inside) {
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
  char *p = (char*)malloc((lineWidth + 3) * sizeof(char));
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
  free(p);
}

void Widget::drawTooltip( Widget *parent ) {
  int xpos = ((Window*)parent)->getSDLHandler()->mouseX -  parent->getX();
  int ypos = ((Window*)parent)->getSDLHandler()->mouseY -  parent->getY() - Window::TOP_HEIGHT;
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
  ((Window*)parent)->getSDLHandler()->drawTooltip( xpos, ypos, 450, 0, 0, tooltip ); 
}

