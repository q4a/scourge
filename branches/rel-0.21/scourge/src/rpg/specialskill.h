/***************************************************************************
                   specialskill.h  -  Special skill class
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
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "../storable.h"

/**
  *@author Gabor Torok
  */

/// A special ability (the type you can put into quickspell slots).
class SpecialSkill : public Storable {
private:
	std::string name;
	std::string displayName;
	std::string description;
	int type, event;
	std::string squirrelFuncPrereq;
	std::string squirrelFuncAction;
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
	static std::map<std::string, SpecialSkill*> skillsByName;

	inline static SpecialSkill *findByName( const char *name, bool showError = true ) {
		std::string s = name;
		SpecialSkill *ss = ( skillsByName.find( s ) == skillsByName.end() ? NULL : skillsByName[ s ] );
		if ( !ss && showError ) std::cerr << "*** Error can't find special skill: " << name << std::endl;
		return ss;
	}

	inline static int getSpecialSkillCount() {
		return skills.size();
	}
	inline static SpecialSkill *getSpecialSkill( int index ) {
		return skills[ index ];
	}

	static void initSkills();
	static void unInitSkills();

	SpecialSkill( const char *name,
	              const char *displayName,
	              const char *description,
	              int type,
	              int event,
	              const char *squirrelFuncPrereq,
	              const char *squirrelFuncAction,
	              int iconTileX,
	              int iconTileY );
	virtual ~SpecialSkill();

	inline int getStorableType() {
		return Storable::SPECIAL_STORABLE;
	}
	inline const char *isStorable() {
		return( getType() == SKILL_TYPE_MANUAL ?
		        NULL :
		        _( "Only 'manual' capabilities can be stored." ) );
	}

	inline const char *getName() {
		return name.c_str();
	}
	inline const char *getDisplayName() {
		return displayName.c_str();
	}
	inline const char *getDescription() {
		return description.c_str();
	}
	inline const char *getSquirrelFunctionPrereq() {
		return squirrelFuncPrereq.c_str();
	}
	inline const char *getSquirrelFunctionAction() {
		return squirrelFuncAction.c_str();
	}
	inline int getType() {
		return type;
	}
	inline int getEvent() {
		return event;
	}
	inline int getIconTileX() {
		return iconTileX;
	}
	inline int getIconTileY() {
		return iconTileY;
	}

	// FIXME: read these from skills.txt
	inline int getSpeed() {
		return 3;
	}
	inline int getDistance() {
		return 10;
	}
};

#endif
