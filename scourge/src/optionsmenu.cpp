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
  mainWin = new Window( scourge->getSDLHandler(),
						100, 50, 525, 505, 
						strdup("Options"), 
						scourge->getShapePalette()->getGuiTexture() );
}

OptionsMenu::~OptionsMenu(){
}

bool OptionsMenu::handleEvent(Widget *widget, SDL_Event *event) {
  return false;
}

bool OptionsMenu::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    break;
  case SDL_KEYDOWN:
    switch(event->key.keysym.sym) {
	case SDLK_ESCAPE:
	  hide();
	  return true;
    default: break;
    }
  default: break;  
  }  
  return false;
}
