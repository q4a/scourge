/***************************************************************************
                          pceditor.cpp  -  description
                             -------------------
    begin                : Tue Jul 10 2006
    copyright            : (C) 2006 by Gabor Torok
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
#include "pceditor.h"
#include "gui/window.h"
#include "gui/cardcontainer.h"
#include "gui/button.h"
#include "gui/textfield.h"
#include "gui/scrollinglabel.h"
#include "gui/scrollinglist.h"
#include "gui/canvas.h"
#include "scourge.h"
#include "shapepalette.h"
#include "characterinfo.h"
#include "rpg/character.h"
#include "rpg/spell.h"
#include "render/glshape.h"
#include "render/Md2.h"

using namespace std;

#define PORTRAIT_SIZE 150
#define MODEL_SIZE 210

bool willModelPlaySound = false;

// this is here to compile faster (otherwise shapepalette needs to be incl.)
std::map<CharacterModelInfo*, GLShape*> shapesMap;

PcEditor::PcEditor( Scourge *scourge ) {
	this->scourge = scourge;

	int x = 50;
	int y = 30;
	int w = 500;
	int h = 420;

	win = new Window( scourge->getSDLHandler(),
										x, y, w, h,
										"Character Editor", 
										false, 
										Window::BASIC_WINDOW, 
										"default" );
  win->setVisible( false );
  win->setModal( true );  

	x = 10;	
	int buttonHeight = 17;
	int buttonSpace = 5;
	y = buttonSpace;
	int firstColWidth = 115;
	int secondColStart = firstColWidth + 20;
	int secondColWidth = w - secondColStart - 20;

	nameButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Name", true );
	y += buttonHeight + buttonSpace;
	profButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Profession", true );
	y += buttonHeight + buttonSpace;
	statsButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Stats", true );
	y += buttonHeight + buttonSpace;
	deityButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Deity", true );
	y += buttonHeight + buttonSpace;
	imageButton = win->createButton( x, y, firstColWidth, y + buttonHeight, "Image", true );
	y += buttonHeight + buttonSpace;
	okButton = win->createButton( x, 
																h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - buttonHeight, 
																firstColWidth, 
																h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT, 
																"Ok" );
	cancelButton = win->createButton( x + firstColWidth + buttonSpace, 
																		h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - buttonHeight, 
																		firstColWidth + buttonSpace + firstColWidth, 
																		h - x - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT, 
																		"Cancel" );

	cards = new CardContainer( win );  


	// ----------------------------------------------
	// name
	Label *p = cards->createLabel( secondColStart, 30, "Name:", NAME_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );
	nameField = new TextField( secondColStart, 50, 32 );
	cards->addWidget( nameField, NAME_TAB );
	cards->addWidget( new ScrollingLabel( secondColStart, 80, 
																				secondColWidth, 50, 
																				"Enter a name for this character." ), 
										NAME_TAB );


	// ----------------------------------------------
	// class
	p = cards->createLabel( secondColStart, 30, "Profession:", CLASS_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );


  charType = new ScrollingList( secondColStart, 50, 
																secondColWidth, 80, 
																scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( charType, CLASS_TAB );
  charTypeStr = (char**)malloc( Character::rootCharacters.size() * sizeof(char*));
  for(int i = 0; i < (int)Character::rootCharacters.size(); i++) {
    charTypeStr[i] = (char*)malloc( 120 * sizeof(char) );
    strcpy( charTypeStr[i], Character::rootCharacters[i]->getName() );
  }
  charType->setLines( (int)Character::rootCharacters.size(), (const char**)charTypeStr );
  int charIndex = (int)( (float)( Character::rootCharacters.size() ) * rand()/RAND_MAX );
  charType->setSelectedLine( charIndex );
  charTypeDescription = new ScrollingLabel( secondColStart, 140, 
																						secondColWidth, 150, 
																						Character::rootCharacters[charIndex]->getDescription() );
	cards->addWidget( charTypeDescription, CLASS_TAB );


	// ----------------------------------------------
	// stats
	p = cards->createLabel( secondColStart, 30, "Statistics:", STAT_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );

	int n = 0;
	for( int i = 0; n < 10 && i < (int)Skill::skills.size(); i++ ) {
		Skill *skill = Skill::skills[i];
		if( skill->getGroup()->isStat() ) {
			y = 60 + n * buttonHeight;
			cards->createLabel( secondColStart, y, skill->getName(), STAT_TAB );
			skillValue[n] = cards->createLabel( secondColStart + 120, y, 
																					"0", 
																					STAT_TAB );
			skillPlus[n] = cards->createButton( secondColStart + 150, y - 10,
																					secondColStart + 150 + 25, y - 10 + buttonHeight,
																					"+", 
																					STAT_TAB );
			skillPlus[n]->setFontType( Constants::SCOURGE_DEFAULT_FONT );
			skillMinus[n] = cards->createButton( secondColStart + 180, y - 10,
																					 secondColStart + 180 + 25, y - 10 + buttonHeight,
																					 "-", 
																					 STAT_TAB );
			skillMinus[n]->setFontType( Constants::SCOURGE_DEFAULT_FONT );

			n++;
		}
	}
	cards->createLabel( secondColStart + 180 + 25 + 15, 60, "Points Remaining:", STAT_TAB );
	remainingLabel = cards->createLabel( secondColStart + 180 + 25 + 15, 80, "0", STAT_TAB );

	int detailsHeight = 145;
  detailsInfo = new CharacterInfoUI( scourge );
  detailsCanvas = new Canvas( secondColStart, 200, 
															secondColStart + secondColWidth, 200 + detailsHeight, 
															detailsInfo );
  cards->addWidget( detailsCanvas, STAT_TAB );



	// ----------------------------------------------
	// deity
	p = cards->createLabel( secondColStart, 30, "Patron Deity:", DEITY_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );

  int deityHeight = 100;
  deityType = new ScrollingList( secondColStart, 50, 
                                 secondColWidth, deityHeight, 
                                 scourge->getShapePalette()->getHighlightTexture() );
  cards->addWidget( deityType, DEITY_TAB );
  deityTypeStr = (char**)malloc( MagicSchool::getMagicSchoolCount() * sizeof(char*) );
  for( int i = 0; i < MagicSchool::getMagicSchoolCount(); i++ ) {
    deityTypeStr[i] = (char*)malloc( 120 * sizeof( char ) );
    strcpy( deityTypeStr[i], MagicSchool::getMagicSchool( i )->getDeity() );
  }
  deityType->setLines( MagicSchool::getMagicSchoolCount(), (const char**)deityTypeStr );
  int deityIndex = (int)( (float)( MagicSchool::getMagicSchoolCount() * rand()/RAND_MAX ) );
  deityType->setSelectedLine( deityIndex );

  deityTypeDescription = new ScrollingLabel( secondColStart, 50 + 10 + deityHeight, 
                                             secondColWidth, 170, 
                                             MagicSchool::getMagicSchool( deityIndex )->getDeityDescription() );
  cards->addWidget( deityTypeDescription, DEITY_TAB );


	// ----------------------------------------------
	// appearence
	p = cards->createLabel( secondColStart, 30, "Appearance:", IMAGE_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );

  // portrait
  int imageWidth = PORTRAIT_SIZE;
  portrait = new Canvas( secondColStart, 50, 
                         secondColStart + imageWidth, 50 + PORTRAIT_SIZE, this );
  cards->addWidget( portrait, IMAGE_TAB );
  portraitIndex = (int)( (float)( scourge->getShapePalette()->getPortraitCount() ) * rand()/RAND_MAX );
  prevPortrait = cards->createButton( secondColStart, 50 + PORTRAIT_SIZE + 10,
                                      secondColStart + imageWidth / 2 - 5, 50 + PORTRAIT_SIZE + 10 + buttonHeight, 
                                      "<<", IMAGE_TAB );
  nextPortrait = cards->createButton( secondColStart + imageWidth / 2 + 5, 50 + PORTRAIT_SIZE + 10,
                                      secondColStart + imageWidth, 50 + PORTRAIT_SIZE + 10 + buttonHeight, 
                                      "    >>", IMAGE_TAB );
  // model
  int modelStart = secondColStart + imageWidth + 5;
  //int modelWidth = w - 10 - modelStart;
	int modelWidth = PORTRAIT_SIZE;
  model = new Canvas( modelStart, 50, modelStart + modelWidth, 50 + MODEL_SIZE, this );
  cards->addWidget( model, IMAGE_TAB );
  modelIndex = (int)( (float)( scourge->getShapePalette()->getCharacterModelInfoCount() ) * rand()/RAND_MAX );
  prevModel = cards->createButton( modelStart, 50 + MODEL_SIZE + 10,
                                   modelStart + modelWidth / 2 - 5, 50 + MODEL_SIZE + 10 + buttonHeight, 
                                   "<<", IMAGE_TAB );
  nextModel = cards->createButton( modelStart + modelWidth / 2 + 5, 50 + MODEL_SIZE + 10,
                                   modelStart + modelWidth, 50 + MODEL_SIZE + 10 + buttonHeight,
                                   "    >>", IMAGE_TAB );
}

PcEditor::~PcEditor() {
	delete win;
	deleteLoadedShapes();
}

void PcEditor::deleteLoadedShapes() {
  for( map<CharacterModelInfo*, GLShape*>::iterator i=shapesMap.begin(); i!=shapesMap.end(); ++i ) {
    CharacterModelInfo *cmi = i->first;
    GLShape *shape = i->second;  
    scourge->getShapePalette()->decrementSkinRefCount( cmi->model_name, cmi->skin_name );
    delete shape;
  }
  shapesMap.clear();
}

void PcEditor::setCreature( Creature *creature ) {
	deleteLoadedShapes();
	this->creature = creature;
	cards->setActiveCard( NAME_TAB );
	nameButton->setSelected( true );
	profButton->setSelected( false );
	statsButton->setSelected( false );
	deityButton->setSelected( false );
	imageButton->setSelected( false );
	win->setVisible( true );
}

void PcEditor::handleEvent( Widget *widget, SDL_Event *event ) {
	if( widget == cancelButton ) {
		win->setVisible( false );
	} else if( widget == okButton ) {
		win->setVisible( false );
	} else if( widget == nameButton ) {
		cards->setActiveCard( NAME_TAB );
		nameButton->setSelected( true );
		profButton->setSelected( false );
		statsButton->setSelected( false );
		deityButton->setSelected( false );
		imageButton->setSelected( false );
	} else if( widget == profButton ) {
		cards->setActiveCard( CLASS_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( true );
		statsButton->setSelected( false );
		deityButton->setSelected( false );
		imageButton->setSelected( false );
	} else if( widget == statsButton ) {
		cards->setActiveCard( STAT_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( false );
		statsButton->setSelected( true );
		deityButton->setSelected( false );
		imageButton->setSelected( false );
	} else if( widget == deityButton ) {
		cards->setActiveCard( DEITY_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( false );
		statsButton->setSelected( false );
		deityButton->setSelected( true );
		imageButton->setSelected( false );
	} else if( widget == imageButton ) {
		cards->setActiveCard( IMAGE_TAB );
		nameButton->setSelected( false );
		profButton->setSelected( false );
		statsButton->setSelected( false );
		deityButton->setSelected( false );
		imageButton->setSelected( true );
  } else if( widget == prevPortrait ) {
    if( portraitIndex > 0 ) {
      portraitIndex--;
    } else {
      portraitIndex = scourge->getShapePalette()->getPortraitCount() - 1;
    }
    saveUI();
  } else if( widget == nextPortrait ) {
    if( portraitIndex < scourge->getShapePalette()->getPortraitCount() - 1 ) {
      portraitIndex++;
    } else {
      portraitIndex = 0;
    }
    saveUI();
  } else if( widget == prevModel ) {
    if( modelIndex > 0 ) {
      modelIndex--;
    } else {
      modelIndex = scourge->getShapePalette()->getCharacterModelInfoCount() - 1;
    }
    saveUI();
    willModelPlaySound = true;
  } else if( widget == nextModel ) {
    if( modelIndex < scourge->getShapePalette()->getCharacterModelInfoCount() - 1 ) {
      modelIndex++;
    } else {
      modelIndex = 0;
    }
    saveUI();
    willModelPlaySound = true;
  }
}

void PcEditor::drawWidgetContents( Widget *w ) {
  if( w == portrait ) {
    glPushMatrix();
    glEnable( GL_TEXTURE_2D );
    glDisable( GL_CULL_FACE );
    glColor4f( 1, 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, 
                   scourge->getShapePalette()->getPortraitTexture( portraitIndex ) );
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
  } else if( w == model ) {
    // draw model
    CharacterModelInfo *cmi = scourge->getShapePalette()->
      getCharacterModelInfo( modelIndex );
    GLShape *shape;
    if( shapesMap.find( cmi ) == shapesMap.end() ) {
      shape = 
        scourge->getShapePalette()->getCreatureShape( cmi->model_name, 
                                                      cmi->skin_name, 
                                                      cmi->scale );
      shapesMap[ cmi ] = shape;
      shape->setCurrentAnimation( MD2_STAND );
    } else {
      shape = shapesMap[ cmi ];
    }
    if( willModelPlaySound ) {
      scourge->playCharacterSound( cmi->model_name, 
                                   GameAdapter::SELECT_SOUND );
      willModelPlaySound = false;
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

void PcEditor::saveUI() {

}
