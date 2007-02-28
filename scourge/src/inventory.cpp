/***************************************************************************
                          inventory.cpp  -  description
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

#include "inventory.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "sound.h"
#include "sdlhandler.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include "scourge.h"
#include "item.h"
#include "creature.h"
#include "tradedialog.h"
#include "sqbinding/sqbinding.h"
#include "characterinfo.h"
#include "shapepalette.h"
#include "skillsview.h"
#include "gui/confirmdialog.h"

using namespace std;

/**
  *@author Gabor Torok
  */


#define START_OF_SECOND_BUTTON_SET 140

#define APPLY_SKILL_MODS 1

Inventory::Inventory(Scourge *scourge) {
  this->scourge = scourge;



  // allocate strings for list
  this->formationText = (char**)malloc(10 * sizeof(char*));
  for(int i = 0; i < 10; i++) {
    this->formationText[i] = (char*)malloc(120 * sizeof(char));
  }
  this->schoolColors = (Color*)malloc( MagicSchool::getMagicSchoolCount() * sizeof( Color ) );
  this->itemColor = (Color*)malloc(MAX_INVENTORY_SIZE * sizeof(Color));
  this->pcInvText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  this->itemIcon = (GLuint*)malloc(MAX_INVENTORY_SIZE * sizeof(GLuint));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->pcInvText[i] = (char*)malloc(120 * sizeof(char));
  }
  this->stateLine = (char**)malloc(Constants::STATE_MOD_COUNT * sizeof(char*));
  this->icons = (GLuint*)malloc(Constants::STATE_MOD_COUNT * sizeof(GLuint));
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    this->stateLine[i] = (char*)malloc(120 * sizeof(char));
  }
  this->protStateLine = (char**)malloc(Constants::STATE_MOD_COUNT * sizeof(char*));
  this->protIcons = (GLuint*)malloc(Constants::STATE_MOD_COUNT * sizeof(GLuint));
  for(int i = 0; i < Constants::STATE_MOD_COUNT; i++) {
    this->protStateLine[i] = (char*)malloc(120 * sizeof(char));
  }
  this->objectiveText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  this->missionColor = (Color*)malloc(MAX_INVENTORY_SIZE * sizeof(Color));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->objectiveText[i] = (char*)malloc(120 * sizeof(char));
  }
  this->schoolText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->schoolText[i] = (char*)malloc(120 * sizeof(char));
  }
  this->spellText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  this->spellIcons = (GLuint*)malloc(MAX_INVENTORY_SIZE * sizeof(GLuint));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->spellText[i] = (char*)malloc(120 * sizeof(char));
  }
  this->specialColor = (Color*)malloc(MAX_INVENTORY_SIZE * sizeof(Color));
  this->specialText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  this->specialIcons = (GLuint*)malloc(MAX_INVENTORY_SIZE * sizeof(GLuint));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->specialText[i] = (char*)malloc(120 * sizeof(char));
  }
  selected = selectedMode = 0;

	confirmDialog = new ConfirmDialog( scourge->getSDLHandler() );

  // construct UI
  mainWin = new Window( scourge->getSDLHandler(),
                        scourge->getSDLHandler()->getScreen()->w - Scourge::INVENTORY_WIDTH, 
                        scourge->getSDLHandler()->getScreen()->h - Scourge::PARTY_GUI_HEIGHT - 
                        Scourge::INVENTORY_HEIGHT - Window::SCREEN_GUTTER,
                        Scourge::INVENTORY_WIDTH, Scourge::INVENTORY_HEIGHT,
                        "", false, Window::SIMPLE_WINDOW, "default" );
  mainWin->setLocked( true );
  mainWin->setAnimation( Window::SLIDE_UP );

  int buttonHeight = 20;
  int descriptionHeight = 100;
  int yy = 0;
  inventoryButton = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Inventory", true);
  yy += buttonHeight;
  skillsButton   = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Skills", true);
  yy += buttonHeight;
  spellsButton   = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Spells", true);
  yy += buttonHeight;
  specialButton   = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Capabilities", true);
  yy += buttonHeight;
  missionButton   = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Mission", true);
  yy += buttonHeight;
  partyButton   = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Party", true);
  yy += buttonHeight;

  yy = mainWin->getHeight() - 50;
  closeButton = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Hide" );

  cards = new CardContainer(mainWin);


  // -------------------------------------------
  // inventory page	
  cards->createLabel(115, 10, "Inventory:", INVENTORY, Constants::RED_COLOR); 
  inventoryWeightLabel = cards->createLabel(190, 10, NULL, INVENTORY);

  coinsLabel = cards->createLabel(300, 155, NULL, INVENTORY);
  cards->createLabel(115, 155, "Equipped Items:", INVENTORY, Constants::RED_COLOR);

  invList = new ScrollingList(115, 15, 295, 125, scourge->getShapePalette()->getHighlightTexture(), this, 30);
  cards->addWidget(invList, INVENTORY);
  cards->createLabel(115, 430, Constants::getMessage(Constants::EXPLAIN_DRAG_AND_DROP), INVENTORY);

  paperDoll = new Canvas(115, 160, 411, 191 + (Constants::INVENTORY_COUNT * 15), this, this);
  cards->addWidget(paperDoll, INVENTORY);

  yy = START_OF_SECOND_BUTTON_SET;
  equipButton    = cards->createButton( 0, yy, 105, yy + buttonHeight, "Don/Doff", INVENTORY);
  yy+=buttonHeight;
  openButton     = cards->createButton( 0, yy, 105, yy + buttonHeight, Constants::getMessage(Constants::OPEN_CONTAINER_LABEL), INVENTORY ); 
  yy+=buttonHeight;
  eatDrinkButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Eat/Drink", INVENTORY );
  yy+=buttonHeight;
  castScrollButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Cast Spell", INVENTORY );
  yy+=buttonHeight;
  transcribeButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Transcribe", INVENTORY );
  yy+=buttonHeight;
  enchantButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Enchant", INVENTORY );
  yy+=buttonHeight;
  infoButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Info", INVENTORY );
  yy+=buttonHeight;
  poolButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Pool Money", INVENTORY );
  yy+=buttonHeight;
  storeItemButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Store", INVENTORY, true );
  yy+=buttonHeight;

  yy+=5;
  cards->createLabel( 3, yy + 14, "Selected Weapon:", INVENTORY );
  yy+=buttonHeight;
  preferredWeaponLocation[0] = Constants::INVENTORY_LEFT_HAND;
  preferredWeaponLocation[1] = Constants::INVENTORY_RIGHT_HAND;
  preferredWeaponLocation[2] = Constants::INVENTORY_WEAPON_RANGED;
  preferredWeaponButton[ 0 ] = 
    cards->createButton( 0, yy, 105, yy + buttonHeight, "Left hand", 
                         INVENTORY, true );
  preferredWeaponButton[ 0 ]->setSelected( false );
  yy+=buttonHeight;
  preferredWeaponButton[ 1 ] = 
    cards->createButton( 0, yy, 105, yy + buttonHeight, "Right hand", 
                         INVENTORY, true );
  preferredWeaponButton[ 1 ]->setSelected( false );
  yy+=buttonHeight;
  preferredWeaponButton[ 2 ] = 
    cards->createButton( 0, yy, 105, yy + buttonHeight, "Ranged", 
                         INVENTORY, true );
  preferredWeaponButton[ 2 ]->setSelected( false );
  yy+=buttonHeight;

  // -------------------------------------------
  // character info
  cards->createLabel(115, 10, "Character stats:", CHARACTER, Constants::RED_COLOR);
  charInfoUI = new CharacterInfoUI( scourge );
  int canvasHeight = 190;
  attrCanvas     = new Canvas( 115, 15, 405, canvasHeight, charInfoUI );
  cards->addWidget( attrCanvas, CHARACTER );

  int skillY = canvasHeight + 15;
  cards->createLabel(115, skillY, "Current State:", CHARACTER, Constants::RED_COLOR);
  stateList = new ScrollingList(115, skillY + 5, 140, 70, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(stateList, CHARACTER);
  
  cards->createLabel(265, skillY, "Protected States:", CHARACTER, Constants::RED_COLOR);
  protStateList = new ScrollingList(265, skillY + 5, 140, 70, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(protStateList, CHARACTER);

  skillY += 85;
  strcpy(skillsStr, "Skills:");
  skillLabel = cards->createLabel(115, skillY, skillsStr, CHARACTER, Constants::RED_COLOR);
  //skillList = new ScrollingList(115, skillY + 5, 290, 405 - ( skillY + 5 ), scourge->getShapePalette()->getHighlightTexture());
  skillList = new SkillsView( scourge, 115, skillY + 5, 290, 405 - ( skillY + 5 ) );
  cards->addWidget( skillList->getWidget(), CHARACTER );

	//yy = START_OF_SECOND_BUTTON_SET;
	yy = skillY - 10;
  skillModLabel = cards->createLabel( 5, yy + 10, "Skill: 0", CHARACTER );
  yy+=buttonHeight;
	addModButton = cards->createButton( 0, yy, 52, yy + buttonHeight, "Add", CHARACTER );
	delModButton = cards->createButton( 53, yy, 105, yy + buttonHeight, "Del", CHARACTER );
  yy+=buttonHeight;
	acceptModButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Accept", CHARACTER );
  yy+=buttonHeight;

  // -------------------------------------------
  // spellbook
  cards->createLabel(115, 10, "School of magic: (with provider deity)", SPELL, Constants::RED_COLOR);
  schoolList = new ScrollingList(115, 15, 290, 100, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(schoolList, SPELL);
  cards->createLabel(115, 135, "Spells memorized:", SPELL, Constants::RED_COLOR);
  spellList = new ScrollingList(115, 140, 290, 150, scourge->getShapePalette()->getHighlightTexture(), NULL, 30);
  cards->addWidget(spellList, SPELL);
  cards->createLabel(115, 310, "Spell notes:", SPELL, Constants::RED_COLOR);
  spellDescriptionLabel = new ScrollingLabel( 115, 325, 290, descriptionHeight, "" );
  cards->addWidget(spellDescriptionLabel, SPELL);

  yy = START_OF_SECOND_BUTTON_SET;
  castButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Cast", SPELL);
  yy+=buttonHeight;
  storeSpellButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Store", SPELL, true);
  yy+=buttonHeight;
  storable = NULL;

  // -------------------------------------------
  // special skills
  cards->createLabel(115, 10, "Special Capabilities", SPECIAL, Constants::RED_COLOR);
  cards->createLabel(115, 25, "(If grayed-out, you don't have the capability.)", SPECIAL, Constants::RED_COLOR);
  specialList = new ScrollingList(115, 30, 290, 265, scourge->getShapePalette()->getHighlightTexture(), NULL, 30);
  cards->addWidget(specialList, SPECIAL);
  cards->createLabel(115, 310, "Capability Description:", SPECIAL, Constants::RED_COLOR);
  specialDescriptionLabel = new ScrollingLabel( 115, 325, 290, descriptionHeight, "" );
  cards->addWidget(specialDescriptionLabel, SPECIAL);

  yy = START_OF_SECOND_BUTTON_SET;
  useSpecialButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Use", SPECIAL);
  yy += buttonHeight;
  storeSpecialButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Store", SPECIAL, true);
  yy += buttonHeight;
  //storeSpell = NULL;

  // -------------------------------------------
  // mission
  cards->createLabel(115, 10, "Current Mission", MISSION, Constants::RED_COLOR);
  missionDescriptionLabel = new ScrollingLabel(115, 25, 295, 275, "");
  cards->addWidget(missionDescriptionLabel, MISSION);
  cards->createLabel(115, 320, "Mission Objectives", MISSION, Constants::RED_COLOR);
  objectiveList = new ScrollingList(115, 325, 295, 100, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(objectiveList, MISSION);



  // -------------------------------------------
  // party
  cards->createLabel( 115, 10, "Group formation:", PARTY );
  formationList = new ScrollingList(115, 20, 295, 100, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(formationList, PARTY);
  strcpy( formationText[0], "Diamond" );
  strcpy( formationText[1], "Staggered" );
  strcpy( formationText[2], "Square" );
  strcpy( formationText[3], "Row" );
  strcpy( formationText[4], "Scout" );
  strcpy( formationText[5], "Cross" );
  formationList->setLines( 6, (const char**)formationText );

  cards->createLabel( 115, 145, "Interface Layout:", PARTY );
  layoutButton1 = cards->createButton( 115, 155, 205, 175, "Floating", PARTY );
  layoutButton1->setToggle( true );
  layoutButton2 = cards->createButton( 210, 155, 300, 175, "Bottom", PARTY );
  layoutButton2->setToggle( true );
  layoutButton4 = cards->createButton( 305, 155, 395, 175, "Inventory", PARTY );
  layoutButton4->setToggle( true );

  squirrelWindow = cards->createButton( 115, 180, 245, 200, "Show Console", PARTY, true );
  squirrelWindow->setSelected( scourge->getSquirrelConsole()->isVisible() );

//  saveGameButton = cards->createButton( 115, 205, 245, 225, "Save Game", PARTY );

  setSelectedPlayerAndMode(0, INVENTORY);
}

Inventory::~Inventory() {
}

void Inventory::showSpells() {
  setSelectedPlayerAndMode( selected, SPELL );
}

void Inventory::showSpecial() {
  setSelectedPlayerAndMode( selected, SPECIAL );
}

void Inventory::showSkills() {
  setSelectedPlayerAndMode( selected, CHARACTER );
}

void Inventory::drawWidgetContents(Widget *w) {
  GuiTheme *theme = mainWin->getTheme();
  //Creature *p = scourge->getParty()->getParty(selected);

  if(w == paperDoll) {
    float x = 125;
    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
      Item *item = scourge->getParty()->getParty(selected)->getEquippedInventory(i);
      if(item) {
        if( ( 1 << i ) == 
            scourge->getParty()->getParty(selected)->getPreferredWeapon() ) {
          glColor4f( 0.05f, 0.35f, 0.5f, 1 );
        } else if( theme->getSelectionBackground() ) {
          glColor4f( theme->getSelectionBackground()->color.r,
                     theme->getSelectionBackground()->color.g,
                     theme->getSelectionBackground()->color.b,
                     theme->getSelectionBackground()->color.a );
        } else {
          w->applySelectionColor();
        }
        glBegin( GL_QUADS );
        glVertex2f( x, i * 16 );
        glVertex2f( x, (i + 1) * 16 );
        glVertex2f( w->getWidth(), (i + 1) * 16 );
        glVertex2f( w->getWidth(), i * 16 );
        glEnd();
      }
    }

    ShapePalette *shapePal = scourge->getShapePalette();
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0);        
    glPixelZoom( 1.0, -1.0 );
    glRasterPos2f( 1, 1 );
    glDrawPixels(shapePal->paperDoll->w, shapePal->paperDoll->h,
                 GL_BGRA, GL_UNSIGNED_BYTE, shapePal->paperDollImage);
    glDisable(GL_ALPHA_TEST);

    if( theme->getButtonBorder() ) {
      glColor4f( theme->getButtonBorder()->color.r,
                 theme->getButtonBorder()->color.g,
                 theme->getButtonBorder()->color.b,
                 theme->getButtonBorder()->color.a );
    } else {
      w->applyBorderColor();
    }
    glBegin( GL_LINES );
    glVertex2f( x, 0 );
    glVertex2f( x, w->getHeight() );
    glEnd();

    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
      Item *item = scourge->getParty()->getParty(selected)->getEquippedInventory(i);

      if( theme->getButtonBorder() ) {
        glColor4f( theme->getButtonBorder()->color.r,
                   theme->getButtonBorder()->color.g,
                   theme->getButtonBorder()->color.b,
                   theme->getButtonBorder()->color.a );
      } else {
        w->applyBorderColor();
      }
      glBegin( GL_LINES );
      glVertex2f( x, (i + 1) * 16 );
      glVertex2f( w->getWidth(), (i + 1) * 16 );
      glEnd();

      if( theme->getWindowText() ) {
        glColor4f( theme->getWindowText()->r,
                   theme->getWindowText()->g,
                   theme->getWindowText()->b,
                   theme->getWindowText()->a );
      } else {
        w->applyColor();
      }      
      if(!item) continue;
      scourge->getSDLHandler()->texPrint( x + 5, (i + 1) * 16 - 4, item->getItemName());
    }
  }
}

bool Inventory::handleEvent(Widget *widget, SDL_Event *event) {
  Creature *creature = scourge->getParty()->getParty(selected);
  if( widget == mainWin->closeButton || widget == closeButton ) {
    // mainWin->setVisible(false);
    scourge->toggleInventoryWindow();
  } else if(widget == inventoryButton) setSelectedPlayerAndMode(selected, INVENTORY);
  else if(widget == skillsButton) setSelectedPlayerAndMode(selected, CHARACTER);
  else if(widget == spellsButton) setSelectedPlayerAndMode(selected, SPELL);
  else if(widget == specialButton) setSelectedPlayerAndMode(selected, SPECIAL);
  else if(widget == missionButton)  setSelectedPlayerAndMode(selected, MISSION);
  else if(widget == partyButton) setSelectedPlayerAndMode(selected, PARTY);
  else if(widget == invList && 
					scourge->getTargetSelectionFor() && 
					invList->getEventType() == ScrollingList::EVENT_ACTION ) {
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1) {
      Item *item = scourge->getParty()->getParty(selected)->getInventory(itemIndex);
      scourge->handleTargetSelectionOfItem( item );
    }
  } else if(widget == infoButton || 
          (widget == invList && scourge->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT)) {
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1) {
      Item *item = scourge->getParty()->getParty(selected)->getInventory(itemIndex);
      scourge->getInfoGui()->setItem( item );
      if(!scourge->getInfoGui()->getWindow()->isVisible()) 
        scourge->getInfoGui()->getWindow()->setVisible( true );
    }
  } else if( widget == poolButton ) {
    for( int i = 0; i < scourge->getSession()->getParty()->getPartySize(); i++ ) {
      Creature *c = scourge->getSession()->getParty()->getParty(i);
      if( c != creature ) {
        creature->setMoney( creature->getMoney() + c->getMoney() );
        c->setMoney( 0 );
      }
    }
    char msg[120];
    sprintf( msg, "Party members give all their money to %s.", creature->getName() );
    scourge->getMap()->addDescription( msg );
    refresh();
  } else if( widget == paperDoll && scourge->getSDLHandler()->mouseButton == SDL_BUTTON_RIGHT ) {
    int invLocation = ( scourge->getSDLHandler()->mouseY - paperDoll->getY() - mainWin->getY() ) / 16 - 1;
    if( invLocation >= 0 && invLocation < Constants::INVENTORY_COUNT ) {
      Item *item = scourge->getParty()->getParty(selected)->getEquippedInventory( invLocation );
      if( item ) {
        scourge->getInfoGui()->setItem( item );
        if( !scourge->getInfoGui()->getWindow()->isVisible() ) 
          scourge->getInfoGui()->getWindow()->setVisible( true );
      }
    }
  } else if(widget == openButton) {
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1) {
      Item *item = scourge->getParty()->getParty(selected)->getInventory(itemIndex);
      if(item->getRpgItem()->getType() == RpgItem::CONTAINER) {
        scourge->openContainerGui(item);
      }
    }
  } else if(widget == equipButton) {
    equipItem();
  } else if(widget == eatDrinkButton) {
    if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead)) {
      scourge->showMessageDialog(Constants::getMessage(Constants::DEAD_CHARACTER_ERROR));
    } else {
      int itemIndex = invList->getSelectedLine();  
      if(itemIndex > -1 && 
         creature->getInventoryCount() > itemIndex) {

        // this action will occur in the next battle round

        creature->setAction(Constants::ACTION_EAT_DRINK, 
                            creature->getInventory(itemIndex),
                            NULL);
        creature->setTargetCreature(creature);
        if(!mainWin->isLocked()) mainWin->setVisible(false);

        //		if(scourge->getParty()->getParty(selected)->eatDrink(itemIndex)){
        //		  scourge->getParty()->getParty(selected)->removeInventory(itemIndex);                
        //		}
        // refresh screen
        //setSelectedPlayerAndMode(selected, INVENTORY);
      }
    }
  } else if(widget == schoolList) {
    int n = schoolList->getSelectedLine();
    if(n != -1 && n < MagicSchool::getMagicSchoolCount()) {
      showMemorizedSpellsInSchool( scourge->getParty()->getParty(selected), 
                                   MagicSchool::getMagicSchool(n));
    }
  } else if(widget == spellList) {
    Spell *spell = getSelectedSpell();
    if(spell) showSpellDescription(spell);
//  } else if( widget == quickSpell ) {
    //Spell *spell = getSelectedSpell();
    //if(spell) creature->setQuickSpell( 0, spell );
  } else if(widget == specialList) {
    showSpecialDescription( getSelectedSpecial() );
  } else if(widget == storeSpecialButton) {
    if( storeSpecialButton->isSelected() ) {
      storable = getSelectedSpecial();
      if( storable ) {
        const char *p = storable->isStorable();
        if( p ) {
          scourge->showMessageDialog( (char*)p );
          storable = NULL;
        }
        if( !creature->hasSpecialSkill( (SpecialSkill*)storable ) ) {
          scourge->showMessageDialog( Constants::getMessage( Constants::UNMET_CAPABILITY_PREREQ_ERROR ) );
          storable = NULL;
        }
      }
      if( !storable ) {
        storeSpecialButton->setSelected( false );
      }
    }
  } else if(widget == useSpecialButton) {
    storable = getSelectedSpecial();
    if( storable && 
        ((SpecialSkill*)storable)->getType() == SpecialSkill::SKILL_TYPE_MANUAL &&
        creature->hasSpecialSkill((SpecialSkill*)storable) ) {

      creature->
        setAction( Constants::ACTION_SPECIAL, 
                   NULL,
                   NULL,
                   (SpecialSkill*)storable );
      creature->setTargetCreature(creature);

      // set this as a quickspell if there is space
      for( int i = 0; i < 12; i++ ) {
        if( !creature->getQuickSpell( i ) ) {
          creature->setQuickSpell( i, storable );
          break;
        }
      }

      if(!mainWin->isLocked()) mainWin->setVisible(false);
    }
  } else if( widget == storeSpellButton ) {
    if( storeSpellButton->isSelected() ) {
      storable = getSelectedSpell();
      if( storable ) {
        const char *p = storable->isStorable();
        if( p ) {
          scourge->showMessageDialog( (char*)p );
          storable = NULL;
        }
      }
      if( !storable ) {
        storeSpellButton->setSelected( false );
      }
    }
  } else if(widget == castButton) {
    Spell *spell = getSelectedSpell();
    if(spell) {
      if(spell->getMp() > creature->getMp()) {
        scourge->showMessageDialog("Not enough Magic Points to cast this spell!");
      } else {
        // set this as a quickspell if there is space
        for( int i = 0; i < 12; i++ ) {
          if( !creature->getQuickSpell( i ) ) {
            creature->setQuickSpell( i, spell );
            break;
          }
        }

        creature->setAction(Constants::ACTION_CAST_SPELL, 
                            NULL,
                            spell);
        if(!spell->isPartyTargetAllowed()) {
          scourge->setTargetSelectionFor(creature);
        } else {
          creature->setTargetCreature(creature);
        }
        if(!mainWin->isLocked()) mainWin->setVisible(false);
      }
    }
  } else if(widget == storeItemButton) {
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1 && creature->getInventoryCount() > itemIndex) {
      Item *item = creature->getInventory(itemIndex);
      if( item->getSpell() ) {
        storable = (Storable*)item;
      } else {
        scourge->showMessageDialog("This item is out of charges.");
      }
    } else {
      scourge->showMessageDialog("You may only store items with spells.");
    }
    if( !storable ) {
      storeItemButton->setSelected( false );
    }
  } else if(widget == castScrollButton) {
    // no MP-s used when casting from scroll, but the scroll is destroyed.
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1 && creature->getInventoryCount() > itemIndex) {
      Item *item = creature->getInventory(itemIndex);
      if( item->getSpell() ) {
        if( item->getRpgItem()->getMaxCharges() == 0 || item->getCurrentCharges() > 0 ) {
          creature->setAction(Constants::ACTION_CAST_SPELL, 
                              item,
                              item->getSpell());
          if(!item->getSpell()->isPartyTargetAllowed()) {
            scourge->setTargetSelectionFor(creature);
          } else {
            creature->setTargetCreature(creature);
          }
          if(!mainWin->isLocked()) mainWin->setVisible(false);
        } else {
          scourge->showMessageDialog("This item is out of charges.");
        }
      } else {
        scourge->showMessageDialog("You cannot cast a spell with this item.");
      }
    }
  } else if(widget == transcribeButton) {
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1 && creature->getInventoryCount() > itemIndex) {
      Item *item = creature->getInventory(itemIndex);
      if(item->getSpell()) {
        if(creature->getSkill(item->getSpell()->getSchool()->getSkill()) > item->getSpell()->getLevel() * 5 &&
           creature->getMp() > 0) {
          bool res = creature->addSpell(item->getSpell());
          if(res) {
            scourge->showMessageDialog("Spell was entered into your spellbook.");
            // destroy the scroll
            creature->removeInventory(itemIndex);
            refresh();
            char msg[120];
            sprintf(msg, "%s crumbles into dust.", item->getItemName());
            scourge->getMap()->addDescription(msg);
          } else {
            scourge->showMessageDialog("You already know this spell");
          }
        } else {
        scourge->showMessageDialog("You are not proficient enough to transcribe this scroll.");
        }
      } else {
        scourge->showMessageDialog("You can only transcribe scrolls!");
      }
    }
  } else if(widget == enchantButton) {
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1 && creature->getInventoryCount() > itemIndex) {
      Item *item = creature->getInventory(itemIndex);
      if(item->isMagicItem()) {
        scourge->showMessageDialog("This item is already enchanted.");
      } else if(!item->getRpgItem()->isEnchantable()) {
        scourge->showMessageDialog("This item cannot be enchanted.");
      } else {
        Date now = scourge->getParty()->getCalendar()->getCurrentDate();
        if(now.isADayLater(creature->getLastEnchantDate())) {
          int level = (int)((float)creature->getSkill( Skill::ENCHANT_ITEM ) * rand()/RAND_MAX);					
          if(level > 20) {
            int level = creature->getSkill( Skill::ENCHANT_ITEM );
            item->enchant( (level - 20) / 20 );
            refresh();
            scourge->showMessageDialog("You succesfully enchanted an item!");
            char tmp[255];
            item->getDetailedDescription(tmp);
            char msg[255];
            sprintf(msg, "You created: %s.", tmp);
            scourge->getMap()->addDescription(msg);
            creature->startEffect( Constants::EFFECT_SWIRL, Constants::DAMAGE_DURATION * 4 );
          } else {
            scourge->showMessageDialog("You failed to enchant the item.");
          }
          creature->setLastEnchantDate(now);
        } else {
          scourge->showMessageDialog("You can only enchant one item per day.");
        }
      }
    }
  } else if( widget == formationList ) {
    scourge->getParty()->setFormation( formationList->getSelectedLine() );
  } else if(widget == layoutButton1) {
    scourge->setUILayout(Constants::GUI_LAYOUT_ORIGINAL);
  } else if(widget == layoutButton2) {
    scourge->setUILayout(Constants::GUI_LAYOUT_BOTTOM);
  //} else if(widget == layoutButton3) {
//    setUILayout(Constants::GUI_LAYOUT_SIDE);
  } else if(widget == layoutButton4) {
    scourge->setUILayout(Constants::GUI_LAYOUT_INVENTORY);
  } else if( widget == squirrelWindow ) {
    scourge->getSquirrelConsole()->setVisible( squirrelWindow->isSelected() );
		/*
  } else if( widget == saveGameButton ) {
    if( !scourge->saveGame( scourge->getSession() ) ) {
      scourge->showMessageDialog( "Error saving game!" );
    } else {
      scourge->showMessageDialog( "Game saved successfully." );
    }
		*/		
  } else if( widget == preferredWeaponButton[ 0 ] ) {
    if( creature->isEquippedWeapon( Constants::INVENTORY_LEFT_HAND ) ) {
      creature->setPreferredWeapon( Constants::INVENTORY_LEFT_HAND );
    }
    preferredWeaponButton[0]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_LEFT_HAND );
    preferredWeaponButton[1]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_RIGHT_HAND );
    preferredWeaponButton[2]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_WEAPON_RANGED );
  } else if( widget == preferredWeaponButton[ 1 ] ) {
    if( creature->isEquippedWeapon( Constants::INVENTORY_RIGHT_HAND ) ) {
      creature->setPreferredWeapon( Constants::INVENTORY_RIGHT_HAND );
    }
    preferredWeaponButton[0]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_LEFT_HAND );
    preferredWeaponButton[1]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_RIGHT_HAND );
    preferredWeaponButton[2]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_WEAPON_RANGED );
  } else if( widget == preferredWeaponButton[ 2 ] ) {
    if( creature->isEquippedWeapon( Constants::INVENTORY_WEAPON_RANGED ) ) {
      creature->setPreferredWeapon( Constants::INVENTORY_WEAPON_RANGED );
    }
    preferredWeaponButton[0]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_LEFT_HAND );
    preferredWeaponButton[1]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_RIGHT_HAND );
    preferredWeaponButton[2]->setSelected( creature->getPreferredWeapon() == Constants::INVENTORY_WEAPON_RANGED );
  } else if( widget == addModButton ) {
		int skill = skillList->getSelectedLine();
		if( skill > -1 && creature->getAvailableSkillMod() > 0 ) {
			if( Skill::skills[ skill ]->getGroup()->isStat() ) {
				scourge->showMessageDialog( "Stats cannot be improved this way." );
			}	else {
				creature->setSkillMod( skill, creature->getSkillMod( skill ) + 1 );
				creature->setAvailableSkillMod( creature->getAvailableSkillMod() - 1 );
				refresh();
			}
		}
	} else if( widget == delModButton ) {
		int skill = skillList->getSelectedLine();
		if( skill > -1 && creature->getSkillMod( skill ) > 0 ) {
			creature->setSkillMod( skill, creature->getSkillMod( skill ) - 1 );
			creature->setAvailableSkillMod( creature->getAvailableSkillMod() + 1 );
			refresh();
		}
	} else if( widget == acceptModButton ) {
		bool hasMods = false;
		for( int i = 0; i < Skill::SKILL_COUNT; i++ ) {
			if( creature->getSkillMod( i ) > 0 ) {
				hasMods = true;
				break;
			}
		}
		if( hasMods ) {
			if( creature->getAvailableSkillMod() > 0 ) {
				scourge->showMessageDialog( "You still have skill points to distribute." );
			} else {
				assert( !confirmDialog->isVisible() );
				confirmDialog->setMode( APPLY_SKILL_MODS );
				confirmDialog->setText( "Are you sure you want to apply the selected skill points?" );
				confirmDialog->setVisible( true );
			}
		} else {
			scourge->showMessageDialog( "This character has no skill points to apply." );
		}
	} else if( widget == confirmDialog->okButton ) {
		confirmDialog->setVisible( false );
		if( confirmDialog->getMode() == APPLY_SKILL_MODS ) {
			creature->applySkillMods();
		} else {
			cerr << "*** Error: unknown confirm dialog mode: " << confirmDialog->getMode() << endl;
		}
		refresh();
	}	else if( widget == confirmDialog->cancelButton ) {
		confirmDialog->setVisible( false );
	}
  return false;
}

