/***************************************************************************
               scourgeview.cpp  -  Manages the ingame main view
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
#include "common/constants.h"
#include "scourgeview.h"
#include "battle.h"
#include "creature.h"
#include "pathmanager.h"
#include "item.h"
#include "party.h"
#include "minimap.h"
#include "rpg/rpglib.h"
#include "texteffect.h"
#include "session.h"
#include "shapepalette.h"
#include "render/renderlib.h"
#include "sdlhandler.h"
#include "gui/progress.h"
#include "gui/scrollinglist.h"
#include "debug.h"
#include "textscroller.h"
#include "sound.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

#define INFO_INTERVAL 3000

Uint32 areaTicks = 0;
#define AREA_SPEED 50
GLfloat areaRot = 0.0f;
#define AREA_ROT_DELTA 0.2f

ScourgeView::ScourgeView( Scourge *scourge ) {
	this->scourge = scourge;
	needToCheckInfo = false;
	outlineColor = new Color( 0.3f, 0.3f, 0.5f, 0.5f );
	disabledTrapColor = new Color( 0.5, 0.5, 0.5, 0.5f );
	enabledTrapColor = new Color( 1, 1, 0, 0.5f );
	debugTrapColor = new Color( 0, 1, 1, 0.5f );
	textEffect = NULL;
	textEffectTimer = 0;
	needToCheckDropLocation = true;
	targetWidth = 0.0f;
	targetWidthDelta = 0.05f;
	lastTargetTick = SDL_GetTicks();
	turnProgress = NULL;
}

void ScourgeView::initUI() {
	quadric = gluNewQuadric();
	turnProgress = new Progress( scourge->getSDLHandler(), scourge->getSession()->getShapePalette()->getProgressTexture(),
	                             scourge->getSession()->getShapePalette()->getProgressHighlightTexture(), 10, false, true, false );
}

ScourgeView::~ScourgeView() {
	delete turnProgress;
	delete debugTrapColor;
	delete enabledTrapColor;
	delete disabledTrapColor;
	delete outlineColor;
}

void ScourgeView::drawView() {
	if ( scourge->getSession()->isShowingChapterIntro() ) {
		endScissorToMap();
		scourge->hideGui(); // HACK: Hide party UI in the final chapter "outro".
		drawChapterIntro();
		return;
	}

	// make a move (player, monsters, etc.)
	scourge->playRound();

	scourge->updatePartyUI();

	checkForDropTarget();
	checkForInfo();

	// in TB combat, center map when monster's turn
	centerOnMonsterInTB();

	// apply movie transformations
	if ( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		scourge->getSession()->getCutscene()->updateCameraPosition();
	}

	// cull back faces
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );

	scourge->getMap()->draw();
	
	drawMapInfos();

	scourge->getSession()->getWeather()->drawWeather();

	ambientObjectSounds();

	if ( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		endScissorToMap();
	} else {
		scourge->getMiniMap()->drawMap();

		// the boards outside the map
		drawOutsideMap();

		endScissorToMap();

		drawBorder();

		drawTextEffect();

		scourge->getDescriptionScroller()->draw();
	}

	// HACK: A container window may have been closed by hitting the Esc. button.
	if ( scourge->getSDLHandler()->windowWasClosed ) {
//		scourge->removeClosedContainerGuis();
		scourge->getSDLHandler()->windowWasClosed = false;
	}
}

// flip all the switches (seemingly at random) to try to restore the rendering pipeline...
void ScourgeView::endScissorToMap() {
	glEnable( GL_TEXTURE_2D );
	glDisable( GL_CULL_FACE );
	glDisable( GL_SCISSOR_TEST );
	glDisable( GL_BLEND );
	/*  if( scourge->getPreferences()->getStencilbuf() &&
	      scourge->getPreferences()->getStencilBufInitialized() ) {
	   glClear( GL_STENCIL_BUFFER_BIT );
	   glColorMask( 1, 1, 1, 1 );
	   glStencilFunc( GL_EQUAL, 1, 0xffffffff );
	   glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
	   glDisable(GL_STENCIL_TEST);
	  } */
	//glDepthMask( GL_TRUE );
	glDisable( GL_ALPHA_TEST );
// glColor4f( 1, 1, 1, 1 );
}

#define MAX_AMBIENT_OBJECT_DISTANCE 11
void ScourgeView::ambientObjectSounds() {
	// look for ambient sounds
	int xp = toint( scourge->getPlayer()->getX() + scourge->getPlayer()->getShape()->getWidth() / 2 );
	int yp = toint( scourge->getPlayer()->getY() - scourge->getPlayer()->getShape()->getDepth() / 2 );
	for ( int xx = xp - MAX_AMBIENT_OBJECT_DISTANCE; xx < xp + MAX_AMBIENT_OBJECT_DISTANCE; xx++ ) {
		for ( int yy = yp - MAX_AMBIENT_OBJECT_DISTANCE; yy < yp + MAX_AMBIENT_OBJECT_DISTANCE; yy++ ) {
			for ( int zz = 0; zz < MAP_VIEW_HEIGHT; zz++ ) {
				Location *pos = scourge->getMap()->getPosition( xx, yy, zz );
				if ( pos && pos->shape && ( ( GLShape* )pos->shape )->getAmbientName() != "" ) {
					float dist = Constants::distance( scourge->getPlayer()->getX(), scourge->getPlayer()->getY(),
					                                  scourge->getPlayer()->getShape()->getWidth(),
					                                  scourge->getPlayer()->getShape()->getDepth(),
					                                  pos->x, pos->y, pos->shape->getWidth(), pos->shape->getDepth() );
					if ( dist <= MAX_AMBIENT_OBJECT_DISTANCE ) {
						float percent = 100 - ( dist / static_cast<float>( MAX_AMBIENT_OBJECT_DISTANCE ) ) * 100.0f + 20;
						if ( percent > 100 )
							percent = 100;
						int panning = scourge->getMap()->getPanningFromMapXY( pos->x, pos->y );
						scourge->getSession()->getSound()->playObjectSound( ( ( GLShape* )pos->shape )->getAmbientName(), toint( percent ), panning );

						// fixme: it should play every unique sound not just the first
						return;
					}
				}
			}
		}
	}
}

int chapterTextTimer = 0;
#define CHAPTER_TEXT_SPEED 90
#define CHAPTER_TEXT_DELTA 1

