/***************************************************************************
                          terraingenerator.h  -  description
                             -------------------
    begin                : Thu Jan 15 2006
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
#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <map>
#include "constants.h"

class Scourge;
class Map;
class ShapePalette;
class Mission;
class Progress;

class TerrainGenerator {
protected:
  Scourge *scourge;
  int level;
  int depth;
  bool stairsDown, stairsUp;
  Mission *mission;
  Progress *progress;
  Uint32 start;

public:
  TerrainGenerator( Scourge *scourge, 
                    int level, 
                    int depth, 
                    bool stairsDown, 
                    bool stairsUp, 
                    Mission *mission, 
                    int progressSteps );
  virtual ~TerrainGenerator();

  void toMap( Map *map, ShapePalette *shapePal );

  void updateStatus(const char *statusMessage);

  virtual void generate( Map *map, ShapePalette *shapePal ) = 0;
};

#endif