void Inventory::moveItemTo(int playerIndex) {
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
     scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
    if(playerIndex != selected) {
      scourge->getParty()->getParty(playerIndex)->
      addInventory(scourge->getParty()->getParty(selected)->removeInventory(itemIndex), true);
      // recreate strings in list
      refresh();
    }
  }
}

bool Inventory::handleEvent(SDL_Event *event) {
  switch(event->type) {
  case SDL_MOUSEBUTTONUP:
    break;     
  case SDL_KEYUP:
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE: 
    hide();
    return true;
    default: break;
    }
  default: break;
  }
  return false;
}

void Inventory::setSelectedPlayerAndMode(int player, int mode) {
  selected = player;
  selectedMode = mode;

  inventoryButton->setSelected(selectedMode == INVENTORY);
  skillsButton->setSelected(selectedMode == CHARACTER);
  spellsButton->setSelected(selectedMode == SPELL);
  specialButton->setSelected(selectedMode == SPECIAL);
  missionButton->setSelected(selectedMode == MISSION);
  partyButton->setSelected(selectedMode == PARTY);

  // show only the ui elements belonging to the current mode
  cards->setActiveCard(selectedMode);   

  // arrange the gui
  int stateCount;
  int objectiveCount = 0;
  Creature * selectedP = scourge->getParty()->getParty(selected);

  sprintf(nameAndClassStr, "%s, %s (level %d, %s) (%s)", 
          selectedP->getName(), 
          selectedP->getCharacter()->getName(), 
          selectedP->getLevel(),
          (selectedP->getSex() == Constants::SEX_MALE ? "M" : "F" ),
          MagicSchool::getMagicSchool(selectedP->getDeityIndex())->getDeity());
  //nameAndClassLabel->setText(nameAndClassStr);  
  mainWin->setTitle( nameAndClassStr );

  charInfoUI->setCreature( mainWin, selectedP );
	char tmp[80];
  switch(selectedMode) {
  case CHARACTER:         

    stateCount = 0;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(selectedP->getStateMod(t)) {
        sprintf(stateLine[stateCount], "%s", _( Constants::STATE_DISPLAY_NAMES[t] ) );
        icons[stateCount] = scourge->getShapePalette()->getStatModIcon(t);
        stateCount++;
      }
    }
    stateList->setLines(stateCount, (const char**)stateLine, 
                        (const Color *)NULL, (stateCount ? (const GLuint*)icons : NULL));

    stateCount = 0;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(selectedP->getProtectedStateMod(t)) {
        sprintf(protStateLine[stateCount], "%s", _( Constants::STATE_DISPLAY_NAMES[t] ) );
        protIcons[stateCount] = scourge->getShapePalette()->getStatModIcon(t);
        stateCount++;
      }
    }
    protStateList->setLines(stateCount, (const char**)protStateLine, 
                            (const Color *)NULL, (stateCount ? (const GLuint*)protIcons : NULL));
		
    skillList->setCreature( selectedP, scourge->getParty() );

		sprintf( tmp, "Skill: %d", selectedP->getAvailableSkillMod() );
		skillModLabel->setText( tmp );
    break;

  case INVENTORY:
    sprintf(inventoryWeightStr, " (Total : %2.2fkg / %2.2fkg)", 
            selectedP->getInventoryWeight(), selectedP->getMaxInventoryWeight());     
    inventoryWeightLabel->setText(inventoryWeightStr);
    sprintf(coinsStr, "Coins: %d", selectedP->getMoney());
    coinsLabel->setText(coinsStr);
    for(int t = 0; t < selectedP->getInventoryCount(); t++) {
      Item *item = selectedP->getInventory(t);
      int location = selectedP->getEquippedIndex(t);
      char s[100];
      item->getDetailedDescription(s);
      sprintf(pcInvText[t], "%s %s", (location > -1 ? "(E)" : ""), s);
      if( !item->isMagicItem() ) {
        if( mainWin->getTheme()->getWindowText() ) {
          itemColor[t].r = mainWin->getTheme()->getWindowText()->r;
          itemColor[t].g = mainWin->getTheme()->getWindowText()->g;
          itemColor[t].b = mainWin->getTheme()->getWindowText()->b;
        } else {
          itemColor[t].r = 0;
          itemColor[t].g = 0;
          itemColor[t].b = 0;
        }
      } else {
        itemColor[t].r = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->r;
        itemColor[t].g = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->g;
        itemColor[t].b = Constants::MAGIC_ITEM_COLOR[ item->getMagicLevel() ]->b;
      }
      itemColor[t].a = 1;
      itemIcon[t] = scourge->getShapePalette()->tilesTex[ item->getRpgItem()->getIconTileX() ][ item->getRpgItem()->getIconTileY() ];
    }    
    for(int t = selectedP->getInventoryCount(); 
       t < MAX_INVENTORY_SIZE; t++) {
      strcpy(pcInvText[t], "");
    }
    invList->setLines(selectedP->getInventoryCount(), 
                      (const char **)pcInvText,
                      itemColor, 
                      itemIcon);
    preferredWeaponButton[ 0 ]->
      setSelected( selectedP->getPreferredWeapon() == Constants::INVENTORY_LEFT_HAND );
    preferredWeaponButton[ 1 ]->
      setSelected( selectedP->getPreferredWeapon() == Constants::INVENTORY_RIGHT_HAND );
    preferredWeaponButton[ 2 ]->
      setSelected( selectedP->getPreferredWeapon() == Constants::INVENTORY_WEAPON_RANGED );
    /*
    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
      Item *item = selectedP->getEquippedInventory(i);
      invEquipLabel[i]->setText((char *)(item ? item->getItemName() : NULL));
    }
    */
    break;

  case SPELL:
    for(int t = 0; t < MagicSchool::getMagicSchoolCount(); t++) {
      MagicSchool *school = MagicSchool::getMagicSchool(t);   
      sprintf(schoolText[t], "%s (%s)", school->getName(), school->getDeity());
      bool found = false;
      for( int r = 0; r < school->getSpellCount(); r++ ) {
        if( scourge->getParty()->getParty(selected)->isSpellMemorized( school->getSpell( r ) ) ) {
          found = true;
          break;
        }
      }
      schoolColors[t].r = 1;
      schoolColors[t].g = 1;
      schoolColors[t].b = ( found ? 0 : 1 );
      schoolColors[t].a = 1;
      if(t == 0) {
        showMemorizedSpellsInSchool(scourge->getParty()->getParty(selected), school);
      }
    }
    schoolList->setLines(MagicSchool::getMagicSchoolCount(), 
                         (const char**)schoolText, 
                         schoolColors);
    break;
	case SPECIAL:
		knownSpecialSkills.clear();
    for(int t = 0; t < SpecialSkill::getSpecialSkillCount(); t++) {
      SpecialSkill *ss = SpecialSkill::getSpecialSkill(t);   
			if( scourge->getParty()->getParty( selected )->hasSpecialSkill( ss ) ) {
				// display recurring skills as automatic
				sprintf(specialText[knownSpecialSkills.size()], "%s (%s)", 
								ss->getName(), 
								(ss->getType() == SpecialSkill::SKILL_TYPE_MANUAL ? "M" : "A"));
				specialIcons[knownSpecialSkills.size()] = scourge->getShapePalette()->spellsTex[ ss->getIconTileX() ][ ss->getIconTileY() ];
				if( scourge->getParty()->getParty( selected )->hasSpecialSkill( ss ) ) {
					specialColor[knownSpecialSkills.size()].r = 1;
					specialColor[knownSpecialSkills.size()].g = 1;
					specialColor[knownSpecialSkills.size()].b = 1;
				} else {
					specialColor[knownSpecialSkills.size()].r = 0.5f;
					specialColor[knownSpecialSkills.size()].g = 0.5f;
					specialColor[knownSpecialSkills.size()].b = 0.5f;
				}
				knownSpecialSkills.push_back( ss );
			}
    }
    specialList->setLines(knownSpecialSkills.size(), 
                          (const char**)specialText,
                          specialColor,
                          specialIcons);
		showSpecialDescription( getSelectedSpecial() );
    break;
  case LOG:
    break;
  case MISSION:
    if(scourge->getSession()->getCurrentMission()) {
      sprintf(missionText, "%s:|Depth: %d out of %d.|%s", 
              scourge->getSession()->getCurrentMission()->getName(),
              (scourge->getCurrentDepth() + 1),
              scourge->getSession()->getCurrentMission()->getDepth(),
              scourge->getSession()->getCurrentMission()->getDescription());
      objectiveCount = 
      scourge->getSession()->getCurrentMission()->getItemCount() +
      scourge->getSession()->getCurrentMission()->getCreatureCount();   
      for(int t = 0; t < scourge->getSession()->getCurrentMission()->getItemCount(); t++) {
        sprintf(objectiveText[t], "Find %s. %s", 
                scourge->getSession()->getCurrentMission()->getItem(t)->getName(),
                (scourge->getSession()->getCurrentMission()->getItemHandled(t) ? 
                 "(completed)" : "(not yet found)"));
        if(scourge->getSession()->getCurrentMission()->getItemHandled(t)) {
          missionColor[t].r = 0.2f;
          missionColor[t].g = 0.7f;
          missionColor[t].b = 0.2f;
        } else {
          missionColor[t].r = 0.7f;
          missionColor[t].g = 0.2f;
          missionColor[t].b = 0.2f;
        }
      }
      int start = scourge->getSession()->getCurrentMission()->getItemCount();
      for(int t = 0; t < scourge->getSession()->getCurrentMission()->getCreatureCount(); t++) {
        sprintf(objectiveText[start + t], "Vanquish %s. %s", 
                scourge->getSession()->getCurrentMission()->getCreature(t)->getType(),
                (scourge->getSession()->getCurrentMission()->getCreatureHandled(t) ? 
                 "(completed)" : "(not yet done)"));
        if(scourge->getSession()->getCurrentMission()->getCreatureHandled(t)) {
          missionColor[start + t].r = 0.2f;
          missionColor[start + t].g = 0.7f;
          missionColor[start + t].b = 0.2f;
        } else {
          missionColor[start + t].r = 0.7f;
          missionColor[start + t].g = 0.2f;
          missionColor[start + t].b = 0.2f;
        }
      }
      start += scourge->getSession()->getCurrentMission()->getCreatureCount();
      for(int t = objectiveCount; t < MAX_INVENTORY_SIZE; t++) {
        strcpy(objectiveText[t], "");
      }
      if( !objectiveCount ) {
        objectiveCount = 1;
        sprintf( objectiveText[0], "Special. %s", 
                 ( scourge->getSession()->getCurrentMission()->isCompleted() ?
                   "(completed)" : "(not yet done)" ) );
        if( scourge->getSession()->getCurrentMission()->isCompleted() ) {
          missionColor[0].r = 0.2f;
          missionColor[0].g = 0.7f;
          missionColor[0].b = 0.2f;
        } else {
          missionColor[0].r = 0.7f;
          missionColor[0].g = 0.2f;
          missionColor[0].b = 0.2f;
        }
      }
    } else {
      strcpy(missionText, "");
      for(int t = 0; t < MAX_INVENTORY_SIZE; t++) {
        strcpy(objectiveText[t], "");
      }
    }
    missionDescriptionLabel->setText(missionText);
    objectiveList->setLines(objectiveCount, 
                            (const char **)objectiveText,
                            missionColor);
    break;
  case PARTY:
    layoutButton1->setSelected( scourge->getLayoutMode() == Constants::GUI_LAYOUT_ORIGINAL );
    layoutButton2->setSelected( scourge->getLayoutMode() == Constants::GUI_LAYOUT_BOTTOM );
    layoutButton4->setSelected( scourge->getLayoutMode() == Constants::GUI_LAYOUT_INVENTORY );
    break;
  }
}

