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
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "item.h"
#include "creature.h"
#include "characterinfo.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "userconfiguration.h"
#include "util.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/canvas.h"
#include "gui/scrollinglist.h"
#include "gui/cardcontainer.h"
#include "gui/scrollinglabel.h"
#include "shapepalette.h"

using namespace std;

/**
  *@author Gabor Torok
  */

#define PORTRAIT_SIZE 150
#define MODEL_SIZE 210
#define AVAILABLE_SKILL_POINTS 30
#define LEVEL 10

typedef struct _Preset {
  char name[80];
  int deity;
  int charClass;
  int portrait;
  int model;
} Preset;

Preset presets[] = {
  { "Lezidor", 4, 0, 3, 0 }, // ranger
  { "Kaz-Mokh", 1, 1, 1, 1 }, // knight
  { "Arcoraxe", 2, 6, 8, 15 }, // conjuror
  { "Deiligiliam", 0, 8, 6, 7 }, // naturalist
};

// this is here to compile faster (otherwise shapepalette needs to be incl.)
std::map<CharacterModelInfo*, GLShape*> shapes;

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
  //mainWin->setModal( true );  
  mainWin->setLocked( true );  

  
  step = INTRO_TEXT;

  cards = new CardContainer( mainWin );
  cards->setActiveCard( INTRO_TEXT );

  intro = new Label( 30, 100, 
                     "You have arrived... as we knew you would. The sand swirls gently in the hourglass of time and reveals all. Sooner or later even the proudest realize that there is no more adventure to be had in the city of Horghh. But fear not! The S.C.O.U.R.G.E. Vermin Extermination Services Company takes good care of its employees. You will be payed in gold, fed nourishing gruel on most days and have access to the company training grounds and shops. Should you sustain injuries or a debilitating predicament (including but not limited to: poison, curses, possession or death ) our clerics will provide healing at a reduced cost. Positions fill up fast, but there are always some available (...) so sign up with a cheerful heart and a song in your step. Your past glories cannot possibly compare to the wonder and excitement that lies ahead in the ...uh.. sewers of your new vocation!", 94, Constants::SCOURGE_LARGE_FONT, 24 );
  cards->addWidget( intro, INTRO_TEXT );
  

  cancel = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 80, 
                                w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 50, 
                                "I will not join", INTRO_TEXT );
  toChar0 = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 80, 
                                 w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 50, 
                                 "Ready to exterminate", INTRO_TEXT );

  for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {
    tmp[ i ] = NULL;
    createCharUI( 1 + i, &( info[ i ] ) );
  }
  createParty( (Creature**)tmp, NULL, false );
  for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {
    info[i].detailsInfo->setCreature( mainWin, tmp[i] );    
    // set to preset
    info[i].name->setText( presets[i].name );
    info[i].deityType->setSelectedLine( presets[i].deity );
    setDeityType( i, presets[i].deity );
    info[i].charType->setSelectedLine( presets[i].charClass );
    setCharType( i, presets[i].charClass );
    info[i].portraitIndex = presets[i].portrait;
    info[i].modelIndex = presets[i].model;
  }
  saveUI( (Creature**)tmp );

  toLastChar = cards->createButton( w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 80, 
                                    w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 50, 
                                    "Back", OUTRO_TEXT );
  done = cards->createButton( w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 80, 
                              w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 50, 
                              "Enter Head Quarters", OUTRO_TEXT );

  Label *outro = new Label( 30, 100, 
                     "Good luck on your missions brave adventurers! Be sure to explore the S.C.O.U.R.G.E. head-quarters. You have all been given some basic gear but anything you find around the office is yours to keep or sell.||Remember, you are not only ridding a subterranian lair of unwanted vermin, you are also an integral part of the Corporation. The company, you see, is like a big family where everyone takes part in the daily chores. Our joint success can only be brought about by your... I mean, our, hard work. Now, some of you may regrettably meet your unfortunate end while on your travels, but rest assured, this risk has been considered in the S.C.O.U.R.G.E. business plan. Together we can't fail!", 94, Constants::SCOURGE_LARGE_FONT, 24 );
  cards->addWidget( outro, OUTRO_TEXT );

}

