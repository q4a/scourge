/***************************************************************************
                          renderedcreature.h  -  description
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
#include "renderedcreature.h"
#include "effect.h"
#include "map.h"

RenderedCreature::RenderedCreature( Preferences *preferences, 
                                    Shapes *shapes, 
                                    Map *levelMap ) {
  this->preferences = preferences;
  this->shapes = shapes;
  this->levelMap = levelMap;
  x = y = z = 0; 
  damageEffectCounter = 0;
  effectDuration = Constants::DAMAGE_DURATION;
  effect = NULL;
  effectType = Constants::EFFECT_FLAMES;
}

RenderedCreature::~RenderedCreature() {
  if( effect ) delete effect;
}

void RenderedCreature::startEffect( int effect_type, int duration, GLuint delay ) {
  // show an effect
  if( isEffectOn() && effect_type == getEffectType() ) {
    return;
  }  
  getEffect()->deleteParticles();
  resetDamageEffect();
  setEffectType( effect_type );
  effectDuration = duration;

  // need to do this to make sure effect shows up
  levelMap->refresh();
}

Effect *RenderedCreature::getEffect() {
  if( !effect ) {
    effect = 
      new Effect( preferences, shapes, getShape() );
  }
  return effect;
}

