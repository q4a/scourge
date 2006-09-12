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
#include "creature.h"
#include "shapepalette.h"
#include "characterinfo.h"
#include "debug.h"
#include "rpg/character.h"
#include "rpg/spell.h"
#include "render/glshape.h"
#include "render/Md2.h"

using namespace std;

// added comment to see if cia works (again)

#define AVAILABLE_SKILL_POINTS 5
#define PORTRAIT_SIZE 150
#define MODEL_SIZE 210

bool willModelPlaySound = false;

// this is here to compile faster (otherwise shapepalette needs to be incl.)
std::map<CharacterModelInfo*, GLShape*> shapesMap;

PcEditor::PcEditor( Scourge *scourge ) {
	this->scourge = scourge;
	this->creature = NULL;
  this->deleteCreature = false;

	availableSkillMod = AVAILABLE_SKILL_POINTS;

	createUI();

	deleteLoadedShapes();
}

PcEditor::~PcEditor() {
	delete win;
	if( deleteCreature ) delete creature;
  for(int i = 0; i < (int)Character::rootCharacters.size(); i++) {
		free( charTypeStr[i] );
	}
	free( charTypeStr );
  for( int i = 0; i < MagicSchool::getMagicSchoolCount(); i++ ) {
		free( deityTypeStr[i] );
	}
	free( deityTypeStr );
	deleteLoadedShapes();
}

void PcEditor::setCreature( Creature *c ) {
  if( deleteCreature ) delete creature;

  if( c ) {
    this->creature = c;
    deleteCreature = false;
  } else {
    // create a tmp creature to use for the ui	
    this->creature = createPartyMember();
    deleteCreature = true;
  }

  availableSkillMod = AVAILABLE_SKILL_POINTS;

	detailsInfo->setCreature( win, creature );

	loadUI();

	cards->setActiveCard( NAME_TAB );
	nameButton->setSelected( true );
	profButton->setSelected( false );
	statsButton->setSelected( false );
	deityButton->setSelected( false );
	imageButton->setSelected( false );

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

/**
 * Save the moving parts into the creature.
 */
void PcEditor::saveUI() {
	// name
	char *s = nameField->getText();
	bool deleteS = false;
	if( !s || !strlen( s ) ) {
		//s = presets[i].name;
		s = Rpg::createName();
		deleteS = true;
	}
	creature->replaceName( strdup( s ) );
	if( deleteS ) free( s );
	// character type
	int index = charType->getSelectedLine();  
	Character *c = Character::rootCharacters[ index ];
	creature->setCharacter( c );
	creature->setLevel( STARTING_PARTY_LEVEL ); 
	creature->setExp(0);
	creature->setHp();
	creature->setMp();

	// deity
	creature->setDeityIndex( deityType->getSelectedLine() );
	
	// assign portraits
	creature->setPortraitTextureIndex( portraitIndex );
}

void PcEditor::loadUI() {
	if( creature ) {
		nameField->setText( creature->getName() );

		for(int i = 0; i < (int)Character::rootCharacters.size(); i++) {
			if( Character::rootCharacters[i] == creature->getCharacter() ) {
				charType->setSelectedLine( i );
				break;
			}
		}

		if( charType->getSelectedLine() > -1 ) 
			charTypeDescription->setText( Character::rootCharacters[charType->getSelectedLine()]->getDescription() );

		char message[300];
		int n = 0;
		for( int i = 0; n < 10 && i < (int)Skill::skills.size(); i++ ) {
			Skill *sk = Skill::skills[i];
			if( sk->getGroup()->isStat() ) {
				sprintf( message, "%d (%d)", creature->getSkill( i ), creature->getSkillMod( i ) );
				skillValue[n++]->setText( message );
			}
		}
		sprintf( message, "%d", availableSkillMod );
		remainingLabel->setText( message );

		int deityIndex = (int)( (float)( MagicSchool::getMagicSchoolCount() * rand()/RAND_MAX ) );
		deityType->setSelectedLine( deityIndex );
		deityTypeDescription->setText( MagicSchool::getMagicSchool( deityIndex )->getDeityDescription() );

		for( int i = 0; i < scourge->getShapePalette()->getPortraitCount(); i++ ) {
			if( creature->getPortraitTextureIndex() == i ) {
				portraitIndex = i;
				break;
			}
		}

		for( int i = 0; i < scourge->getShapePalette()->getCharacterModelInfoCount(); i++ ) {
			if( !strcmp( creature->getModelName(), scourge->getShapePalette()->getCharacterModelInfo( i )->model_name ) &&
					!strcmp( creature->getSkinName(), scourge->getShapePalette()->getCharacterModelInfo( i )->skin_name ) ) {
				modelIndex = i;
				break;
			}
		}

	} else {
		nameField->setText( "" );
		int n = 0;
		for( int i = 0; n < 10 && i < (int)Skill::skills.size(); i++ ) {
			Skill *sk = Skill::skills[i];
			if( sk->getGroup()->isStat() ) {
				skillValue[n++]->setText( "" );
			}
		}
	}
}

void PcEditor::rollSkills() {
	availableSkillMod = AVAILABLE_SKILL_POINTS;
	rollSkillsForCreature( creature );  		
	loadUI();
}

void PcEditor::rollSkillsForCreature( Creature *c ) {
	for(int i = 0; i < Skill::SKILL_COUNT; i++) {
		int n;
		if( Skill::skills[i]->getGroup()->isStat() ) {
			n = c->getCharacter()->getSkill( i ) + (int)( 14.0f * rand() / RAND_MAX ) + 1;
		} else {
			
			// create the starting value as a function of the stats
			n = 0;
			for( int t = 0; t < Skill::skills[i]->getPreReqStatCount(); t++ ) {
				int index = Skill::skills[i]->getPreReqStat( t )->getIndex();
				n += c->getSkill( index );
			}
			n = (int)( ( n / (float)( Skill::skills[i]->getPreReqStatCount() ) ) * 
								 (float)( Skill::skills[i]->getPreReqMultiplier() ) );
		}
		c->setSkill( i, n );
		c->setSkillMod( i, 0 );
	}
}

Creature *PcEditor::createPartyMember() {
	Creature *c = new Creature( scourge->getSession(), 
                              Character::rootCharacters[ charType->getSelectedLine() ], 
                              strdup( nameField->getText() ), 
                              modelIndex );
	c->setLevel( STARTING_PARTY_LEVEL ); 
	c->setExp(0);
	c->setHp();
	c->setMp();
	c->setHunger((int)(5.0f * rand()/RAND_MAX) + 5);
	c->setThirst((int)(5.0f * rand()/RAND_MAX) + 5); 

  // stats
  if( creature ) {
    // copy skills from prototype creature
    for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
      c->setSkill( i, creature->getSkill( i, true ) );
      c->setSkillMod( i, 0 );
    }
  } else {
    // roll the skills
    rollSkillsForCreature( c );
  }

	// deity
	c->setDeityIndex( deityType->getSelectedLine() );
	
	// assign portraits
	c->setPortraitTextureIndex( portraitIndex );

	return c;
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
	} else if( widget == charType ) {
		setCharType( charType->getSelectedLine() );
	} else if( widget == deityType ) {
		setDeityType( deityType->getSelectedLine() );
  } else if( widget == reroll ) {
    rollSkills();
  } else {
    int n = 0;
    for( int i = 0; n < 10 && i < (int)Skill::skills.size(); i++ ) {
      Skill *sk = Skill::skills[i];
      if( sk->getGroup()->isStat() ) {
        if( widget == skillMinus[n] ) {
          if( creature->getSkillMod( n ) > 0 && 
              availableSkillMod < AVAILABLE_SKILL_POINTS ) {
            creature->setSkillMod( n, creature->getSkillMod( n ) - 1 );
            availableSkillMod++;
          }
          loadUI();
        } else if( widget == skillPlus[n] ) {
          if( creature->getSkill( n, true ) < 20 && 
              availableSkillMod > 0 ) {
            creature->setSkillMod( n, creature->getSkillMod( n ) + 1 );
            availableSkillMod--;
          }
          loadUI();
        }
        n++;
      }
    }
  }
}

