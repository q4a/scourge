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

ContainerGui::ContainerGui(Scourge *scourge, Item *container, int x, int y) {
  this->scourge = scourge;
  this->container = container;

  win = new Window( scourge->getSDLHandler(),
						x, y, 300, 300, 
						container->getRpgItem()->getName(), 
						scourge->getShapePalette()->getGuiTexture() );
  closeButton = new Button( 195, 5, 295, 35, Constants::getMessage(Constants::CLOSE_LABEL) );
  win->addWidget((Widget*)closeButton);
  player1Button = new Button( 195, 40, 295, 70, scourge->getParty(0)->getName() );
  win->addWidget((Widget*)player1Button);
  player2Button = new Button( 195, 75, 295, 105, scourge->getParty(1)->getName() );
  win->addWidget((Widget*)player2Button);
  player3Button = new Button( 195, 110, 295, 140, scourge->getParty(2)->getName() );
  win->addWidget((Widget*)player3Button);
  player4Button = new Button( 195, 145, 295, 175, scourge->getParty(3)->getName() );
  win->addWidget((Widget*)player4Button);
  dropButton = new Button( 195, 180, 295, 210, Constants::getMessage(Constants::DROP_ITEM_LABEL) );
  win->addWidget((Widget*)dropButton);
  openButton = new Button( 195, 215, 295, 245, Constants::getMessage(Constants::OPEN_CONTAINER_LABEL) );
  win->addWidget((Widget*)openButton);
  list = new ScrollingList(5, 5, 185, 295 - (Window::TOP_HEIGHT + Window::BOTTOM_HEIGHT + 5));
  win->addWidget((Widget*)list);

  
  //  setLines(int count, const char *s[])

  win->setVisible(true);
}

ContainerGui::~ContainerGui() {
  delete list;
  delete closeButton;
  delete dropButton;
  delete openButton;
  delete player1Button;
  delete player2Button;
  delete player3Button;
  delete player4Button;
  delete win;
}

bool ContainerGui::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
	break;     
  case SDL_KEYUP:
	switch(event->key.keysym.sym) {
	  //	case SDLK_ESCAPE: 
	  //	  win->setVisible(false);
	  //	  return true;
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
