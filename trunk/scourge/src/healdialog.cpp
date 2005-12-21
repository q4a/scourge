/***************************************************************************
  healdialog.cpp  -  description
-------------------
    begin                : 9/9/2005
    copyright            : (C) 2005 by Gabor Torok
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

#include "healdialog.h"
#include "scourge.h"
#include "creature.h"
#include "rpg/rpglib.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/scrollinglist.h"
#include "gui/scrollinglabel.h"
#include "shapepalette.h"

#define COINS_AVAILABLE "Coins Available:"

HealDialog::HealDialog( Scourge *scourge ) {
  this->scourge = scourge;
  this->creature = NULL;
  int w = 400;
  int h = 300;
  win = 
    scourge->createWindow( 50, 50, 
                           w, h, 
                           Constants::getMessage( Constants::HEAL_DIALOG_TITLE ) );
  creatureLabel = win->createLabel( 10, 15, "" );
  coinLabel = win->createLabel( 10, 30, COINS_AVAILABLE );
  spellList = new ScrollingList( 10, 40, w - 20, 110, 
                                 scourge->getHighlightTexture(), 
                                 NULL, 30 );
  win->addWidget( spellList );
  spellDescription = new ScrollingLabel( 10, 160, w - 20, 70, "" );
  win->addWidget( spellDescription );

  h = 20;
  int y = win->getHeight() - Window::BOTTOM_HEIGHT - Window::TOP_HEIGHT - h - 10;
  applyButton = win->createButton( w - 160, y, w - 90, y + h, "Buy" );
  closeButton = win->createButton( w - 80, y, w - 10, y + h, "Close" );

  this->spellText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  this->spellIcons = (GLuint*)malloc(MAX_INVENTORY_SIZE * sizeof(GLuint));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->spellText[i] = (char*)malloc(120 * sizeof(char));
  }
}

HealDialog::~HealDialog() {
  delete win;
}

void HealDialog::setCreature( Creature *creature ) {
  this->creature = creature;
  updateUI();
  win->setVisible( true );
}

void HealDialog::updateUI() {

  spells.clear();
  prices.clear();

  char s[255];
  sprintf( s, "%s (level %d)", creature->getName(), creature->getNpcInfo()->level );
  creatureLabel->setText( s );
  sprintf( s, "%s %d", 
           COINS_AVAILABLE, 
           scourge->getParty()->getPlayer()->getMoney() );
  coinLabel->setText( s );

  for( int i = 0; i < MagicSchool::getMagicSchoolCount(); i++ ) {
    MagicSchool *school = MagicSchool::getMagicSchool( i );
    for( int r = 0; r < school->getSpellCount(); r++ ) {
      Spell *spell = school->getSpell( r );
      if( spell->isFriendly() && 
          spell->hasStateModPrereq() &&
          spell->getLevel() <= creature->getNpcInfo()->level ) {

        // level-based mark-up
        int price = spell->getExp() + 
          (int)Util::getRandomSum( (float)(spell->getExp() / 2), spell->getLevel() );
        // HQ is discounted (FIXME: shouldn't be hardcoded)
        if( scourge->isInHQ() ) price = (int)( (float)price / 15.0f );
        // 25% variance based on leadership skill.
        float skill = (float)( scourge->getParty()->getPlayer()->getSkill( Constants::LEADERSHIP ) );
        int percentage = (int)( (float)price * ( 100.0f - skill ) / 100.0f * 0.25f );
        prices[ spell ] = price + percentage;


        spell->describe( s );
        sprintf( spellText[ spells.size() ], "$%d %s", prices[ spell ], s );
        if( spells.size() == 0 ) {
          showSpellDescription(spell);
        }
        spellIcons[ spells.size() ] = 
          scourge->getShapePalette()->
          spellsTex[ spell->getIconTileX() ][ spell->getIconTileY() ];

        // must be last op.
        spells.push_back( spell );
      }
    }
  }
  if( spells.size() == 0 ) spellDescription->setText( "" );
  spellList->setLines( spells.size(), 
                       (const char**)spellText, 
                       NULL, 
                       spellIcons );

}

void HealDialog::handleEvent( Widget *widget, SDL_Event *event ) {
  if( widget == closeButton || widget == win->closeButton ) {
    win->setVisible( false );
  } else if( widget == applyButton ) {
    int line = spellList->getSelectedLine();
    if( line > -1 ) {
      Spell *spell = spells[ line ];
      heal( spell, prices[ spell ] );
    }
  } else if( widget == spellList ) {
    int line = spellList->getSelectedLine();
    if( line > -1 ) {
      Spell *spell = spells[ line ];
      showSpellDescription( spell );
    }
  }
}
  
void HealDialog::heal( Spell *spell, int price ) {
  if( price > scourge->getParty()->getPlayer()->getMoney() ) {
    scourge->showMessageDialog( "You can't afford the healing." );
    return;
  }

  // take the $$$
  scourge->getParty()->getPlayer()->setMoney( 
    scourge->getParty()->getPlayer()->getMoney() - 
    price );
  // update the coin label
  char s[255];
  sprintf( s, "%s %d", 
           COINS_AVAILABLE, 
           scourge->getParty()->getPlayer()->getMoney() );
  coinLabel->setText( s );

  // set up the spell
  creature->setAction( Constants::ACTION_CAST_SPELL, NULL, spell );
  //if( spell->isPartyTargetAllowed() ) {
    creature->setTargetCreature( scourge->getParty()->getPlayer() );
  //}

  // unpause the game
  scourge->getParty()->toggleRound( false );

  // cast the spell 
  // (FIXME: this is a hack. The spell should be cast via the usual battle mechanism.)
  creature->getBattle()->castSpell( true );

  scourge->showMessageDialog( "The healing spell was cast." );
}

void HealDialog::showSpellDescription( Spell *spell ) {
  spellDescription->setText( (char*)( spell->getNotes() ? spell->getNotes() : "" ) );
}
