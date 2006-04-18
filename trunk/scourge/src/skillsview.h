/***************************************************************************
                          skillsview.h  -  description
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

#ifndef SKILLS_VIEW_H
#define SKILLS_VIEW_H

#include <iostream>
#include <string>
#include <vector>
#include "constants.h"
#include "session.h"

/**
  *@author Gabor Torok
  */

class Session;
class Creature;
class Window;
class ScrollingList;
class Widget;
class CreatureGroupInfo;

class SkillsView {
private:
  Scourge *scourge;
  Creature *creature;
  Window *win;
  ScrollingList *skillList;
  char **skillLine;
  Color *skillColor;
  int filter;

public:
  SkillsView( Scourge *scourge, int x, int y, int w, int h );
  ~SkillsView();

  void setCreature( Creature *creature, CreatureGroupInfo *info=NULL );
  inline Creature *getCreature() { return creature; }

  int getSelectedLine();
  void setSelectedLine( int n );

  inline void clearSkillGroupFilters() { filter = 0; }
  inline void addSkillGroupFilter( int group ) { filter += ( 1 << group ); }

  inline Widget *getWidget() { return skillList; }

};

#endif

