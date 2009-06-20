/***************************************************************************
         projectilerenderer.cpp  -  Well, it renders projectiles :-)
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

#include "../common/constants.h"
#include "projectilerenderer.h"
#include "map.h"
#include "renderedprojectile.h"
#include "render.h"

using namespace std;

void ShapeProjectileRenderer::drawPath( Map *map, RenderedProjectile *proj, std::vector<CVector3> *path ) {
	// draw the last step only
	CVector3 last = path->back();

	glsDisable( GLS_CULL_FACE );

	glPushMatrix();

	glTranslatef( last.x, last.y, last.z );

	glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );

	( ( GLShape* )shape )->setCameraPos( map->getXPos(), map->getYPos(), map->getZPos(), last.x, last.y, last.z );

	// orient and draw the projectile
	float f = proj->getAngle() + 90;
	if ( f < 0 ) f += 360;
	if ( f >= 360 ) f -= 360;

	glRotatef( f, 0.0f, 0.0f, 1.0f );

	// for projectiles, set the correct camera angle
	if ( proj->getAngle() < 90 ) {
		( ( GLShape* )shape )->setCameraRot( map->getXRot(), map->getYRot(), map->getZRot() + proj->getAngle() + 90 );
	} else if ( proj->getAngle() < 180 ) {
		( ( GLShape* )shape )->setCameraRot( map->getXRot(), map->getYRot(), map->getZRot() - proj->getAngle() );
	} else {
		( ( GLShape* )shape )->setCameraRot( map->getXRot(), map->getYRot(), map->getZRot() );
	}

	if ( shape->isBlended() ) {
		glsDisable( GLS_DEPTH_MASK );
		glsEnable( GLS_BLEND );

		shape->setupBlending();
	}

	shape->draw();

	if ( shape->isBlended() ) {
		shape->endBlending();

		glsDisable( GLS_BLEND );
		glsEnable( GLS_DEPTH_MASK );
	}

	glPopMatrix();
}

EffectProjectileRenderer::EffectProjectileRenderer( Map *map, Preferences *prefs, Shapes *shapes, int effectType, int timeToLive ) {
	this->effectType = effectType;
	this->timeToLive = timeToLive;
	effects.resize( MAX_EFFECT_COUNT );
	for ( int i = 0; i < MAX_EFFECT_COUNT; i++ ) {
		effects[i] = new Effect( map, prefs, shapes, 2, 2 );
	}
}

EffectProjectileRenderer::~EffectProjectileRenderer() {
	for ( int i = 0; i < MAX_EFFECT_COUNT; i++ ) {
		delete effects[i];
	}
}

void EffectProjectileRenderer::drawPath( Map *map, RenderedProjectile *proj, std::vector<CVector3> *path ) {
	int maxSteps = static_cast<int>( path->size() );
	if ( maxSteps > MAX_EFFECT_COUNT ) maxSteps = MAX_EFFECT_COUNT;

	glsDisable( GLS_CULL_FACE | GLS_DEPTH_MASK );
	glsEnable( GLS_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE );

	for ( int i = 0; i < maxSteps; i++ ) {
		CVector3 v = ( *path )[i];

		glPushMatrix();

		glTranslatef( v.x, v.y, v.z );

		glColor4f( 1.0f, 1.0f, 1.0f, 0.9f );

		float percent = static_cast<float>( i ) / static_cast<float>( maxSteps );

		if ( percent > 0.5f ) {
			percent += ( percent - 0.5f );
		}

		effects[i]->draw( effectType, 0, percent );

		glPopMatrix();
	}

	glsDisable( GLS_BLEND );
	glsEnable( GLS_DEPTH_MASK );
}