void ScourgeView::drawChapterIntro() {
	glDisable( GL_TEXTURE_2D );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

	//glEnable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_NOTEQUAL, 0);
	glPushMatrix();
	glLoadIdentity( );
	glPixelZoom( 1.0, -1.0 );

	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
	glColor4f( 1, 0.35f, 0, 1 );
	scourge->getSDLHandler()->texPrint( 10, 36, scourge->getMissionTitle() );
	glColor4f( 1, 1, 1, 1 );

// int px = ( scourge->getScreenWidth() - scourge->getSession()->getChapterImageWidth() ) / 2;
// if( px < 0 )
//  px = 0; // needed to show image in low resolution
// int py = 40;
	int imageX = 0;
	int imageY = 40;
	float imageScale = ( float )scourge->getScreenWidth() / ( float )scourge->getSession()->getChapterImageWidth();
	int imageW = ( float )scourge->getSession()->getChapterImageWidth() * imageScale;
	int imageH = ( float )scourge->getSession()->getChapterImageHeight() * imageScale;

	int textHeight = scourge->getScreenHeight() - imageH - imageY;

	// pos the text the first time
	if ( scourge->getChapterTextPos() == -2000 ) {
		scourge->setChapterTextPos( textHeight );
	}

	Texture image = scourge->getSession()->getChapterImageTexture();
	if ( image.isSpecified() ) {
		glEnable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );

		glPushMatrix();

		image.glBind();
		glColor4f( 1, 1, 1, 1 );

		glLoadIdentity();
		glTranslatef( imageX, imageY, 0 );
//          glNormal3f( 0, 0, 1 );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2i( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2i( 1, 0 );
		glVertex2i( imageW, 0 );
		glTexCoord2i( 0, 1 );
		glVertex2i( 0, imageH );
		glTexCoord2i( 1, 1 );
		glVertex2i( imageW, imageH );
		glEnd();

		glPopMatrix();
	}

	scourge->getChapterIntroWin()->move( 0, imageH + imageY + 10 - 30 );

	glScissor( 150, 0, scourge->getScreenWidth() - 150, textHeight );
	glEnable( GL_SCISSOR_TEST );
	int offset = scourge->getChapterTextPos();
	glTranslatef( 160, ( scourge->getScreenHeight() - textHeight ), 0 );
	glColor4f( 1, 0.9f, 0.8f, 1 );
	for ( unsigned int i = 0; i < scourge->getChapterText()->size(); i++ ) {
		string s = ( *scourge->getChapterText() )[i];
		int ypos = i * 36 + offset;
		scourge->getSDLHandler()->texPrint( 0, ypos, s.c_str() );
	}
	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

	int size = 50;
	glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBegin( GL_QUADS );
	glColor4f( 0, 0, 0, 0 );
	glVertex2i( 0, size );
	glColor4f( 0, 0, 0, 1 );
	glVertex2i( 0, 0 );
	glColor4f( 0, 0, 0, 1 );
	glVertex2i( scourge->getScreenWidth() - 150, 0 );
	glColor4f( 0, 0, 0, 0 );
	glVertex2i( scourge->getScreenWidth() - 150, size );

	glColor4f( 0, 0, 0, 0 );
	glVertex2i( 0, textHeight - size );
	glColor4f( 0, 0, 0, 1 );
	glVertex2i( 0, textHeight );
	glColor4f( 0, 0, 0, 1 );
	glVertex2i( scourge->getScreenWidth() - 150, textHeight );
	glColor4f( 0, 0, 0, 0 );
	glVertex2i( scourge->getScreenWidth() - 150, textHeight - size );
	glEnd();
	glDisable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );

	glDisable( GL_SCISSOR_TEST );

	Uint32 now = SDL_GetTicks();
	if ( now - chapterTextTimer > CHAPTER_TEXT_SPEED && offset > ( static_cast<int>( scourge->getChapterText()->size() ) * -36 ) ) {
		scourge->setChapterTextPos( offset - CHAPTER_TEXT_DELTA );
		chapterTextTimer = now;
	}

	//glDisable(GL_ALPHA_TEST);
	glPopMatrix();
}

void ScourgeView::centerOnMonsterInTB() {
	scourge->getMap()->setMapCenterCreature( NULL );
	if ( scourge->inTurnBasedCombat() ) {
		Battle *battle = scourge->getCurrentBattle();
		Creature *c = battle->getCreature();
		if ( !c->isPartyMember() || c->getStateMod( StateMod::possessed ) ) {
			scourge->getMap()->setMapCenterCreature( c );
			scourge->getMap()->center( toint( c->getX() ), toint( c->getY() ), true );
		}
	}
}

void ScourgeView::drawOutsideMap() {
	// cover the area outside the map
	if ( scourge->getMap()->getViewWidth() < scourge->getSDLHandler()->getScreen()->w ||
	        scourge->getMap()->getViewHeight() < scourge->getSDLHandler()->getScreen()->h ) {
		//glPushAttrib( GL_ENABLE_BIT );
		glDisable( GL_CULL_FACE );
		glDisable( GL_DEPTH_TEST );
		glEnable( GL_TEXTURE_2D );
		glPushMatrix();
		glColor3f( 1, 1, 1 );
		scourge->getSession()->getShapePalette()->getGuiWoodTexture().glBind();

		//    float TILE_W = 510 / 2.0f;
		float TILE_H = 270 / 2.0f;

		glLoadIdentity();
		glTranslatef( scourge->getMap()->getViewWidth(), 0, 0 );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2f( 1, 0 );
		glVertex2i( scourge->getSDLHandler()->getScreen()->w - scourge->getMap()->getViewWidth(), 0 );
		glTexCoord2f( 0, scourge->getSDLHandler()->getScreen()->h / TILE_H );
		glVertex2i( 0, scourge->getSDLHandler()->getScreen()->h );
		glTexCoord2f( 1, scourge->getSDLHandler()->getScreen()->h / TILE_H );
		glVertex2i( scourge->getSDLHandler()->getScreen()->w - scourge->getMap()->getViewWidth(), scourge->getSDLHandler()->getScreen()->h );
		glEnd();

		glLoadIdentity();
		glTranslatef( 0, scourge->getMap()->getViewHeight(), 0 );
		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0, 0 );
		glVertex2i( 0, 0 );
		glTexCoord2f( 1, 0 );
		glVertex2i( scourge->getMap()->getViewWidth(), 0 );
		glTexCoord2f( 0, scourge->getMap()->getViewHeight() / TILE_H );
		glVertex2i( 0, scourge->getMap()->getViewHeight() );
		glTexCoord2f( 1, scourge->getMap()->getViewHeight() / TILE_H );
		glVertex2i( scourge->getMap()->getViewWidth(), scourge->getMap()->getViewHeight() );
		glEnd();

		glPopMatrix();
		//    glPopAttrib();
		glEnable( GL_CULL_FACE );
		glEnable( GL_DEPTH_TEST );
		glDisable( GL_TEXTURE_2D );
	}
}

