/***************************************************************************
                          gui.cpp  -  description
                             -------------------
    begin                : Thu Aug 28 2003
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

#include "gui.h"

Gui::Gui(Scourge *scourge){
  this->scourge = scourge;
  windowCount = 0;
  regionCount = 0;
  windowStackCount = 0;  
  scrollingListCount = 0;
  currentScroller = -1;
  alpha = 0.5f;
  alphaInc = 0.05f;
  lastTick = 0;
}

Gui::~Gui(){
}

int Gui::addWindow(int x, int y, int w, int h,
               void (Gui::*callbackFunc)(int, int)) {
  if(windowCount >= MAX_WINDOW_COUNT) return -1;
  windows[windowCount].x = x;
  windows[windowCount].y = y;
  windows[windowCount].w = w;
  windows[windowCount].h = h;
  windows[windowCount].visible = true;
  windows[windowCount].callbackFunc = callbackFunc;
  windows[windowCount].id = windowCount;
  int ret = windowCount++;
  return ret;
}

void Gui::removeWindow(int id) {
    int index = -1;             
    for(int i = 0; i < windowCount; i++) {
        if(windows[i].id == id) {
            index = i;
            break;
        }
    }
    if(index == -1) return;
    for(int i = index; i < windowCount - 1; i++) {
        memcpy(&(windows[i]), &(windows[i + 1]), sizeof(GUIWindow));
    }
    windowCount--;
}

void Gui::removeAllWindows() {
  fprintf(stderr, "gui::removeAllWindows!!!");
  windowCount = 0;
}

void Gui::pushWindows() {
    if(windowStackCount >= MAX_STACK_COUNT) return;
    windowStack[windowStackCount].windowCount = windowCount;
    for(int i = 0; i < windowCount; i++) {
        memcpy(&(windowStack[windowStackCount].windows[i]), &(windows[i]), sizeof(GUIWindow));
    }

    // fixme
	removeAllScrollingLists();

    windowStack[windowStackCount].regionCount = regionCount;
    for(int i = 0; i < regionCount; i++) {
        memcpy(&(windowStack[windowStackCount].regions[i]), &(regions[i]), sizeof(ActiveRegion));
    }
    windowStackCount++;
    windowCount = regionCount = 0;
}

void Gui::popWindows() {
    if(windowStackCount == 0) return;
    windowStackCount--;    
    windowCount = windowStack[windowStackCount].windowCount;
    for(int i = 0; i < windowCount; i++) {
        memcpy(&(windows[i]), &(windowStack[windowStackCount].windows[i]), sizeof(GUIWindow));
    }

    // fixme
	removeAllScrollingLists();

    regionCount = windowStack[windowStackCount].regionCount;
    for(int i = 0; i < regionCount; i++) {
        memcpy(&(regions[i]), &(windowStack[windowStackCount].regions[i]), sizeof(ActiveRegion));
    }
}

void Gui::setWindowVisible(int id, bool b) {
    int index = -1;             
    for(int i = 0; i < windowCount; i++) {
        if(windows[i].id == id) {
            index = i;
            break;
        }
    }
    if(index == -1) return;
    windows[index].visible = b;
}

bool Gui::isWindowVisible(int id) {
    int index = -1;             
    for(int i = 0; i < windowCount; i++) {
        if(windows[i].id == id) {
            index = i;
            break;
        }
    }
    if(index == -1) return false;
    return windows[index].visible;
}

void Gui::drawWindows() {
  for(int i = 0; i < windowCount; i++) {
    if(windows[i].visible) drawGui(windows[i]);
  }
}

void Gui::drawGui(GUIWindow win) {
  // draw the interface
  glLoadIdentity( );

  glEnable( GL_TEXTURE_2D );
  //glEnable( GL_LIGHTING );
  //glEnable( GL_LIGHT2 );

  // tile the background
  glColor3f(1.0f, 0.6f, 0.3f);
  glTranslated(win.x, win.y, 0);
  glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getGuiTexture() );
	glBegin (GL_QUADS);
      glTexCoord2f (0.0f, 0.0f);
	    glVertex2i (0, 0);
      
      glTexCoord2f (0.0f, win.h/(float)TILE_H);
	    glVertex2i (0, win.h);
      
      glTexCoord2f (win.w/(float)TILE_W, win.h/(float)TILE_H);
	    glVertex2i (win.w, win.h);
      
	    glTexCoord2f (win.w/(float)TILE_W, 0.0f);      
	    glVertex2i (win.w, 0);
	glEnd ();

	//glDisable( GL_LIGHT2 );
  //glDisable( GL_LIGHTING );
  glDisable( GL_TEXTURE_2D );

  // add a border
  glColor3f(1.0f, 0.6f, 0.3f);
  glBegin(GL_LINES);
  	glVertex2d(win.w, win.h);
    glVertex2d(0, win.h);
    
  	glVertex2d(0, 0);
    glVertex2d(win.w, 0);
    
  	glVertex2d(0, 0);
    glVertex2d(0, win.h);
    
  	glVertex2d(win.w, 0);
    glVertex2d(win.w, win.h);		            
	glEnd();

//  glTranslated(-x, -y, 0);
  if(win.callbackFunc)
    (*this.*(win.callbackFunc))(win.x, win.y);

//  fprintf(stderr, "event=%d button=%d\n", scourge->getSDLHandler()->mouseEvent, scourge->getSDLHandler()->mouseButton);
//  fprintf(stderr, "SDL_MOUSEBUTTONUP=%d SDL_MOUSEBUTTONDOWN=%d\n", SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONDOWN);
  if(currentScroller != -1) {  
//      fprintf(stderr, "1\n");
      if(!scourge->getSDLHandler()->mouseDragging) {
          currentScroller = -1;
      } else {      
          showCurrentRegion(currentScroller);
          moveScrollingList(currentScroller);
      }
  } else {
//      fprintf(stderr, "2\n");
      // check active regions
      int currentRegion = testActiveRegions(scourge->getSDLHandler()->mouseX, 
                                            scourge->getSDLHandler()->mouseY);
      if(currentRegion != -1) {
          showCurrentRegion(currentRegion);
          if(scourge->getSDLHandler()->mouseDragging) {          
          //if(scourge->getSDLHandler()->mouseEvent == SDL_MOUSEBUTTONDOWN) {              
              initScroller(currentRegion);
          }
      } else {
          if(scourge->getSDLHandler()->mouseEvent == SDL_MOUSEBUTTONDOWN ||
             (scourge->getSDLHandler()->mouseEvent == SDL_MOUSEMOTION && 
              scourge->getSDLHandler()->mouseButton)) {
              selectScrollingItem(scourge->getSDLHandler()->mouseX, 
                                  scourge->getSDLHandler()->mouseY);
          }
      }
  }

  glEnable( GL_TEXTURE_2D );
  //glEnable( GL_LIGHTING );  
}

void Gui::drawMainMenu(int x, int y) {
  scourge->getMainMenu()->drawMenu(x, y);
}

void Gui::drawDescriptions(int x, int y) {
  scourge->drawTopWindow();
}

void Gui::drawInventory(int x, int y) {
  scourge->getInventory()->drawInventory();
}

void Gui::addActiveRegion(int x1, int y1, 
                         int x2, int y2, 
                         int id, 
                         SDLEventHandler *eventHandler) {
    if(regionCount >= MAX_REGION_COUNT) return;
    regions[regionCount].x1 = x1;
    regions[regionCount].y1 = y1;
    regions[regionCount].x2 = x2;
    regions[regionCount].y2 = y2;
    regions[regionCount].eventHandler = eventHandler;
    regions[regionCount].id = id;
    regionCount++;
}

void Gui::outlineActiveRegion(int id, const char *label) {
  int index = -1;
  for(int i = 0; i < regionCount; i++) {
	if(regions[i].id == id) {
	  index = i;
	  break;
	}
  }
  if(index == -1) return;
  glBegin(GL_LINES);
  glVertex2d(regions[index].x1, regions[index].y1);
  glVertex2d(regions[index].x1, regions[index].y2);
  glVertex2d(regions[index].x2, regions[index].y1);
  glVertex2d(regions[index].x2, regions[index].y2);
  glVertex2d(regions[index].x1, regions[index].y1);
  glVertex2d(regions[index].x2, regions[index].y1);
  glVertex2d(regions[index].x1, regions[index].y2);
  glVertex2d(regions[index].x2, regions[index].y2);
  glEnd();

  if(label) {
	float y = (float)(regions[index].y2 - regions[index].y1) / 2.0 + regions[index].y1 + 5;
	scourge->getSDLHandler()->texPrint(regions[index].x1 + 10, y, 
									   "%s", label);
  }
}

void Gui::moveActiveRegion(int x1, int y1, 
                           int x2, int y2, 
                           int id) {
    int index = -1;
    for(int i = 0; i < regionCount; i++) {
        if(regions[i].id == id) {
            index = i;
            break;
        }
    }
    if(index == -1) return;
    regions[index].x1 = x1;
    regions[index].y1 = y1;
    regions[index].x2 = x2;
    regions[index].y2 = y2;
}

void Gui::removeActiveRegion(int id) {
    int index = -1;
    for(int i = 0; i < regionCount; i++) {
        if(regions[i].id == id) {
            index = i;
            break;
        }
    }
    if(index == -1) return;
    for(int i = index; i < regionCount - 1; i++) {
        memcpy(&(regions[i]), &(regions[i + 1]), sizeof(ActiveRegion));
    }
    regionCount--;
}

int Gui::testActiveRegions(int mousex, int mousey) {
    for(int i = 0; i < regionCount; i++) {
        if(regions[i].x1 <= mousex && regions[i].x2 > mousex &&
           regions[i].y1 <= mousey && regions[i].y2 > mousey) return regions[i].id;   
    }
    return -1;
}

void Gui::debugActiveRegions() {    
    glPushMatrix();
    glLoadIdentity();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    for(int i = 0; i < regionCount; i++) {
        glBegin (GL_QUADS);
        glColor4f(1, 0, 0, 0.5f);
        glVertex3i (regions[i].x1, regions[i].y1, 10);
        glColor4f(0, 1, 0, 0.5f);
        glVertex3i (regions[i].x1, regions[i].y2, 10);      
        glColor4f(0, 0, 1, 0.5f);
        glVertex3i (regions[i].x2, regions[i].y2, 10);
        glColor4f(1, 1, 0, 0.5f);
        glVertex3i (regions[i].x2, regions[i].y1, 10);
        glEnd ();            
    }
    glDisable(GL_BLEND);

    glPopMatrix();
}

void Gui::showCurrentRegion(int id) {
    int currentRegion = -1;
    for(int i = 0; i < regionCount; i++) {
        if(regions[i].id == id) currentRegion = i;
    }
    if(currentRegion >= 0 && currentRegion < regionCount) {
        //fprintf(stderr, ">>> currentRegion=%d\n", currentRegion);   
        glPushMatrix();
        glLoadIdentity();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_BLEND);
        glBegin (GL_QUADS);
        glColor4f(1, 0, 0, alpha);
        glVertex3i (regions[currentRegion].x1, regions[currentRegion].y1, 200);
        glColor4f(0, 1, 0, alpha);
        glVertex3i (regions[currentRegion].x1, regions[currentRegion].y2, 200);      
        glColor4f(0, 0, 1, alpha);
        glVertex3i (regions[currentRegion].x2, regions[currentRegion].y2, 200);
        glColor4f(1, 1, 0, alpha);
        glVertex3i (regions[currentRegion].x2, regions[currentRegion].y1, 200);
        glEnd ();                    
        glDisable(GL_BLEND);
        glPopMatrix();

        GLint t = SDL_GetTicks();
        if(lastTick == 0 || t - lastTick > 50) {
            lastTick = t;
            alpha += alphaInc;
            if(alpha >= 0.7f || alpha < 0.4f) alphaInc *= -1.0f;
        }


        glPushMatrix();
        glLoadIdentity();
        glBegin(GL_LINES);
        glVertex3i (regions[currentRegion].x1, regions[currentRegion].y1, 100);
        glVertex3i (regions[currentRegion].x1, regions[currentRegion].y2, 100);

        glVertex3i (regions[currentRegion].x1, regions[currentRegion].y2, 100);
        glVertex3i (regions[currentRegion].x2, regions[currentRegion].y2, 100);

        glVertex3i (regions[currentRegion].x2, regions[currentRegion].y1, 100);
        glVertex3i (regions[currentRegion].x2, regions[currentRegion].y2, 100);

        glVertex3i (regions[currentRegion].x1, regions[currentRegion].y1, 100);
        glVertex3i (regions[currentRegion].x2, regions[currentRegion].y1, 100);

        glEnd();
        glPopMatrix();        
    }
}

int Gui::addScrollingList(int x1, int y1, int x2, int y2, int activeRegion) {
    scrollingList[scrollingListCount].x1 = x1;
    scrollingList[scrollingListCount].x2 = x2;
    scrollingList[scrollingListCount].y1 = y1;
    scrollingList[scrollingListCount].y2 = y2;
    scrollingList[scrollingListCount].pos = 0;
    scrollingList[scrollingListCount].id = activeRegion;
    addActiveRegion(0, 0, 0, 0, activeRegion, this);
    scrollingList[scrollingListCount].height = -1;
    scrollingList[scrollingListCount].lineSelected = -1;
    scrollingListCount++;
    return activeRegion;
}

void Gui::removeAllScrollingLists() {
  for(int i = 0; i < scrollingListCount; i++) {
	removeActiveRegion(scrollingList[i].id);
  }
  scrollingListCount = 0;
}

void Gui::drawScrollingList(int id, int count, const char *list[]) {
    int index = -1;
    for(int i = 0; i < scrollingListCount; i++) {
        if(scrollingList[i].id == id) index = i;
    }    
    if(index == -1) return;


    int h = scrollingList[index].y2 - scrollingList[index].y1;
    int innerHeight = count * 15;

    int height = (int)((float)h / ((float)innerHeight / (float)h));
	if(h >= innerHeight) height = h;
    if(height < 40) height = 40;
    if(height != scrollingList[index].height) {
        scrollingList[index].height = height;
        int offset = (int)(scrollingList[index].y1 + 
                           ((float)(h) / 100.0) * (float)scrollingList[index].pos);
        moveActiveRegion(scrollingList[index].x1, 
                         offset, 
                         scrollingList[index].x1 + 20, 
                         offset + scrollingList[index].height, 
                         id);
    }

    glPushMatrix();
    
    glScissor(scrollingList[index].x1, 
              scourge->getSDLHandler()->getScreen()->h - scrollingList[index].y2,
              scrollingList[index].x2 - scrollingList[index].x1, 
              scrollingList[index].y2 - scrollingList[index].y1);
    glEnable(GL_SCISSOR_TEST);
    float offset = scrollingList[index].y1 - 
        (((count + 1) * 15.0) / 100.0) * (float)scrollingList[index].pos;
    scrollingList[index].top = (int)offset;

    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
    glLoadIdentity();
    glTranslatef(scrollingList[index].x1, offset, 0);
    for(int i = 0; i < count; i++) {
        scourge->getSDLHandler()->texPrint(25, (i + 1) * 15, "%s", list[i]);
    }

    if(scrollingList[index].lineSelected > -1) {            
        glLoadIdentity();
        glTranslatef(scrollingList[index].x1, offset, 0);
        glColor4f(0.3f, 0.2f, 0.1f, 0.5f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_BLEND);
        int linepos = scrollingList[index].lineSelected * 15 + 4;
        glBegin(GL_QUADS);
        glVertex3i (20, linepos, 10);        
        glVertex3i (scrollingList[index].x2, linepos, 10);
        glVertex3i (scrollingList[index].x2, linepos + 16, 10);
        glVertex3i (20, linepos + 16, 10);
        glEnd();
        glDisable(GL_BLEND);
        glColor4f(1.0f, 0.6f, 0.4f, 1.0f);
    }


    glDisable(GL_SCISSOR_TEST);

    // draw the bounding box
    //glColor4f(1.0f, 1.0f, 0.4f, 1.0f);
    glLoadIdentity();
    glTranslatef(scrollingList[index].x1, scrollingList[index].y1, 0);
    glBegin(GL_LINES);
    glVertex3i (0, 0, 10);
    glVertex3i (0, scrollingList[index].y2 - scrollingList[index].y1, 10);
    glVertex3i (0, scrollingList[index].y2 - scrollingList[index].y1, 10);
    glVertex3i (scrollingList[index].x2 - scrollingList[index].x1, scrollingList[index].y2 - scrollingList[index].y1, 10);
    glVertex3i (scrollingList[index].x2 - scrollingList[index].x1, 0, 10);
    glVertex3i (scrollingList[index].x2 - scrollingList[index].x1, scrollingList[index].y2 - scrollingList[index].y1, 10);
    glVertex3i (0, 0, 10);
    glVertex3i (scrollingList[index].x2 - scrollingList[index].x1, 0, 10);
    glVertex3i (20, 0, 10);
    glVertex3i (20, scrollingList[index].y2 - scrollingList[index].y1, 10);
    glEnd();

    // draw the scroller
    offset = scrollingList[index].y1 + 
        ((float)(scrollingList[index].y2 - scrollingList[index].y1) / 100.0) * (float)scrollingList[index].pos;
    glLoadIdentity();
    glTranslatef(scrollingList[index].x1, offset, 0);

    glColor4f(0.3f, 0.2f, 0.1f, 0.5f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);
    glBegin(GL_QUADS);
    glVertex3i (0, 0, 10);        
    glVertex3i (20, 0, 10);
    glVertex3i (20, scrollingList[index].height, 10);
    glVertex3i (0, scrollingList[index].height, 10);
    glEnd();
    glDisable(GL_BLEND);
    glColor4f(1.0f, 0.6f, 0.4f, 1.0f);

    glBegin(GL_LINES);
    glVertex3i (0, 0, 10);
    glVertex3i (20, 0, 10);
    glVertex3i (0, scrollingList[index].height, 10);
    glVertex3i (20, scrollingList[index].height, 10);
    glEnd();

    glPopMatrix();    
}

void Gui::moveScrollingList(int id) {
    int index = -1;
    for(int i = 0; i < scrollingListCount; i++) {
        if(scrollingList[i].id == id) index = i;
    }    
    if(index == -1) return;

    int index2 = -1;
    for(int i = 0; i < regionCount; i++) {
        if(regions[i].id == id) index2 = i;
    }    
    if(index2 == -1) return;

    int h = scrollingList[index].y2 - scrollingList[index].y1;
    int pos = (int)((float)((scourge->getSDLHandler()->mouseY - 
                             scrollingList[index].activeOffset) - 
                            scrollingList[index].y1) 
              / ((float)h / 100.0));
    if(pos < 0) pos = 0;
    int offset = (int)(scrollingList[index].y1 + 
        ((float)h / 100.0) * (float)pos);
     if(offset + scrollingList[index].height > scrollingList[index].y2) {
         offset = scrollingList[index].y2 - scrollingList[index].height;
         pos = (int)((float)(offset - scrollingList[index].y1) / ((float)h / 100.0));
     }

     scrollingList[index].pos = pos;
     moveActiveRegion(scrollingList[index].x1, 
                      offset, 
                      scrollingList[index].x1 + 20, 
                      offset + scrollingList[index].height, id);
}

void Gui::initScroller(int id) {
    int index = -1;
    for(int i = 0; i < scrollingListCount; i++) {
        if(scrollingList[i].id == id) index = i;
    }    
    if(index == -1) return;

    // if it's a scroller store it
    currentScroller = id;

    int index2 = -1;
    for(int i = 0; i < regionCount; i++) {
        if(regions[i].id == id) index2 = i;
    }    
    if(index2 == -1) return;
    scrollingList[index].activeOffset = 
        scourge->getSDLHandler()->mouseY - 
        regions[index2].y1;
}

bool Gui::handleEvent(SDL_Event *event) {
  // handle scrolling list callbacks
  fprintf(stderr, "THIS IS NEVER CALLED, RIGHT? In handleEvent!\n");
  return false;
}

void Gui::selectScrollingItem(int mousex, int mousey) {
    int index = -1;
    for(int i = 0; i < scrollingListCount; i++) {
        if(scrollingList[i].x1 + 20 <= mousex && scrollingList[i].x2 > mousex &&
           scrollingList[i].y1 <= mousey && scrollingList[i].y2 > mousey) {
            index = i;
            break;
        }
    }
    if(index == -1) return;
    scrollingList[index].lineSelected = 
        (int)((float)(mousey - scrollingList[index].top) / 15.0);
}

int Gui::getLineSelected(int id) {
    int index = -1;
    for(int i = 0; i < scrollingListCount; i++) {
        if(scrollingList[i].id == id) index = i;
    }    
    if(index == -1) return -1;
	return scrollingList[index].lineSelected;
}
