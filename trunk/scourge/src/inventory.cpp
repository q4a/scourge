

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
    this->selected = 0;
    this->selectedMode = 0;
    this->modeName[0] = "Properties";
    this->modeName[1] = "Inventory";
    this->modeName[2] = "Spells";
    this->modeName[3] = "Accomplishments";
    this->invText = (char**)malloc(Creature::SKILL_COUNT * sizeof(char*));
    for(int i = 0; i < Creature::SKILL_COUNT; i++) {
        this->invText[i] = (char*)malloc(120 * sizeof(char));
    }
}

Inventory::~Inventory() {
}

void Inventory::show() {
    scourge->getGui()->pushWindows();
    createGui();
    scourge->getSDLHandler()->pushHandlers(this, this);
}

void Inventory::createGui() {
    win = scourge->getGui()->addWindow(1, 1,
                                       scourge->getSDLHandler()->getScreen()->w - 1,
                                       scourge->getSDLHandler()->getScreen()->h - 1,
                                       &Gui::drawInventory);
    // add some active regions
    scourge->getGui()->addActiveRegion(0, 0, 105, 120, Constants::INV_PLAYER_0, this);
    scourge->getGui()->addActiveRegion(0, 120, 105, 240, Constants::INV_PLAYER_1, this);
    scourge->getGui()->addActiveRegion(0, 240, 105, 360, Constants::INV_PLAYER_2, this);
    scourge->getGui()->addActiveRegion(0, 360, 105, 480, Constants::INV_PLAYER_3, this);
    scourge->getGui()->addActiveRegion(105, scourge->getSDLHandler()->getScreen()->h - 30, 255, scourge->getSDLHandler()->getScreen()->h, Constants::INV_MODE_PROPERTIES, this);
    scourge->getGui()->addActiveRegion(255, scourge->getSDLHandler()->getScreen()->h - 30, 405, scourge->getSDLHandler()->getScreen()->h, Constants::INV_MODE_INVENTORY, this);
    scourge->getGui()->addActiveRegion(405, scourge->getSDLHandler()->getScreen()->h - 30, 555, scourge->getSDLHandler()->getScreen()->h, Constants::INV_MODE_SPELLS, this);
    scourge->getGui()->addActiveRegion(555, scourge->getSDLHandler()->getScreen()->h - 30, 705, scourge->getSDLHandler()->getScreen()->h, Constants::INV_MODE_LOG, this);
    scourge->getGui()->addActiveRegion(0, scourge->getSDLHandler()->getScreen()->h - 30, 105, scourge->getSDLHandler()->getScreen()->h, Constants::ESCAPE, this);

    skillList = scourge->getGui()->   
        addScrollingList(120, 250, 350, 500,
                         Constants::SKILL_LIST);
}

void Inventory::drawView(SDL_Surface *screen) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    scourge->getGui()->drawWindows();
}

bool Inventory::handleEvent(SDL_Event *event) {
    switch(event->type) {
    case SDL_MOUSEBUTTONUP:
        if(event->button.button) {
            if(processMouseClick(event->button.x, event->button.y, event->button.button)) {
                scourge->getGui()->popWindows();
                return true;
            }
        }
        break;     
    case SDL_KEYDOWN:
        switch(event->key.keysym.sym) {
        case SDLK_ESCAPE: 
            scourge->getGui()->popWindows();
            return true;
        default: break;
        }
    default: break;
    }
    return false;
}

bool Inventory::processMouseClick(int x, int y, int button) {
    int region = scourge->getGui()->testActiveRegions(x, y);
//    fprintf(stderr, "*** region=%d\n", region);
    if(region == Constants::INV_PLAYER_0 || region == Constants::INV_PLAYER_1 ||
       region == Constants::INV_PLAYER_2 || region == Constants::INV_PLAYER_3) {
        selected = region;
    } else if(region == Constants::INV_MODE_PROPERTIES || region == Constants::INV_MODE_INVENTORY ||
              region == Constants::INV_MODE_SPELLS || region == Constants::INV_MODE_LOG) {
        selectedMode = region - Constants::INV_MODE_PROPERTIES;
    } else if(region == Constants::ESCAPE) {
        return true;
    }
    return false;
}

void Inventory::drawInventory() {
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    drawParty();

    glColor3f(1.0f, 0.6f, 0.3f);
    glBegin(GL_LINES);
        glVertex2d(105, 0);
        glVertex2d(105, scourge->getSDLHandler()->getScreen()->h);
    glEnd();

    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    drawModeButtons();
    
    glColor3f(1.0f, 0.6f, 0.3f);
    glBegin(GL_LINES);
        glVertex2d(0, scourge->getSDLHandler()->getScreen()->h - 30);
        glVertex2d(scourge->getSDLHandler()->getScreen()->w, 
                   scourge->getSDLHandler()->getScreen()->h - 30);
    glEnd();

    switch(selectedMode) {
    case CHARACTER:
        drawCharacterInfo(); break;
    case INVENTORY:
        drawInventoryInfo(); break;
    case SPELL:
        drawSpellInfo(); break;
    case LOG:
        drawLogInfo(); break;
    }
}