void ScourgeView::checkForInfo() {
	Uint16 mapx, mapy, mapz;
	// change cursor when over a hostile creature
	if ( scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_NORMAL ||
	        scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_ATTACK ||
	        scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_RANGED ||
	        scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_MOVE ||
	        scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_TALK ||
	        scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_USE ) {
		if ( scourge->getSDLHandler()->mouseIsMovingOverMap ) {
			bool handled = false;
			mapx = scourge->getMap()->getCursorMapX();
			mapy = scourge->getMap()->getCursorMapY();
			mapz = scourge->getMap()->getCursorMapZ();
			if ( mapx < MAP_WIDTH ) {
				Location *pos = scourge->getMap()->getLocation( mapx, mapy, mapz );
				if ( !pos )
					pos = scourge->getMap()->getItemLocation( mapx, mapy );
				else {
					int cursor;
					if ( pos->creature && scourge->getParty()->getPlayer()->canAttack( pos->creature, &cursor ) ) {
						if ( pos->creature->isNpc() )
							scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_TALK );
						else
							scourge->getSDLHandler()->setCursorMode( cursor );

						handled = true;
					} else if ( getOutlineColor( pos ) ) {
						scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_USE );
						handled = true;
					}
				}
			}

			mapx = scourge->getMap()->getCursorFlatMapX();
			mapy = scourge->getMap()->getCursorFlatMapY();
			int trapIndex = scourge->getMap()->getTrapAtLoc( mapx, mapy + 2 );
			if ( trapIndex > -1 ) {
				Trap *trap = scourge->getMap()->getTrapLoc( trapIndex );
				if ( trap->discovered && trap->enabled ) {
					scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_USE );
					handled = true;
				}
			}

			if ( !handled ) scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_NORMAL );
		}
	}

	if ( scourge->getUserConfiguration()->getTooltipEnabled() && SDL_GetTicks() - scourge->getSDLHandler()->lastMouseMoveTime >
	        ( Uint32 )( scourge->getUserConfiguration()->getTooltipInterval() * 10 ) ) {
		if ( needToCheckInfo ) {
			needToCheckInfo = false;

			// check location
			Uint16 mapx = scourge->getMap()->getCursorMapX();
			Uint16 mapy = scourge->getMap()->getCursorMapY();
			Uint16 mapz = scourge->getMap()->getCursorMapZ();
			if ( mapx < MAP_WIDTH ) {
				Location *pos = scourge->getMap()->getLocation( mapx, mapy, mapz );
				if ( !pos )
					pos = scourge->getMap()->getItemLocation( mapx, mapy );
				else {
					std::string s;
					void *obj = NULL;
					if ( pos->creature ) {
						obj = pos->creature;
						//((Creature*)(pos->creature))->getDetailedDescription( s );
						s = _( ( ( Creature* )( pos->creature ) )->getName() );
					} else if ( pos->item ) {
						obj = pos->item;
						( ( Item* )( pos->item ) )->getDetailedDescription( s );
					} else if ( pos->shape ) {
						obj = pos->shape;
						s = scourge->getSession()->getShapePalette()->getRandomDescription( pos->shape->getDescriptionGroup() );
					}

					if ( obj ) {
						bool found = false;
						// Don't show info about the same object twice
						// FIXME: use lookup table
						for ( map<InfoMessage *, Uint32>::iterator i = infos.begin(); i != infos.end(); ++i ) {
							InfoMessage *message = i->first;
							if ( message->obj == obj || ( message->x == pos->x && message->y == pos->y && message->z == pos->z ) ) {
								found = true;
								break;
							}
						}
						if ( !found ) {
							InfoMessage *message = new InfoMessage( s.c_str(), obj, pos->x + pos->shape->getWidth() / 2,
							                                        pos->y - 1 - pos->shape->getDepth() / 2, pos->z + pos->shape->getHeight() / 2 );

							infos[ message ] = SDL_GetTicks();
						}
					}
				}
			}
		} else {
			needToCheckInfo = true;
		}
	}
	// timeout descriptions
	// http://www.velocityreviews.com/forums/t283023-stl-stdmap-erase.html
	Uint32 now = SDL_GetTicks();
	for ( map<InfoMessage *, Uint32>::iterator i = infos.begin(); i != infos.end(); ) {
		InfoMessage *message = i->first;
		Uint32 time = i->second;
		if ( now - time > INFO_INTERVAL ) {
			infos.erase( i++ );
			delete message;
		} else {
			++i;
		}
	}
}

void ScourgeView::resetInfos() {
	// http://www.velocityreviews.com/forums/t283023-stl-stdmap-erase.html
	for ( map<InfoMessage *, Uint32>::iterator i = infos.begin(); i != infos.end(); ) {
		InfoMessage *message = i->first;
		infos.erase( i++ );
		delete message;
	}
}

void ScourgeView::drawMapInfos() {
	// draw stuff on top of the map
	scourge->getMap()->initMapView();
	drawCreatureInfos();
	drawInfos();
}