void Inventory::receive(Widget *widget) {
  if(widget == paperDoll) {
    if(putItem() != -1) equipItem();
  } else {

    /*
    for(int i = 0; i < scourge->getParty()->getPartySize(); i++) {
      if(scourge->getParty()->getPlayer() == scourge->getParty()->getParty(i)) {
        n = i;
        //break;
      }
    }
    setSelectedPlayerAndMode(n, selectedMode);
    */

    // don't check for widget, it may be party member portrait.
    putItem();
  }
}

bool Inventory::startDrag(Widget *widget, int x, int y) {
  if( scourge->getTradeDialog()->getWindow()->isVisible() ) {
    scourge->showMessageDialog( "Can't change inventory while trading." );
    return false;
  }
  
  if(widget == invList) {
    int itemIndex = invList->getSelectedLine();  
//    cerr << "equipped? " << scourge->getParty()->getParty(selected)->isEquipped( itemIndex ) << endl;
//    cerr << "cursed? " << scourge->getParty()->getParty(selected)->getInventory( itemIndex )->isCursed() << endl;
    if( scourge->getParty()->getParty(selected)->isEquipped( itemIndex ) &&
        scourge->getParty()->getParty(selected)->getInventory( itemIndex )->isCursed() ) {
      scourge->showMessageDialog( "Can't remove cursed item!" );
      return false;
    }
    dropItem();
    return true;
  } else if(widget == paperDoll) {
    if(y > 0 && y < Constants::INVENTORY_COUNT * 16) {
      // what's equiped at this inventory slot?
      Item *item = scourge->getParty()->getParty(selected)->getItemAtLocation((1 << (y / 16)));
      if(item) {
        if( item->isCursed() ) {
          scourge->showMessageDialog( "Can't remove cursed item!" );
          return false;
        } else {

          // find its index in the inventory
          int index = scourge->getParty()->getParty(selected)->findInInventory(item);
          // select it
          invList->setSelectedLine(index);
          // drop it
          dropItem();

          return true;
        }
      }
    }
  }
  return false;
}