void Inventory::drawParty() {
    int h = 120;
    int y;  
    for(int i = 0; i < 4; i++) {
        //y = 10 + i * h;
        //glRasterPos2f( 10, y );
        //scourge->getShapePalette()->drawPortrait(scourge->getParty(i)->getPortraitIndex());        


        glPushMatrix();
        glLoadIdentity();
        glEnable( GL_TEXTURE_2D );
        glEnable( GL_LIGHTING ); 
        glEnable( GL_LIGHT2 );        
        glTranslatef( 20, 10 + i * h + 90, 0);
        glRotatef(90, 1, 0, 0);
        glScalef(0.8, 0.8, 0.8);
        scourge->getParty(i)->draw();
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_LIGHTING ); 
        glDisable( GL_LIGHT2 );
        glPopMatrix();

        scourge->getSDLHandler()->texPrint(10, 10 + i * h + 100, "%s", scourge->getParty(i)->getName());

        if(selected == i) {
            y = 10 + i * h;
            glColor4f(0.6f, 0.4f, 0.2f, 0.5f);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glEnable(GL_BLEND);
            glBegin (GL_QUADS);
                glVertex3i (0, y - 10, 10);
                glVertex3i (0, y + h - 10, 10);      
                glVertex3i (105, y + h - 10, 10);
                glVertex3i (105, y - 10, 10);
            glEnd ();
            glDisable(GL_BLEND);
            glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
        }
    }      
}

void Inventory::drawModeButtons() {

    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint(10, 
                                       scourge->getSDLHandler()->getScreen()->h - 20, 
                                       "Back");

    int w = 150;
    for(int i = 0; i < 4; i++) {
        glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
        scourge->getSDLHandler()->texPrint(110 + i * w, 
                                           scourge->getSDLHandler()->getScreen()->h - 20, 
                                           "%s", modeName[i]);
        glColor3f(1.0f, 0.6f, 0.3f);
        glBegin(GL_LINES);
            glVertex2d(105 + (i + 1) * w, scourge->getSDLHandler()->getScreen()->h - 30);
            glVertex2d(105 + (i + 1) * w, scourge->getSDLHandler()->getScreen()->h);
        glEnd();
        if(selectedMode == i) {
            glColor4f(0.6f, 0.4f, 0.2f, 0.5f);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glEnable(GL_BLEND);
            glBegin (GL_QUADS);
                glVertex3i (105 + i * w, scourge->getSDLHandler()->getScreen()->h - 30, 10);
                glVertex3i (105 + i * w, scourge->getSDLHandler()->getScreen()->h, 10);      
                glVertex3i (105 + (i + 1) * w, scourge->getSDLHandler()->getScreen()->h, 10);
                glVertex3i (105 + (i + 1) * w, scourge->getSDLHandler()->getScreen()->h - 30, 10);
            glEnd ();
            glDisable(GL_BLEND);
            glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
        }
    }
}

void Inventory::drawCharacterInfo() {
    int xx = 110, yy = 0;
    int x = 10, y = 10;
    int i = selected;

    x += xx;
    y += yy;
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 10), "%s", scourge->getParty(i)->getName());
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 24), "%s", scourge->getParty(i)->getCharacter()->getName());
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 38), "Level: %d", scourge->getParty(i)->getLevel());
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 52), "Exp: %u", scourge->getParty(i)->getExp());
    scourge->getSDLHandler()->texPrint((float)x, (float)(y + 66), "HP: %d", scourge->getParty(i)->getHp());


    y = yy + 100;
    x = xx + GUI_PLAYER_INFO_W / 2 + 10;
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint((float)x, (float)(y), "Current State:");    
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);

    y += 10;
    glBegin(GL_LINES);
        glVertex2d(x, y);
        glVertex2d(x + GUI_PLAYER_INFO_W / 2 - 20, y);
    glEnd();

    y += 24;
    for(int t = 0; t < Creature::STATE_MOD_COUNT; t++) {
      if(scourge->getParty(i)->getStateMod(t)) {
        scourge->getSDLHandler()->texPrint((float)x, (float)(y), "%s", Creature::STATE_NAMES[t]);
        y += 14;
      }
    }

    y = yy + 100;
    x = xx + 10;
    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint((float)x, (float)(y), "Attributes:");
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);

    y += 10;
    glBegin(GL_LINES);
        glVertex2d(x, y);
        glVertex2d(x + GUI_PLAYER_INFO_W / 2 - 20, y);
    glEnd();

    y += 24;
    for(int t = 0; t < Creature::ATTR_COUNT; t++) {
      scourge->getSDLHandler()->texPrint((float)x, (float)(y), "%s: %d", Creature::ATTR_NAMES[t], scourge->getParty(i)->getAttr(t));
      y += 14;
    }    

    glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    scourge->getSDLHandler()->texPrint(120, 245, "Skills:");    
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
    for(int t = 0; t < Creature::SKILL_COUNT; t++) {
        sprintf(invText[t], "%d - %s", scourge->getParty(i)->getSkill(t), Creature::SKILL_NAMES[t]);
    }

    scourge->getGui()->drawScrollingList(skillList, Creature::SKILL_COUNT, (const char**)invText);
}
void Inventory::drawInventoryInfo() {
}
void Inventory::drawSpellInfo() {
}
void Inventory::drawLogInfo() {
}


