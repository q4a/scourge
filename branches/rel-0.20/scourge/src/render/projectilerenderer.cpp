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
#include "map.h"
#include "renderedprojectile.h"
#include "render.h"

using namespace std;

void ShapeProjectileRenderer::drawPath( Map *map, 
																				RenderedProjectile *proj, 
																				std::vector<CVector3> *path ) {
	// draw the last step only
	CVector3 last = path->back();

	glPushMatrix();
	shape->setupToDraw();
	glTranslatef( last.x, last.y, last.z );
	glColor4f(1, 1, 1, 0.9f);
	glDisable( GL_CULL_FACE );
	((GLShape*)shape)->setCameraPos( map->getXPos(), 
																	 map->getYPos(), 
																	 map->getZPos(), 
																	 last.x, last.y, last.z );

	// orient and draw the projectile
	float f = proj->getAngle() + 90;
	if(f < 0) f += 360;
	if(f >= 360) f -= 360;
	glRotatef( f, 0, 0, 1 );
	
	// for projectiles, set the correct camera angle
	if( proj->getAngle() < 90 ) {
		((GLShape*)shape)->setCameraRot( map->getXRot(), 
																		 map->getYRot(),
																		 map->getZRot() + proj->getAngle() + 90 );		
	} else if( proj->getAngle() < 180) {
		((GLShape*)shape)->setCameraRot( map->getXRot(), 
																		 map->getYRot(),
																		 map->getZRot() - proj->getAngle() );		
	} else {
		((GLShape*)shape)->setCameraRot( map->getXRot(), 
																		 map->getYRot(),
																		 map->getZRot() );
	}

	if( shape->drawLater() ) {
		glEnable( GL_BLEND );
		glDepthMask( GL_FALSE );
		shape->setupBlending();
	}
	shape->draw();
	if( shape->drawLater() ) {
		shape->endBlending();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
	glPopMatrix();
}

EffectProjectileRenderer::EffectProjectileRenderer(Map *map, Preferences *prefs, Shapes *shapes, int effectType, int timeToLive) {
	this->effectType = effectType;
	this->timeToLive = timeToLive;
	effects.resize( MAX_EFFECT_COUNT );
	for( int i = 0; i < MAX_EFFECT_COUNT; i++ ) {
		effects[i] = new Effect( map, prefs, shapes, 2, 2 );
	}
}

EffectProjectileRenderer::~EffectProjectileRenderer() {
	for( int i = 0; i < MAX_EFFECT_COUNT; i++ ) {
		delete effects[i];
	}
}

void EffectProjectileRenderer::drawPath( Map *map, RenderedProjectile *proj, std::vector<CVector3> *path ) {
	int maxSteps = static_cast<int>(path->size());
	if( maxSteps > MAX_EFFECT_COUNT )
		maxSteps = MAX_EFFECT_COUNT;

	for( int i = 0; i < maxSteps; i++ ) {
		CVector3 v = (*path)[i];
		glPushMatrix();
		glTranslatef( v.x, v.y, v.z );
		glColor4f( 1, 1, 1, 0.9f );
		glDisable( GL_CULL_FACE );
		glEnable( GL_BLEND );
		glDepthMask( GL_FALSE );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE );

		float percent = static_cast<float>(i) / static_cast<float>(maxSteps);
		if( percent > 0.5f ) {
			percent += ( percent - 0.5f );
		}
		effects[i]->draw( effectType, 0, percent );
		
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glPopMatrix();
	}	
}

