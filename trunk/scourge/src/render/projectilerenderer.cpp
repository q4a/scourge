/***************************************************************************
                          projectilerenderer.cpp  -  description
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

#include "projectilerenderer.h"

EffectProjectileRenderer::EffectProjectileRenderer( Effect *effect, int effectType, int timeToLive ) {
  this->effect = effect;
  this->effectType = effectType;
  this->timeToLive = timeToLive;
}

EffectProjectileRenderer::~EffectProjectileRenderer() {
  delete effect;
}

void EffectProjectileRenderer::draw() {
  effect->draw( effectType, 0 );
}

void EffectProjectileRenderer::setCameraRot( float x, float y, float z ) {
}

bool EffectProjectileRenderer::drawLater() {
  return true;
}

void EffectProjectileRenderer::setupBlending() {
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE );
}

void EffectProjectileRenderer::endBlending() {
  glDisable( GL_BLEND );
}