int Inventory::putItem() {
  Item *item = scourge->getMovingItem();
  if(item) {
    if(scourge->getParty()->getParty(selected)->addInventory(item)) {
      // message: the player accepted the item
      char message[120];
      sprintf(message, "%s picks up %s.", 
              scourge->getParty()->getParty(selected)->getName(),
              item->getItemName());
      scourge->getMap()->addDescription(message);
      scourge->endItemDrag();
      int index = scourge->getParty()->getParty(selected)->findInInventory(item);
      setSelectedPlayerAndMode(selected, INVENTORY);
      invList->setSelectedLine(index);
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
      return index;
    } else {
      // message: the player's inventory is full
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_FAILED);
      scourge->showMessageDialog("You can't fit the item!");
    }
  }
  return -1;
}

void Inventory::equipItem() {
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
     scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
    Item *item = scourge->getParty()->getParty(selected)->getInventory(itemIndex);

    char *err = scourge->getParty()->getParty(selected)->canEquipItem( item );
    if( err ) {
      scourge->showMessageDialog( err );
      scourge->getSDLHandler()->getSound()->playSound( Window::DROP_FAILED );
    } else {
      scourge->getParty()->getParty(selected)->equipInventory(itemIndex);
      // recreate list strings
      refresh();
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
    }
  }
}

