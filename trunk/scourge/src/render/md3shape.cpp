/***************************************************************************
                          md3shape.cpp  -  description
                             -------------------
    begin                : Thu Aug 31 2006
    copyright            : (C) 2006 by Gabor Torok
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

#include <string>  
#include "Md3.h"
#include "Md2.h"
#include "md3shape.h"

using namespace std;

//#define DEBUG_MD2 1

MD3Shape::MD3Shape( CModelMD3 *md3, float div, 
										GLuint texture[], int width, int depth, int height,
										char *name, int descriptionGroup, Uint32 color, Uint8 shapePalIndex ) :
  AnimatedShape( width, depth, height, name, descriptionGroup, color, shapePalIndex ) {
  this->md3 = md3;    
  this->div = div; 
}

MD3Shape::~MD3Shape() {
}

void MD3Shape::draw() {

#ifdef DEBUG_MD2
  //if( glIsEnabled( GL_TEXTURE_2D ) ) {
    glPushMatrix();
    debugShape->draw();
    glPopMatrix();
  //}
#endif

  glPushMatrix();

  // rotate to upright
  glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );

  // move to the middle of the space
  //glTranslatef( ((float)width / DIV) / 2.0f, 
                //0.25f / DIV, 
                //-((float)depth / DIV) / 2.0f );
  glTranslatef( ((float)(width) / 2.0f) / DIV, 
                0.25f / DIV, 
                -(((float)(depth) / 2.0f) / DIV ) );

  // rotate to movement angle
  glRotatef(getAngle() - 90, 0.0f, 1.0f, 0.0f);

  // To make our model render somewhat faster, we do some front back culling.
  // It seems that Quake2 orders their polygons clock-wise.  
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );
  bool textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
  glEnable( GL_TEXTURE_2D );
	
	glScalef( div, div, div );
	md3->setAnimationPaused( isAnimationPaused() );
	md3->DrawModel();
 
  if( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
  glDisable(GL_CULL_FACE);
  glPopMatrix();    
}

void MD3Shape::outline( float r, float g, float b ) {
	useShadow = true;
  GLboolean blend;
  glGetBooleanv( GL_BLEND, &blend );
  //glEnable( GL_BLEND );
  //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	GLboolean texture = glIsEnabled( GL_TEXTURE_2D );
  glDisable( GL_TEXTURE_2D );
	glFrontFace( GL_CCW );
  glPolygonMode( GL_BACK, GL_LINE );
  glLineWidth( 4 );
  glEnable( GL_CULL_FACE );
  glCullFace( GL_FRONT );
  glColor3f( r, g, b );  
  
	glPushMatrix();
  // rotate to upright
  glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
  glTranslatef( ((float)(width) / 2.0f) / DIV, 
                0.25f / DIV, 
                -(((float)(depth) / 2.0f) / DIV ) );

  // rotate to movement angle
  glRotatef(getAngle() - 90, 0.0f, 1.0f, 0.0f);

	//glTranslatef( -md3->getMin()[2], -md3->getMin()[0], -md3->getMin()[1] );
	//glTranslatef( -md3->getMax()[2] / 2, 0, -md3->getMax()[1] / 2 );
	glScalef( div, div, div );
	//glTranslatef( 0, -md3->getMin()[0] * div, 0 );
	md3->setAnimationPaused( isAnimationPaused() );
	md3->DrawModel();


  glPopMatrix();    
  
	glLineWidth( 1 );
  glDisable( GL_CULL_FACE );
  glPolygonMode( GL_BACK, GL_FILL );
	if( !blend ) glDisable( GL_BLEND );
	if( texture ) glEnable( GL_TEXTURE_2D );
  useShadow = false;
  glColor4f(1, 1, 1, 0.9f);	
}

void MD3Shape::setCurrentAnimation(int numAnim, bool force) {    
	// convert to MD3 animation (I know this is lame)
	switch( numAnim ) {
	case MD2_ATTACK:
		md3->SetTorsoAnimation( "TORSO_ATTACK", force );
		md3->SetLegsAnimation( "LEGS_IDLE", force );
		break;
	case MD2_STAND:
		md3->SetTorsoAnimation( "TORSO_STAND", force );
		md3->SetLegsAnimation( "LEGS_IDLE", force );
		break;
	case MD2_RUN:
		md3->SetTorsoAnimation( "TORSO_STAND", force );
		md3->SetLegsAnimation( "LEGS_WALK", force );
		break;
	case MD2_WAVE:
	case MD2_POINT:
	case MD2_SALUTE:
	case MD2_TAUNT:
		md3->SetTorsoAnimation( "TORSO_STAND", force );
		md3->SetLegsAnimation( "LEGS_IDLE", force );
		break;
	case MD2_PAIN1:		
	case MD2_PAIN2:
	case MD2_PAIN3:
		md3->SetTorsoAnimation( "TORSO_STAND", force );
		md3->SetLegsAnimation( "LEGS_IDLE", force );
		break;
	default:
		cerr << "*** WARN: Unhandled movement in MD3Shape::setCurrentAnimation. numAnim=" << numAnim << endl;
	}

}

void MD3Shape::setupToDraw() {
}