PartyEditor::~PartyEditor() {
  for( int i = 0; i < 4; i++ ) {
    if( tmp[ i ] ) {
      delete tmp[ i ];
      tmp[ i ] = NULL;
    }
  }
  deleteLoadedShapes();
}

void PartyEditor::deleteLoadedShapes() {
  for( map<CharacterModelInfo*, GLShape*>::iterator i=shapes.begin(); i!=shapes.end(); ++i ) {
    CharacterModelInfo *cmi = i->first;
    GLShape *shape = i->second;  
    scourge->getShapePalette()->decrementSkinRefCount( cmi->model_name, cmi->skin_name );
    delete shape;
  }
  shapes.clear();
}

void PartyEditor::reset() { 
  step = INTRO_TEXT; 
  cards->setActiveCard( step ); 
}

void PartyEditor::handleEvent( Widget *widget, SDL_Event *event ) {
  
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
        setCharType( i, info[i].charType->getSelectedLine() );
      } else if( widget == info[i].deityType ) {
        setDeityType( i, info[i].deityType->getSelectedLine() );
      } else if( widget == info[i].prevPortrait ) {
        if( info[i].portraitIndex > 0 ) {
          info[i].portraitIndex--;
          saveUI( (Creature**)tmp );
        }
      } else if( widget == info[i].nextPortrait ) {
        if( info[i].portraitIndex < scourge->getShapePalette()->getPortraitCount() - 1 ) {
          info[i].portraitIndex++;
          saveUI( (Creature**)tmp );
        }
      } else if( widget == info[i].prevModel ) {
        if( info[i].modelIndex > 0 ) {
          info[i].modelIndex--;
          saveUI( (Creature**)tmp );
        }
      } else if( widget == info[i].nextModel ) {
        if( info[i].modelIndex < scourge->getShapePalette()->getCharacterModelInfoCount() - 1 ) {
          info[i].modelIndex++;
          saveUI( (Creature**)tmp );
        }
      } else if( widget == info[i].skillRerollButton ) {
        rollSkills( &( info[ i ] ) );
//        updateUI( &( info[ i ] ), i );
        saveUI( (Creature**)tmp );
      } else if( widget == info[i].skillAddButton && info[i].availableSkillMod ) {
        int n = info[i].skills->getSelectedLine();
        Character *c = Character::character_list[ info->charType->getSelectedLine() ];
        // is it ok?
        int newValue = info[ i ].skill[ n ] + info[ i ].skillMod[ n ] + 1;
        if( newValue >= c->getMinSkillLevel( n ) && 
            newValue <= c->getMaxSkillLevel( n ) &&
            newValue <= 99 ) {
          info[ i ].availableSkillMod--;
          info[ i ].skillMod[ n ]++;
//          updateUI( &( info[ i ] ), i );
          saveUI( (Creature**)tmp );
        }
      } else if( widget == info[i].skillSubButton && info[i].availableSkillMod < AVAILABLE_SKILL_POINTS ) {
        int n = info[i].skills->getSelectedLine();
        Character *c = Character::character_list[ info->charType->getSelectedLine() ];
        // is it ok?
        int newValue = info[ i ].skill[ n ] + info[ i ].skillMod[ n ] - 1;
        if( newValue >= c->getMinSkillLevel( n ) && 
            newValue <= c->getMaxSkillLevel( n ) &&
            newValue <= 99 &&
            info[ i ].skillMod[ n ] ) {
          info[ i ].availableSkillMod++;
          info[ i ].skillMod[ n ]--;
//          updateUI( &( info[ i ] ), i );
          saveUI( (Creature**)tmp );
        }
      } else if( widget == info[i].skills ) {
        info[i].skillDescription->setText( Constants::SKILL_DESCRIPTION[ info[i].skills->getSelectedLine() ] );
      }
    }
  }
  if( oldStep != step ) cards->setActiveCard( step );

}