void ScourgeView::drawCreatureInfos() {
	// creatures first
	for ( int i = 0; i < scourge->getSession()->getCreatureCount(); i++ ) {
		//if(!session->getCreature(i)->getStateMod(Constants::dead) &&
		Creature *creature = scourge->getSession()->getCreature( i );
		if ( scourge->getMap()->isLocationVisible( toint( creature->getX() ), toint( creature->getY() ) ) && 
				scourge->getMap()->isLocationInLight( toint( creature->getX() ), toint( creature->getY() ), creature->getShape() ) &&
				scourge->getSession()->isVisible( creature ) ) {
			showCreatureInfo( scourge->getSession()->getCreature( i ), false, false, false,
			                  ( scourge->getSession()->getCreature( i )->getCharacter() ? true : false ) );
		}
	}
	// party next so red target circle shows over gray
	for ( int i = 0; i < scourge->getParty()->getPartySize(); i++ ) {
		bool player = scourge->getParty()->getPlayer() == scourge->getParty()->getParty( i );
		if ( scourge->inTurnBasedCombat() && scourge->getParty()->isRealTimeMode() ) {
			player = ( scourge->getParty()->getParty( i ) == scourge->getCurrentBattle()->getCreature() );
		}
		showCreatureInfo( scourge->getParty()->getParty( i ), player, ( scourge->getMap()->getSelectedDropTarget() &&
		                  scourge->getMap()->getSelectedDropTarget()->creature == scourge->getParty()->getParty( i ) ),
		                  !scourge->getParty()->isPlayerOnly(), false );
	}
}

void ScourgeView::drawInfos() {
	if ( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		return;
	}

	float xpos2, ypos2, zpos2;
	for ( map<InfoMessage *, Uint32>::iterator i = infos.begin(); i != infos.end(); ++i ) {

		InfoMessage *message = i->first;
		xpos2 = ( static_cast<float>( message->x - scourge->getMap()->getX() ) * MUL );
		ypos2 = ( static_cast<float>( message->y - scourge->getMap()->getY() ) * MUL );
		zpos2 = ( static_cast<float>( message->z ) * MUL );

		scourge->getSDLHandler()->drawTooltip( xpos2, ypos2, zpos2, -( scourge->getMap()->getZRot() ), -( scourge->getMap()->getYRot() ),
		                                       message->message, 0, 0.15f, 0.05f, 1.0f / scourge->getSession()->getMap()->getZoom() );
	}

	glEnable( GL_DEPTH_TEST );

}

void ScourgeView::checkForDropTarget() {
	if ( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		return;
	}

	// find the drop target
	if ( scourge->getMovingItem() ) {

		// is the mouse moving?
		if ( !scourge->getSDLHandler()->mouseIsMovingOverMap ) {
			if ( needToCheckDropLocation ) {
				needToCheckDropLocation = false;

				// check location
				Location *dropTarget = NULL;
				Uint16 mapx = scourge->getMap()->getCursorMapX();
				Uint16 mapy = scourge->getMap()->getCursorMapY();
				Uint16 mapz = scourge->getMap()->getCursorMapZ();
				if ( mapx < MAP_WIDTH ) {
					dropTarget = scourge->getMap()->getLocation( mapx, mapy, mapz );
					if ( !( dropTarget && ( dropTarget->creature || ( dropTarget->item &&
					                                                  ( ( Item* )( dropTarget->item ) )->getRpgItem()->getType() == RpgItem::CONTAINER ) ) ) ) {
						dropTarget = NULL;
					}
				}
				scourge->getMap()->setSelectedDropTarget( dropTarget );
			}
		} else {
			needToCheckDropLocation = true;
		}
	}
}

void ScourgeView::drawBorder() {
	if ( scourge->getMap()->getViewWidth() == scourge->getSDLHandler()->getScreen()->w &&
	        scourge->getMap()->getViewHeight() == scourge->getSDLHandler()->getScreen()->h &&
	        !scourge->getUserConfiguration()->getFrameOnFullScreen() ) return;

	glPushMatrix();
	glLoadIdentity();

	// ok change: viewx, viewy always 0
	//glTranslatef(viewX, viewY, 100);
	glTranslatef( 0, 0, 100 );

	//  glDisable(GL_BLEND);
	glDisable( GL_DEPTH_TEST );

	// draw border
	glColor4f( 1, 1, 1, 1 );

	int w = ( scourge->getMap()->getViewWidth() == scourge->getSDLHandler()->getScreen()->w ?
	          scourge->getMap()->getViewWidth() :
	          scourge->getMap()->getViewWidth() - Window::SCREEN_GUTTER );
	int h = ( scourge->getMap()->getViewHeight() == scourge->getSDLHandler()->getScreen()->h ?
	          scourge->getMap()->getViewHeight() :
	          scourge->getMap()->getViewHeight() - Window::SCREEN_GUTTER );
	float TILE_W = 20.0f;
	float TILE_H = 120.0f;

	scourge->getSession()->getShapePalette()->getBorderTexture().glBind();
	glBegin( GL_QUADS );
	// left
	glTexCoord2f ( 0.0f, 0.0f );
	glVertex2i ( 0, 0 );
	glTexCoord2f ( 0.0f, h / TILE_H );
	glVertex2i ( 0, h );
	glTexCoord2f ( TILE_W / TILE_W, h / TILE_H );
	glVertex2i ( static_cast<int>( TILE_W ), h );
	glTexCoord2f ( TILE_W / TILE_W, 0.0f );
	glVertex2i ( static_cast<int>( TILE_W ), 0 );

	// right
	int gutter = 5;
	glTexCoord2f ( TILE_W / TILE_W, 0.0f );
	glVertex2i ( w - static_cast<int>( TILE_W ) + gutter, 0 );
	glTexCoord2f ( TILE_W / TILE_W, h / TILE_H );
	glVertex2i ( w - static_cast<int>( TILE_W ) + gutter, h );
	glTexCoord2f ( 0.0f, h / TILE_H );
	glVertex2i ( w + gutter, h );
	glTexCoord2f ( 0.0f, 0.0f );
	glVertex2i ( w + gutter, 0 );
	glEnd();

	TILE_W = 120.0f;
	TILE_H = 20.0f;
	scourge->getSession()->getShapePalette()->getBorder2Texture().glBind();
	glBegin( GL_QUADS );
	// top
	glTexCoord2f ( 0.0f, 0.0f );
	glVertex2i ( 0, 0 );
	glTexCoord2f ( 0.0f, TILE_H / TILE_H );
	glVertex2i ( 0, static_cast<int>( TILE_H ) );
	glTexCoord2f ( w / TILE_W, TILE_H / TILE_H );
	glVertex2i ( w, static_cast<int>( TILE_H ) );
	glTexCoord2f ( w / TILE_W, 0.0f );
	glVertex2i ( w, 0 );

	// bottom
	glTexCoord2f ( w / TILE_W, TILE_H / TILE_H );
	glVertex2i ( 0, h - static_cast<int>( TILE_H ) + gutter );
	glTexCoord2f ( w / TILE_W, 0.0f );
	glVertex2i ( 0, h + gutter );
	glTexCoord2f ( 0.0f, 0.0f );
	glVertex2i ( w, h + gutter );
	glTexCoord2f ( 0.0f, TILE_H / TILE_H );
	glVertex2i ( w, h - static_cast<int>( TILE_H ) + gutter );
	glEnd();

	int gw = 220;
	int gh = 64;

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	scourge->getSession()->getShapePalette()->getGargoyleTexture().glBind();

	glPushMatrix();
	glLoadIdentity();
	//glTranslatef(10, -5, 0);
	//glRotatef(20, 0, 0, 1);
	glBegin( GL_TRIANGLE_STRIP );
	// top left
	glTexCoord2f ( 0, 0 );
	glVertex2i ( 0, 0 );
	glTexCoord2f ( ( 1.0f / gw ) * ( gw - 1 ), 0 );
	glVertex2i ( gw, 0 );
	glTexCoord2f ( 0, 1 );
	glVertex2i ( 0, gh );
	glTexCoord2f ( ( 1.0f / gw ) * ( gw - 1 ), 1 );
	glVertex2i ( gw, gh );
	glEnd();
	glPopMatrix();

	// top right
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( w - gw, 0, 0 );
	//glRotatef(-20, 0, 0, 1);
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f ( ( 1.0f / gw ) * ( gw - 1 ), 0 );
	glVertex2i ( 0, 0 );
	glTexCoord2f ( 0, 0 );
	glVertex2i ( gw, 0 );
	glTexCoord2f ( ( 1.0f / gw ) * ( gw - 1 ), 1 );
	glVertex2i ( 0, gh );
	glTexCoord2f ( 0, 1 );
	glVertex2i ( gw, gh );
	glEnd();

	glPopMatrix();

	//glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glEnable( GL_DEPTH_TEST );
	glPopMatrix();
}

