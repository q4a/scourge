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
  dropButton = new Button( 195, 40, 295, 70, Constants::getMessage(Constants::DROP_ITEM_LABEL) );
  win->addWidget((Widget*)dropButton);
  openButton = new Button( 195, 75, 295, 105, Constants::getMessage(Constants::OPEN_CONTAINER_LABEL) );
  win->addWidget((Widget*)openButton);
  list = new ScrollingList(5, 5, 185, 275 - (Window::TOP_HEIGHT + Window::BOTTOM_HEIGHT + 5), this);
  win->addWidget((Widget*)list);
  label = new Label(5, 265, Constants::getMessage(Constants::EXPLAIN_DRAG_AND_DROP));
  win->addWidget(label);

  // allocate memory for the contained item descriptions
  this->containedItemNames = (char**)malloc(Item::MAX_CONTAINED_ITEMS * sizeof(char*));
  for(int i = 0; i < Item::MAX_CONTAINED_ITEMS; i++) {
	this->containedItemNames[i] = (char*)malloc(120 * sizeof(char));
  }

  showContents();

  win->setVisible(true);
}

ContainerGui::~ContainerGui() {
  for(int i = 0; i < Item::MAX_CONTAINED_ITEMS; i++) {
	free(containedItemNames[i]);
  }
  free(containedItemNames);

  delete label;
  delete list;
  delete closeButton;
  delete dropButton;
  delete openButton;
  delete win;
}

void ContainerGui::showContents() {
  for(int i = 0; i < container->getContainedItemCount(); i++) {
	container->getContainedItem(i)->getDetailedDescription(containedItemNames[i]);
  }
  list->setLines(container->getContainedItemCount(), 
				 (const char **)containedItemNames);
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
  } else if(widget == dropButton) {
	dropItem();
  } else if(widget == openButton) {
	int n = list->getSelectedLine();
	if(n > -1 && 
	   container->getContainedItem(n)->getRpgItem()->getType() == RpgItem::CONTAINER) {	
	  scourge->openContainerGui(container->getContainedItem(n));
	}
  }
  return false;
}

void ContainerGui::receive(Widget *widget) {
  char message[120];
  if(scourge->getMovingItem() && 
	 scourge->getMovingItem() != container) {
	if(container->addContainedItem(scourge->getMovingItem())) {
	  // message: the container accepted the item
	  sprintf(message, "%s is placed in %s.", 
			  scourge->getMovingItem()->getRpgItem()->getName(), 
			  container->getRpgItem()->getName());
	  scourge->getMap()->addDescription(message);	  
	  scourge->endItemDrag();
	  showContents();
	} else {
	  // message: the container is full
	}
  }
}

void ContainerGui::startDrag(Widget *widget) {
  dropItem();
}

void ContainerGui::dropItem() {
  int n = list->getSelectedLine();
  if(n > -1) {
	Item *item = container->removeContainedItem(n);
	if(item) {
	  scourge->startItemDragFromGui(item);
	  showContents();
	}
  }
}

