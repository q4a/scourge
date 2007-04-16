/***************************************************************************
                          scourgeview.h  -  description
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

#ifndef SCOURGE_VIEW_H
#define SCOURGE_VIEW_H

#include "common/constants.h"
#include "sdlscreenview.h"
#include <map>

class Scourge;
class Location;
class TextEffect;
class ScrollingList;
class Creature;
class Item;
class Progress;

class InfoMessage {
public:
  char message[300];
  void *obj;
  int x, y, z;

  InfoMessage( char *s, void *obj, int x, int y, int z ) {
    strcpy( this->message, s );
    this->obj = obj;
    this->x = x;
    this->y = y;
    this->z = z;
  }

  ~InfoMessage() {
  }
};

class ScourgeView : public SDLScreenView {
private:
  Scourge *scourge;
  bool needToCheckInfo;
  std::map<InfoMessage *, Uint32> infos;
  Color *outlineColor;
  TextEffect *textEffect;
  GLint textEffectTimer;
  bool needToCheckDropLocation;
  float targetWidth, targetWidthDelta;
  Uint32 lastTargetTick;
  GLUquadric *quadric;
  Progress *turnProgress;

public:
  ScourgeView( Scourge *scourge );
  virtual ~ScourgeView();

  /**
    The main app loop calls this method to repaint the screen. In this implementation the 
    following happens: the round is played, the map is drawn, the map overlay (circles around
    the good guys, names of creatues, etc.) is painted and some extra updates to other components
    (like the minimap, message ui, etc.) occur.    
  */
  virtual void drawView();

  /**
    The main app loop calls this after the drawView and the UI (windows) have been drawn.
    In this implementation, the dragged item is drawn over the map.
  */
  virtual void drawAfter();

  Color *getOutlineColor( Location *pos );
  bool startTextEffect( char *message );
  void resetInfos();
  void initUI();

protected:
  void centerOnMonsterInTB();
  void drawTextEffect();
  void drawOutsideMap();
  void checkForInfo();
  void drawMapInfos();
  void drawCreatureInfos();
  void drawInfos();
  void checkForDropTarget();
  void drawBorder();
  void showCreatureInfo( Creature *creature, bool player, bool selected, bool groupMode, bool wanderingHero );
  void drawDraggedItem();
	void drawDisk( float w, float diff );
};

#endif


