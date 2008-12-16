/***************************************************************************
                    skillsview.cpp  -  Skill viewer widget
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
#include "common/constants.h"
#include "skillsview.h"
#include "gui/window.h"
#include "gui/scrollinglist.h"
#include "shapepalette.h"
#include "creature.h"
#include "party.h"

/**
  *@author Gabor Torok
  */

SkillsView::SkillsView( Scourge *scourge, int x, int y, int w, int h ) {
	this->scourge = scourge;
	this->creature = NULL;

	skillList = new ScrollingList( x, y, w, h,
	                               scourge->getShapePalette()->getHighlightTexture() );
}

SkillsView::~SkillsView() {
	delete skillList;
}

void SkillsView::setCreature( Creature *creature, CreatureGroupInfo *info ) {
	this->creature = creature;
	int lineCounter = 0;
	for ( int t = 0; t < Skill::SKILL_COUNT; t++ ) {
		if ( creature->getSkill( t ) == 0 ) continue;

		// check the filter
		SkillGroup *group = Skill::skills[t]->getGroup();
		if ( !filter.empty() &&
		        filter.find( group ) == filter.end() ) continue;

		if ( Skill::skills[ t ]->getGroup()->isStat() ) {
			skillColor[ lineCounter ].r = 0.7f;
			skillColor[ lineCounter ].g = 1;
			skillColor[ lineCounter ].b = 0.7f;
		} else {
			skillColor[ lineCounter ].r = 1;
			skillColor[ lineCounter ].g = 1;
			skillColor[ lineCounter ].b = 1;
		}

		// FIXME: this should be replaced by percentage bars
		//int maxSkill = selectedP->getCharacter()->getSkill( t );
		//bool maxFound = ( maxSkill >= 0 );
		bool maxFound = ( info && creature == info->getHighestSkillPC( t ) );
		char str[20];
		snprintf( str, 20, "%d", creature->getSkill( t, false ) );
		skillLine[ lineCounter ] = str;
		if ( creature->getSkillMod( t ) > 0 ) {
			snprintf( str, 20, "(%d)", creature->getSkillMod( t ) );
			skillLine[ lineCounter ] += str;
		}
		skillLine[ lineCounter ] += " - ";
		skillLine[ lineCounter ] += Skill::skills[ t ]->getDisplayName();

		if ( creature->getSkillMod( t ) > 0 ) {
			skillColor[ lineCounter ].r = 0;
			skillColor[ lineCounter ].g = 1;
			skillColor[ lineCounter ].b = 0;
		} else if ( maxFound ) {
			skillColor[ lineCounter ].r = 0;
			skillColor[ lineCounter ].g = 0.75f;
			skillColor[ lineCounter ].b = 1;
		}
		lineCounter++;
	}

	int line = skillList->getSelectedLine();
	if ( line < 0 ) line = 0;
	skillList->setLines( lineCounter,
	                     skillLine,
	                     ( const Color* )skillColor );
	skillList->setSelectedLine( line );
}

int SkillsView::getSelectedLine() {
	return skillList->getSelectedLine();
}

void SkillsView::setSelectedLine( int n ) {
	skillList->setSelectedLine( n );
}