void PartyEditor::setCharType( int pcIndex, int charIndex ) {
  if( charIndex > -1 ) {     
    Character *character = Character::character_list[ charIndex ];
    if( character ) {
      info[ pcIndex ].charTypeDescription->setText( character->getDescription() );
      rollSkills( &( info[ pcIndex ] ) );
      saveUI( (Creature**)tmp );
    }
  }
}

void PartyEditor::setDeityType( int pcIndex, int deityIndex ) {
  if( deityIndex > -1 ) {
    MagicSchool *school = MagicSchool::getMagicSchool( deityIndex );
    if( school ) info[ pcIndex ].deityTypeDescription->setText( school->getDeityDescription() );
    saveUI( (Creature**)tmp );
  }
}

void PartyEditor::createCharUI( int n, CharacterInfo *info ) {
  // FIXME: copy-paste from constructor
  int w = scourge->getSDLHandler()->getScreen()->w - Window::SCREEN_GUTTER * 2;
  int h = 600;
  
  //  int col2X = ( scourge->getScreenWidth() > 800 ? 500 : 350 );
  int skillColWidth = 290;
  int col2X = w - PORTRAIT_SIZE - 20 - skillColWidth;

  // title
  char msg[80];
  sprintf( msg, "Create character %d out of %d", n - INTRO_TEXT, MAX_PARTY_SIZE );
  Label *title = new Label( 30, 25, msg, 0, Constants::SCOURGE_LARGE_FONT );
  cards->addWidget( title, n );

  // name
  cards->createLabel( 30, 50, "Name:", n, Constants::RED_COLOR );
  info->name = new TextField( 100, 40, 20 );
  cards->addWidget( info->name, n );

  // deity
  int deityHeight = 100;
  cards->createLabel( 30, 80, "Chosen Deity:", n, Constants::RED_COLOR );
  info->deityType = new ScrollingList( 30, 90, 150, deityHeight, scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( info->deityType, n );
  info->deityTypeStr = (char**)malloc( MagicSchool::getMagicSchoolCount() * sizeof(char*));
  for(int i = 0; i < MagicSchool::getMagicSchoolCount(); i++) {
    info->deityTypeStr[i] = (char*)malloc(120 * sizeof(char));
    strcpy( info->deityTypeStr[i], MagicSchool::getMagicSchool( i )->getDeity() );
  }
  info->deityType->setLines( MagicSchool::getMagicSchoolCount(), (const char**)info->deityTypeStr );
  int deityIndex = (int)( (float)( MagicSchool::getMagicSchoolCount() * rand()/RAND_MAX ) );
  info->deityType->setSelectedLine( deityIndex );

  info->deityTypeDescription = new ScrollingLabel( 190, 90, col2X - 10 - 190, deityHeight, 
                                                   MagicSchool::getMagicSchool( deityIndex )->getDeityDescription() );
  cards->addWidget( info->deityTypeDescription, n );
  
  // character type
  int charTypeHeight = 160;
  int charTypeY = 110 + deityHeight;
  cards->createLabel( 30, charTypeY, "Character Type:", n, Constants::RED_COLOR );
  info->charType = new ScrollingList( 30, charTypeY + 10, 150, charTypeHeight, scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( info->charType, n );
  info->charTypeStr = (char**)malloc( Character::character_list.size() * sizeof(char*));
  for(int i = 0; i < (int)Character::character_list.size(); i++) {
    info->charTypeStr[i] = (char*)malloc(120 * sizeof(char));
    strcpy( info->charTypeStr[i], Character::character_list[i]->getName() );
  }
  info->charType->setLines( (int)Character::character_list.size(), (const char**)info->charTypeStr );
  int charIndex = (int)( (float)( Character::character_list.size() ) * rand()/RAND_MAX );
  info->charType->setSelectedLine( charIndex );
  info->charTypeDescription = new ScrollingLabel( 190, charTypeY + 10, col2X - 10 - 190, charTypeHeight, 
                                                  Character::character_list[charIndex]->getDescription() );
  cards->addWidget( info->charTypeDescription, n );

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

  // details 
  int detailsHeight = 145;
  info->detailsInfo = new CharacterInfoUI( scourge );
  info->detailsCanvas = new Canvas( col2X, 10, col2X + skillColWidth, 10 + detailsHeight, info->detailsInfo );
  cards->addWidget( info->detailsCanvas, n );  

  // skills
  int buttonWidth =  skillColWidth / 3;
  info->skillLabel = cards->createLabel( col2X, 30 + detailsHeight, "Remaining skill points:", n, Constants::RED_COLOR );
  info->skills = new ScrollingList( col2X, 30 + detailsHeight + 5, 
                                    skillColWidth, 310 - ( 30 + detailsHeight + 5 + 10 ),
                                    scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( info->skills, n );
  info->skillAddButton = cards->createButton( col2X, 310, col2X + buttonWidth - 5, 330, " + ", n );
  info->skillRerollButton = cards->createButton( col2X + buttonWidth, 310, col2X + buttonWidth * 2 - 5, 330, " Reroll ", n );
  info->skillSubButton = cards->createButton( col2X + buttonWidth * 2, 310, col2X + buttonWidth * 3 - 5, 330, " - ", n );
  info->skillDescription = new ScrollingLabel( col2X, 340, skillColWidth, 100, "" );
  cards->addWidget( info->skillDescription, n );

  rollSkills( info );

  info->skillColor = (Color*)malloc( Constants::SKILL_COUNT * sizeof( Color ) );
  info->skillLine = (char**)malloc( Constants::SKILL_COUNT * sizeof( char* ) );
  for( int i = 0; i < Constants::SKILL_COUNT; i++ ) {
    info->skillLine[i] = (char*)malloc(120 * sizeof(char));
  }
  updateUI( info, n - 1 );
  info->skills->setSelectedLine( 0 );
  info->skillDescription->setText( Constants::SKILL_DESCRIPTION[ info->skills->getSelectedLine() ] );

  info->back = cards->createButton( w - w / 2 - 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 80, 
                                    w - w / 2 - 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 50, 
                                    "Back", n );
  info->next = cards->createButton( w - w / 2 + 10, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 80, 
                                    w - w / 2 + 160, h - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 50, 
                                    "Next", n );
}

void PartyEditor::drawWidgetContents( Widget *w ) {
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
      // draw model
      CharacterModelInfo *cmi = scourge->getShapePalette()->
        getCharacterModelInfo( info[i].modelIndex );
      GLShape *shape;
      if( shapes.find( cmi ) == shapes.end() ) {
        shape = 
          scourge->getShapePalette()->getCreatureShape(cmi->model_name, 
                                                       cmi->skin_name, 
                                                       cmi->scale );
        shapes[ cmi ] = shape;
        shape->setCurrentAnimation( MD2_STAND );
      } else {
        shape = shapes[ cmi ];
      }      
      glPushMatrix();
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );
      glClearDepth( 1.0f );
      glEnable(GL_DEPTH_TEST);
      glDisable( GL_BLEND );
      glDepthMask(GL_TRUE);
      glEnable( GL_TEXTURE_2D );
      glTranslatef( 130, MODEL_SIZE + 10, 500 );
      glRotatef( 90, 1, 0, 0 );
      glRotatef( 180, 0, 0, 1 );
      glScalef( 2, 2, 2 );
      glColor4f( 1, 1, 1, 1 );
      //glDisable( GL_SCISSOR_TEST );
      shape->draw();
      glDisable( GL_TEXTURE_2D );
      glDepthMask(GL_FALSE);
      glDisable(GL_DEPTH_TEST);
      glDisable( GL_BLEND );
      glPopMatrix();
    }
  }
}

