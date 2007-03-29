/***************************************************************************
                          portrait.h  -  description
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

#ifndef PORTRAIT_H
#define PORTRAIT_H

#include <iostream>
#include <vector>
#include "common/constants.h"
#include "gui/window.h"
#include "gui/button.h"
#include "gui/draganddrop.h"
#include "gui/canvas.h"
#include "gui/widgetview.h"

/**
  *@author Gabor Torok
  */

class Creature;
class Scourge;
class Storable;
class ConfirmDialog;
class Item;
class PcUi;
class Skill;

class Portrait : public WidgetView {
private:
	PcUi *pcUi;
	Creature *creature;
	GLuint backgroundTexture, barTexture;
  Canvas *canvas;
	int x, y, w, h;
	int mode;
	int skillOffset;

public:

	enum {
		STATS_MODE=0,
		SKILLS_MODE
	};

	Portrait( PcUi *pcUi, int x, int y, int w, int h );
	~Portrait();

	inline void setMode( int n ) { mode = n; }
	inline int getMode() { return mode; }
	void scrollSkillsUp();
	void scrollSkillsDown();
  inline Widget *getWidget() { return canvas; }
	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );
	void setCreature( Creature *creature );

	void drawWidgetContents( Widget *w );

protected:
	void drawBar( int x, int y, int value, int maxValue=100, int r=0, int g=1, int b=0, int a=1, int mod=0 );
  void drawHorizontalLine( int y );
	void showStats();
	void showSkills();
	void drawSkill( Skill *skill, int yy );
};

#endif

