

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
#include "specialskill.h"

using namespace std;

/**
  *@author Gabor Torok
  */


#define START_OF_SECOND_BUTTON_SET 140

Inventory::Inventory(Scourge *scourge) {
  this->scourge = scourge;

  // allocate strings for list
  this->formationText = (char**)malloc(10 * sizeof(char*));
  for(int i = 0; i < 10; i++) {
    this->formationText[i] = (char*)malloc(120 * sizeof(char));
  }
  this->itemColor = (Color*)malloc(MAX_INVENTORY_SIZE * sizeof(Color));
  this->pcInvText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  this->itemIcon = (GLuint*)malloc(MAX_INVENTORY_SIZE * sizeof(GLuint));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->pcInvText[i] = (char*)malloc(120 * sizeof(char));
  }
  this->skillLine = (char**)malloc(Constants::SKILL_COUNT * sizeof(char*));
  for(int i = 0; i < Constants::SKILL_COUNT; i++) {
    this->skillLine[i] = (char*)malloc(120 * sizeof(char));
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
  this->specialText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
  this->specialIcons = (GLuint*)malloc(MAX_INVENTORY_SIZE * sizeof(GLuint));
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->specialText[i] = (char*)malloc(120 * sizeof(char));
  }
  selected = selectedMode = 0;

  // construct UI
  mainWin = new Window( scourge->getSDLHandler(),
                        scourge->getSDLHandler()->getScreen()->w - Scourge::INVENTORY_WIDTH, 
                        scourge->getSDLHandler()->getScreen()->h - Scourge::PARTY_GUI_HEIGHT - 
                        Scourge::INVENTORY_HEIGHT - Window::SCREEN_GUTTER,
                        Scourge::INVENTORY_WIDTH, Scourge::INVENTORY_HEIGHT,
                        "Party Information", false, Window::SIMPLE_WINDOW, "default" );
  mainWin->setLocked( true );
  mainWin->setAnimation( Window::SLIDE_UP );

  int buttonHeight = 20;
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

  yy = mainWin->getHeight() - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - 10 - 20;
  closeButton = mainWin->createButton( 0, yy, 105, yy + buttonHeight, "Hide" );

  cards = new CardContainer(mainWin);


  // -------------------------------------------
  // inventory page	
  cards->createLabel(115, 10, "Inventory:", INVENTORY, Constants::RED_COLOR); 
  inventoryWeightLabel = cards->createLabel(190, 10, NULL, INVENTORY);

  coinsLabel = cards->createLabel(300, 170, NULL, INVENTORY);
  cards->createLabel(115, 170, "Equipped Items:", INVENTORY, Constants::RED_COLOR);

  invList = new ScrollingList(115, 15, 295, 140, scourge->getShapePalette()->getHighlightTexture(), this, 30);
  cards->addWidget(invList, INVENTORY);
  cards->createLabel(115, 430, Constants::getMessage(Constants::EXPLAIN_DRAG_AND_DROP), INVENTORY);

  paperDoll = new Canvas(115, 175, 411, 206 + (Constants::INVENTORY_COUNT * 15), this, this);
  cards->addWidget(paperDoll, INVENTORY);

  yy = START_OF_SECOND_BUTTON_SET;
  equipButton    = cards->createButton( 0, yy, 105, yy + buttonHeight, "Don/Doff", INVENTORY);
  yy+=buttonHeight;
  openButton     = cards->createButton( 0, yy, 105, yy + buttonHeight, Constants::getMessage(Constants::OPEN_CONTAINER_LABEL), INVENTORY ); 
  yy+=buttonHeight;
  eatDrinkButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Eat/Drink", INVENTORY );
  yy+=buttonHeight;
  castScrollButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Cast Scroll", INVENTORY );
  yy+=buttonHeight;
  transcribeButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Transcribe", INVENTORY );
  yy+=buttonHeight;
  enchantButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Enchant", INVENTORY );
  yy+=buttonHeight;
  infoButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Info", INVENTORY );


  // -------------------------------------------
  // character info
  nameAndClassLabel = cards->createLabel(115, 10, NULL, CHARACTER, Constants::RED_COLOR);
  attrCanvas     = new Canvas( 115, 15, 405, 115, this );
  cards->addWidget( attrCanvas, CHARACTER );

  cards->createLabel(115, 130, "Current State:", CHARACTER, Constants::RED_COLOR);
  stateList = new ScrollingList(115, 135, 140, 70, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(stateList, CHARACTER);
  
  cards->createLabel(265, 130, "Protected States:", CHARACTER, Constants::RED_COLOR);
  protStateList = new ScrollingList(265, 135, 140, 70, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(protStateList, CHARACTER);

  strcpy(skillsStr, "Skills:");
  skillLabel = cards->createLabel(115, 220, skillsStr, CHARACTER, Constants::RED_COLOR);
  skillList = new ScrollingList(115, 225, 290, 180, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(skillList, CHARACTER);
  skillAddButton = cards->createButton( 115, 410, 200, 410 + buttonHeight, " + ", CHARACTER);
  skillSubButton = cards->createButton( 320, 410, 405, 410 + buttonHeight, " - ", CHARACTER);
  levelUpButton = cards->createButton( 205, 410, 315, 410 + buttonHeight, "Apply Points", CHARACTER);


  // -------------------------------------------
  // spellbook
  cards->createLabel(115, 10, "School of magic: (with provider deity)", SPELL, Constants::RED_COLOR);
  schoolList = new ScrollingList(115, 15, 290, 100, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(schoolList, SPELL);
  cards->createLabel(115, 135, "Spells memorized:", SPELL, Constants::RED_COLOR);
  spellList = new ScrollingList(115, 140, 290, 150, scourge->getShapePalette()->getHighlightTexture(), NULL, 30);
  cards->addWidget(spellList, SPELL);
  cards->createLabel(115, 310, "Spell notes:", SPELL, Constants::RED_COLOR);
  spellDescriptionLabel = new Label(115, 325, "", 50);
  cards->addWidget(spellDescriptionLabel, SPELL);

  yy = START_OF_SECOND_BUTTON_SET;
  castButton = cards->createButton( 0, yy, 105, yy + buttonHeight, "Cast", SPELL);
  yy+=buttonHeight;
  storeSpellButton = cards->createButton( 0, yy + buttonHeight, 105, 160, "Store", SPELL, true);
  yy+=buttonHeight;
  storeSpell = NULL;

  // -------------------------------------------
  // special skills
  cards->createLabel(115, 10, "Special Capabilities", SPECIAL, Constants::RED_COLOR);
  specialList = new ScrollingList(115, 15, 400, 100, scourge->getShapePalette()->getHighlightTexture(), NULL, 30);
  cards->addWidget(specialList, SPECIAL);
  cards->createLabel(115, 310, "Capability Description:", SPECIAL, Constants::RED_COLOR);
  specialDescriptionLabel = new Label(115, 325, "", 50);
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
  layoutButton1 = cards->createButton( 115, 155, 195, 175, "Floating", PARTY );
  layoutButton1->setToggle( true );
  layoutButton2 = cards->createButton( 200, 155, 280, 175, "Bottom", PARTY );
  layoutButton2->setToggle( true );
  layoutButton4 = cards->createButton( 285, 155, 365, 175, "Inventory", PARTY );
  layoutButton4->setToggle( true );

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
  Creature *p = scourge->getParty()->getParty(selected);

  if(w == paperDoll) {
    float x = 125;
    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
      Item *item = scourge->getParty()->getParty(selected)->getEquippedInventory(i);
      if(item) {

        if( theme->getSelectionBackground() ) {
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
//    glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_NOTEQUAL, 0);        
    glPixelZoom( 1.0, -1.0 );
    glRasterPos2f( 1, 1 );
    glDrawPixels(shapePal->paperDoll->w, shapePal->paperDoll->h,
                 GL_BGRA, GL_UNSIGNED_BYTE, shapePal->paperDollImage);
    //glDisable(GL_ALPHA_TEST);

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

  } else {
    int y = 15;
    char s[80];
    sprintf(s, "Exp: %u(%u)", p->getExp(), p->getExpOfNextLevel());
    //if(p->getStateMod(Constants::leveled)) {
    if( p->getAvailableSkillPoints() > 0 ) {
      glColor4f( 1.0f, 0.2f, 0.0f, 1.0f );
    } else {
      if( theme->getWindowText() ) {
        glColor4f( theme->getWindowText()->r,
                   theme->getWindowText()->g,
                   theme->getWindowText()->b,
                   theme->getWindowText()->a );
      } else {
        w->applyColor();
      }
    }
    scourge->getSDLHandler()->texPrint(5, y, s);
    if( theme->getWindowText() ) {
      glColor4f( theme->getWindowText()->r,
                 theme->getWindowText()->g,
                 theme->getWindowText()->b,
                 theme->getWindowText()->a );
    } else {
      w->applyColor();
    }
    sprintf(s, "HP: %d (%d)", p->getHp(), p->getMaxHp());
    scourge->getSDLHandler()->texPrint(5, y + 15, s);
    sprintf(s, "MP: %d (%d)", p->getMp(), p->getMaxMp());
    scourge->getSDLHandler()->texPrint(5, y + 30, s);
    sprintf(s, "AC: %d (%d)", p->getSkillModifiedArmor(), p->getArmor());
    scourge->getSDLHandler()->texPrint(5, y + 45, s);
    sprintf(s, "Thirst: %d (10)", p->getThirst());
    scourge->getSDLHandler()->texPrint(5, y + 60, s);
    sprintf(s, "Hunger: %d (10)", p->getHunger());
    scourge->getSDLHandler()->texPrint(5, y + 75, s);

    Util::drawBar( 160,  y - 3, 120, (float)p->getExp(), (float)p->getExpOfNextLevel(), 1.0f, 0.65f, 1.0f, false, theme );
    Util::drawBar( 160, y + 12, 120, (float)p->getHp(), (float)p->getMaxHp(), -1, -1, -1, true, theme );
    Util::drawBar( 160, y + 27, 120, (float)p->getMp(), (float)p->getMaxMp(), 0.45f, 0.65f, 1.0f, false, theme );
    Util::drawBar( 160, y + 42, 120, (float)p->getSkillModifiedArmor(), (float)p->getArmor(), 0.45f, 0.65f, 1.0f, false, theme );
    Util::drawBar( 160, y + 57, 120, (float)p->getThirst(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
    Util::drawBar( 160, y + 72, 120, (float)p->getHunger(), 10.0f, 0.45f, 0.65f, 1.0f, false, theme );
  }
}

bool Inventory::handleEvent(Widget *widget, SDL_Event *event) {
  Creature *creature = scourge->getParty()->getParty(selected);
  char *error = NULL;
  if( widget == mainWin->closeButton || widget == closeButton ) {
    // mainWin->setVisible(false);
    scourge->toggleInventoryWindow();
  } else if(widget == inventoryButton) setSelectedPlayerAndMode(selected, INVENTORY);
  else if(widget == skillsButton) setSelectedPlayerAndMode(selected, CHARACTER);
  else if(widget == spellsButton) setSelectedPlayerAndMode(selected, SPELL);
  else if(widget == specialButton) setSelectedPlayerAndMode(selected, SPECIAL);
  else if(widget == missionButton)  setSelectedPlayerAndMode(selected, MISSION);
  else if(widget == partyButton) setSelectedPlayerAndMode(selected, PARTY);
  else if(widget == invList && scourge->getTargetSelectionFor() ) {
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
      scourge->getInfoGui()->setItem( item, scourge->getParty()->getParty(selected)->getSkill(Constants::IDENTIFY_ITEM_SKILL) );
      if(!scourge->getInfoGui()->getWindow()->isVisible()) 
        scourge->getInfoGui()->getWindow()->setVisible( true );
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
  } else if(widget == skillAddButton) {
    if( scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
        scourge->getParty()->getParty(selected)->getAvailableSkillPoints() == 0 ) {
      error = Constants::getMessage(Constants::LEVEL_UP_ERROR);
    } else if(scourge->getParty()->getParty(selected)->getAvailableSkillPoints() <= 0) {
      //	  error = Constants::getMessage(Constants::OUT_OF_POINTS_ERROR);
    } else {
      int itemIndex = skillList->getSelectedLine();  
      if(itemIndex <= -1) {
        error = Constants::getMessage(Constants::NO_SKILL_ERROR);
      } else {
        scourge->getParty()->getParty(selected)->incSkillMod(itemIndex);
        refresh();
      }
    }
    if(error) {
      cerr << error << endl;
      scourge->showMessageDialog(error);
    }
  } else if(widget == skillSubButton) {
    if( scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
        scourge->getParty()->getParty(selected)->getUsedSkillPoints() == 0 ) {
      error = Constants::getMessage(Constants::LEVEL_UP_ERROR);
    } else if(scourge->getParty()->getParty(selected)->getAvailableSkillPoints() == 
              scourge->getParty()->getParty(selected)->getCharacter()->getSkillBonus()) {
      //	  error = Constants::getMessage(Constants::OUT_OF_POINTS_ERROR);
    } else {
      int itemIndex = skillList->getSelectedLine();  
      if(itemIndex <= -1) {
        error = Constants::getMessage(Constants::NO_SKILL_ERROR);
      } else {
        scourge->getParty()->getParty(selected)->decSkillMod(itemIndex);
        refresh();
      }
    }
    if(error) {
      cerr << error << endl;
      scourge->showMessageDialog(error);
    }
  } else if(widget == levelUpButton) {
    if( scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
        scourge->getParty()->getParty(selected)->getUsedSkillPoints() == 0 ) {
      error = Constants::getMessage(Constants::LEVEL_UP_ERROR);
    } else {
      scourge->getParty()->getParty(selected)->applySkillMod();
      refresh();
    }
    if(error) {
      cerr << error << endl;
      scourge->showMessageDialog(error);
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
    SpecialSkill *special = getSelectedSpecial();
    if(special) showSpecialDescription(special);
  } else if(widget == storeSpecialButton) {
    cerr << "FIXME: storeSpecialButton";
  } else if(widget == useSpecialButton) {
    cerr << "FIXME: useSpecialButton";
  } else if( widget == storeSpellButton ) {
    if( storeSpellButton->isSelected() ) {
      storeSpell = getSelectedSpell();
      if( !storeSpell ) {
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
  } else if(widget == castScrollButton) {
    // no MP-s used when casting from scroll, but the scroll is destroyed.
    int itemIndex = invList->getSelectedLine();  
    if(itemIndex > -1 && creature->getInventoryCount() > itemIndex) {
      Item *item = creature->getInventory(itemIndex);
      if(item->getSpell()) {
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
        scourge->showMessageDialog("You can only cast objects of magical nature!");
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
          int level = (int)((float)creature->getSkill( Constants::ENCHANT_ITEM_SKILL ) * rand()/RAND_MAX);
          if(level > 20) {
            int level = creature->getSkill( Constants::ENCHANT_ITEM_SKILL );
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
    //        case SDLK_ESCAPE: 
    //		  hide();
    //		  return true;
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
  switch(selectedMode) {
  case CHARACTER:         

    sprintf(skillsStr, "Skills: (Available points: %d)", selectedP->getAvailableSkillPoints());
    skillLabel->setText( skillsStr );


    sprintf(nameAndClassStr, "%s, %s (level %d) (%s)", 
            selectedP->getName(), 
            selectedP->getCharacter()->getName(), 
            selectedP->getLevel(),
            MagicSchool::getMagicSchool(selectedP->getDeityIndex())->getDeity());
    nameAndClassLabel->setText(nameAndClassStr);  

    stateCount = 0;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(selectedP->getStateMod(t)) {
        sprintf(stateLine[stateCount], "%s", Constants::STATE_NAMES[t]);
        icons[stateCount] = scourge->getShapePalette()->getStatModIcon(t);
        stateCount++;
      }
    }
    stateList->setLines(stateCount, (const char**)stateLine, 
                        (const Color *)NULL, (stateCount ? (const GLuint*)icons : NULL));

    stateCount = 0;
    for(int t = 0; t < Constants::STATE_MOD_COUNT; t++) {
      if(selectedP->getProtectedStateMod(t)) {
        sprintf(protStateLine[stateCount], "%s", Constants::STATE_NAMES[t]);
        protIcons[stateCount] = scourge->getShapePalette()->getStatModIcon(t);
        stateCount++;
      }
    }
    protStateList->setLines(stateCount, (const char**)protStateLine, 
                            (const Color *)NULL, (stateCount ? (const GLuint*)protIcons : NULL));


    for(int t = 0; t < Constants::SKILL_COUNT; t++) {
      sprintf(skillLine[t], "%d(%d) - %s", 
              selectedP->getSkill(t), 
              selectedP->getSkillMod(t), 
              Constants::SKILL_NAMES[t]);
    }
    skillList->setLines(Constants::SKILL_COUNT, (const char**)skillLine);
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
      if(t == 0) {
        showMemorizedSpellsInSchool(scourge->getParty()->getParty(selected), school);
      }
    }
    schoolList->setLines(MagicSchool::getMagicSchoolCount(), 
                         (const char**)schoolText);
    break;
  case SPECIAL:
    for(int t = 0; t < SpecialSkill::getSpecialSkillCount(); t++) {
      SpecialSkill *ss = SpecialSkill::getSpecialSkill(t);   
      sprintf(specialText[t], "%s (%s)", 
              ss->getName(), 
              (ss->getType() == SpecialSkill::SKILL_TYPE_AUTOMATIC ? "A" : "M"));
      specialIcons[t] = scourge->getShapePalette()->spellsTex[ ss->getIconTileX() ][ ss->getIconTileY() ];
    }
    specialList->setLines(SpecialSkill::getSpecialSkillCount(), 
                          (const char**)specialText,
                          NULL,
                          specialIcons);
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
    Character *character = scourge->getParty()->getParty(selected)->getCharacter();
/*
    cerr << "RA=" << Character::getCharacterIndexByShortName("RA") <<
      " KN=" << Character::getCharacterIndexByShortName("KN") <<
      " TI=" << Character::getCharacterIndexByShortName("TI") << 
      " AS=" << Character::getCharacterIndexByShortName("AS") << 
      " AR=" << Character::getCharacterIndexByShortName("AR") << 
      " LO=" << Character::getCharacterIndexByShortName("LO") << 
      " CO=" << Character::getCharacterIndexByShortName("CO") << 
      " SU=" << Character::getCharacterIndexByShortName("SU") << 
      " NA=" << Character::getCharacterIndexByShortName("NA") << 
      " MO=" << Character::getCharacterIndexByShortName("MO") << 
      " this=" << character->getShortName() << "=" << Character::getCharacterIndexByShortName(character->getShortName()) <<
      " item=" << item->getRpgItem()->getName() << 
      " acl=" << item->getRpgItem()->getAcl(Character::getCharacterIndexByShortName(character->getShortName())) << 
      " all acl=" << item->getRpgItem()->getAllAcl() << endl;
*/

    if(!item->getRpgItem()->getAcl(Character::getCharacterIndexByShortName(character->getShortName()))) {
      scourge->showMessageDialog(Constants::getMessage(Constants::ITEM_ACL_VIOLATION));
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_FAILED);
      return;
    }
    if( item->getLevel() > scourge->getParty()->getParty(selected)->getLevel() ) {
      scourge->showMessageDialog(Constants::getMessage(Constants::ITEM_LEVEL_VIOLATION));
      scourge->getSDLHandler()->getSound()->playSound(Window::DROP_FAILED);
      return;
    }
    scourge->getParty()->getParty(selected)->equipInventory(itemIndex);
    // recreate list strings
    refresh();
    scourge->getSDLHandler()->getSound()->playSound(Window::DROP_SUCCESS);
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
  invList->setSelectedLine(oldLine);
  skillList->setSelectedLine(oldSkillLine);
}

void Inventory::positionWindow() {
  mainWin->move( scourge->getPartyWindow()->getX() + 
                 scourge->getPartyWindow()->getWidth() - 
                 mainWin->getWidth(),
                 scourge->getPartyWindow()->getY() - mainWin->getHeight() );
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
  if(n != -1 && n < SpecialSkill::getSpecialSkillCount()) {
    ss = SpecialSkill::getSpecialSkill(n);
  }
  return ss;
}

void Inventory::showSpecialDescription(SpecialSkill *ss) {
  specialDescriptionLabel->setText((char*)(ss->getDescription() ? ss->getDescription() : ""));
}