void PartyEditor::drawAfter() {
}   
      
void PartyEditor::createParty( Creature **pc, int *partySize, bool addRandomInventory ) {

  deleteLoadedShapes();

  int pcCount = 4;

  for( int i = 0; i < pcCount; i++ ) {
    char *s = info[i].name->getText();
    if( !s || !strlen( s ) ) s = presets[i].name;
    int index = info[i].charType->getSelectedLine();  
    Character *c = Character::character_list[ index ];
    pc[i] = new Creature( scourge->getSession(), c, 
                          strdup( s ), 
                          info[i].modelIndex, false );
    pc[i]->setLevel( LEVEL ); 
    pc[i]->setExp(0);
    pc[i]->setHp();
    pc[i]->setMp();
    pc[i]->setHunger((int)(5.0f * rand()/RAND_MAX) + 5);
    pc[i]->setThirst((int)(5.0f * rand()/RAND_MAX) + 5); 
  }

  saveUI( pc );

  if( addRandomInventory ) addStartingInventory( pc, pcCount );

  if( partySize ) *partySize = pcCount;
}

void PartyEditor::addStartingInventory( Creature **pc, int partySize ) {
  for( int i = 0; i < partySize; i++ ) {
    // add a weapon anyone can wield
    int n = (int)(5.0f * rand()/RAND_MAX);
    switch(n) {
    case 0: pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Smallbow"), LEVEL ), true); break;
    case 1: pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Short sword"), LEVEL ), true); break;
    case 2: pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Dagger"), LEVEL ), true); break;
    case 3: pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Wooden club"), LEVEL )); break;
    case 4: pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Quarter Staff"), LEVEL ), true); break;
    }
    int invIndex = 0;
    pc[i]->equipInventory( invIndex++ );
  
    // add some armor
    if(0 == (int)(4.0f * rand()/RAND_MAX)) {
      pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Horned helmet"), LEVEL ), true);
      pc[i]->equipInventory( invIndex++ );
    }
    if(0 == (int)(3.0f * rand()/RAND_MAX)) {
      pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Buckler"), LEVEL ), true);
      pc[i]->equipInventory( invIndex++ );
    }
    
    // some potions
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Health potion"), LEVEL ), true);  
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Magic potion"), LEVEL ), true);  
    if(0 == (int)(4.0f * rand()/RAND_MAX))
      pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Liquid armor"), LEVEL ), true);  
    
    // some food
    for(int t = 0; t < (int)(6.0f * rand()/RAND_MAX); t++) {
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Apple")), true);
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Bread")), true);
      if(0 == (int)(4.0f * rand()/RAND_MAX))
      pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Mushroom")), true);
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Big egg")), true);
      if(0 == (int)(4.0f * rand()/RAND_MAX))
        pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Mutton meat")), true);
    }
    
    // some spells
    if(pc[i]->getMaxMp() > 0) {
      // useful spells
      pc[i]->addSpell(Spell::getSpellByName("Flame of Azun"));
      pc[i]->addSpell(Spell::getSpellByName("Ole Taffy's purty colors"));
      // attack spell
      if(0 == (int)(2.0f * rand()/RAND_MAX))
        pc[i]->addSpell(Spell::getSpellByName("Silent knives"));
      else
        pc[i]->addSpell(Spell::getSpellByName("Stinging light"));
      // defensive spell
      if(0 == (int)(2.0f * rand()/RAND_MAX))
        pc[i]->addSpell(Spell::getSpellByName("Lesser healing touch"));
      else
        pc[i]->addSpell(Spell::getSpellByName("Body of stone"));
  
  
      // testing
      if( LEVEL > 1 ) {
        pc[i]->addSpell(Spell::getSpellByName("Ring of Harm"));
        pc[i]->addSpell(Spell::getSpellByName("Malice Storm"));
        pc[i]->addSpell(Spell::getSpellByName("Unholy Decimator"));
        pc[i]->addSpell(Spell::getSpellByName("Remove curse"));
        pc[i]->addSpell(Spell::getSpellByName("Teleportation"));
        pc[i]->addSpell(Spell::getSpellByName("Recall to life"));
        pc[i]->setMp( 5000 );
        pc[i]->setMoney( 10000 );

        if( i == 0 ) pc[i]->addInventory(scourge->getSession()->newItem(RpgItem::getItemByName("Brand of Iconoclast")), true);
      }
    }
  }
}

