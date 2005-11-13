/***************************************************************************
                          rpglib.h  -  description
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

#ifndef RPG_LIB_H
#define RPG_LIB_H

/**
 * A way for external classes to this dir to get everything in one include file.
 */

// The max value of a skill under normal circumstances. 
#define MAX_SKILL 20

// the max penalty for using items w/o enough skill
#define PROFICIENCY_MAX_PENALTY 4.0f
            
#include "character.h"
#include "monster.h"
#include "rpgitem.h"
#include "spell.h"
#include "specialskill.h"

#endif
