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
#include "common/constants.h"
#include "session.h"
#include "rpg/rpg.h"

/**
  *@author Gabor Torok
  */

class Session;
class Creature;
class Window;
class ScrollingList;
class Widget;
class CreatureGroupInfo;
class SkillGroup;

class SkillsView {

public:
	enum { SKILL_SIZE = 120 };
  SkillsView( Scourge *scourge, int x, int y, int w, int h );
  ~SkillsView();

  void setCreature( Creature *creature, CreatureGroupInfo *info=NULL );
  inline Creature *getCreature() { return creature; }

  int getSelectedLine();
  void setSelectedLine( int n );

  inline void clearSkillGroupFilters() { filter.clear(); }
  inline void addSkillGroupFilter( SkillGroup *group ) { filter.insert( group ); }

  inline Widget *getWidget() { return skillList; }

private:
  Scourge *scourge;
  Creature *creature;
  Window *win;
  ScrollingList *skillList;
  std::string skillLine[ Skill::SKILL_COUNT ];
  Color skillColor[ Skill::SKILL_COUNT ];
  std::set<SkillGroup*> filter;
};

#endif

