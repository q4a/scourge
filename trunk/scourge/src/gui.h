/***************************************************************************
                          gui.h  -  description
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

#ifndef GUI_H
#define GUI_H

#include "constants.h"
#include "sdlhandler.h"
#include "scourge.h"

/**
  *@author Gabor Torok
  */

class Scourge;
  
class Gui : public SDLEventHandler {
private:
  typedef struct _GUIWindow {
    int x, y, w, h;
    void (Gui::*callbackFunc)(int, int);    
    bool visible;
    int id;
  } GUIWindow;

  typedef struct _ActiveRegion {
      int x1, y1, x2, y2;
      SDLEventHandler *eventHandler;
      int id;
  } ActiveRegion;

  typedef struct _ScrollingList {
	int x1, y1, x2, y2;
	int id;
	int pos; // scroller pos in % (50 is in the middle)
	int height; // height of the scroll bar
	int activeOffset;
	int top; // the top of the inner content
	int lineSelected;
  } ScrollingList;

  static const int MAX_WINDOW_COUNT = 100;  
  static const int MAX_REGION_COUNT = 100;  
  static const int MAX_SCROLLING_COUNT = 100;

  typedef struct _WindowStack {
      GUIWindow windows[MAX_WINDOW_COUNT];
      int windowCount;
      ActiveRegion regions[MAX_REGION_COUNT];
      int regionCount;
  } WindowStack;

  Scourge *scourge;
  
  int windowCount;
  GUIWindow windows[MAX_WINDOW_COUNT];
  
  int regionCount;
  ActiveRegion regions[MAX_REGION_COUNT];  

  int scrollingListCount;
  ScrollingList scrollingList[MAX_SCROLLING_COUNT];

  static const int MAX_STACK_COUNT = 10;
  int windowStackCount;
  WindowStack windowStack[MAX_STACK_COUNT];

  int currentScroller;

  // the image is 510x270
  static const int TILE_W=510 / 2;
  static const int TILE_H=270 / 2; 

  float alpha, alphaInc;
  GLint lastTick;

public: 
	Gui(Scourge *scourge);
	~Gui();

  /**
    Return created window's id, or -1 on error.
   */
  int addWindow(int x, int y, int w, int h,
               void (Gui::*callbackFunc)(int, int));
  void removeWindow(int id);
  void removeAllWindows();

  void drawWindows();

  void setWindowVisible(int id, bool b);
  bool isWindowVisible(int id);

  
  void drawMainMenu(int x, int y);

  void drawOptionsMenu(int x, int y);

  void drawDescriptions(int x, int y);

  void drawInventory(int x, int y);

  void pushWindows();
  void popWindows();

  /**
   * Add a region which, when clicked will make a callback to its event handler.
   */
  void addActiveRegion(int x1, int y1, int x2, int y2, int id, SDLEventHandler *eventHandler);

  /**
   * Draw the button w. optional label
   */
  void outlineActiveRegion(int id, const char *label = NULL);

  void removeActiveRegion(int id);

  int testActiveRegions(int mousex, int mousey);

  void debugActiveRegions();

  void showCurrentRegion(int id);

  int addScrollingList(int x, int y, int w, int h, int activeRegion);

  void removeAllScrollingLists();

  int getLineSelected(int id);

  void drawScrollingList(int id, int count, const char *list[]);

  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);

  void debug();

protected:
  void drawGui(GUIWindow win);  
  void moveScrollingList(int id);
  void moveActiveRegion(int x1, int y1, int x2, int y2, int id);
  void initScroller(int id);
  void selectScrollingItem(int mousex, int mousey);
};

#endif

