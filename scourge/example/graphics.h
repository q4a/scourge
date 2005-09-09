/***************************************************************************
                          main.cpp  -  description
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
#ifndef GRAPHICS_H
#define GRAPHICS_H  
 
#include <constants.h>
#include <preferences.h>
#include <render/renderlib.h>
#include "game.h"

class Graphics : public MapAdapter {
private:
  bool stencilBufferUsed; 
  int lastWidth, lastHeight;
  SDL_Surface *screen;
  GLint T0, Frames;
  double fps;
  int videoFlags;
  Uint16 mouseX, mouseY;
  bool mouseMovingOverMap;

public:
  Graphics();
  ~Graphics();

  void mainLoop( Game *game );
  void setVideoMode( Preferences * uc );

  // MapAdapter implementation
  virtual inline int getScreenWidth() { return lastWidth; }
  virtual inline int getScreenHeight() { return lastHeight; }
  virtual inline Uint16 getMouseX() { return mouseX; }
  virtual inline Uint16 getMouseY() { return mouseY; }
  virtual inline bool isMouseIsMovingOverMap() { return mouseMovingOverMap; }
  virtual inline RenderedCreature *getPlayer() { return NULL; }
  virtual inline void setDebugStr(char *s) { 
    //cerr << s << endl; 
  }
  virtual inline bool isMissionCreature( RenderedCreature *creature ) { return false; }
  virtual inline void colorMiniMapPoint(int x, int y, Shape *shape, Location *pos=NULL) {}
  virtual inline void eraseMiniMapPoint(int x, int y) {}
  virtual inline bool hasParty() { return false; }
  virtual inline int getPartySize() { return 0; }
  virtual inline RenderedCreature *getParty( int index ) { return NULL; }
  virtual inline RenderedItem *load( ItemInfo *info ) { return NULL; }
  virtual inline RenderedCreature *load( CreatureInfo *info ) { return NULL; }
  virtual inline void loadMapData( const char *name ) { }
  virtual inline void saveMapData( const char *name ) { }
  virtual inline Color *getOutlineColor( Location *pos ) { return NULL; }
  virtual inline void setView() { setPerspective(); }

private:
  void quit( int returnCode );
  void setPerspective();  
  void resizeWindow( int width, int height );
  void initGL( GLvoid );
  bool testModesInFormat( SDL_PixelFormat *format, Uint32 flags );
  int testModes( Uint32 flags, bool findMaxBpp );
  void drawScreen( Game *game );
};

#endif
