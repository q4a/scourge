/***************************************************************************
                          renderedprojectile.cpp  -  description
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

#include "renderedprojectile.h"
#include "renderedcreature.h"

using namespace std;

map<RenderedCreature*, vector<RenderedProjectile*>*> RenderedProjectile::projectiles;

RenderedProjectile::RenderedProjectile() {
}

RenderedProjectile::~RenderedProjectile() {
}

void RenderedProjectile::addProjectile( RenderedProjectile *proj ) {
  vector<RenderedProjectile*> *v;
  if( projectiles.find( proj->getCreature() ) == projectiles.end() ) {
    v = new vector<RenderedProjectile*>();
    projectiles[ proj->getCreature() ] = v;
  } else {
    v = projectiles[ proj->getCreature() ];
  }
  v->push_back( proj );
}

void RenderedProjectile::resetProjectiles() {
  for( map<RenderedCreature *, vector<RenderedProjectile*>*>::iterator i = projectiles.begin(); 
       i != projectiles.end(); ) {
    vector<RenderedProjectile*> *v = i->second;
    for( vector<RenderedProjectile*>::iterator e2 = v->begin(); 
         e2 != v->end(); 
         ++e2 ) {
      RenderedProjectile *proj = *e2;  
      delete proj;
    }
    delete v;
    projectiles.erase( i++ );
  }
}

void RenderedProjectile::removeProjectile( RenderedProjectile *p ) {
  if( projectiles.find( p->getCreature() ) != projectiles.end() ) {
    vector<RenderedProjectile*> *v = projectiles[ p->getCreature() ];
    for( vector<RenderedProjectile*>::iterator e = v->begin(); e != v->end(); ++e ) {
      RenderedProjectile *proj = *e;  
      if( proj == p ) {
        v->erase( e );
		if( v->empty() ) {
          projectiles.erase( p->getCreature() );
        }
        return;
      }
    }
  }
}

