/***************************************************************************
                          partyeditor.cpp  -  description
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

#include "partyeditor.h"

/**
  *@author Gabor Torok
  */

#define PORTRAIT_SIZE 150
#define MODEL_SIZE 200

PartyEditor::PartyEditor(Scourge *scourge) {
  this->scourge = scourge;
  lastTick = 0;
  zrot = 180.0f;
  int x = Window::SCREEN_GUTTER;
  int y = ( scourge->getSDLHandler()->getScreen()->h - 600 ) / 2;
  int w = scourge->getSDLHandler()->getScreen()->w - Window::SCREEN_GUTTER * 2;
  int h = 600;
  mainWin = new Window( scourge->getSDLHandler(),
                        x, y, w, h,
                        "Create your party of brave souls...", false, Window::BASIC_WINDOW, "default" );

  mainWin->setVisible( false );
  mainWin->setModal( true );  
  mainWin->setLocked( true );  

  
  step = INTRO_TEXT;

  cards = new CardContainer( mainWin );
  cards->setActiveCard( INTRO_TEXT );

  intro = new Label( 150, 100, 
                     "You have arrived... as we knew you would. The sand swirls gently in the hourglass of time and reveals all. Sooner or later even the proudest realize that there is no more adventure to be had in the city of Horghh. But fear not! The S.C.O.U.R.G.E. Vermin Extermination Services Company takes good care of its employees. You will be payed in gold, fed nourishing gruel on most days and have access to the company training grounds and shops. Should you sustain injuries or a debilitating predicament (including but not limited to: poison, curses, possession or death ) our clerics will provide healing at a reduced cost. Positions fill up fast, but there are always some available (...) so sign up with a cheerful heart and a song in your step. Your past glories cannot possible compare to the wonder and excitement that lies ahead in the ...uh.. sewers of your new vocation!", 94, SDLHandler::LARGE_FONT, 24 );
  cards->addWidget( intro, INTRO_TEXT );
  

  cancel = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                "I will not join", INTRO_TEXT );
  toChar0 = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                 w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                 "Ready to exterminate", INTRO_TEXT );

  for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {
    createCharUI( 1 + i, &( info[ i ] ) );
  }

  toLastChar = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                    w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                    "Back", OUTRO_TEXT );
  done = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                              w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                              "Enter Head Quarters", OUTRO_TEXT );

}

PartyEditor::~PartyEditor() {
}

void PartyEditor::reset() { 
  step = INTRO_TEXT; 
  cards->setActiveCard( step ); 
}

void PartyEditor::handleEvent( Widget *widget ) {

  //
  // cancel and done are handled in mainmenu.cpp
  //

  int oldStep = step;
  if( widget == toChar0 ) {
    step = CREATE_CHAR_0;
  } else if( widget == toLastChar ) {
    step = CREATE_CHAR_3;
  } else {
    for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {
      if( widget == info[i].back ) {
        if( i == 0 ) step = INTRO_TEXT;
        else step = 1 + ( i - 1 );
      } else if( widget == info[i].next ) {
        if( i == MAX_PARTY_SIZE - 1 ) step = OUTRO_TEXT;
        else step = 1 + ( i + 1 );
      } else if( widget == info[i].charType ) {
        int index = info[i].charType->getSelectedLine();  
        if( index > -1 ) {
          Character *character = Character::character_list[ index ];
          if( character ) info[i].charTypeDescription->setText( character->getDescription() );
        }
      } else if( widget == info[i].prevPortrait ) {
        if( info[i].portraitIndex > 0 ) {
          info[i].portraitIndex--;
        }
      } else if( widget == info[i].nextPortrait ) {
        if( info[i].portraitIndex < scourge->getShapePalette()->getPortraitCount() - 1 ) {
          info[i].portraitIndex++;
        }
      } else if( widget == info[i].prevModel ) {
        if( info[i].modelIndex > 0 ) {
          info[i].modelIndex--;
        }
      } else if( widget == info[i].nextModel ) {
        if( info[i].modelIndex < scourge->getShapePalette()->getCharacterModelInfoCount() - 1 ) {
          info[i].modelIndex++;
        }
      }
    }
  }
  if( oldStep != step ) cards->setActiveCard( step );

}

