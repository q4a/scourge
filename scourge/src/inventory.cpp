

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

/**
  *@author Gabor Torok
  */


Inventory::Inventory(Scourge *scourge) {
  this->scourge = scourge;

  // allocate strings for list
  this->pcInvText = (char**)malloc(MAX_INVENTORY_SIZE * sizeof(char*));
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
  for(int i = 0; i < MAX_INVENTORY_SIZE; i++) {
    this->spellText[i] = (char*)malloc(120 * sizeof(char));
  }
  selected = selectedMode = 0;

  // construct UI
  mainWin = new Window( scourge->getSDLHandler(),
                        100, 50, Scourge::INVENTORY_WIDTH, Scourge::INVENTORY_HEIGHT,
                        strdup("Party Information"), 
                        scourge->getShapePalette()->getGuiTexture() );

  char label[80];
  memset(label, ' ', 78);
  label[79] = 0;
  playerButton[0]  = mainWin->createButton( 0, 0, 105, 30, strdup(label), true);
  playerButton[1]  = mainWin->createButton( 0, 30, 105, 60, strdup(label), true);
  playerButton[2]  = mainWin->createButton( 0, 60, 105, 90, strdup(label), true );
  playerButton[3]  = mainWin->createButton( 0, 90, 105, 120, strdup(label), true );

  int yy = Scourge::INVENTORY_HEIGHT - Window::TOP_HEIGHT - Window::BOTTOM_HEIGHT - (4 * 30);
  inventoryButton = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Inventory"), true);
  yy += 30;
  skillsButton   = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Skills"), true);
  yy += 30;
  spellsButton   = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Spells"), true);
  yy += 30;
  missionButton   = mainWin->createButton( 0, yy, 105, yy + 30, strdup("Mission"), true);
  yy += 30;
  cards = new CardContainer(mainWin);

  // inventory page	
  cards->createLabel(115, 15, strdup("Inventory:"), INVENTORY, Constants::RED_COLOR); 
  inventoryWeightLabel = cards->createLabel(190, 15, NULL, INVENTORY);

  coinsLabel = cards->createLabel(300, 212, NULL, INVENTORY);
  cards->createLabel(115, 212, strdup("Equipped Items:"), INVENTORY, Constants::RED_COLOR);

  paperDoll = new Canvas(115, 220, 411, 251 + (Constants::INVENTORY_COUNT * 15), this, this);
  cards->addWidget(paperDoll, INVENTORY);

  invList = new ScrollingList(115, 20, 295, 175, scourge->getShapePalette()->getHighlightTexture(), this);
  cards->addWidget(invList, INVENTORY);
  cards->createLabel(115, 475, Constants::getMessage(Constants::EXPLAIN_DRAG_AND_DROP), INVENTORY);

  yy = 160;
  equipButton    = cards->createButton( 0, yy, 105, yy + 30, strdup("Don/Doff"), INVENTORY);
  yy+=30;
  openButton     = cards->createButton( 0, yy, 105, yy + 30, Constants::getMessage(Constants::OPEN_CONTAINER_LABEL), INVENTORY ); 
  yy+=30;
  eatDrinkButton = cards->createButton( 0, yy, 105, yy + 30, strdup("Eat/Drink"), INVENTORY );
  yy+=30;
  castScrollButton = cards->createButton( 0, yy, 105, yy + 30, strdup("Cast Scroll"), INVENTORY );
  yy+=30;
  transcribeButton = cards->createButton( 0, yy, 105, yy + 30, strdup("Transcribe"), INVENTORY );
  yy+=30;
  enchantButton = cards->createButton( 0, yy, 105, yy + 30, strdup("Enchant"), INVENTORY );

  // character info
  nameAndClassLabel = cards->createLabel(115, 45, NULL, CHARACTER, Constants::RED_COLOR);
  attrCanvas     = new Canvas( 115, 50, 405, 150, this );
  cards->addWidget( attrCanvas, CHARACTER );

  cards->createLabel(115, 165, strdup("Current State:"), CHARACTER, Constants::RED_COLOR);
  stateList = new ScrollingList(115, 170, 140, 70, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(stateList, CHARACTER);
  
  cards->createLabel(265, 165, strdup("Protected States:"), CHARACTER, Constants::RED_COLOR);
  protStateList = new ScrollingList(265, 170, 140, 70, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(protStateList, CHARACTER);

  strcpy(skillsStr, "Skills:");
  cards->createLabel(115, 255, skillsStr, CHARACTER, Constants::RED_COLOR);
  skillModLabel = cards->createLabel(220, 255, NULL, CHARACTER);
  skillList = new ScrollingList(115, 260, 290, 180, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(skillList, CHARACTER);
  skillAddButton = cards->createButton( 115, 445, 200, 475, strdup(" + "), CHARACTER);
  skillSubButton = cards->createButton( 320, 445, 405, 475, strdup(" - "), CHARACTER);
  levelUpButton = cards->createButton( 205, 445, 315, 475, strdup("Level Up"), CHARACTER);

  // spellbook
  cards->createLabel(115, 45, strdup("School of magic: (with provider deity)"), 
                     SPELL, Constants::RED_COLOR);
  schoolList = new ScrollingList(115, 50, 290, 100, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(schoolList, SPELL);
  cards->createLabel(115, 170, strdup("Spells memorized:"), SPELL, Constants::RED_COLOR);
  spellList = new ScrollingList(115, 175, 290, 150, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(spellList, SPELL);
  cards->createLabel(115, 345, strdup("Spell notes:"), SPELL, Constants::RED_COLOR);
  spellDescriptionLabel = new Label(115, 360, strdup(""), 58);
  cards->addWidget(spellDescriptionLabel, SPELL);
  castButton = cards->createButton( 0, 160, 105, 190, strdup("Cast"), SPELL);


  // mission
  cards->createLabel(115, 45, strdup("Current Mission"), MISSION, Constants::RED_COLOR);
  missionDescriptionLabel = new Label(115, 60, strdup(""), 50);
  cards->addWidget(missionDescriptionLabel, MISSION);
  cards->createLabel(115, 280, strdup("Mission Objectives"), MISSION, Constants::RED_COLOR);
  objectiveList = new ScrollingList(115, 285, 295, 175, scourge->getShapePalette()->getHighlightTexture());
  cards->addWidget(objectiveList, MISSION);

  setSelectedPlayerAndMode(0, INVENTORY);
}

Inventory::~Inventory() {
}

void Inventory::drawWidget(Widget *w) {
  Creature *p = scourge->getParty()->getParty(selected);

  if(w == paperDoll) {
    float x = 125;
    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
      Item *item = scourge->getParty()->getParty(selected)->getEquippedInventory(i);
      if(item) {
        w->applySelectionColor();
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

    w->applyBorderColor();
    glBegin( GL_LINES );
    glVertex2f( x, 0 );
    glVertex2f( x, w->getHeight() );
    glEnd();

    for(int i = 0; i < Constants::INVENTORY_COUNT; i++) {
      Item *item = scourge->getParty()->getParty(selected)->getEquippedInventory(i);

      w->applyBorderColor();
      glBegin( GL_LINES );
      glVertex2f( x, (i + 1) * 16 );
      glVertex2f( w->getWidth(), (i + 1) * 16 );
      glEnd();

      glColor3f( 0, 0, 0 );
      if(!item) continue;
      scourge->getSDLHandler()->texPrint( x + 5, (i + 1) * 16 - 4, item->getItemName());
    }

  } else {
    int y = 15;
    char s[80];
    sprintf(s, "Exp: %u(%u)", p->getExp(), p->getExpOfNextLevel());
    if(p->getStateMod(Constants::leveled)) {
      glColor4f( 1.0f, 0.2f, 0.0f, 1.0f );
    } else {
      w->applyColor();
    }
    scourge->getSDLHandler()->texPrint(5, y, s);
    w->applyColor();
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

    Util::drawBar( 160,  y - 3, 120, (float)p->getExp(), (float)p->getExpOfNextLevel(), 1.0f, 0.65f, 1.0f, false );
    Util::drawBar( 160, y + 12, 120, (float)p->getHp(), (float)p->getMaxHp() );
    Util::drawBar( 160, y + 27, 120, (float)p->getMp(), (float)p->getMaxMp(), 0.45f, 0.65f, 1.0f, false );
    Util::drawBar( 160, y + 42, 120, (float)p->getSkillModifiedArmor(), (float)p->getArmor(), 0.45f, 0.65f, 1.0f, false );
    Util::drawBar( 160, y + 57, 120, (float)p->getThirst(), 10.0f, 0.45f, 0.65f, 1.0f, false );
    Util::drawBar( 160, y + 72, 120, (float)p->getHunger(), 10.0f, 0.45f, 0.65f, 1.0f, false );
  }
}

bool Inventory::handleEvent(Widget *widget, SDL_Event *event) {
  Creature *creature = scourge->getParty()->getParty(selected);
  char *error = NULL;
  if(widget == mainWin->closeButton) mainWin->setVisible(false);
  else if(widget == playerButton[0]) setSelectedPlayerAndMode(0, selectedMode);
  else if(widget == playerButton[1]) setSelectedPlayerAndMode(1, selectedMode);
  else if(widget == playerButton[2]) setSelectedPlayerAndMode(2, selectedMode);
  else if(widget == playerButton[3]) setSelectedPlayerAndMode(3, selectedMode);
  else if(widget == inventoryButton) setSelectedPlayerAndMode(selected, INVENTORY);
  else if(widget == skillsButton) setSelectedPlayerAndMode(selected, CHARACTER);
  else if(widget == spellsButton) setSelectedPlayerAndMode(selected, SPELL);
  else if(widget == missionButton)  setSelectedPlayerAndMode(selected, MISSION);
  else if(widget == openButton) {
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
    if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
       !scourge->getParty()->getParty(selected)->getStateMod(Constants::leveled)) {
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
    if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
       !scourge->getParty()->getParty(selected)->getStateMod(Constants::leveled)) {
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
    if(scourge->getParty()->getParty(selected)->getStateMod(Constants::dead) || 
       !scourge->getParty()->getParty(selected)->getStateMod(Constants::leveled)) {
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
  } else if(widget == castButton) {
    Spell *spell = getSelectedSpell();
    if(spell) {
      if(spell->getMp() > creature->getMp()) {
        scourge->showMessageDialog("Not enough Magic Points to cast this spell!");
      } else {
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
      if(item->getMagicAttrib()) {
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
  }
  return false;
}

void Inventory::moveItemTo(int playerIndex) {
  int itemIndex = invList->getSelectedLine();  
  if(itemIndex > -1 && 
     scourge->getParty()->getParty(selected)->getInventoryCount() > itemIndex) {
    if(playerIndex != selected) {
      scourge->getParty()->getParty(playerIndex)->
      addInventory(scourge->getParty()->getParty(selected)->removeInventory(itemIndex));
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

  playerButton[0]->setSelected(selected == 0);
  playerButton[1]->setSelected(selected == 1);
  playerButton[2]->setSelected(selected == 2);
  playerButton[3]->setSelected(selected == 3);
  inventoryButton->setSelected(selectedMode == INVENTORY);
  skillsButton->setSelected(selectedMode == CHARACTER);
  spellsButton->setSelected(selectedMode == SPELL);
  missionButton->setSelected(selectedMode == MISSION);

  // show only the ui elements belonging to the current mode
  cards->setActiveCard(selectedMode);   

  // arrange the gui
  int stateCount;
  Creature * selectedP = scourge->getParty()->getParty(selected);
  switch(selectedMode) {
  case CHARACTER:         

    sprintf(skillsStr, "Skills: (Available points: %d)", selectedP->getAvailableSkillPoints());


    sprintf(nameAndClassStr, "%s, %s (level %d)", selectedP->getName(), 
            selectedP->getCharacter()->getName(), selectedP->getLevel());
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
      sprintf(pcInvText[t], "%s %s", (location > -1 ? " *" : "   "), s);
    }
    for(int t = selectedP->getInventoryCount(); 
       t < MAX_INVENTORY_SIZE; t++) {
      strcpy(pcInvText[t], "");
    }
    invList->setLines(selectedP->getInventoryCount(), 
                      (const char **)pcInvText);
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
  case LOG:
    break;
  case MISSION:
    int objectiveCount = 0;
    if(scourge->getSession()->getCurrentMission()) {
      sprintf(missionText, "%s: %s", 
              scourge->getSession()->getCurrentMission()->getName(), 
              scourge->getSession()->getCurrentMission()->getStory());
      objectiveCount = 
      scourge->getSession()->getCurrentMission()->getObjective()->itemCount +
      scourge->getSession()->getCurrentMission()->getObjective()->monsterCount;   
      for(int t = 0; t < scourge->getSession()->getCurrentMission()->getObjective()->itemCount; t++) {
        sprintf(objectiveText[t], "Find %s. %s", 
                scourge->getSession()->getCurrentMission()->getObjective()->item[t]->getName(),
                (scourge->getSession()->getCurrentMission()->getObjective()->itemHandled[t] ? 
                 "(completed)" : "(not yet found)"));
        if(scourge->getSession()->getCurrentMission()->getObjective()->itemHandled[t]) {
          missionColor[t].r = 0.2f;
          missionColor[t].g = 0.7f;
          missionColor[t].b = 0.2f;
        } else {
          missionColor[t].r = 0.7f;
          missionColor[t].g = 0.2f;
          missionColor[t].b = 0.2f;
        }
      }
      int start = scourge->getSession()->getCurrentMission()->getObjective()->itemCount;
      for(int t = 0; t < scourge->getSession()->getCurrentMission()->getObjective()->monsterCount; t++) {
        sprintf(objectiveText[start + t], "Vanquish %s. %s", 
                scourge->getSession()->getCurrentMission()->getObjective()->monster[t]->getType(),
                (scourge->getSession()->getCurrentMission()->getObjective()->monsterHandled[t] ? 
                 "(completed)" : "(not yet done)"));
        if(scourge->getSession()->getCurrentMission()->getObjective()->monsterHandled[t]) {
          missionColor[start + t].r = 0.2f;
          missionColor[start + t].g = 0.7f;
          missionColor[start + t].b = 0.2f;
        } else {
          missionColor[start + t].r = 0.7f;
          missionColor[start + t].g = 0.2f;
          missionColor[start + t].b = 0.2f;
        }
      }
      start += scourge->getSession()->getCurrentMission()->getObjective()->monsterCount;
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
  }
}

/*
// FIXME: this doesn't work: I can't get it to draw above the window
void Inventory::drawInventory() {
  // draw the characters on top of the UI
  glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
  int h = 120;
//  int y;  
  for(int i = 0; i < 4; i++) {

    // why do I need these 2 lines? Otherwise the models go behind the window
    //	glDisable(GL_DEPTH_TEST);
    //glEnable(GL_DEPTH_TEST);

    glPushMatrix();
    glLoadIdentity();

//	cerr << "z=" << mainWin->getZ() << endl;
    glTranslatef( mainWin->getX(), mainWin->getY() + Window::TOP_HEIGHT, mainWin->getZ() + 200 );
    glTranslatef( 20, 10 + i * h + 90, 300);

    glRotatef(90, 1, 0, 0);
    glScalef(0.8, 0.8, 0.8);  
    glEnable( GL_TEXTURE_2D );
    glColor4f( 1, 1, 1, 1 );
    mainWin->scissorToWindow();
    scourge->getParty()->getParty(i)->draw();
    glDisable( GL_SCISSOR_TEST );
    glDisable( GL_TEXTURE_2D );
    glPopMatrix();
  }
}
*/

void Inventory::receive(Widget *widget) {
  if(widget == invList) {
    putItem();
  } else if(widget == paperDoll) {
    if(putItem() != -1) equipItem();
  }
}

bool Inventory::startDrag(Widget *widget, int x, int y) {
  if(widget == invList) {
    dropItem();
    return true;
  } else if(widget == paperDoll) {
    if(y > 0 && y < Constants::INVENTORY_COUNT * 16) {
      // what's equiped at this inventory slot?
      Item *item = scourge->getParty()->getParty(selected)->getItemAtLocation((1 << (y / 16)));
      if(item) {
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
    scourge->setMovingItem(item, 
                           scourge->getParty()->getParty(selected)->getX(), 
                           scourge->getParty()->getParty(selected)->getY(), 
                           scourge->getParty()->getParty(selected)->getZ());
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

void Inventory::show(bool animate) { 
  mainWin->setVisible(true, animate); 

  // find selected player. FIXME: this is inefficient
  int n = selected;
  char buttonText[80];

  for(int i = 0; i < scourge->getParty()->getPartySize(); i++) {
    if(scourge->getParty()->getPlayer() == scourge->getParty()->getParty(i)) {
      n = i;
      //break;
    }

    sprintf(buttonText, "%s (%s)", 
            scourge->getParty()->getParty(i)->getName(),
            scourge->getParty()->getParty(i)->getCharacter()->getShortName());
    playerButton[i]->getLabel()->setTextCopy(buttonText);
    playerButton[i]->setVisible(true);
  }
  for(int i = scourge->getParty()->getPartySize(); i < MAX_PARTY_SIZE; i++) {
    playerButton[i]->setVisible(false);
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
      spellCount++;
    }
  }
  if(spellCount == 0) spellDescriptionLabel->setText("");
  spellList->setLines(spellCount, 
                      (const char**)spellText);
}

void Inventory::showSpellDescription(Spell *spell) {
  spellDescriptionLabel->setText((char*)(spell->getNotes() ? spell->getNotes() : ""));
}
