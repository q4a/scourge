/***************************************************************************
                          containergui.cpp -  description
                             -------------------
    begin                : Sat May 3 2003
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

#include "containergui.h"

ContainerGui::ContainerGui(Scourge *scourge, Item *container, int id, int x, int y) {
  this->scourge = scourge;
  this->container = container;
  this->id = id;

  win = new Window( scourge->getSDLHandler(),
						x, y, 300, 300, 
						container->getRpgItem()->getName(), 
						scourge->getShapePalette()->getGuiTexture() );
  closeButton = new Button( 195, 5, 295, 35, Constants::getMessage(Constants::CLOSE) );
  win->addWidget((Widget*)closeButton);

  win->setVisible(true);
}

ContainerGui::~ContainerGui() {
  delete closeButton;
  delete win;
}

bool ContainerGui::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
	break;     
  case SDL_KEYUP:
	switch(event->key.keysym.sym) {
	case SDLK_ESCAPE: 
	  win->setVisible(false);
	  return true;
	default: break;
	}
  default: break;
  }
  return false;
}

bool ContainerGui::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == closeButton) {
	win->setVisible(false);
	return true;
  }
  return false;
}