bool ScourgeView::startTextEffect( char *message ) {
	if ( !textEffect ) {
		int x = scourge->getScreenWidth() / 2 - ( strlen( message ) / 2 * 18 );
		int y = scourge->getScreenHeight() / 2 - 50;
		//cerr << "x=" << x << " y=" << y << endl;
		textEffect = new TextEffect( scourge, x, y, message );
		textEffect->setActive( true );
		textEffectTimer = SDL_GetTicks();
		return true;
	} else {
		return false;
	}
}

void ScourgeView::drawTextEffect() {
	static char message[255];

	// draw the current text effect
	if ( textEffect ) {
		if ( SDL_GetTicks() - textEffectTimer < 5000 ) {
			if ( scourge->getUserConfiguration()->getFlaky() ) {
				// When we don't use a fancy TextEffect, use blinking normal text
				if ( ( SDL_GetTicks() % 500 ) > 199 ) {
					glDisable( GL_DEPTH_TEST );
					glEnable( GL_BLEND );

					glPushMatrix();
					glLoadIdentity();
					glEnable( GL_TEXTURE_2D );

					glColor4f( 1, 1, 0, 1 );
					scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
					strncpy( message, textEffect->getText(), 255 );
					int x = ( scourge->getUserConfiguration()->getW() / 2 ) - ( scourge->getSDLHandler()->textWidth( message ) / 2 );
					int y = ( scourge->getUserConfiguration()->getH() / 2 );
					scourge->getSDLHandler()->texPrint( x, y, message );
					scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

					glDisable( GL_TEXTURE_2D );
					glPopMatrix();

					glDisable( GL_BLEND );
					glEnable( GL_DEPTH_TEST );
				}
			} else {
				textEffect->draw();
			}
		} else {
			delete textEffect;
			textEffect = NULL;
		}
	}
}

// check for interactive items.
Color *ScourgeView::getOutlineColor( Location *pos ) {
	Color *ret = NULL;
	if ( pos->item || pos->creature || ( pos->shape && pos->shape->isInteractive() ) ) {
		ret = outlineColor;
	} else if ( scourge->getMap()->isSecretDoor( pos ) ) {
		// try to detect the secret door
		if ( pos->z > 0 || scourge->getMap()->isSecretDoorDetected( pos ) ) {
			ret = outlineColor;
		}
	}
	return ret;
}

void ScourgeView::drawDisk( float w, float diff ) {
	float n = w * 2;

	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_CULL_FACE );

	glEnable( GL_TEXTURE_2D );
	scourge->getShapePalette()->getSelection().glBind();
	glBegin( GL_TRIANGLE_STRIP );
// glNormal3f( 0, 0, 1 );
	glTexCoord2i( 0, 0 );
	glVertex2f( -diff, -diff );
	glTexCoord2i( 1, 0 );
	glVertex2f( n + diff, -diff );
	glTexCoord2i( 0, 1 );
	glVertex2f( -diff, n + diff );
	glTexCoord2i( 1, 1 );
	glVertex2f( n + diff, n + diff );
	glEnd();
	glDisable( GL_TEXTURE_2D );
}

