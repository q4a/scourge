/***************************************************************************
                          optionsmenu.cpp  -  description
                             -------------------
    begin                : Tue Aug 12 2003
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

#include "optionsmenu.h"

OptionsMenu::OptionsMenu(Scourge *scourge){
  this->scourge = scourge;
  this->win = -1;  
}
OptionsMenu::~OptionsMenu(){
}

void OptionsMenu::show() {
  scourge->getGui()->pushWindows();
  win = scourge->getGui()->addWindow(0, 0,
									 scourge->getSDLHandler()->getScreen()->w,
									 scourge->getSDLHandler()->getScreen()->h,
									 &Gui::drawOptionsMenu);
  scourge->getGui()->addActiveRegion(60, 245, 310, 265, Constants::MENU_0, this);

  // switch handlers
  scourge->getSDLHandler()->pushHandlers(this, this);
}

void OptionsMenu::drawView(SDL_Surface *screen) {
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
  glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
  glClearDepth( 1.0f );

  // draw the logo
  glPushMatrix();
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_NOTEQUAL, 0);        
  glPixelZoom( 1.0, -1.0 );
  glRasterPos2f( 10, 10 );
  glDrawPixels(scourge->getShapePalette()->logo->w, 
			   scourge->getShapePalette()->logo->h,
			   GL_BGRA, GL_UNSIGNED_BYTE, scourge->getShapePalette()->logoImage);
  glDisable(GL_ALPHA_TEST);

  glPopMatrix();
  glEnable( GL_TEXTURE_2D );
  glEnable(GL_DEPTH_TEST);

  // draw the window
  scourge->getGui()->drawWindows();
}

void OptionsMenu::drawMenu(int x, int y) {
  scourge->getGui()->outlineActiveRegion(Constants::MENU_0, "Video Options");
}

bool OptionsMenu::handleEvent(Widget *widget, SDL_Event *event) {
  return false;
}

bool OptionsMenu::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    if(event->button.button) {
        int region = scourge->getGui()->testActiveRegions(event->button.x, event->button.y);
		fprintf(stderr, "region=%d\n", region);
		break;
    }
    break;
  case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
	case SDLK_ESCAPE:
	  // pop windows
	  scourge->getGui()->popWindows();
	  return true;
    default: break;
    }
  default: break;  
  }  
  return false;
}
