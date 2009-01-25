/***************************************************************************
     md3shape.cpp  -  Extends AnimatedShape with MD2 specific functions
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

#include "../common/constants.h"
#include <string>
#include "Md3.h"
#include "Md2.h"
#include "md3shape.h"
#include "modelwrapper.h"

using namespace std;

//#define DEBUG_MD2 1

MD3Shape::MD3Shape( CModelMD3 *md3, ModelLoader *loader, float div,
                    Texture texture[], int width, int depth, int height,
                    char const* name, int descriptionGroup, Uint32 color, Uint8 shapePalIndex ) :
		AnimatedShape( width, depth, height, name, descriptionGroup, color, shapePalIndex ) {
	// clone the md3 so we have our own animation data
	this->md3 = md3;
	this->loader = loader;
	this->div = div;
	numOfMaterialsUpper = numOfMaterialsLower = numOfMaterialsHead = 0;
	aiLower.currentAnim = aiUpper.currentAnim = aiHead.currentAnim = 0;
	aiLower.currentFrame = aiUpper.currentFrame = aiHead.currentFrame = 0;
	aiLower.lastTime = aiUpper.lastTime = aiHead.lastTime = 0;
	aiLower.nextFrame = aiUpper.nextFrame = aiHead.nextFrame = 0;
	aiLower.t = aiUpper.t = aiHead.t = 0;
	md3->SetTorsoAnimation( "TORSO_STAND", true, this );
	md3->SetLegsAnimation( "LEGS_IDLE", true, this );
	this->cleanupDone = false;
}

MD3Shape::~MD3Shape() {
	if ( !cleanupDone ) {
		cerr << "*** WARN: call cleanup first!" << endl;
	}
}

void MD3Shape::cleanup() {
	// this has to be done before the md3 is killed
	pMaterialLower.clear();
	pMaterialUpper.clear();
	pMaterialHead.clear();
	m_Textures.clear();
	cleanupDone = true;
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
	//glTranslatef( (static_cast<float>(width) * MUL) / 2.0f,
	//0.25f * MUL,
	//-(static_cast<float>(depth) * MUL) / 2.0f );
	glTranslatef( ( static_cast<float>( width ) / 2.0f ) * MUL, 0.25f * MUL, -( ( static_cast<float>( depth ) / 2.0f ) * MUL ) );

	// rotate to movement angle
	glRotatef( getAngle() - 90, 0.0f, 1.0f, 0.0f );

	// To make our model render somewhat faster, we do some front back culling.
	// It seems that Quake2 orders their polygons clock-wise.
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	GLboolean textureWasEnabled = glIsEnabled( GL_TEXTURE_2D );
	if( isShade() ) {
		glDisable( GL_TEXTURE_2D );
	} else {
		glEnable( GL_TEXTURE_2D );
	}

	glScalef( div, div, div );
	md3->setAnimationPaused( isAnimationPaused() );
	md3->DrawModel( this );

	if( isShade() ) {
		if ( textureWasEnabled ) glEnable( GL_TEXTURE_2D );
	} else {
		if ( !textureWasEnabled ) glDisable( GL_TEXTURE_2D );
	}
	glDisable( GL_CULL_FACE );
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
	glTranslatef( ( static_cast<float>( width ) / 2.0f ) * MUL, 0.25f * MUL, -( ( static_cast<float>( depth ) / 2.0f ) * MUL ) );

	// rotate to movement angle
	glRotatef( getAngle() - 90, 0.0f, 1.0f, 0.0f );

	//glTranslatef( -md3->getMin()[2], -md3->getMin()[0], -md3->getMin()[1] );
	//glTranslatef( -md3->getMax()[2] / 2, 0, -md3->getMax()[1] / 2 );
	glScalef( div, div, div );
	//glTranslatef( 0, -md3->getMin()[0] * div, 0 );
	md3->setAnimationPaused( isAnimationPaused() );
	md3->DrawModel( this );

	glPopMatrix();

	glLineWidth( 1 );
	glDisable( GL_CULL_FACE );
	glPolygonMode( GL_BACK, GL_FILL );
	if ( !blend ) glDisable( GL_BLEND );
	if ( texture ) glEnable( GL_TEXTURE_2D );
	useShadow = false;
	glColor4f( 1, 1, 1, 0.9f );
}

void MD3Shape::setModelAnimation() {

// cerr << "setting md3 model animation to=" << currentAnim << endl;

	// convert to MD3 animation (I know this is lame)
	switch ( currentAnim ) {
	case MD2_ATTACK:
		md3->SetTorsoAnimation( "TORSO_ATTACK", true, this );
		md3->SetLegsAnimation( "LEGS_IDLE", true, this );
		break;
	case MD2_STAND:
		md3->SetTorsoAnimation( "TORSO_STAND", true, this );
		md3->SetLegsAnimation( "LEGS_IDLE", true, this );
		break;
	case MD2_RUN:
		md3->SetTorsoAnimation( "TORSO_STAND", true, this );
		md3->SetLegsAnimation( "LEGS_WALK", true, this );
		break;
	case MD2_WAVE:
	case MD2_POINT:
	case MD2_SALUTE:
	case MD2_TAUNT:
		md3->SetTorsoAnimation( "TORSO_STAND", true, this );
		md3->SetLegsAnimation( "LEGS_IDLE", true, this );
		break;
	case MD2_PAIN1:
		md3->SetTorsoAnimation( "TORSO_STAND2", true, this );
		md3->SetLegsAnimation( "LEGS_IDLE", true, this );
	case MD2_PAIN2:
	case MD2_PAIN3:
		md3->SetTorsoAnimation( "TORSO_GESTURE", true, this );
		md3->SetLegsAnimation( "LEGS_IDLE", true, this );
		break;
	default:
		cerr << "*** WARN: Unhandled movement in MD3Shape::setCurrentAnimation. currentAnim=" << currentAnim << endl;
	}
}

AnimInfo *MD3Shape::getAnimInfo( t3DModel *model ) {
	if ( model == md3->GetModel( kLower ) ) {
		return &aiLower;
	} else if ( model == md3->GetModel( kUpper ) ) {
		return &aiUpper;
	} else if ( model == md3->GetModel( kHead ) ) {
		return &aiHead;
	} else {
		cerr << "*** Error: can't find animation info for model." << endl;
		return NULL;
	}
}

std::vector<tMaterialInfo> *MD3Shape::getMaterialInfos( t3DModel *pModel ) {
	if ( pModel == md3->GetModel( kLower ) ) {
		return &pMaterialLower;
	} else if ( pModel == md3->GetModel( kUpper ) ) {
		return &pMaterialUpper;
	} else if ( pModel == md3->GetModel( kHead ) ) {
		return &pMaterialHead;
	} else {
		cerr << "*** Error: can't find material info for model." << endl;
		return NULL;
	}
}

int MD3Shape::getNumOfMaterials( t3DModel *pModel ) {
	if ( pModel == md3->GetModel( kLower ) ) {
		return numOfMaterialsLower;
	} else if ( pModel == md3->GetModel( kUpper ) ) {
		return numOfMaterialsUpper;
	} else if ( pModel == md3->GetModel( kHead ) ) {
		return numOfMaterialsHead;
	} else {
		cerr << "*** Error: can't find num of materials for model." << endl;
		return 0;
	}
}

void MD3Shape::setNumOfMaterials( t3DModel *pModel, int n ) {
	if ( pModel == md3->GetModel( kLower ) ) {
		numOfMaterialsLower = n;
	} else if ( pModel == md3->GetModel( kUpper ) ) {
		numOfMaterialsUpper = n;
	} else if ( pModel == md3->GetModel( kHead ) ) {
		numOfMaterialsHead = n;
	} else {
		cerr << "*** Error: can't set num of materials for model." << endl;
	}
}