//#define BASE_DEBUG 1
void ScourgeView::showCreatureInfo( Creature *creature, bool player, bool selected, bool groupMode, bool wanderingHero ) {

	if ( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		return;
	}

	glPushMatrix();
	//showInfoAtMapPos(creature->getX(), creature->getY(), creature->getZ(), creature->getName());

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_FALSE );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_CULL_FACE );

	// draw circle
	double w = ( static_cast<double>( creature->getShape()->getWidth() ) / 2.0f ) * MUL;
	//double w = ((static_cast<double>(creature->getShape()->getWidth()) / 2.0f) + 1.0f) * MUL;
	double s = 0.45f;

	float xpos2, ypos2, zpos2;

	Uint32 t = SDL_GetTicks();
	if ( t - lastTargetTick > 45 ) {
		// initialize target width
		if ( targetWidth == 0.0f ) {
			targetWidth = s;
			targetWidthDelta *= -1.0f;
		}
		// targetwidth oscillation
		targetWidth += targetWidthDelta;
		if ( ( targetWidthDelta < 0 && targetWidth < -s ) || ( targetWidthDelta > 0 && targetWidth >= s ) )
			targetWidthDelta *= -1.0f;

		lastTargetTick = t;
	}

	// show path
	if ( !creature->getStateMod( StateMod::dead ) && ( ( PATH_DEBUG  ) || ( player && scourge->inTurnBasedCombat() ) ) ) {
		//glDisable( GL_DEPTH_TEST );
		std::vector<Location>* path = creature->getPathManager()->getPath();
		for ( std::vector<Location>::iterator i = path->begin() + creature->getPathManager()->getPositionOnPath(); i != path->end();
		        i++ ) {

			if ( player )
				glColor4f( 1, 0.4f, 0.0f, 0.5f );
			else
				glColor4f( 0, 0.4f, 1, 0.5f );

			xpos2 = ( static_cast<float>( i->x - scourge->getMap()->getX() ) * MUL );
			ypos2 = ( static_cast<float>( i->y - scourge->getMap()->getY() ) * MUL );
			zpos2 = scourge->getMap()->getGroundHeight( i->x / OUTDOORS_STEP, i->y / OUTDOORS_STEP ) * MUL;

			scourge->getMap()->getRender()->drawGroundTex( scourge->getShapePalette()->getNamedTexture( "path" ),
			                                  i->x + creature->getShape()->getWidth() / 2,
			                                  i->y - creature->getShape()->getWidth() / 2 - 1, 0.4f, 0.4f );

			/*
			   glPushMatrix();
			   //glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
			glTranslatef( xpos2 + w, ypos2 - w - 1 * MUL, zpos2 + 5);
			   gluDisk(quadric, 0, 4, 12, 1);
			   glPopMatrix();
			*/
		}
		//glEnable( GL_DEPTH_TEST );
	}

	// Yellow for move creature target
	if ( !creature->getStateMod( StateMod::dead ) && player && creature->getSelX() > -1 && !creature->getTargetCreature() &&
	        !( creature->getSelX() == toint( creature->getX() ) && creature->getSelY() == toint( creature->getY() ) ) ) {
		// draw target
		glColor4f( 1.0f, 0.75f, 0.0f, 0.5f );

		scourge->getMap()->getRender()->drawGroundTex( scourge->getShapePalette()->getSelection(), creature->getSelX() - targetWidth / 2,
		                                  creature->getSelY() + targetWidth / 2, creature->getShape()->getWidth() + targetWidth,
		                                  creature->getShape()->getDepth() + targetWidth );

		xpos2 = ( static_cast<float>( creature->getSelX() - scourge->getMap()->getX() ) * MUL );
		ypos2 = ( static_cast<float>( creature->getSelY() - scourge->getMap()->getY() ) * MUL );
		float groundHeight = scourge->getMap()->findMaxHeightPos( creature->getSelX(), creature->getSelY(), 0, true );
		zpos2 = groundHeight * MUL;
		glPushMatrix();
		glTranslatef( xpos2, ypos2 - w * 2 - 1 * MUL, zpos2 );

		glDisable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );
		glDepthMask( GL_FALSE );
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glDisable( GL_CULL_FACE );

		// in TB mode and paused?
		if ( scourge->inTurnBasedCombat() && !( scourge->getParty()->isRealTimeMode() ) ) {
			char cost[40];
			snprintf( cost, 40, "%s: %d", _( "Move" ), static_cast<int>( creature->getPathManager()->getPathRemainingSize() ) );
			scourge->getSDLHandler()->drawTooltip( 0, 0, 0, -( scourge->getMap()->getZRot() ), -( scourge->getMap()->getYRot() ),
			                                       cost, 0.5f, 0.2f, 0.0f, 1.0f / scourge->getMap()->getZoom() );
		}
		glPopMatrix();
		glEnable( GL_DEPTH_TEST );
	}

	// red for attack target
	if ( !creature->getStateMod( StateMod::dead ) && player && creature->getTargetCreature() &&
	        !creature->getTargetCreature()->getStateMod( StateMod::dead ) ) {
		//double tw = (static_cast<double>(creature->getTargetCreature()->getShape()->getWidth()) / 2.0f) * MUL;
		glColor4f( 1.0f, 0.15f, 0.0f, 0.5f );

		scourge->getMap()->getRender()->drawGroundTex( scourge->getShapePalette()->getSelection(),
		                                  creature->getTargetCreature()->getX(),
		                                  creature->getTargetCreature()->getY() + creature->getTargetCreature()->getShape()->getDepth() / 2.0f - 1,
		                                  creature->getTargetCreature()->getShape()->getWidth(),
		                                  creature->getTargetCreature()->getShape()->getDepth() );

	}

	xpos2 = ( creature->getX() - static_cast<float>( scourge->getMap()->getX() ) ) * MUL;
	ypos2 = ( creature->getY() - static_cast<float>( scourge->getMap()->getY() ) ) * MUL;
	float groundHeight = scourge->getMap()->findMaxHeightPos( creature->getX(), creature->getY(), creature->getZ(), true );
	zpos2 = groundHeight * MUL;
	//zpos2 = creature->getZ() * MUL;

	if ( creature->getAction() != Constants::ACTION_NO_ACTION ) {
		glColor4f( 0, 0.7f, 1, 0.5f );
	} else if ( selected ) {
		glColor4f( 0, 1, 1, 0.5f );
	} else if ( player ) {
		glColor4f( 0.0f, 1.0f, 0.0f, 0.5f );
	} else if ( creature->isNpc() ) {
		glColor4f( 0.75f, 1.0f, 0.0f, 0.5f );
	} else if ( wanderingHero ) {
		glColor4f( 0, 1.0f, 0.75f, 0.5f );
	} else {
		glColor4f( 0.7f, 0.7f, 0.7f, 0.25f );
	}

	if ( !creature->getStateMod( StateMod::dead ) && ( groupMode || player || !creature->isPartyMember() || wanderingHero ) ) {
		scourge->getMap()->getRender()->drawGroundTex( scourge->getShapePalette()->getSelection(),
		                                  creature->getX(), creature->getY() + creature->getShape()->getDepth() / 2.0f - 1,
		                                  creature->getShape()->getWidth(), creature->getShape()->getDepth() );
	}

	// draw state mods
	if ( !creature->getStateMod( StateMod::dead ) && ( groupMode || player || !creature->isPartyMember() || wanderingHero ) ) {
		glEnable( GL_TEXTURE_2D );
		int n = 16;
		int count = 0;
		Texture icon;
		char name[255];
		Color color;
		for ( int i = 0; i < StateMod::STATE_MOD_COUNT + 2; i++ ) {
			if ( scourge->getStateModIcon( &icon, name, &color, creature, i ) ) {
				glPushMatrix();
				glTranslatef( xpos2 + w, ypos2 - ( w * 2.0f ) - ( 1.0f * MUL ) + w, zpos2 + 5 );
				float angle = -( count * 30 ) - ( scourge->getMap()->getZRot() + 180 );

				glRotatef( angle, 0, 0, 1 );
				glTranslatef( w + 15, 0, 0 );
				glRotatef( ( count * 30 ) + 180, 0, 0, 1 );
				glTranslatef( -7, -7, 0 );

				icon.glBind();
//        glNormal3f( 0, 0, 1 );
				glBegin( GL_TRIANGLE_STRIP );
				if ( icon.isSpecified() ) glTexCoord2i( 0, 0 );
				glVertex2i( 0, 0 );
				if ( icon.isSpecified() ) glTexCoord2i( 1, 0 );
				glVertex2i( n, 0 );
				if ( icon.isSpecified() ) glTexCoord2i( 0, 1 );
				glVertex2i( 0, n );
				if ( icon.isSpecified() ) glTexCoord2i( 1, 1 );
				glVertex2i( n, n );
				glEnd();
				glPopMatrix();
				count++;
			}
		}
		glDisable( GL_TEXTURE_2D );
	}

	if ( !creature->getStateMod( StateMod::dead ) ) {

#ifdef BASE_DEBUG
		// base debug
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );
		glPushMatrix();
		glColor4f( 1, 1, 1, 1 );
		glTranslatef( xpos2, ypos2, zpos2 + 5 );
		glBegin( GL_LINE_LOOP );
		glVertex2i( 0, 0 );
		glVertex2i( w * 2, w * 2 );
		glVertex2i( w * 2, 0 );
		glVertex2i( 0, w * 2 );
		glEnd();
		char debugMessage[80];
		sprintf( debugMessage, "w: %d", creature->getShape()->getWidth() );
		scourge->getSDLHandler()->texPrint( 0, 0, debugMessage );
		glPopMatrix();
