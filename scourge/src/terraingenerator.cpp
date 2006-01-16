/***************************************************************************
                          terraingenerator.cpp  -  description
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
#include "terraingenerator.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "scourge.h"
#include "shapepalette.h"
#include "board.h"
#include "gui/progress.h"
#include "item.h"
#include "creature.h"

using namespace std;

TerrainGenerator::TerrainGenerator( Scourge *scourge, 
                                    int level, 
                                    int depth, 
                                    bool stairsDown, 
                                    bool stairsUp, 
                                    Mission *mission, 
                                    int progressSteps ) {
  this->scourge = scourge;
  this->level = level;
  this->depth = depth;
  this->stairsUp = stairsUp;
  this->stairsDown = stairsDown;
  this->mission = mission;

  progress = new Progress(scourge->getSDLHandler(), 
                          scourge->getSession()->getShapePalette()->getProgressTexture(),
                          scourge->getSession()->getShapePalette()->getProgressHighlightTexture(),
                          progressSteps, false, true );

}

TerrainGenerator::~TerrainGenerator() {
  delete progress;
}

void TerrainGenerator::updateStatus(const char *statusMessage) {
  progress->updateStatus(statusMessage);
  Uint32 now = SDL_GetTicks();
//  cerr << "+++ " << statusMessage << ". Previous task's time=" << (now - start) << endl;
  start = now;
}

void TerrainGenerator::toMap( Map *map, ShapePalette *shapePal ) {	 
  start = SDL_GetTicks();
  generate( map, shapePal );
}

