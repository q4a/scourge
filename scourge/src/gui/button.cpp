/***************************************************************************
                          button.cpp  -  description
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
#include "button.h"
#include "window.h"
#include "guitheme.h"

/**
  *@author Gabor Torok
  */

Button::Button(int x1, int y1, int x2, int y2, GLuint highlight, char *label) : 
  Widget(x1, y1, x2 - x1, y2 - y1) {
  this->x2 = x2;
  this->y2 = y2;
  setLabel( label );
  labelPos = CENTER;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
  toggle = selected = false;
  inside = false;
  this->highlight = highlight;
  this->glowing = false;
  this->inverse = false;
}

Button::~Button() {
}

void Button::drawWidget(Widget *parent) {
  GuiTheme *theme = ((Window*)parent)->getTheme();

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

  if( strlen( label ) ) {
    glPushMatrix();
    int ypos;
    switch(getLabelPosition()) {
    case TOP: ypos = 13; break;
    case BOTTOM: ypos = (y2 - y) - 2; break;
    default: ypos = (y2 - y) / 2 + 5;
    }
    glTranslated( 5, ypos, 0);
    if( selected && theme->getButtonSelectionText() ) {
      glColor4f( theme->getButtonSelectionText()->r,
                 theme->getButtonSelectionText()->g,
                 theme->getButtonSelectionText()->b,
                 theme->getButtonSelectionText()->a );
    } else if( theme->getButtonText() ) {
      glColor4f( theme->getButtonText()->r,
                 theme->getButtonText()->g,
                 theme->getButtonText()->b,
                 theme->getButtonText()->a );
    } else {
      applyColor();      
    }
    ((Window*)parent)->getSDLHandler()->texPrint(0, 0, label);
    glPopMatrix();
  }
}

bool Button::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inverse = false;
  inside = isInside(x, y);
  if(inside) ((Window*)parent)->setLastWidget(this);
  // handle it
  switch( event->type ) {
  case SDL_KEYUP:
  if(hasFocus()) {
    if(event->key.keysym.sym == SDLK_RETURN) {
      if(toggle) selected = (selected ? false : true);
      return true;
    }
  }
  break;
  case SDL_MOUSEMOTION:
  inverse = ( inside && event->motion.state == SDL_PRESSED );
	break;
  case SDL_MOUSEBUTTONUP:
	if(inside && toggle) selected = (selected ? false : true);
	return inside;
  case SDL_MOUSEBUTTONDOWN:
  inverse = inside;
	break;
  default:
	break;
  }
  return false;
}

void Button::removeEffects(Widget *parent) {
  inside = false;
}

