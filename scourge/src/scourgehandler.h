/***************************************************************************
                          scourgehandler.h  -  description
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

#ifndef SCOURGE_HANDLER_H
#define SCOURGE_HANDLER_H

#include "common/constants.h"                            
#include "sdleventhandler.h"

class Scourge;

class ScourgeHandler : public SDLEventHandler {
private:
  Scourge *scourge;
  bool willStartDrag;
  int willStartDragX, willStartDragY;  


public:
  ScourgeHandler( Scourge *scourge );
  virtual ~ScourgeHandler();

  /**
    Respond to keyboard and mouse events in this method.
    @param event the actual SDL_Event structure as captured by the main app loop.
    @return true to exit from the current screen, false otherwise
  */
  bool handleEvent(SDL_Event *event);
  
  /**
    Respond to UI (windowing) events in this method.
    @param widget The widget which fired the event (e.g.: button, list, etc.)
    @param event the actual SDL_Event structure as captured by the main app loop.
    @return true to exit from the current screen, false otherwise
  */
  bool handleEvent(Widget *widget, SDL_Event *event);

protected:
  void processGameMouseClick( Uint16 x, Uint16 y, Uint8 button );
  bool handleCreatureClick( Uint16 mapx, Uint16 mapy, Uint16 mapz );
  bool handlePartyEvent( Widget *widget, SDL_Event *event );
  void quickSpellAction( int index, int button=SDL_BUTTON_LEFT );
  int handleBoardEvent(Widget *widget, SDL_Event *event);

};

#endif