void PcEditor::setCharType( int charIndex ) {
  if( charIndex > -1 ) {     
    Character *character = Character::rootCharacters[ charIndex ];
    if( character ) {
      charTypeDescription->setText( character->getDescription() );
			saveUI();
      rollSkills();      
    }
  }
}

void PcEditor::setDeityType( int deityIndex ) {
  if( deityIndex > -1 ) {
    MagicSchool *school = MagicSchool::getMagicSchool( deityIndex );
    if( school ) deityTypeDescription->setText( school->getDeityDescription() );
    saveUI();
  }
}

#define TEXT_WIDTH 55

void PcEditor::createUI() {	
	int w = 500;
	int h = 420;
	int x = scourge->getScreenWidth() / 2 - ( w / 2 );
	int y = scourge->getScreenHeight() / 2 - ( h / 2 );

	win = new Window( scourge->getSDLHandler(),
										x, y, w, h,
										"Character Details", 
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
																h - x - buttonHeight - win->getGutter(), 
																firstColWidth, 
																h - x - win->getGutter(), 
																"Accept" );
	cancelButton = win->createButton( x + firstColWidth + buttonSpace, 
																		h - x - buttonHeight - win->getGutter(), 
																		firstColWidth + buttonSpace + firstColWidth, 
																		h - x - win->getGutter(), 
																		"Dismiss" );

	cards = new CardContainer( win );  


	// ----------------------------------------------
	// name
	Label *p = cards->createLabel( secondColStart, 30, "Name:", NAME_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );
	nameField = new TextField( secondColStart, 50, 32 );
  char *s = Rpg::createName();
  nameField->setText( s );
  free( s );
	cards->addWidget( nameField, NAME_TAB );
	cards->addWidget( new Label( secondColStart, 90, 
															 "What is your name, great hero? Enter it here, but choose wisely! You will not be able to change it again.",
															 TEXT_WIDTH ), 
										NAME_TAB );


	// ----------------------------------------------
	// class
	p = cards->createLabel( secondColStart, 30, "Profession:", CLASS_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );

	cards->addWidget( new Label( secondColStart, 50, 
															 "Which starting profession most closely resembles your hero? \
Your character will evolve into a more powerful and refined practicer \
of the chosen art. Below is a brief description of the strengths and \
weaknesses of each profession.",
															 TEXT_WIDTH ), 
										CLASS_TAB );


  charType = new ScrollingList( secondColStart, 130, 
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
  charTypeDescription = new ScrollingLabel( secondColStart, 230, 
																						secondColWidth, 130, 
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
			skillValue[n] = cards->createLabel( secondColStart + 105, y, 
																					"", 
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
  reroll = cards->createButton( secondColStart + 180 + 25 + 15, 100, w - 10, 120, "Reroll", STAT_TAB );

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

	cards->addWidget( new Label( secondColStart, 50, 
															 "At times on your journey you may be reduced to nothing but a prayer. \
Therefore it is important to keep in mind to whom such requests are directed. Given below is a list \
of known deities of the land with a brief description for each.",
															 TEXT_WIDTH ), 
										DEITY_TAB );

  int deityHeight = 100;
  deityType = new ScrollingList( secondColStart, 130, 
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

  deityTypeDescription = new ScrollingLabel( secondColStart, 130 + 10 + deityHeight, 
                                             secondColWidth, 120, 
                                             MagicSchool::getMagicSchool( deityIndex )->getDeityDescription() );
  cards->addWidget( deityTypeDescription, DEITY_TAB );


	// ----------------------------------------------
	// appearence
	p = cards->createLabel( secondColStart, 30, "Appearance:", IMAGE_TAB );
	p->setFontType( Constants::SCOURGE_LARGE_FONT );

	cards->addWidget( new Label( secondColStart, 50, 
															 "You may now choose a portrait and a character model \
to represent your hero. Your appearance is only a matter of personal choice, it will \
not affect the game in any way.",
															 TEXT_WIDTH ), 
										IMAGE_TAB );

  // portrait
	int yy = 110;
  int imageWidth = PORTRAIT_SIZE;
  portrait = new Canvas( secondColStart, yy, 
                         secondColStart + imageWidth, yy + PORTRAIT_SIZE, this );
  cards->addWidget( portrait, IMAGE_TAB );
  portraitIndex = (int)( (float)( scourge->getShapePalette()->getPortraitCount() ) * rand()/RAND_MAX );
  prevPortrait = cards->createButton( secondColStart, yy + PORTRAIT_SIZE + 10,
                                      secondColStart + imageWidth / 2 - 5, yy + PORTRAIT_SIZE + 10 + buttonHeight, 
                                      "<<", IMAGE_TAB );
  nextPortrait = cards->createButton( secondColStart + imageWidth / 2 + 5, yy + PORTRAIT_SIZE + 10,
                                      secondColStart + imageWidth, yy + PORTRAIT_SIZE + 10 + buttonHeight, 
                                      "    >>", IMAGE_TAB );
  // model
  int modelStart = secondColStart + imageWidth + 25;
  //int modelWidth = w - 10 - modelStart;
	int modelWidth = PORTRAIT_SIZE;
  model = new Canvas( modelStart, yy, 
											modelStart + modelWidth, yy + MODEL_SIZE, this );
  cards->addWidget( model, IMAGE_TAB );
  modelIndex = (int)( (float)( scourge->getShapePalette()->getCharacterModelInfoCount() ) * rand()/RAND_MAX );
  prevModel = cards->createButton( modelStart, yy + MODEL_SIZE + 10,
                                   modelStart + modelWidth / 2 - 5, yy + MODEL_SIZE + 10 + buttonHeight, 
                                   "<<", IMAGE_TAB );
  nextModel = cards->createButton( modelStart + modelWidth / 2 + 5, yy + MODEL_SIZE + 10,
                                   modelStart + modelWidth, yy + MODEL_SIZE + 10 + buttonHeight,
                                   "    >>", IMAGE_TAB );
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
		glBegin( GL_QUADS );
		glColor3f( 0, 0.1f, 0.25f );
		glVertex2f( 0, 0 );
		glColor3f( 0, 0, 0 );
		glVertex2f( 0, model->getHeight() );
		glColor3f( 0, 0.1f, 0.25f );
		glVertex2f( model->getWidth(), model->getHeight() );
		glColor3f( 0, 0, 0 );
		glVertex2f( model->getWidth(), 0 );
		glEnd();
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

