/***************************************************************************
                          render.h  -  description
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

#ifndef RENDER_H
#define RENDER_H

// definitely outside of dir                    
#include "../constants.h"
#include "../session.h"
#include "../shapepalette.h"
#include "../persist.h"
#include "../util.h"
#include "../battle.h"

#include "../rpg/rpglib.h"

#include "../events/potionexpirationevent.h"

// forward decl.
class Session;
class Spell;
class MagicSchool;
class Dice;
class Monster;
class Character;
class Shape;
class Item;
class Creature;

/** 
 *@author Gabor Torok
 */
 
 
/**
 * The class thru which this directory talks to the rest of the code.
 * Never include this file outside this dir. (use renderlib.h instead.)
 */
class Render {
  public:
};

#endif