/**
 * Save the moving parts into the creature.
 */
void PartyEditor::saveUI( Creature **pc ) {
  for( int i = 0; i < MAX_PARTY_SIZE; i++ ) {

    // name
    char *s = info[i].name->getText();
    if( !s || !strlen( s ) ) s = presets[i].name;
    pc[i]->replaceName( strdup( s ) );

    // character type
    int index = info[i].charType->getSelectedLine();  
    Character *c = Character::character_list[ index ];
    pc[i]->setCharacter( c );
    pc[i]->setLevel( LEVEL ); 
    pc[i]->setExp(0);
    pc[i]->setHp();
    if( LEVEL > 1 ) pc[i]->setMp( 5000 );
    else pc[i]->setMp();

    // model and portrait
    // (not saved b/c it makes no difference to the rpg values.)

    // deity
    pc[i]->setDeityIndex( info[i].deityType->getSelectedLine() );
    
    // assign portraits
    pc[i]->setPortraitTextureIndex( info[i].portraitIndex );
    
    // compute starting skill levels
    for(int t = 0; t < Constants::SKILL_COUNT; t++) {
      pc[i]->setSkill( t, info[i].skill[ t ] + info[i].skillMod[ t ] );
    }

    updateUI( &( info[i] ), i );
  }
}

