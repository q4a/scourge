/***************************************************************************
                          infogui.cpp  -  description
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

#include "infogui.h"
#include "item.h"
#include "shapepalette.h"
#include "rpg/rpglib.h"

InfoGui::InfoGui(Scourge *scourge) {
  this->scourge = scourge;

  int width = 350;
  int height = 400;

  int x = (scourge->getSDLHandler()->getScreen()->w - width) / 2;
  int y = (scourge->getSDLHandler()->getScreen()->h - height) / 2;

  win = scourge->createWindow( x, y, width, height, Constants::getMessage(Constants::INFO_GUI_TITLE) );
  int bx = width / 2 - 52;
  int by = height - (30 + Window::BOTTOM_HEIGHT + Window::TOP_HEIGHT);
  openButton = new Button( bx, by, bx + 105, by + 25, 
                           scourge->getShapePalette()->getHighlightTexture(), 
                           Constants::getMessage(Constants::CLOSE_LABEL) );
  win->addWidget((Widget*)openButton);
  
  int n = 48;
  image = new Canvas( width - n - 10, 10, width - 10, 10 + n, this );
  win->addWidget( image );

  win->createLabel(10, 10, "Name:", Constants::RED_COLOR);
  strcpy(name, "");
  nameLabel = new Label(10, 25, name, 56);
  win->addWidget(nameLabel);

  win->createLabel(10, 80, "Detailed Description:", Constants::RED_COLOR);
  strcpy(description, "");
  label = new ScrollingLabel( 10, 95, width - 20, by - 105, description );
  win->addWidget(label);
}

InfoGui::~InfoGui() {
  delete win;
}

void InfoGui::setItem(Item *item, int level) { 
  this->item = item; 
  this->setInfoDetailLevel( level );
}

void InfoGui::setInfoDetailLevel(int level) { 
  infoDetailLevel = level; 
  describe(); 
}

bool InfoGui::handleEvent(Widget *widget, SDL_Event *event) {
  if(widget == win->closeButton) {
    win->setVisible(false);
    return true;
  } else if(widget == openButton) {
    win->setVisible(false);
  }
  return false;
}

void InfoGui::drawWidgetContents(Widget *w) {
  if( w == image && item ) {
    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_EQUAL, 0xff );
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    //    glTranslatef( x, y, 0 );
    glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ] );
    glColor4f(1, 1, 1, 1);
    
    glBegin( GL_QUADS );
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 0, 0 );
    glVertex3f( 0, 0, 0 );
    glTexCoord2f( 0, 1 );
    glVertex3f( 0, image->getHeight(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex3f( image->getWidth(), image->getHeight(), 0 );
    glTexCoord2f( 1, 0 );
    glVertex3f( image->getWidth(), 0, 0 );
    glEnd();
    glPopMatrix();
    
    glDisable( GL_ALPHA_TEST );
    glDisable(GL_TEXTURE_2D);
  }
}

void InfoGui::describe() {
  // describe item
  if(!item) return;
  item->getDetailedDescription(name);
  nameLabel->setText(name);

  char tmp[1000];

  // detailed description
  strcpy(description, item->getRpgItem()->getLongDesc());
  strcat(description, "|");

  // basic info
  sprintf(tmp, "Level: %d|", item->getLevel());
  strcat( description, tmp );
  sprintf(tmp, "Weight: %.2f|", item->getWeight());
  strcat( description, tmp );
  sprintf(tmp, "Price: %d|", item->getPrice());
  strcat( description, tmp );
  if( item->getAction() ) {
    sprintf(tmp, "Action: %d|", item->getAction());
    strcat( description, tmp );
  }
  if( item->getSpeed() > 0 ) {
    sprintf(tmp, "Speed: %d|", item->getSpeed());
    strcat( description, tmp );
  }
  if( item->getDistance() > Constants::MIN_DISTANCE ) {
    sprintf(tmp, "Distance: %d|", item->getDistance());
    strcat( description, tmp );
  }
  if( item->getMaxCharges() > 0 ) {
    sprintf(tmp, "Max charges: %d|", item->getMaxCharges());
    strcat( description, tmp );
  }
  if( item->getDuration() > 0 ) {
    sprintf(tmp, "Duration: %d|", item->getDuration());
    strcat( description, tmp );
  }



  // DEBUG
  //infoDetailLevel = 100;
  // DEBUG


  bool missedSomething = false;
  if(item->isMagicItem()) {
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      sprintf(tmp, "|%d bonus to %s.", 
              item->getBonus(),
              (item->getRpgItem()->isWeapon() ? "attack and damage" : "armor points"));
      strcat(description, tmp);
    } else missedSomething = true;
    if( item->getDamageMultiplier() > 1 ) {
      if( infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
        if( item->getDamageMultiplier() == 2 ) {
          sprintf( tmp, "|Double damage");
          strcat( description, tmp );
        } else if( item->getDamageMultiplier() == 3 ) {
          sprintf( tmp, "|Tripple damage");
          strcat( description, tmp );
        } else if( item->getDamageMultiplier() == 4 ) {
          sprintf( tmp, "|Quad damage");
          strcat( description, tmp );
        } else if( item->getDamageMultiplier() > 4 ) {
          sprintf( tmp, "|%dX damage", item->getDamageMultiplier());
          strcat( description, tmp );
        }
        if( item->getMonsterType() ) {
          char *p = Monster::getDescriptiveType( item->getMonsterType() );
          sprintf( tmp, " vs. %s.", ( p ? p : item->getMonsterType() ));
          strcat( description, tmp );
        } else {
          sprintf( tmp, " vs. any creature.");
          strcat( description, tmp );
        }
      } else missedSomething = true;
    }
    if(item->getSchool() ) {
      if( infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
        if(item->getRpgItem()->isWeapon()) {
          sprintf(tmp, "|Extra damage of %s %s magic.", 
                  item->describeMagicDamage(),
                  item->getSchool()->getName());
        } else {
          sprintf(tmp, "|Extra %d pts of %s magic resistance.", 
                  item->getMagicResistance(),
                  item->getSchool()->getName());
        }
        strcat(description, tmp);
      } else missedSomething = true;
    }
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      strcpy(tmp, "|Sets state mods:");
      bool found = false;
      for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
        if(item->isStateModSet(i)) {
          strcat(tmp, " ");
          strcat(tmp, Constants::STATE_NAMES[i]);
          found = true;
        }
      }
      if(found) strcat(description, tmp);
    } else missedSomething = true;
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      strcpy(tmp, "|Protects from state mods:");
      bool found = false;
      for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
        if(item->isStateModProtected(i)) {
          strcat(tmp, " ");
          strcat(tmp, Constants::STATE_NAMES[i]);
          found = true;
        }
      }
      if(found) strcat(description, tmp);
    } else missedSomething = true;
    if(infoDetailLevel > (int)(100.0f * rand()/RAND_MAX)) {
      bool found = false;
      map<int,int> *skillBonusMap = item->getSkillBonusMap();
      for(map<int, int>::iterator i=skillBonusMap->begin(); i!=skillBonusMap->end(); ++i) {
        int skill = i->first;
        int bonus = i->second;
        sprintf(tmp, " %s+%d", Constants::SKILL_NAMES[skill], bonus);
        found = true;
      }
      if(found) {
        strcat(description, "|Bonuses to skills:");
        strcat(description, tmp);
      }
    } else missedSomething = true;
    // cursed is hard to detect
    if( item->isCursed() ) {
      if( item->getShowCursed() || 
          infoDetailLevel > (int)(200.0f * rand()/RAND_MAX)) {
        strcat(description, "|This item is cursed!");
      }
    }
  } else if(item->getRpgItem()->getType() == RpgItem::SCROLL) {
    strcat(description, "|");
    strcat(description, "School: ");
    strcat(description, item->getSpell()->getSchool()->getName() );
    strcat(description, "|");
    strcat(description, item->getSpell()->getNotes());
  }

  if( missedSomething ) {
    strcat( description, "|You have a feeling there is more to this item than what you've been able to glean..." );
  }

  label->setText(description);
}

