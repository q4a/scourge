/***************************************************************************
                          textfield.cpp  -  description
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

#include "textfield.h"

/**
  *@author Gabor Torok
  */
  
#define AVG_CHAR_WIDTH 6  
#define OFFSET 5

TextField::TextField(int x, int y, int numChars):
  Widget( x, y, x + numChars * AVG_CHAR_WIDTH, 16 ) {
  this->inside = false;
  this->numChars = numChars;
  this->text = (char*)malloc((numChars + 1) * sizeof(char));
  this->pos = 0;
  this->maxPos = 0;
} 

TextField::~TextField() {
  free(text);
}
  
bool TextField::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  inside = hasFocus();
  // handle it
  if(inside) {
    switch( event->type ) {
    case SDL_KEYDOWN:
    //case SDL_KEYUP:
    //cerr << "key: " << event->key.keysym.sym << " maxPos=" << maxPos << " pos=" << pos << " numChars=" << numChars << endl;
    if(event->key.keysym.sym >= SDLK_SPACE && event->key.keysym.sym <= SDLK_z && 
       maxPos < numChars) {
      for(int i = maxPos; i > pos; i--) {
        text[i] = text[i - 1];
      }
      text[pos] = event->key.keysym.sym;
      pos++;
      maxPos++;
    } else if((event->key.keysym.sym == SDLK_BACKSPACE && pos > 0) ||
              (event->key.keysym.sym == SDLK_DELETE && pos < maxPos)) {
      for(int i = (event->key.keysym.sym == SDLK_BACKSPACE ? pos - 1 : pos); i < maxPos - 1; i++) {
        text[i] = text[i + 1];
      }
      if(event->key.keysym.sym == SDLK_BACKSPACE) pos--;
      maxPos--;
    } else if(event->key.keysym.sym == SDLK_LEFT && pos > 0) {
      pos--;
    } else if(event->key.keysym.sym == SDLK_RIGHT && pos < maxPos) {
      pos++;
    } else if(event->key.keysym.sym == SDLK_HOME) {
      pos = 0;
    } else if(event->key.keysym.sym == SDLK_END) {
      pos = maxPos;
    }
    break;
    default:
    break;
    }
  }
  return false;
}

void TextField::drawWidget(Widget *parent) {
  
  //glColor3f( 1, 1, 1 );
  applyHighlightedBorderColor();
  glPushMatrix();
  glBegin(GL_QUADS);
  glVertex2d(0, 0);
  glVertex2d(0, getHeight());
  glVertex2d(getWidth(), getHeight());
  glVertex2d(getWidth(), 0);
  glEnd();
  glPopMatrix();

  applyColor();
//  ((Window*)parent)->getSDLHandler()->texPrintMono(OFFSET, 12, getText());
  
  char letter[2];
  for(int i = 0; i < maxPos; i++) {
    glPushMatrix();
    glTranslatef( OFFSET + i * AVG_CHAR_WIDTH, 0, 0 );
    letter[0] = text[i];
    letter[1] = '\0';
    ((Window*)parent)->getSDLHandler()->texPrintMono(0, 12, letter);
    glPopMatrix();
  }

  // border
  applyBorderColor();
  glPushMatrix();
  glBegin(GL_LINES);
  glVertex2d(0, 0);
  glVertex2d(0, getHeight());
  glVertex2d(getWidth(), 0);
  glVertex2d(getWidth(), getHeight());
  glVertex2d(0, 0);
  glVertex2d(getWidth(), 0);
  glVertex2d(0, getHeight());
  glVertex2d(getWidth(), getHeight());
  glEnd();
  glPopMatrix();
  if(inside) {
    glLineWidth( 3.0f );
    // cursor
    glLineWidth( 2.0f );
    glPushMatrix();
    glTranslatef( OFFSET + pos * AVG_CHAR_WIDTH + 1, 0, 0 );    
    glBegin(GL_LINES);
    glVertex2d(0, 0);
    glVertex2d(0, getHeight());
    glEnd();
    glPopMatrix();
    glLineWidth( 1.0f );
  }
}