void Inventory::dropItem() {
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
     scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
    Item *item = scourge->getParty()->getParty(selected)->removeInventory(itemIndex);
    scourge->startItemDragFromGui(item);
    /*
    scourge->setMovingItem(item, 
                           scourge->getParty()->getParty(selected)->getX(), 
                           scourge->getParty()->getParty(selected)->getY(), 
                           scourge->getParty()->getParty(selected)->getZ());
    */                           
    char message[120];
    sprintf(message, "%s drops %s.", 
            scourge->getParty()->getParty(selected)->getName(),
            item->getItemName());
    scourge->getMap()->addDescription(message);
    setSelectedPlayerAndMode(selected, INVENTORY);
  }
}

void Inventory::refresh(int player) {
  if(player == -1) player = selected;
  //setSelectedPlayerAndMode(selected, INVENTORY);

  // FIXME: remember other lists too
  int oldLine = invList->getSelectedLine();
  int oldSkillLine = skillList->getSelectedLine();
  setSelectedPlayerAndMode(player, selectedMode);
  if( oldLine >= 0 ) invList->setSelectedLine(oldLine);
  if( oldSkillLine >= 0 ) skillList->setSelectedLine(oldSkillLine);
}

void Inventory::positionWindow() {
	if( mainWin->getAnimation() == Window::SLIDE_UP ) {
		mainWin->move( scourge->getPartyWindow()->getX() + 
									 scourge->getPartyWindow()->getWidth() - 
									 mainWin->getWidth(),
									 scourge->getPartyWindow()->getY() - mainWin->getHeight() );
	}
}                 

