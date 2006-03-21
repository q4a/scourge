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

#ifndef GUI_GUI_H
#define GUI_GUI_H

#include "../constants.h"

class Widget;

/**
 * This is the only class thru which the gui interacts with the rest of scourge.
 * It is important to keep this in mind so that:
 * -the gui classes can be used by other apps
 * -compilation is sped up (by not including things from outside of this dir.
 */
class ScourgeGui {
public:
  ScourgeGui() {
  }

  virtual ~ScourgeGui() {
  }

  virtual void drawScreen() = 0;
  virtual void playSound( const char *file ) = 0;
  virtual void texPrint(GLfloat x, GLfloat y, const char *fmt, ...) = 0;
  virtual int textWidth( const char *fmt, ... ) = 0;
  virtual int getScreenWidth() = 0;
  virtual int getScreenHeight() = 0;
  virtual void setCursorMode( int n, bool useTimer=false ) = 0;
  virtual int getCursorMode() = 0;
  virtual GLuint getHighlightTexture() = 0;
  virtual Uint16 getMouseX() = 0;
  virtual Uint16 getMouseY() = 0;
  virtual void drawTooltip( float xpos2, float ypos2, float zpos2, 
                            float zrot, float yrot, 
                            char *message,
                            float r=0, float g=0.15f, float b=0.05f ) = 0;
  virtual void setFontType( int fontType ) = 0;
  virtual GLuint loadSystemTexture( char *line ) = 0;
  virtual void unlockMouse() = 0;
  virtual void lockMouse( Widget *widget ) = 0;
  virtual void allWindowsClosed() = 0;
  virtual void blockEvent() = 0;
};                                    

#endif
