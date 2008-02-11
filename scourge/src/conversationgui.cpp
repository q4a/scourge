/***************************************************************************
                          conversationgui.cpp  -  description
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

#include "conversationgui.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "creature.h"
#include "tradedialog.h"
#include "healdialog.h"
#include "donatedialog.h"
#include "traindialog.h"
#include "uncursedialog.h"
#include "identifydialog.h"
#include "rechargedialog.h"
#include "board.h"
#include "shapepalette.h"
#include "sqbinding/sqbinding.h"
#include "uncursedialog.h"
#include "identifydialog.h"
#include "rechargedialog.h"

using namespace std;

#define TRADE_WORD "trade"
#define TRAIN_WORD "train"
#define UNCURSE_WORD "uncurse"
#define IDENTIFY_WORD "identify"
#define RECHARGE_WORD "recharge"
#define HEAL_WORD "heal"
#define DONATE_WORD "donate"

ConversationGui::ConversationGui(Scourge *scourge) {
  this->scourge = scourge;

  int width = 500;
  int height = 320;

  int x = (scourge->getSDLHandler()->getScreen()->w - width) / 2;
  int y = (scourge->getSDLHandler()->getScreen()->h - height) / 2;

  win = scourge->createWindow( x, y, width, height, Constants::getMessage(Constants::CONVERSATION_GUI_TITLE) );

  label = win->createLabel( 10, 15, _( "Talking to " ) );

  int sy = 130;
  canvas = new Canvas( width - 130, 5, width - 10, sy - 10, this );
  win->addWidget( canvas );
  
  answer = new ScrollingLabel( 10, 25, width - 150, 215, "" );
  answer->setWordClickedHandler( this );
  Color color;
  color.r = 1;
  color.g = 1;
  color.b = 0;
  color.a = 1;
  answer->addColoring( '$', color );
  win->addWidget( answer );

  list = new ScrollingList( width - 130, sy, 120, 110, scourge->getShapePalette()->getHighlightTexture() );
  win->addWidget( list );
  wordCount = 0;

  sy = 260;
  win->createLabel( 9, sy, _( "Talk about:" ) );
  entry = new TextField( 80, sy - 10, 28 );
  win->addWidget( entry );

  y = sy + 10;
  x = width - 82;
  closeButton = win->createButton( x, y, x + 72, y + 20, _( "Close" ) );

  cards = new CardContainer( win );
  x = 10;

  // commoner
  cards->createLabel( x, y + 13, _( "Commoner: No services" ), Constants::NPC_TYPE_COMMONER );

  // sage
  cards->createLabel( x, y + 13, _( "Sage:" ), Constants::NPC_TYPE_SAGE );  
  identifyButton = cards->createButton( x + 70, y, x + 160, y + 20, _( "Identify item" ), Constants::NPC_TYPE_SAGE );
  uncurseItemButton = cards->createButton( x + 165, y, x + 255, y + 20, _( "Uncurse" ), Constants::NPC_TYPE_SAGE );
  rechargeButton = cards->createButton( x + 260, y, x + 350, y + 20, _( "Recharge" ), Constants::NPC_TYPE_SAGE );

  // healer
  cards->createLabel( x, y + 13, _( "Healer:" ), Constants::NPC_TYPE_HEALER );  
  healButton = cards->createButton( x + 70, y, x + 160, y + 20, _( "Heal" ), Constants::NPC_TYPE_HEALER );
  donateButton = cards->createButton( x + 165, y, x + 255, y + 20, _( "Donate" ), Constants::NPC_TYPE_HEALER );

  // trainer
  cards->createLabel( x, y + 13, _( "Trainer:" ), Constants::NPC_TYPE_TRAINER );  
  trainButton = cards->createButton( x + 70, y, x + 160, y + 20, _( "Train" ), Constants::NPC_TYPE_TRAINER );

  // merchant
  cards->createLabel( x, y + 13, _( "Merchant:" ), Constants::NPC_TYPE_MERCHANT );  
  tradeButton = cards->createButton( x + 70, y, x + 160, y + 20, _( "Trade" ), Constants::NPC_TYPE_MERCHANT );
  
  win->setVisible( false );
}

ConversationGui::~ConversationGui() {
  delete win;
}

bool ConversationGui::handleEvent(Widget *widget, SDL_Event *event) {
  if( widget == win->closeButton || 
      widget == closeButton ) {
    hide();
  } else if( widget == list && 
             list->getEventType() == ScrollingList::EVENT_ACTION ) {
    int index = list->getSelectedLine();
    if( index > -1 ) {
      wordClicked( words[ index ] );
    }
  } else if( widget == entry && 
             entry->getEventType() == TextField::EVENT_ACTION ) {
    wordClicked( string(entry->getText()) );
    entry->clearText();
  } else if( widget == tradeButton ) {
    scourge->getTradeDialog()->setCreature( creature );
  } else if( widget == trainButton ) {
    scourge->getTrainDialog()->setCreature( creature );
  } else if( widget == identifyButton ) {
    scourge->getIdentifyDialog()->setCreature( creature );
  } else if( widget == uncurseItemButton ) {
    scourge->getUncurseDialog()->setCreature( creature );
  } else if( widget == rechargeButton ) {
    scourge->getRechargeDialog()->setCreature( creature );
  } else if( widget == healButton ) {
    scourge->getHealDialog()->setCreature( creature );
  } else if( widget == donateButton ) {
    scourge->getDonateDialog()->setCreature( creature );
  }

  return false;
}

void ConversationGui::start( Creature *creature ) {
  char *s = Mission::getIntro( creature->getMonster()->getType() );
	if( !s ) s = Mission::getIntro( creature->getName() );
  bool useCreature = ( s ? true : false );
  if( !s ) {
    s = Mission::getIntro();
  }
  start( creature, s, useCreature );
}

void ConversationGui::start( Creature *creature, char *message, bool useCreature ) {
  // pause the game
  scourge->getParty()->toggleRound( true );
  this->creature = creature;
  this->useCreature = useCreature;
  char tmp[ 80 ];
  snprintf( tmp, 80, _( "Talking to %s" ), _( creature->getName() ) );
  label->setText( tmp );
  answer->setText( message );
  win->setVisible( true );
  wordCount = 0;
  list->setLines( wordCount, words );

  // show the correct buttons
  cards->setActiveCard( creature->getNpcInfo() ? creature->getNpcInfo()->type : Constants::NPC_TYPE_COMMONER );
}

void ConversationGui::wordClicked( std::string const& pWord ) {

  // convert to lower case
	string word = pWord;
	//cerr << "Clicked: " << word << endl;
  Util::toLowerCase( word );
  //cerr << "LOWER Clicked: " << word << endl;

  // try to get the answer from script
  char first[255];
  if( useCreature ) {
		char *s = Mission::getFirstKeyPhrase( creature->getMonster()->getType(), word.c_str() );
		if( !s ) s = Mission::getFirstKeyPhrase( creature->getName(), word.c_str() );
		if( !s ) cerr << "*** warn no first keyphrase for: " << word << endl;
		strcpy( first, ( s ? s : word.c_str() ) );
	} else {
		strcpy( first, Mission::getFirstKeyPhrase( word.c_str() ) );
	}

  char answerStr[255];
  scourge->getSession()->getSquirrel()->
    callConversationMethod( "converse", creature, first, answerStr );
  if( !strlen( answerStr ) ) {
    // Get the answer the usual way
    if( creature ) {
      if( word == TRADE_WORD &&
          creature->getNpcInfo() &&
          creature->getNpcInfo()->type == Constants::NPC_TYPE_MERCHANT ) scourge->getTradeDialog()->setCreature( creature );
      else if( word == HEAL_WORD &&
               creature->getNpcInfo() &&
               creature->getNpcInfo()->type == Constants::NPC_TYPE_HEALER ) scourge->getHealDialog()->setCreature( creature );
      else if( word == TRAIN_WORD &&
               creature->getNpcInfo() &&
               creature->getNpcInfo()->type == Constants::NPC_TYPE_TRAINER ) scourge->getTrainDialog()->setCreature( creature );
      else if( word == UNCURSE_WORD &&
          creature->getNpcInfo() &&
          creature->getNpcInfo()->type == Constants::NPC_TYPE_SAGE ) scourge->getUncurseDialog()->setCreature( creature );
      else if( word == IDENTIFY_WORD &&
          creature->getNpcInfo() &&
          creature->getNpcInfo()->type == Constants::NPC_TYPE_SAGE ) scourge->getIdentifyDialog()->setCreature( creature );
      else if( word == RECHARGE_WORD &&
          creature->getNpcInfo() &&
          creature->getNpcInfo()->type == Constants::NPC_TYPE_SAGE ) scourge->getRechargeDialog()->setCreature( creature );
      else if( word == DONATE_WORD &&
               creature->getNpcInfo() &&
               creature->getNpcInfo()->type == Constants::NPC_TYPE_HEALER ) scourge->getDonateDialog()->setCreature( creature );
    }
  
    if( useCreature ) {
		char *s = Mission::getAnswer( creature->getMonster()->getType(), word.c_str() );
		if( !s ) s = Mission::getAnswer( creature->getName(), word.c_str() );
	  answer->setText( s );
    } else {
		answer->setText( Mission::getAnswer( word.c_str() ) );
    }
  } else {
    answer->setText( answerStr );
  }

  for( int i = 0; i < wordCount; i++ ) {
	  if( words[i] == word ) {
      // delete it
      for( int t = i; t < wordCount - 1; t++ ) {
        words[t] = words[t + 1];
      }
      wordCount--;
      list->setLines( wordCount, words );
      return;
    }
  }
}

void ConversationGui::showingWord( char *word ) {
  for( int i = 0; i < wordCount; i++ ) {
	  if( words[i] == word ) {
      return;
    }
  }
  // add new word
  if( wordCount < MAX_WORDS ) {
    words[ wordCount++ ] = word;
    list->setLines( wordCount, words );
  }
}

void ConversationGui::drawWidgetContents(Widget *w) {
  if( w == canvas && creature && creature->getMonster() && 
      creature->getMonster()->getPortraitTexture() ) {
    //glEnable( GL_ALPHA_TEST );
    //glAlphaFunc( GL_EQUAL, 0xff );
    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
    //    glTranslatef( x, y, 0 );
    glBindTexture( GL_TEXTURE_2D, creature->getMonster()->getPortraitTexture() );
    glColor4f(1, 1, 1, 1);
    
    glBegin( GL_QUADS );
    glNormal3f( 0, 0, 1 );
    glTexCoord2f( 0, 0 );
    glVertex3f( 0, 0, 0 );
    glTexCoord2f( 0, 1 );
    glVertex3f( 0, canvas->getHeight(), 0 );
    glTexCoord2f( 1, 1 );
    glVertex3f( canvas->getWidth(), canvas->getHeight(), 0 );
    glTexCoord2f( 1, 0 );
    glVertex3f( canvas->getWidth(), 0, 0 );
    glEnd();
    glPopMatrix();
    
    //glDisable( GL_ALPHA_TEST );
    glDisable(GL_TEXTURE_2D);
  }
}

void ConversationGui::hide() {
  win->setVisible( false );
	// unpause the game
	scourge->getParty()->toggleRound( false );
}