void Inventory::show(bool animate) { 
  positionWindow();
  mainWin->setVisible(true, animate); 

  // find selected player. FIXME: this is inefficient
  int n = selected;
  //char buttonText[80];

  for(int i = 0; i < scourge->getParty()->getPartySize(); i++) {
    if(scourge->getParty()->getPlayer() == scourge->getParty()->getParty(i)) {
      n = i;
      //break;
    }
  }
  setSelectedPlayerAndMode(n, selectedMode); 
}

Spell *Inventory::getSelectedSpell() {
  Creature *creature = scourge->getParty()->getParty(selected);
  MagicSchool *school = NULL;
  int n = schoolList->getSelectedLine();
  if(n != -1 && n < MagicSchool::getMagicSchoolCount()) {
    school = MagicSchool::getMagicSchool(n);
  }
  if(!school) return NULL;

  n = spellList->getSelectedLine();
  if(n != -1 && n < spellList->getLineCount()) {
    int spellCount = 0;
    for(int r = 0; r < school->getSpellCount(); r++) {
      Spell *spell = school->getSpell(r);
      if(creature->isSpellMemorized(spell)) {
        if(n == spellCount) return spell;
        spellCount++;
      }
    }
  }
  return NULL;
}

void Inventory::showMemorizedSpellsInSchool(Creature *creature, MagicSchool *school) {
  int spellCount = 0;
  for(int r = 0; r < school->getSpellCount(); r++) {
    Spell *spell = school->getSpell(r);
    if(creature->isSpellMemorized(spell)) {
      spell->describe(spellText[spellCount]);
      if(spellCount == 0) {
        showSpellDescription(spell);
      }
      spellIcons[spellCount] = scourge->getShapePalette()->spellsTex[ spell->getIconTileX() ][ spell->getIconTileY() ];

      spellCount++;
    }
  }
  if(spellCount == 0) spellDescriptionLabel->setText("");
  spellList->setLines(spellCount, 
                      (const char**)spellText, 
					  NULL, 
					  spellIcons);
}

void Inventory::showSpellDescription(Spell *spell) {
  spellDescriptionLabel->setText((char*)(spell->getNotes() ? spell->getNotes() : ""));
}

SpecialSkill *Inventory::getSelectedSpecial() {
  //Creature *creature = scourge->getParty()->getParty(selected);
  SpecialSkill *ss = NULL;
  int n = specialList->getSelectedLine();
  if(n != -1 && n < (int)knownSpecialSkills.size() ) {
    ss = knownSpecialSkills[ n ];
  }
  return ss;
}

void Inventory::showSpecialDescription(SpecialSkill *ss) {
	if( ss ) specialDescriptionLabel->setText((char*)(ss->getDescription() ? ss->getDescription() : ""));
}

