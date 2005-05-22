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

ConversationGui::ConversationGui(Scourge *scourge) {
  this->scourge = scourge;

  int width = 500;
  int height = 350;

  int x = (scourge->getSDLHandler()->getScreen()->w - width) / 2;
  int y = (scourge->getSDLHandler()->getScreen()->h - height) / 2;

  win = scourge->createWindow( x, y, width, height, Constants::getMessage(Constants::CONVERSATION_GUI_TITLE) );

  label = win->createLabel( 10, 13, "Talking to " );

  int sy = 130;
  canvas = new Canvas( width - 130, 5, width - 10, sy - 10, this );
  win->addWidget( canvas );
  
  answer = new ScrollingLabel( 10, 25, width - 150, 265, "" );
  answer->setWordClickedHandler( this );
  Color color;
  color.r = 1;
  color.g = 1;
  color.b = 0;
  color.a = 1;
  answer->addColoring( '$', color );
  win->addWidget( answer );

  list = new ScrollingList( width - 130, sy, 120, 160, scourge->getShapePalette()->getHighlightTexture() );
  win->addWidget( list );
  words = (char**)malloc(MAX_WORDS * sizeof(char*));
  for(int i = 0; i < MAX_WORDS; i++) {
    words[i] = (char*)malloc(120 * sizeof(char));
  }
  wordCount = 0;

  sy = 310;
  win->createLabel( 12, sy, "Talk about:" );
  entry = new TextField( 90, sy - 10, 25 );
  win->addWidget( entry );

  x = width - 110;
  closeButton = win->createButton( x, sy - 10, x + 100, sy - 10 + 20, "Close" );

  win->setVisible( false );
}

ConversationGui::~ConversationGui() {
  delete win;
  for( int i = 0; i < MAX_WORDS; i++ ) {
    free( words[i] );
  }
  free( words );
}

bool ConversationGui::handleEvent(Widget *widget, SDL_Event *event) {
  if( widget == win->closeButton || 
      widget == closeButton ) {
    win->setVisible(false);
  } else if( widget == list && 
             list->getEventType() == ScrollingList::EVENT_ACTION ) {
    int index = list->getSelectedLine();
    if( index > -1 ) {
      wordClicked( words[ index ] );
    }
  } else if( widget == entry && 
             entry->getEventType() == TextField::EVENT_ACTION ) {
    wordClicked( entry->getText() );
    entry->clearText();
  }
  return false;
}

void ConversationGui::start(Creature *creature) {
  // pause the game
  scourge->getParty()->toggleRound( true );
  this->creature = creature;
  char tmp[ 80 ];
  sprintf( tmp, "Talking to %s", creature->getName() );
  label->setText( tmp );
  char *s = Mission::getIntro( creature->getMonster() );
  useCreature = ( s ? true : false );
  if( !s ) {
    s = Mission::getIntro();
  }
  answer->setText( s );
  win->setVisible( true );
  wordCount = 0;
  list->setLines( wordCount, (const char**)words );
}

void ConversationGui::wordClicked( char *word ) {
  //cerr << "Clicked: " << word << endl;
  if( useCreature ) {
    answer->setText( Mission::getAnswer( creature->getMonster(), word ) );
  } else {
    answer->setText( Mission::getAnswer( word ) );
  }

  for( int i = 0; i < wordCount; i++ ) {
    if( !strcmp( words[i], word ) ) {
      // delete it
      for( int t = i; t < wordCount - 1; t++ ) {
        strcpy( words[t], words[t + 1] );
      }
      wordCount--;
      list->setLines( wordCount, (const char**)words );
      return;
    }
  }
}

void ConversationGui::showingWord( char *word ) {
  for( int i = 0; i < wordCount; i++ ) {
    if( !strcmp( words[i], word ) ) {
      return;
    }
  }
  // add new word
  if( wordCount < MAX_WORDS ) {
    strcpy( words[ wordCount++ ], word );
    list->setLines( wordCount, (const char**)words );
  }
}

void ConversationGui::drawWidget(Widget *w) {
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