void PartyEditor::createCharUI( int n, CharacterInfo *info ) {
  // FIXME: copy-paste from constructor
  int w = scourge->getSDLHandler()->getScreen()->w - Window::SCREEN_GUTTER * 2;
  int h = 600;

  // title
  char msg[80];
  sprintf( msg, "Create character %d out of %d", n - INTRO_TEXT, MAX_PARTY_SIZE );
  Label *title = new Label( 30, 25, msg, 0, SDLHandler::LARGE_FONT );
  cards->addWidget( title, n );

  // name
  cards->createLabel( 30, 50, "Name:", n, Constants::RED_COLOR );
  info->name = new TextField( 100, 40, 20 );
  cards->addWidget( info->name, n );
  
  // character type
  cards->createLabel( 30, 80, "Character Type:", n, Constants::RED_COLOR );
  info->charType = new ScrollingList( 30, 90, 150, 150, scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( info->charType, n );
  info->charTypeStr = (char**)malloc( Character::character_list.size() * sizeof(char*));
  for(int i = 0; i < (int)Character::character_list.size(); i++) {
    info->charTypeStr[i] = (char*)malloc(120 * sizeof(char));
    strcpy( info->charTypeStr[i], Character::character_list[i]->getName() );
  }
  info->charType->setLines( (int)Character::character_list.size(), (const char**)info->charTypeStr );
  info->charTypeDescription = new Label( 190, 100, Character::character_list[0]->getDescription(), 60 );
  cards->addWidget( info->charTypeDescription, n );

  // deity
  cards->createLabel( 30, 260, "Chosen Deity:", n, Constants::RED_COLOR );
  info->deityType = new ScrollingList( 30, 270, 150, 150, scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( info->deityType, n );
  info->deityTypeStr = (char**)malloc( 1 * sizeof(char*));
  for(int i = 0; i < 1; i++) {
    info->deityTypeStr[i] = (char*)malloc(120 * sizeof(char));
    strcpy( info->deityTypeStr[i], "FIXME: deity list" );
  }
  info->deityType->setLines( 1, (const char**)info->deityTypeStr );
  info->deityTypeDescription = new Label( 200, 280, "FIXME: deity description", 60 );
  cards->addWidget( info->deityTypeDescription, n );

  // portrait
  info->portrait = new Canvas( w - PORTRAIT_SIZE - 10, 10, w - 10, 10 + PORTRAIT_SIZE, this );
  cards->addWidget( info->portrait, n );
  info->portraitIndex = (int)( (float)( scourge->getShapePalette()->getPortraitCount() ) * rand()/RAND_MAX );
  info->prevPortrait = cards->createButton( w - 10 - PORTRAIT_SIZE, 20 + PORTRAIT_SIZE,
                                            w - 10 - PORTRAIT_SIZE / 2 - 5, 40 + PORTRAIT_SIZE, 
                                            "<<", n );
  info->nextPortrait = cards->createButton( w - 5 - PORTRAIT_SIZE / 2, 20 + PORTRAIT_SIZE,
                                            w - 10, 40 + PORTRAIT_SIZE, 
                                            "    >>", n );
  // model
  info->model = new Canvas( w - PORTRAIT_SIZE - 10, 50 + PORTRAIT_SIZE, w - 10, 50 + PORTRAIT_SIZE + MODEL_SIZE, this );
  cards->addWidget( info->model, n );
  info->modelIndex = (int)( (float)( scourge->getShapePalette()->getCharacterModelInfoCount() ) * rand()/RAND_MAX );
  info->prevModel = cards->createButton( w - 10 - PORTRAIT_SIZE, 60 + PORTRAIT_SIZE + MODEL_SIZE,
                                         w - 10 - PORTRAIT_SIZE / 2 - 5, 80 + PORTRAIT_SIZE + MODEL_SIZE, 
                                         "<<", n );
  info->nextModel = cards->createButton( w - 5 - PORTRAIT_SIZE / 2, 60 + PORTRAIT_SIZE + MODEL_SIZE,
                                         w - 10, 80 + PORTRAIT_SIZE + MODEL_SIZE, 
                                         "    >>", n );
  

  info->back = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                    w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                    "Back", n );
  info->next = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 130, 
                                    w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 100, 
                                    "Next", n );
}

void PartyEditor::drawWidget( Widget *w ) {
  Uint32 t = SDL_GetTicks();
  if( t - lastTick > 5 ) {
    lastTick = t;
    zrot += 5;
    if( zrot >= 360 ) zrot -= 360;
  }
  for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {
    if( w == info[i].portrait ) {
      glPushMatrix();
      glEnable( GL_TEXTURE_2D );
      glDisable( GL_CULL_FACE );
      glColor4f( 1, 1, 1, 1 );
      glBindTexture( GL_TEXTURE_2D, 
                     scourge->getShapePalette()->getPortraitTexture( info[i].portraitIndex ) );
      glBegin( GL_QUADS );
      glNormal3f( 0, 0, 1 );
      glTexCoord2f( 0, 0 );
      glVertex2i( 0, 0 );
      glTexCoord2f( 1, 0 );
      glVertex2i( PORTRAIT_SIZE, 0 );
      glTexCoord2f( 1, 1 );
      glVertex2i( PORTRAIT_SIZE, PORTRAIT_SIZE );
      glTexCoord2f( 0, 1 );
      glVertex2i( 0, PORTRAIT_SIZE );
      glEnd();
      glDisable( GL_TEXTURE_2D );
      glPopMatrix();
    } else if( w == info[i].model ) {
      /*
      glDisable( GL_TEXTURE_2D );
      glPushMatrix();
      glDisable( GL_CULL_FACE );
      glColor4f( 0, 0, 0, 1 );
      glBegin( GL_QUADS );
      glVertex2i( 0, 0 );
      glVertex2i( PORTRAIT_SIZE, 0 );
      glVertex2i( PORTRAIT_SIZE, MODEL_SIZE );
      glVertex2i( 0, MODEL_SIZE );
      glEnd();
      glPopMatrix();
      */

      // draw model
      CharacterModelInfo *cmi = scourge->getShapePalette()->getCharacterModelInfo( info->modelIndex );
      GLShape *shape = 
        scourge->getShapePalette()->getCreatureShape(cmi->model_name, 
                                                     cmi->skin_name, 
                                                     cmi->scale );      
      glPushMatrix();
      glEnable(GL_DEPTH_TEST);
      glDisable( GL_BLEND );
      glDepthMask(GL_TRUE);
      glEnable( GL_TEXTURE_2D );
      glRotatef( 75, 1, 0, 0 );
      glTranslatef( PORTRAIT_SIZE, MODEL_SIZE * 3, 100 );      
      glRotatef( zrot, 0, 0, 1 );
      glTranslatef( 0, 0, shape->getDepth() / 2.0f / GLShape::DIV );      
      glScalef( 2, 2, 2 );
      glColor4f( 1, 1, 1, 1 );
      glDisable( GL_SCISSOR_TEST );
      shape->draw();
      glDisable( GL_TEXTURE_2D );
      glDepthMask(GL_FALSE);
      glDisable(GL_DEPTH_TEST);
      glPopMatrix();
    }
  }
}