#endif

		/*
		  glPushMatrix();
		glTranslatef( xpos2, ypos2 - w * 2 - 1 * MUL, zpos2 + 5);
		*/
		if ( groupMode || player || !creature->isPartyMember() || wanderingHero ) {
			//gluDisk(quadric, w - s, w, 12, 1);
			//drawDisk( w, 0 );

			// in TB mode, player's turn and paused?
			if ( scourge->inTurnBasedCombatPlayerTurn() && !( scourge->getParty()->isRealTimeMode() ) ) {

				// draw the range
				if ( player ) {
					//glDisable( GL_DEPTH_TEST );

					Uint32 t = SDL_GetTicks();
					if ( areaTicks == 0 || t - areaTicks >= AREA_SPEED ) {
						areaRot += AREA_ROT_DELTA;
						if ( areaRot >= 360.0f ) areaRot -= 360.0f;
					}

					float range = scourge->getParty()->getPlayer()->getBattle()->getRange();
					//float n = ( ( MIN_DISTANCE + range + creature->getShape()->getWidth() ) * 2.0f ) * MUL;
					float nn = ( ( MIN_DISTANCE + range + creature->getShape()->getWidth() ) * 2.0f );

					glColor4f( 0.85f, 0.25f, 0.15f, 0.4f );
					scourge->getMap()->getRender()->drawGroundTex( scourge->getShapePalette()->getAreaTexture(),
					                                  creature->getX() - ( nn - creature->getShape()->getWidth() ) / 2.0f,
					                                  creature->getY() + ( nn - creature->getShape()->getWidth() ) / 2.0f,
					                                  nn, nn, areaRot );
					/*
					     glPushMatrix();

					     Uint32 t = SDL_GetTicks();
					     if( areaTicks == 0 || t - areaTicks >= AREA_SPEED ) {
					       areaRot += AREA_ROT_DELTA;
					       if( areaRot >= 360.0f ) areaRot -= 360.0f;
					     }
					float h = ( creature->getShape()->getWidth() / 2.0f ) * MUL;
					glTranslatef( h, h, 0 );
					     glRotatef( areaRot, 0, 0, 1 );
					     glTranslatef( -( n / 2 ), -( n / 2 ), 0 );

					     glEnable( GL_BLEND );
					     //glBlendFunc( GL_DST_COLOR, GL_ZERO );
					     glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
					     glEnable( GL_TEXTURE_2D );
					     glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getAreaTexture() );
					     glColor4f( 0.85f, 0.25f, 0.15f, 0.4f );
					     glBegin( GL_QUADS );
					     glNormal3f( 0, 0, 1 );
					     glTexCoord2f( 0, 0 );
					     glVertex3f( 0, 0, 0 );
					     glTexCoord2f( 0, 1 );
					     glVertex3f( 0, n, 0 );
					     glTexCoord2f( 1, 1 );
					     glVertex3f( n, n, 0 );
					     glTexCoord2f( 1, 0 );
					     glVertex3f( n, 0, 0 );
					     glEnd();
					     glDisable( GL_TEXTURE_2D );
					     glPopMatrix();
					     //glEnable( GL_DEPTH_TEST );
					*/
				}

				glDisable( GL_TEXTURE_2D );
				glEnable( GL_DEPTH_TEST );
				glDepthMask( GL_FALSE );
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glDisable( GL_CULL_FACE );

				xpos2 = ( static_cast<float>( creature->getX() - scourge->getMap()->getX() ) * MUL );
				ypos2 = ( static_cast<float>( creature->getY() - scourge->getMap()->getY() ) * MUL );
				zpos2 = scourge->getMap()->findMaxHeightPos( creature->getX(), creature->getY(), 0, true ) * MUL;
				glPushMatrix();
				glTranslatef( xpos2, ypos2 - w * 2 - 1 * MUL, zpos2 );

				char cost[40];
				Color color;
				if ( scourge->getParty()->getPlayer()->getBattle()->describeAttack( creature, cost, 40, &color, player ) ) {
					scourge->getSDLHandler()->drawTooltip( 0, 0, 0, -( scourge->getMap()->getZRot() ), -( scourge->getMap()->getYRot() ),
					                                       cost, color.r, color.g, color.b, 1.0f / scourge->getMap()->getZoom() );
					glEnable( GL_DEPTH_TEST );
				}

				glPopMatrix();
			}
		}
	}

	// draw recent damages
	glEnable( GL_TEXTURE_2D );
	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );


	// show recent damages
	glDisable( GL_DEPTH_TEST );
	const float maxPos = 10.0f;
	const Uint32 posSpeed = 70;
	const float posDelta = 0.3f;
	for ( int i = 0; i < creature->getRecentDamageCount(); i++ ) {
		DamagePos *dp = creature->getRecentDamage( i );
		xpos2 = static_cast<float>( creature->getX() + creature->getShape()->getWidth() / 2 - scourge->getMap()->getX() ) * MUL;
		ypos2 = static_cast<float>( creature->getY() - creature->getShape()->getDepth() / 2 - scourge->getMap()->getY() ) * MUL;
		zpos2 = ( static_cast<float>( creature->getShape()->getHeight() * 1.25f ) + dp->pos ) * MUL;
		glPushMatrix();
		//glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
		glTranslatef( xpos2, ypos2, zpos2 );
		// rotate each particle to face viewer
		glRotatef( -( scourge->getMap()->getZRot() ), 0.0f, 0.0f, 1.0f );
		glRotatef( -scourge->getMap()->getYRot(), 1.0f, 0.0f, 0.0f );

		float alpha = static_cast<float>( maxPos - dp->pos ) / ( maxPos * 0.75f );
		if ( creature->isMonster() ) {
			glColor4f( 0.75f, 0.75f, 0.75f, alpha );
		} else {
			glColor4f( 1.0f, 1.0f, 0, alpha );
		}
		scourge->getSDLHandler()->texPrint( 0, 0, "%d", dp->damage );

		glPopMatrix();

		Uint32 now = SDL_GetTicks();
		if ( now - dp->lastTime >= posSpeed ) {
			dp->pos += posDelta;
			dp->lastTime = now;
			if ( dp->pos >= maxPos ) {
				creature->removeRecentDamage( i );
				i--;
			}
		}
	}

	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

	glDisable( GL_TEXTURE_2D );
	glEnable( GL_DEPTH_TEST );

	glEnable( GL_CULL_FACE );
	glDisable( GL_BLEND );
	glDepthMask( GL_TRUE );

	// draw name
	//glTranslatef( 0, 0, 100);
	//getSDLHandler()->texPrint(0, 0, "%s", creature->getName());

	glPopMatrix();
}

