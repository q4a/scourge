/***************************************************************************
               portrait.h  -  The portrait + stats/skills widget
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
#pragma once

#include <iostream>
#include <vector>
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

/// A rectangle that can tell you whether it contains a specific x,y point.
class ActionRect {
public:
	int x, y, x2, y2;
	ActionRect( int x, int y, int x2, int y2 ) {
		this->x = x;
		this->y = y;
		this->x2 = x2;
		this->y2 = y2;
	}
	~ActionRect() {}
	inline bool containsPoint( int px, int py ) {
		return( x <= px && x2 > px && y <= py && y2 > py );
	}
};

/// Widget that displays a char's portrait, stats and equipped weapon.
class Portrait : public WidgetView {
private:
	PcUi *pcUi;
	Creature *creature;
	Texture backgroundTexture;
	Texture barTexture;
	Canvas *canvas;
	int x, y, w, h;
	int mode;
	int skillOffset;
	Skill *currentSkill;
	int currentMode;
	std::map<std::string, ActionRect*> boxes;

public:

	enum {
		STATS_MODE = 0,
		SKILLS_MODE,
		STATE_MODS
	};

	Portrait( PcUi *pcUi, int x, int y, int w, int h );
	~Portrait();

	inline void setMode( int n ) {
		mode = n;
	}
	inline int getMode() {
		return mode;
	}
	void scrollSkillsUp();
	void scrollSkillsDown();
	inline Widget *getWidget() {
		return canvas;
	}
	bool handleEvent( SDL_Event *event );
	bool handleEvent( Widget *widget, SDL_Event *event );
	void setCreature( Creature *creature );

	// WidgetView interface
	virtual void drawWidgetContents( Canvas *w );
	bool onDraw( Widget* widget ); 

protected:
	void drawBar( int x, int y, int value, int maxValue = 100, int r = 0, int g = 1, int b = 0, int a = 1, int mod = 0 );
	void drawHorizontalLine( int y );
	void showStats();
	void showSkills();
	void showStateMods();
	void drawSkill( Skill *skill, int yy );
	bool findCurrentSkill( int px, int py );
	void drawStateModIcon( Texture icon, char *name, Color color, int x, int y, int size );
	void drawResistance( int x, int y, char *icon, int skill );
	void setCurrentWeaponTooltip();
};

#endif