void PartyEditor::rollSkills( CharacterInfo *info ) {
  info->availableSkillMod = AVAILABLE_SKILL_POINTS;
  Character *c = Character::character_list[ info->charType->getSelectedLine() ];
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {

    int n = Creature::rollStartingSkill( scourge->getSession(), LEVEL );

    if( n < c->getMinSkillLevel( i ) ) n = c->getMinSkillLevel( i );
    if( n > c->getMaxSkillLevel( i ) ) n = c->getMaxSkillLevel( i );
    info->skill[ i ] = n;
    info->skillMod[ i ] = 0;
  }  
}

void PartyEditor::updateUI( CharacterInfo *info, int index ) {
  for( int i = 0; i < Constants::SKILL_COUNT; i++ ) {

    if( info->skillMod[i] ) {
      info->skillColor[i].r = 0;
      info->skillColor[i].g = 1;
      info->skillColor[i].b = 0;
    } else {
      if( mainWin->getTheme()->getWindowText() ) {
        info->skillColor[i].r = mainWin->getTheme()->getWindowText()->r;
        info->skillColor[i].g = mainWin->getTheme()->getWindowText()->g;
        info->skillColor[i].b = mainWin->getTheme()->getWindowText()->b;
      } else {
        info->skillColor[i].r = 0;
        info->skillColor[i].g = 0;
        info->skillColor[i].b = 0;
      }
    }

    if( tmp[ index ] ) {
      int adj = tmp[ index ]->getLevelAdjustedSkill( i );
      bool isMax = adj == tmp[ index ]->getCharacter()->getMaxSkillLevel( i );
      bool isMin = adj == tmp[ index ]->getCharacter()->getMinSkillLevel( i );
      sprintf(info->skillLine[i], "%d(%d)(%d %% %s) - %s", 
              info->skill[ i ], 
              info->skillMod[ i ], 
              adj,
              ( isMax ? " MAX" : ( isMin ? " MIN" : "" ) ),
              Constants::SKILL_NAMES[i] );
      if( isMax ) {
        info->skillColor[i].r = 0;
        info->skillColor[i].g = 0.75f;
        info->skillColor[i].b = 1;
      } else if( isMin ) {
        info->skillColor[i].r = 0;
        info->skillColor[i].g = 1;
        info->skillColor[i].b = 0.75f;
      }
    } else {
      sprintf(info->skillLine[i], "%d(%d)(%%0) - %s", 
              info->skill[ i ], 
              info->skillMod[ i ], 
              Constants::SKILL_NAMES[i] );
    }
  }
  int line = info->skills->getSelectedLine();
  info->skills->setLines( Constants::SKILL_COUNT, 
                          (const char**)info->skillLine, 
                          info->skillColor );
  info->skills->setSelectedLine( line );

  char msg[80];
  sprintf( msg, "Remaining skill points: %d", info->availableSkillMod );
  info->skillLabel->setText( msg );
}

bool PartyEditor::isVisible() { 
  return mainWin->isVisible(); 
}

void PartyEditor::setVisible( bool b ) { 
  mainWin->setVisible( b ); 
}