void ScourgeView::drawAfter() {

	drawDraggedItem();

	// draw turn info
	if ( scourge->inTurnBasedCombat() ) {
		//glPushAttrib(GL_ENABLE_BIT);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 20, 20, 0 );
		Creature *c = scourge->getCurrentBattle()->getCreature();
		enum { MSG_SIZE = 80 };
		char msg[ MSG_SIZE ];
		if ( !c->getPathManager()->atEndOfPath() && !c->getBattle()->isInRangeOfTarget() ) {
			snprintf( msg, MSG_SIZE, "%s %d/%d (%s %d)", c->getName(), c->getBattle()->getAP(), c->getBattle()->getStartingAP(),
			          _( "cost" ), static_cast<int>( c->getPathManager()->getPathRemainingSize() ) );
		} else {
			snprintf( msg, MSG_SIZE, "%s %d/%d", c->getName(), c->getBattle()->getAP(), c->getBattle()->getStartingAP() );
		}
		turnProgress->updateStatus( msg, false, c->getBattle()->getAP(), c->getBattle()->getStartingAP(),
		                            c->getPathManager()->getPathRemainingSize() );
		glPopMatrix();
		//glPushAttrib(GL_ENABLE_BIT);
	}

	if ( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		scourge->getSession()->getCutscene()->drawLetterbox();

		// draw creature talk
		for ( int i = 0; i < scourge->getSession()->getCreatureCount(); i++ ) {
			showMovieConversation( scourge->getSession()->getCreature( i ) );
		}
		for ( int i = 0; i < scourge->getParty()->getPartySize(); i++ ) {
			showMovieConversation( scourge->getParty()->getParty( i ) );
		}
	}
}

void ScourgeView::showMovieConversation( Creature *creature ) {
	if ( creature->isTalking() ) {
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );
		glEnable( GL_TEXTURE_2D );

		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 20, scourge->getSession()->getCutscene()->getLetterboxHeight() + 30, 600 );
		creature->drawMoviePortrait( 100, 100 );
		glPopMatrix();


		vector<string> *lines = creature->getSpeechWrapped();
		if ( lines ) {
			scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
			glPushMatrix();
			glLoadIdentity();
			glDisable( GL_DEPTH_TEST );
			glEnable( GL_TEXTURE_2D );
			glTranslatef( 140, scourge->getSession()->getCutscene()->getLetterboxHeight() + 60, 600 );
			glColor4f( 1, 1, 0.75f, 1 );
			char tmp[3000];
			snprintf( tmp, 3000, "%s:", creature->getName() );
			scourge->getSDLHandler()->texPrint( 0, 0, tmp );
			glColor4f( 1, 1, 1, 1 );
			for ( unsigned int i = 0; i < lines->size(); i++ ) {
				scourge->getSDLHandler()->texPrint( 0, ( i + 1 ) * 32, ( *lines )[ i ].c_str() );
			}
			glPopMatrix();
			scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
		}

		glEnable( GL_DEPTH_TEST );
		glEnable( GL_CULL_FACE );
	}
}

void ScourgeView::drawDraggedItem() {
	if ( scourge->getMovingItem() ) {
		glDisable( GL_DEPTH_TEST );
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( scourge->getSDLHandler()->mouseX - 25, scourge->getSDLHandler()->mouseY - 25, 500 );
		//scourge->drawItemIcon( scourge->getMovingItem(), 32 );
		SDL_Rect rect;
		rect.x = rect.y = 0;
		rect.w = rect.h = 64;
		scourge->getMovingItem()->renderIcon( scourge, &rect );
		glPopMatrix();
		glEnable( GL_DEPTH_TEST );
	}
}
