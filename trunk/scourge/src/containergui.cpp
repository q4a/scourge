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
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "sound.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"

using namespace std;

ContainerGui::ContainerGui(Scourge *scourge, Item *container, int x, int y) {
  this->scourge = scourge;
  this->container = container;

  //win = scourge->createWoodWindow( x, y, 320, 300, container->getItemName() );
  win = new Window( scourge->getSDLHandler(),
                    x, y, 320, 300, container->getItemName(), true, 
                    Window::SIMPLE_WINDOW, "wood" );
  openButton = new Button( 5, 5, 105, 25, scourge->getShapePalette()->getHighlightTexture(), Constants::getMessage(Constants::OPEN_CONTAINER_LABEL) );
  win->addWidget((Widget*)openButton);
  infoButton = new Button( 110, 5, 210, 25, scourge->getShapePalette()->getHighlightTexture(), _( "Info" ) );
  win->addWidget((Widget*)infoButton);
  closeButton = new Button( 215, 5, 315, 25, scourge->getShapePalette()->getHighlightTexture(), _( "Close" ) );
  win->addWidget((Widget*)closeButton);

  list = new ScrollingList( 10, 35, 300, 245 - 30, 
                            scourge->getShapePalette()->getHighlightTexture(), this, 30 );
  win->addWidget((Widget*)list);
  label = new Label(5, 270, Constants::getMessage(Constants::EXPLAIN_DRAG_AND_DROP));
  win->addWidget(label);

  // allocate memory for the contained item descriptions
  this->containedItemNames = (char**)malloc(MAX_CONTAINED_ITEMS * sizeof(char*));
  this->itemColor = (Color*)malloc(MAX_INVENTORY_SIZE * sizeof(Color));
  this->itemIcon = (GLuint*)malloc(MAX_INVENTORY_SIZE * sizeof(GLuint));
  for(int i = 0; i < MAX_CONTAINED_ITEMS; i++) {
    this->containedItemNames[i] = (char*)malloc(120 * sizeof(char));
  }

  showContents();

  win->setVisible(true);
}

ContainerGui::~ContainerGui() {
  for(int i = 0; i < MAX_CONTAINED_ITEMS; i++) {
    free(containedItemNames[i]);
  }
  free(containedItemNames);
  free(itemColor);
  free(itemIcon);

  //delete label;
  //delete list;
  //delete openButton;
  delete win;
}

void ContainerGui::showContents() {
  for(int i = 0; i < container->getContainedItemCount(); i++) {
    container->getContainedItem(i)->getDetailedDescription(containedItemNames[i]);
    if( !container->getContainedItem(i)->isMagicItem() ) {
      itemColor[i].r = 1;
      itemColor[i].g = 1;
      itemColor[i].b = 1;
    } else {
      itemColor[i].r = Constants::MAGIC_ITEM_COLOR[ container->getContainedItem(i)->getMagicLevel() ]->r;
      itemColor[i].g = Constants::MAGIC_ITEM_COLOR[ container->getContainedItem(i)->getMagicLevel() ]->g;
      itemColor[i].b = Constants::MAGIC_ITEM_COLOR[ container->getContainedItem(i)->getMagicLevel() ]->b;
    }
    itemColor[i].a = 1;
    itemIcon[i] = scourge->getShapePalette()->tilesTex[ container->getContainedItem(i)->getRpgItem()->getIconTileX() ][ container->getContainedItem(i)->getRpgItem()->getIconTileY() ];
  }
  list->setLines( container->getContainedItemCount(), 
                  (const char **)containedItemNames,
                  itemColor,
                  itemIcon );
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
	if(widget == win->closeButton || widget == closeButton) {
		win->setVisible(false);
		return true;
  } else if(widget == list && scourge->getTargetSelectionFor() ) {
    int itemIndex = list->getSelectedLine();  
    if(itemIndex > -1) {
      Item *item = container->getContainedItem(itemIndex);
      scourge->handleTargetSelectionOfItem( item );
    }
  } else if(widget == infoButton || 
            (widget == list && scourge->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT)) {
      int itemIndex = list->getSelectedLine();  
      if(itemIndex > -1) {
        Item *item = container->getContainedItem(itemIndex);
        scourge->getInfoGui()->setItem( item );
        if(!scourge->getInfoGui()->getWindow()->isVisible()) 
          scourge->getInfoGui()->getWindow()->setVisible( true );
      }
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
	  sprintf(message, _( "%1$s is placed in %2$s." ), 
			  scourge->getMovingItem()->getItemName(), 
			  container->getItemName());
	  scourge->getMap()->addDescription(message);	  
	  scourge->endItemDrag();
	  showContents();
    scourge->getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
	} else {
	  // message: the container is full
    scourge->getSDLHandler()->getSound()->playSound(Window::DROP_FAILED);
    scourge->showMessageDialog( _( "The item won't fit in that container!" ) );
	}
  }
}

bool ContainerGui::startDrag(Widget *widget, int x, int y) {
  dropItem();
  return true;
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

