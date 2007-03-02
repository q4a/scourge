/***************************************************************************
                          specialskill.h  -  description
                             -------------------
    begin                : Sat Oct 19 2005
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

#ifndef SPECIAL_SKILL_H
#define SPECIAL_SKILL_H

#include <iostream>
#include <string>
#include <vector>
#include "../common/constants.h"
#include "../storable.h"

/**
  *@author Gabor Torok
  */

class SpecialSkill : public Storable {
private:
  const char *name;
  const char *description;
  int type, event;
  const char *squirrelFuncPrereq;
  const char *squirrelFuncAction;
  int iconTileX, iconTileY;

public:
  enum {
    SKILL_TYPE_AUTOMATIC,
    SKILL_TYPE_MANUAL,
		SKILL_TYPE_RECURRING,
    SKILL_TYPE_COUNT
  };

  enum {
    SKILL_EVENT_ARMOR,
    SKILL_EVENT_DAMAGE,
    SKILL_EVENT_COUNT
  };

  static std::vector<SpecialSkill*> skills;
  static std::map<std::string,SpecialSkill*> skillsByName;

  inline static SpecialSkill *findByName( const char *name, bool showError=true ) {
    std::string s = name;
    SpecialSkill *ss = ( skillsByName.find( s ) == skillsByName.end() ? NULL : skillsByName[ s ] );
    if( !ss && showError ) std::cerr << "*** Error can't find special skill: " << name << std::endl;
    return ss;
  }

  inline static int getSpecialSkillCount() { return skills.size(); }
  inline static SpecialSkill *getSpecialSkill( int index ) { return skills[ index ]; }

  static void initSkills();

  SpecialSkill( const char *name, 
                const char *description, 
                int type,
                int event,
                const char *squirrelFuncPrereq,
                const char *squirrelFuncAction,
                int iconTileX,
                int iconTileY );
  virtual ~SpecialSkill();

  inline int getStorableType() { return Storable::SPECIAL_STORABLE; }
  inline const char *isStorable() { 
    return( getType() == SKILL_TYPE_MANUAL ? 
            NULL :
            _( "Only 'manual' capabilities can be stored." ) ); 
  }

  inline const char *getName() { return name; }
  inline const char *getDescription() { return description; }
  inline const char *getSquirrelFunctionPrereq() { return squirrelFuncPrereq; }
  inline const char *getSquirrelFunctionAction() { return squirrelFuncAction; }
  inline int getType() { return type; }
  inline int getEvent() { return event; }
  inline int getIconTileX() { return iconTileX; }
  inline int getIconTileY() { return iconTileY; }

  // FIXME: read these from skills.txt
  inline int getSpeed() { return 3; }
  inline int getDistance() { return 10; }
};

#endif
