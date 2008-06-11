/***************************************************************************
                          scourgeview.cpp  -  description
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
#include "render/cutscene.h"

using namespace std;  

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
  lastWeatherUpdate = SDL_GetTicks();
}

void ScourgeView::initUI() {
	quadric = gluNewQuadric();
	turnProgress = new Progress( scourge->getSDLHandler(), scourge->getSession()->getShapePalette()->getProgressTexture(),
                  scourge->getSession()->getShapePalette()->getProgressHighlightTexture(), 10, false, true, false );
}

ScourgeView::~ScourgeView() {
}

void ScourgeView::drawView() {
	if( scourge->getSession()->isShowingChapterIntro() ) {
		
		endScissorToMap();
		
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
	if( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		scourge->getSession()->getCutscene()->updateCameraPosition();
	}

  scourge->getMap()->preDraw();
  scourge->getMap()->draw();
  scourge->getMap()->postDraw();

  drawMapInfos();

  drawWeather();

  ambientObjectSounds();

  if( scourge->getSession()->getCutscene()->isInMovieMode() ) {
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

  // Hack: A container window may have been closed by hitting the Esc. button.
  if(Window::windowWasClosed) {
    scourge->removeClosedContainerGuis();
  }
}

// flip all the switches (seemingly at random) to try to restore the rendering pipeline...
void ScourgeView::endScissorToMap() {
	glEnable( GL_TEXTURE_2D );
  glDisable( GL_CULL_FACE );
  glDisable( GL_SCISSOR_TEST );
	glDisable( GL_BLEND );
  if( scourge->getPreferences()->getStencilbuf() && 
      scourge->getPreferences()->getStencilBufInitialized() ) {
  	glClear( GL_STENCIL_BUFFER_BIT );
  	glColorMask( 1, 1, 1, 1 );
  	glStencilFunc( GL_EQUAL, 1, 0xffffffff );
  	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
  	glDisable(GL_STENCIL_TEST);
  }
  glDepthMask( GL_TRUE );
	glDisable( GL_ALPHA_TEST );
	glColor4f( 1, 1, 1, 1 );
}

#define MAX_AMBIENT_OBJECT_DISTANCE 11
void ScourgeView::ambientObjectSounds() {
	// look for ambient sounds
	int xp = toint( scourge->getPlayer()->getX() + scourge->getPlayer()->getShape()->getWidth() / 2 );
	int yp = toint( scourge->getPlayer()->getY() - scourge->getPlayer()->getShape()->getDepth() / 2 );
	for( int xx = xp - MAX_AMBIENT_OBJECT_DISTANCE; xx < xp + MAX_AMBIENT_OBJECT_DISTANCE; xx++ ) {
		for( int yy = yp - MAX_AMBIENT_OBJECT_DISTANCE; yy < yp + MAX_AMBIENT_OBJECT_DISTANCE; yy++ ) {
			for( int zz = 0; zz < MAP_VIEW_HEIGHT; zz++ ) {
				Location *pos = scourge->getMap()->getPosition( xx, yy, zz );
				if( pos && pos->shape && ((GLShape*)pos->shape)->getAmbientName() != "" ) {
					float dist = Constants::distance(scourge->getPlayer()->getX(), scourge->getPlayer()->getY(), 
																					 scourge->getPlayer()->getShape()->getWidth(),
																					 scourge->getPlayer()->getShape()->getDepth(), 
																					 pos->x, pos->y, pos->shape->getWidth(), pos->shape->getDepth());
					if( dist <= MAX_AMBIENT_OBJECT_DISTANCE ) {
						float percent = 100 - ( dist / static_cast<float>(MAX_AMBIENT_OBJECT_DISTANCE) ) * 100.0f + 20;
						if( percent > 100 )
							percent = 100;
						int panning = scourge->getMap()->getPanningFromMapXY(pos->x, pos->y);
						scourge->getSession()->getSound()->playObjectSound( ((GLShape*)pos->shape)->getAmbientName(), toint( percent ), panning );

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
	glDisable(GL_TEXTURE_2D);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  
  //glEnable(GL_ALPHA_TEST);
  //glAlphaFunc(GL_NOTEQUAL, 0);
  glPushMatrix();
  glLoadIdentity( );
  glPixelZoom( 1.0, -1.0 );

	scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
	glColor4f( 1, 0.35f, 0, 1 );
	scourge->getSDLHandler()->texPrint( 10, 36, scourge->getSession()->getCurrentMission()->getDisplayName() );
	glColor4f( 1, 1, 1, 1 );
	
	int px = ( scourge->getScreenWidth() - scourge->getSession()->getChapterImageWidth() ) / 2;
	if( px < 0 )
		px = 0; // needed to show image in low resolution
	int py = 40;
	int textHeight = scourge->getScreenHeight() - scourge->getSession()->getChapterImageHeight() - py;

	// pos the text the first time
	if( scourge->getChapterTextPos() == -2000 ) {
		scourge->setChapterTextPos( textHeight );
	}

	glRasterPos2f( px, py );
	TextureData const& image = scourge->getSession()->getChapterImage();
	if( !image.empty() ) {
		glDrawPixels( scourge->getSession()->getChapterImageWidth(), scourge->getSession()->getChapterImageHeight(),
									GL_BGR, GL_UNSIGNED_BYTE, &image[0] );
	}

	scourge->getChapterIntroWin()->move( 0, scourge->getSession()->getChapterImageHeight() + py + 10 - 30 );

	glScissor( 150, 0, scourge->getScreenWidth() - 150, textHeight );
  glEnable( GL_SCISSOR_TEST );
	int offset = scourge->getChapterTextPos();
	glTranslatef( 160, ( scourge->getScreenHeight() - textHeight ), 0 );
	glColor4f( 1, 0.9f, 0.8f, 1 );
	for( unsigned int i = 0; i < scourge->getChapterText()->size(); i++ ) {
		string s = (*scourge->getChapterText())[i];
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
	glVertex2d( 0, size );
	glColor4f( 0, 0, 0, 1 );
	glVertex2d( 0, 0 );
	glColor4f( 0, 0, 0, 1 );
	glVertex2d( scourge->getScreenWidth() - 150, 0 );
	glColor4f( 0, 0, 0, 0 );
	glVertex2d( scourge->getScreenWidth() - 150, size );

	glColor4f( 0, 0, 0, 0 );
	glVertex2d( 0, textHeight - size );
	glColor4f( 0, 0, 0, 1 );
	glVertex2d( 0, textHeight );
	glColor4f( 0, 0, 0, 1 );
	glVertex2d( scourge->getScreenWidth() - 150, textHeight );
	glColor4f( 0, 0, 0, 0 );
	glVertex2d( scourge->getScreenWidth() - 150, textHeight - size );
	glEnd();
	glDisable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );

	glDisable( GL_SCISSOR_TEST );

	Uint32 now = SDL_GetTicks();
	if( now - chapterTextTimer > CHAPTER_TEXT_SPEED && offset > ( static_cast<int>( scourge->getChapterText()->size() ) * -36)) {
		scourge->setChapterTextPos( offset - CHAPTER_TEXT_DELTA );
		chapterTextTimer = now;
	}

  //glDisable(GL_ALPHA_TEST);
  glPopMatrix();
}

void ScourgeView::centerOnMonsterInTB() {
  scourge->getMap()->setMapCenterCreature( NULL );
  if( scourge->inTurnBasedCombat() ) {
    Battle *battle = scourge->getCurrentBattle();
    Creature *c = battle->getCreature();
    if( c->isMonster() || c->getStateMod( StateMod::possessed ) ) {
      scourge->getMap()->setMapCenterCreature( c );
      scourge->getMap()->center( toint( c->getX() ), toint( c->getY() ), true );
    }
  }
}

void ScourgeView::drawOutsideMap() {
  // cover the area outside the map
  if(scourge->getMap()->getViewWidth() < scourge->getSDLHandler()->getScreen()->w ||
     scourge->getMap()->getViewHeight() < scourge->getSDLHandler()->getScreen()->h) {
    //glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_CULL_FACE );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    glPushMatrix();
    glColor3f( 1, 1, 1 );
    glBindTexture( GL_TEXTURE_2D, scourge->getSession()->getShapePalette()->getGuiWoodTexture() );

    //    float TILE_W = 510 / 2.0f;
    float TILE_H = 270 / 2.0f;

    glLoadIdentity();
    glTranslatef( scourge->getMap()->getViewWidth(), 0, 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, scourge->getSDLHandler()->getScreen()->h / TILE_H );
    glVertex2i( 0, scourge->getSDLHandler()->getScreen()->h );
    glTexCoord2f( 1, scourge->getSDLHandler()->getScreen()->h / TILE_H );
    glVertex2i( scourge->getSDLHandler()->getScreen()->w - scourge->getMap()->getViewWidth(), scourge->getSDLHandler()->getScreen()->h );
    glTexCoord2f( 1, 0 );
    glVertex2i( scourge->getSDLHandler()->getScreen()->w - scourge->getMap()->getViewWidth(), 0 );
    glEnd();

    glLoadIdentity();
    glTranslatef( 0, scourge->getMap()->getViewHeight(), 0 );
    glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2i( 0, 0 );
    glTexCoord2f( 0, scourge->getMap()->getViewHeight() / TILE_H );
    glVertex2i( 0, scourge->getMap()->getViewHeight() );
    glTexCoord2f( 1, scourge->getMap()->getViewHeight() / TILE_H );
    glVertex2i( scourge->getMap()->getViewWidth(), scourge->getMap()->getViewHeight() );
    glTexCoord2f( 1, 0 );
    glVertex2i( scourge->getMap()->getViewWidth(), 0 );
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
  if( scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_NORMAL ||
      scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_ATTACK ||
      scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_RANGED ||
      scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_MOVE ||
      scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_TALK ||
      scourge->getSDLHandler()->getCursorMode() == Constants::CURSOR_USE ) {
    if( scourge->getSDLHandler()->mouseIsMovingOverMap ) {
      bool handled = false;
      mapx = scourge->getMap()->getCursorMapX();
      mapy = scourge->getMap()->getCursorMapY();
      mapz = scourge->getMap()->getCursorMapZ();
      if( mapx < MAP_WIDTH) {
        Location *pos = scourge->getMap()->getLocation(mapx, mapy, mapz);
				if( !pos )
					pos = scourge->getMap()->getItemLocation( mapx, mapy );
        else {
          int cursor;
          if( pos->creature && scourge->getParty()->getPlayer()->canAttack( pos->creature, &cursor ) ) {
            if(((Creature*)(pos->creature))->isMonster() && ((Creature*)(pos->creature))->getMonster()->isNpc())
              scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_TALK );
            else
              scourge->getSDLHandler()->setCursorMode( cursor );

            handled = true;
          } else if( getOutlineColor( pos ) ) {
            scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_USE );
            handled = true;
          }
        }
      }

			mapx = scourge->getMap()->getCursorFlatMapX();
      mapy = scourge->getMap()->getCursorFlatMapY();
			int trapIndex = scourge->getMap()->getTrapAtLoc( mapx, mapy + 2 );
			if( trapIndex > -1 ) {
				Trap *trap = scourge->getMap()->getTrapLoc( trapIndex );
				if( trap->discovered && trap->enabled ) {
					scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_USE );
					handled = true;
				}
			}

      if( !handled ) scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_NORMAL );
    }
  }

  if( scourge->getUserConfiguration()->getTooltipEnabled() && SDL_GetTicks() - scourge->getSDLHandler()->lastMouseMoveTime >
      (Uint32)( scourge->getUserConfiguration()->getTooltipInterval() * 10) ) {
    if(needToCheckInfo) {
      needToCheckInfo = false;

      // check location
      Uint16 mapx = scourge->getMap()->getCursorMapX();
      Uint16 mapy = scourge->getMap()->getCursorMapY();
      Uint16 mapz = scourge->getMap()->getCursorMapZ();
      if(mapx < MAP_WIDTH) {
        Location *pos = scourge->getMap()->getLocation(mapx, mapy, mapz);
				if( !pos )
					pos = scourge->getMap()->getItemLocation( mapx, mapy );
        else {
					std::string s;
          void *obj = NULL;
          if( pos->creature ) {
            obj = pos->creature;
            //((Creature*)(pos->creature))->getDetailedDescription( s );
            s = _( ((Creature*)(pos->creature))->getName() );
          } else if( pos->item ) {
            obj = pos->item;
            ((Item*)(pos->item))->getDetailedDescription( s );
          } else if( pos->shape ) {
            obj = pos->shape;
            s = scourge->getSession()->getShapePalette()->getRandomDescription( pos->shape->getDescriptionGroup() );
          }

          if( obj ) {
            bool found = false;
            // Don't show info about the same object twice
            // FIXME: use lookup table
            for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
              InfoMessage *message = i->first;
              if( message->obj == obj || ( message->x == pos->x && message->y == pos->y && message->z == pos->z ) ) {
                found = true;
                break;
              }
            }
            if( !found ) {
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
  for (map<InfoMessage *, Uint32>::iterator i = infos.begin(); i != infos.end(); ) {
    InfoMessage *message = i->first;
    Uint32 time = i->second;
    if( now - time > INFO_INTERVAL ) {
      infos.erase( i++ );
      delete message;
    } else {
      ++i;
    }
  }
}

void ScourgeView::resetInfos() {
  // http://www.velocityreviews.com/forums/t283023-stl-stdmap-erase.html
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ) {
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
  for(int i = 0; i < scourge->getSession()->getCreatureCount(); i++) {
    //if(!session->getCreature(i)->getStateMod(Constants::dead) &&
    if( scourge->getMap()->isLocationVisible(toint(scourge->getSession()->getCreature(i)->getX()),
                                             toint(scourge->getSession()->getCreature(i)->getY())) &&
        scourge->getMap()->isLocationInLight(toint(scourge->getSession()->getCreature(i)->getX()),
                                             toint(scourge->getSession()->getCreature(i)->getY()),
                                             scourge->getSession()->getCreature(i)->getShape())) {
			showCreatureInfo( scourge->getSession()->getCreature(i), false, false, false, 
												( scourge->getSession()->getCreature(i)->getCharacter() ? true : false ) );
    }
  }
  // party next so red target circle shows over gray
  for(int i = 0; i < scourge->getParty()->getPartySize(); i++) {
    bool player = scourge->getParty()->getPlayer() == scourge->getParty()->getParty(i);
    if( scourge->inTurnBasedCombat() && scourge->getParty()->isRealTimeMode() ) {
      player = ( scourge->getParty()->getParty(i) == scourge->getCurrentBattle()->getCreature() );
    }
    showCreatureInfo( scourge->getParty()->getParty(i), player, ( scourge->getMap()->getSelectedDropTarget() &&
                      scourge->getMap()->getSelectedDropTarget()->creature == scourge->getParty()->getParty(i) ),
                      !scourge->getParty()->isPlayerOnly(), false );
  }
}

void ScourgeView::drawInfos() {
	if( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		return;
	}

  float xpos2, ypos2, zpos2;
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {

    InfoMessage *message = i->first;
    xpos2 = (static_cast<float>(message->x - scourge->getMap()->getX()) / DIV);
    ypos2 = (static_cast<float>(message->y - scourge->getMap()->getY()) / DIV);
    zpos2 = (static_cast<float>(message->z) / DIV);

    scourge->getSDLHandler()->drawTooltip(xpos2, ypos2, zpos2, -(scourge->getMap()->getZRot()), -(scourge->getMap()->getYRot()),
																					 message->message, 0, 0.15f, 0.05f, 1.0f / scourge->getSession()->getMap()->getZoom() );
  }
}

void ScourgeView::checkForDropTarget() {
	if( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		return;
	}

  // find the drop target
  if( scourge->getMovingItem() ) {

    // is the mouse moving?
    if( !scourge->getSDLHandler()->mouseIsMovingOverMap ) {
      if( needToCheckDropLocation ) {
        needToCheckDropLocation = false;

        // check location
        Location *dropTarget = NULL;
        Uint16 mapx = scourge->getMap()->getCursorMapX();
        Uint16 mapy = scourge->getMap()->getCursorMapY();
        Uint16 mapz = scourge->getMap()->getCursorMapZ();
        if(mapx < MAP_WIDTH) {
          dropTarget = scourge->getMap()->getLocation(mapx, mapy, mapz);
          if(!(dropTarget && (dropTarget->creature || (dropTarget->item && 
             ((Item*)(dropTarget->item))->getRpgItem()->getType() == RpgItem::CONTAINER)))) {
            dropTarget = NULL;
          }
        }
        scourge->getMap()->setSelectedDropTarget(dropTarget);
      }
    } else {
      needToCheckDropLocation = true;
    }
  }
}

void ScourgeView::drawBorder() {
  if(scourge->getMap()->getViewWidth() == scourge->getSDLHandler()->getScreen()->w &&
     scourge->getMap()->getViewHeight() == scourge->getSDLHandler()->getScreen()->h &&
     !scourge->getUserConfiguration()->getFrameOnFullScreen()) return;

  glPushMatrix();
  glLoadIdentity();

  // ok change: viewx, viewy always 0
  //glTranslatef(viewX, viewY, 100);
  glTranslatef(0, 0, 100);

  //  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  // draw border
  glColor4f( 1, 1, 1, 1);

  int w = (scourge->getMap()->getViewWidth() == scourge->getSDLHandler()->getScreen()->w ?
           scourge->getMap()->getViewWidth() :
           scourge->getMap()->getViewWidth() - Window::SCREEN_GUTTER);
  int h = (scourge->getMap()->getViewHeight() == scourge->getSDLHandler()->getScreen()->h ?
           scourge->getMap()->getViewHeight() :
           scourge->getMap()->getViewHeight() - Window::SCREEN_GUTTER);
  float TILE_W = 20.0f;
  float TILE_H = 120.0f;

  glBindTexture( GL_TEXTURE_2D, scourge->getSession()->getShapePalette()->getBorderTexture() );
  glBegin( GL_QUADS );
  // left
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, 0);
  glTexCoord2f (0.0f, h/TILE_H);
  glVertex2i (0, h);
  glTexCoord2f (TILE_W/TILE_W, h/TILE_H);
  glVertex2i (static_cast<int>(TILE_W), h);
  glTexCoord2f (TILE_W/TILE_W, 0.0f);
  glVertex2i (static_cast<int>(TILE_W), 0);

  // right
  int gutter = 5;
  glTexCoord2f (TILE_W/TILE_W, 0.0f);
  glVertex2i (w - static_cast<int>(TILE_W) + gutter, 0);
  glTexCoord2f (TILE_W/TILE_W, h/TILE_H);
  glVertex2i (w - static_cast<int>(TILE_W) + gutter, h);
  glTexCoord2f (0.0f, h/TILE_H);
  glVertex2i (w + gutter, h);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (w + gutter, 0);
  glEnd();

  TILE_W = 120.0f;
  TILE_H = 20.0f;
  glBindTexture( GL_TEXTURE_2D, scourge->getSession()->getShapePalette()->getBorder2Texture() );
  glBegin( GL_QUADS );
  // top
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, 0);
  glTexCoord2f (0.0f, TILE_H/TILE_H);
  glVertex2i (0, static_cast<int>(TILE_H));
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (w, static_cast<int>(TILE_H));
  glTexCoord2f (w/TILE_W, 0.0f);
  glVertex2i (w, 0);

  // bottom
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (0, h - static_cast<int>(TILE_H) + gutter);
  glTexCoord2f (w/TILE_W, 0.0f);
  glVertex2i (0, h + gutter);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (w, h + gutter);
  glTexCoord2f (0.0f, TILE_H/TILE_H);
  glVertex2i (w, h - static_cast<int>(TILE_H) + gutter);
  glEnd();

  int gw = 220;
  int gh = 64;

  glEnable( GL_ALPHA_TEST );
  glAlphaFunc( GL_GREATER, 0 );
  glBindTexture( GL_TEXTURE_2D, scourge->getSession()->getShapePalette()->getGargoyleTexture() );

  glPushMatrix();
  glLoadIdentity();
  //glTranslatef(10, -5, 0);
  //glRotatef(20, 0, 0, 1);
  glBegin( GL_QUADS );
  // top left
  glTexCoord2f (0, 0);
  glVertex2i (0, 0);
  glTexCoord2f (0, 1);
  glVertex2i (0, gh);
  glTexCoord2f ((1.0f / gw) * (gw - 1), 1);
  glVertex2i (gw, gh);
  glTexCoord2f ((1.0f / gw) * (gw - 1), 0);
  glVertex2i (gw, 0);
  glEnd();
  glPopMatrix();

  // top right
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(w - gw, 0, 0);
  //glRotatef(-20, 0, 0, 1);
  glBegin( GL_QUADS );
  glTexCoord2f ((1.0f / gw) * (gw - 1), 0);
  glVertex2i (0, 0);
  glTexCoord2f ((1.0f / gw) * (gw - 1), 1);
  glVertex2i (0, gh);
  glTexCoord2f (0, 1);
  glVertex2i (gw, gh);
  glTexCoord2f (0, 0);
  glVertex2i (gw, 0);
  glEnd();

  glPopMatrix();

  //glEnable( GL_TEXTURE_2D );
  glDisable( GL_ALPHA_TEST );
  glEnable(GL_DEPTH_TEST);
  glPopMatrix();
}

bool ScourgeView::startTextEffect( char *message ) {
  if( !textEffect ) {
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
  if( textEffect ) {
    if( SDL_GetTicks() - textEffectTimer < 5000 ) {
      if( scourge->getUserConfiguration()->getFlaky() ) {
        // When we don't use a fancy TextEffect, use blinking normal text
	if( ( SDL_GetTicks() % 500 ) > 199 ) {
	  glDisable(GL_DEPTH_TEST);
	  glEnable(GL_BLEND);

	  glPushMatrix();
	  glLoadIdentity();
	  glEnable(GL_TEXTURE_2D);

	  glColor4f( 1, 1, 0, 1 );
	  scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
	  strncpy( message, textEffect->getText(), 255 );
	  int x = (scourge->getUserConfiguration()->getW() / 2) - (scourge->getSDLHandler()->textWidth(message) / 2);
	  int y = (scourge->getUserConfiguration()->getH() / 2);
	  scourge->getSDLHandler()->texPrint( x, y, message );
	  scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

	  glDisable(GL_TEXTURE_2D);
	  glPopMatrix();

	  glDisable(GL_BLEND);
	  glEnable(GL_DEPTH_TEST);
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
  if( pos->item || pos->creature || ( pos->shape && pos->shape->isInteractive() ) ) {
    ret = outlineColor;
  } else if( scourge->getMap()->isSecretDoor( pos ) ) {
    // try to detect the secret door
    if( pos->z > 0 || scourge->getMap()->isSecretDoorDetected( pos ) ) {
      ret = outlineColor;
    }
  }
  return ret;
}

void ScourgeView::drawDisk( float w, float diff ) {
	float n = w * 2;

	glEnable( GL_DEPTH_TEST );
  glDepthMask(GL_FALSE);
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glDisable( GL_CULL_FACE );

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getSelection() );
	glBegin( GL_QUADS );
	//glNormal3f( 0, 0, 1 );
	glTexCoord2f( 0, 0 );
	glVertex2f( -diff, -diff );
	glTexCoord2f( 0, 1 );
	glVertex2f( -diff, n + diff );
	glTexCoord2f( 1, 1 );
	glVertex2f( n + diff, n + diff );
	glTexCoord2f( 1, 0 );
	glVertex2f( n + diff, -diff );
	glEnd();
	glDisable( GL_TEXTURE_2D );
}

//#define BASE_DEBUG 1
void ScourgeView::showCreatureInfo( Creature *creature, bool player, bool selected, bool groupMode, bool wanderingHero ) {

	if( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		return;
	}

  glPushMatrix();
  //showInfoAtMapPos(creature->getX(), creature->getY(), creature->getZ(), creature->getName());

  glEnable( GL_TEXTURE_2D );
  glEnable( GL_DEPTH_TEST );
  glDepthMask(GL_FALSE);
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glDisable( GL_CULL_FACE );

  // draw circle
  double w = (static_cast<double>(creature->getShape()->getWidth()) / 2.0f) / DIV;
  //double w = ((static_cast<double>(creature->getShape()->getWidth()) / 2.0f) + 1.0f) / DIV;
  double s = 0.45f;

  float xpos2, ypos2, zpos2;

	Uint32 t = SDL_GetTicks();
	if(t - lastTargetTick > 45) {
		// initialize target width
		if(targetWidth == 0.0f) {
			targetWidth = s;
			targetWidthDelta *= -1.0f;
		}
		// targetwidth oscillation
		targetWidth += targetWidthDelta;
		if( ( targetWidthDelta < 0 && targetWidth < -s ) || ( targetWidthDelta > 0 && targetWidth >= s ) )
			targetWidthDelta *= -1.0f;

		lastTargetTick = t;
	}

  // show path
  if( !creature->getStateMod( StateMod::dead ) &&	( ( PATH_DEBUG  ) || ( player && scourge->inTurnBasedCombat() ) ) ) {
		//glDisable( GL_DEPTH_TEST );
		std::vector<Location>* path = creature->getPathManager()->getPath();
		for( std::vector<Location>::iterator i = path->begin() + creature->getPathManager()->getPositionOnPath(); i != path->end(); 
         i++) {

			if( player )
				glColor4f(1, 0.4f, 0.0f, 0.5f);
			else
				glColor4f(0, 0.4f, 1, 0.5f);

			xpos2 = (static_cast<float>(i->x - scourge->getMap()->getX()) / DIV);
			ypos2 = (static_cast<float>(i->y - scourge->getMap()->getY()) / DIV);
			zpos2 = scourge->getMap()->getGroundHeight( i->x / OUTDOORS_STEP, i->y / OUTDOORS_STEP ) / DIV;

			scourge->getMap()->drawGroundTex( scourge->getShapePalette()->getNamedTexture( "path" ),
																				i->x + creature->getShape()->getWidth() / 2, 
																				i->y - creature->getShape()->getWidth() / 2 - 1, 0.4f, 0.4f );

			/*
      glPushMatrix();
      //glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
			glTranslatef( xpos2 + w, ypos2 - w - 1 / DIV, zpos2 + 5);
      gluDisk(quadric, 0, 4, 12, 1);
      glPopMatrix();
			*/
    }
		//glEnable( GL_DEPTH_TEST );
  }

  // Yellow for move creature target
  if( !creature->getStateMod( StateMod::dead ) && player && creature->getSelX() > -1 && !creature->getTargetCreature() &&
      !(creature->getSelX() == toint(creature->getX()) && creature->getSelY() == toint(creature->getY())) ) {
    // draw target
    glColor4f(1.0f, 0.75f, 0.0f, 0.5f);

		scourge->getMap()->drawGroundTex( scourge->getShapePalette()->getSelection(), creature->getSelX() - targetWidth / 2,
																			creature->getSelY() + targetWidth / 2, creature->getShape()->getWidth() + targetWidth,
																			creature->getShape()->getDepth() + targetWidth );

    xpos2 = (static_cast<float>(creature->getSelX() - scourge->getMap()->getX()) / DIV);
    ypos2 = (static_cast<float>(creature->getSelY() - scourge->getMap()->getY()) / DIV);
		float groundHeight = scourge->getMap()->findMaxHeightPos( creature->getSelX(), creature->getSelY(), 0, true );
		zpos2 = groundHeight / DIV;
    glPushMatrix();
		glTranslatef( xpos2, ypos2 - w * 2 - 1 / DIV, zpos2 );

		glDisable( GL_TEXTURE_2D );
		glDisable( GL_DEPTH_TEST );
		glDepthMask(GL_FALSE);
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glDisable( GL_CULL_FACE );

    // in TB mode and paused?
    if( scourge->inTurnBasedCombat() && !( scourge->getParty()->isRealTimeMode() ) ) {
      char cost[40];
      snprintf( cost, 40, "%s: %d", _( "Move" ), static_cast<int>(creature->getPathManager()->getPathRemainingSize()) );
      scourge->getSDLHandler()->drawTooltip( 0, 0, 0, -( scourge->getMap()->getZRot() ), -( scourge->getMap()->getYRot() ),
                                             cost, 0.5f, 0.2f, 0.0f, 1.0f / scourge->getMap()->getZoom() );
    }
    glPopMatrix();
		glEnable( GL_DEPTH_TEST );
  }

  // red for attack target
  if( !creature->getStateMod( StateMod::dead ) && player && creature->getTargetCreature() &&
      !creature->getTargetCreature()->getStateMod( StateMod::dead ) ) {
    //double tw = (static_cast<double>(creature->getTargetCreature()->getShape()->getWidth()) / 2.0f) / DIV;
    glColor4f(1.0f, 0.15f, 0.0f, 0.5f);

		scourge->getMap()->drawGroundTex( scourge->getShapePalette()->getSelection(), creature->getTargetCreature()->getX(),
																			creature->getTargetCreature()->getY(), creature->getTargetCreature()->getShape()->getWidth(),
																			creature->getTargetCreature()->getShape()->getDepth() );
  }

  xpos2 = (creature->getX() - static_cast<float>(scourge->getMap()->getX())) / DIV;
  ypos2 = (creature->getY() - static_cast<float>(scourge->getMap()->getY())) / DIV;
	float groundHeight = scourge->getMap()->findMaxHeightPos( creature->getX(), creature->getY(), creature->getZ(), true );
	zpos2 = groundHeight / DIV;
	//zpos2 = creature->getZ() / DIV;

	if(creature->getAction() != Constants::ACTION_NO_ACTION) {
    glColor4f(0, 0.7f, 1, 0.5f);
  } else if(selected) {
    glColor4f(0, 1, 1, 0.5f);
  } else if(player) {
    glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
  } else if(creature->getMonster() && creature->getMonster()->isNpc()) {
    glColor4f(0.75f, 1.0f, 0.0f, 0.5f);
	} else if( wanderingHero ) {
		glColor4f( 0, 1.0f, 0.75f, 0.5f);
  } else {
    glColor4f(0.7f, 0.7f, 0.7f, 0.25f);
  }

	if( !creature->getStateMod( StateMod::dead ) && ( groupMode || player || creature->isMonster() || wanderingHero ) ) {
		scourge->getMap()->drawGroundTex( scourge->getShapePalette()->getSelection(), creature->getX(), creature->getY(),
																				creature->getShape()->getWidth(), creature->getShape()->getDepth() );
	}

  // draw state mods
  if( !creature->getStateMod( StateMod::dead ) && ( groupMode || player || creature->isMonster() || wanderingHero ) ) {
    glEnable(GL_TEXTURE_2D);
    int n = 16;
    int count = 0;
		GLuint icon;
		char name[255];
		Color color;
    for(int i = 0; i < StateMod::STATE_MOD_COUNT + 2; i++) {
			if( scourge->getStateModIcon( &icon, name, &color, creature, i ) ) {
        glPushMatrix();
        glTranslatef( xpos2 + w, ypos2 - ( w * 2.0f ) - ( 1.0f / DIV ) + w, zpos2 + 5);
        float angle = -(count * 30) - (scourge->getMap()->getZRot() + 180);

        glRotatef( angle, 0, 0, 1 );
        glTranslatef( w + 15, 0, 0 );
        glRotatef( (count * 30) + 180, 0, 0, 1 );
        glTranslatef( -7, -7, 0 );

				glBindTexture( GL_TEXTURE_2D, icon );
        glBegin( GL_QUADS );
        glNormal3f( 0, 0, 1 );
        if(icon) glTexCoord2f( 0, 0 );
        glVertex3f( 0, 0, 0 );
        if(icon) glTexCoord2f( 0, 1 );
        glVertex3f( 0, n, 0 );
        if(icon) glTexCoord2f( 1, 1 );
        glVertex3f( n, n, 0 );
        if(icon) glTexCoord2f( 1, 0 );
        glVertex3f( n, 0, 0 );
        glEnd();
        glPopMatrix();
        count++;
      }
    }
    glDisable(GL_TEXTURE_2D);
  }

  if( !creature->getStateMod( StateMod::dead ) ) {

#ifdef BASE_DEBUG
    // base debug
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_CULL_FACE );
    glPushMatrix();
    glColor4f( 1, 1, 1, 1 );
    glTranslatef( xpos2, ypos2, zpos2 + 5 );
    glBegin( GL_LINE_LOOP );
    glVertex2f( 0, 0 );
    glVertex2f( w * 2, w * 2 );
    glVertex2f( w * 2, 0 );
    glVertex2f( 0, w * 2 );
    glEnd();
    char debugMessage[80];
    sprintf( debugMessage, "w: %d", creature->getShape()->getWidth() );
    scourge->getSDLHandler()->texPrint( 0, 0, debugMessage );
    glPopMatrix();
#endif

		/*
    glPushMatrix();
		glTranslatef( xpos2, ypos2 - w * 2 - 1 / DIV, zpos2 + 5);
		*/
    if( groupMode || player || creature->isMonster() || wanderingHero ) {
      //gluDisk(quadric, w - s, w, 12, 1);
			//drawDisk( w, 0 );

      // in TB mode, player's turn and paused?
      if( scourge->inTurnBasedCombatPlayerTurn() && !( scourge->getParty()->isRealTimeMode() ) ) {

        // draw the range
        if( player ) {
          //glDisable( GL_DEPTH_TEST );

					Uint32 t = SDL_GetTicks();
          if( areaTicks == 0 || t - areaTicks >= AREA_SPEED ) {
            areaRot += AREA_ROT_DELTA;
            if( areaRot >= 360.0f ) areaRot -= 360.0f;
          }

          float range = scourge->getParty()->getPlayer()->getBattle()->getRange();
          //float n = ( ( MIN_DISTANCE + range + creature->getShape()->getWidth() ) * 2.0f ) / DIV;
					float nn = ( ( MIN_DISTANCE + range + creature->getShape()->getWidth() ) * 2.0f );

					glColor4f( 0.85f, 0.25f, 0.15f, 0.4f );
					scourge->getMap()->drawGroundTex( scourge->getShapePalette()->getAreaTexture(),
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
					float h = ( creature->getShape()->getWidth() / 2.0f ) / DIV;
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
				glDepthMask(GL_FALSE);
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glDisable( GL_CULL_FACE );

				xpos2 = (static_cast<float>(creature->getX() - scourge->getMap()->getX()) / DIV);
				ypos2 = (static_cast<float>(creature->getY() - scourge->getMap()->getY()) / DIV);
				zpos2 = scourge->getMap()->findMaxHeightPos( creature->getX(), creature->getY(), 0, true ) / DIV;
				glPushMatrix();
				glTranslatef( xpos2, ypos2 - w * 2 - 1 / DIV, zpos2 );
				
        char cost[40];
        Color color;
        if( scourge->getParty()->getPlayer()->getBattle()->describeAttack( creature, cost, 40, &color, player ) ) {
          glDisable( GL_DEPTH_TEST );
          scourge->getSDLHandler()->drawTooltip( 0, 0, 0, -( scourge->getMap()->getZRot() ), -( scourge->getMap()->getYRot() ),
                                                 cost, color.r, color.g, color.b, 1.0f / scourge->getMap()->getZoom() );
          glEnable( GL_DEPTH_TEST );
        }
				
				glPopMatrix();
      }
    }
  }

  // draw recent damages
  glEnable(GL_TEXTURE_2D);
  scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );


  // show recent damages
  glDisable(GL_DEPTH_TEST);
  const float maxPos = 10.0f;
  const Uint32 posSpeed = 70;
  const float posDelta = 0.3f;
  for( int i = 0; i < creature->getRecentDamageCount(); i++ ) {
    DamagePos *dp = creature->getRecentDamage( i );
    xpos2 = static_cast<float>( creature->getX() + creature->getShape()->getWidth() / 2 - scourge->getMap()->getX()) / DIV;
    ypos2 = static_cast<float>( creature->getY() - creature->getShape()->getDepth() / 2 - scourge->getMap()->getY()) / DIV;
    zpos2 = (static_cast<float>(creature->getShape()->getHeight() * 1.25f) + dp->pos ) / DIV;
    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    glTranslatef( xpos2, ypos2, zpos2 );
    // rotate each particle to face viewer
    glRotatef( -( scourge->getMap()->getZRot() ), 0.0f, 0.0f, 1.0f);
    glRotatef( -scourge->getMap()->getYRot(), 1.0f, 0.0f, 0.0f);

    float alpha = static_cast<float>( maxPos - dp->pos ) / ( maxPos * 0.75f );
    if( creature->isMonster() ) {
      glColor4f(0.75f, 0.75f, 0.75f, alpha );
    } else {
      glColor4f(1.0f, 1.0f, 0, alpha );
    }
    scourge->getSDLHandler()->texPrint( 0, 0, "%d", dp->damage );

    glPopMatrix();

    Uint32 now = SDL_GetTicks();
    if( now - dp->lastTime >= posSpeed ) {
      dp->pos += posDelta;
      dp->lastTime = now;
      if( dp->pos >= maxPos ) {
        creature->removeRecentDamage( i );
        i--;
      }
    }
  }

  scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );

  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);

  glEnable( GL_CULL_FACE );
  glDisable( GL_BLEND );
  glDisable( GL_DEPTH_TEST );
  glDepthMask(GL_TRUE);

  // draw name
  //glTranslatef( 0, 0, 100);
  //getSDLHandler()->texPrint(0, 0, "%s", creature->getName());

  glPopMatrix();
}

void ScourgeView::drawAfter() {

	drawDraggedItem();

	// draw turn info
	if( scourge->inTurnBasedCombat() ) {
		//glPushAttrib(GL_ENABLE_BIT);
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 20, 20, 0 );
		Creature *c = scourge->getCurrentBattle()->getCreature();
		enum { MSG_SIZE = 80 };
		char msg[ MSG_SIZE ];
		if( !c->getPathManager()->atEndOfPath() && !c->getBattle()->isInRangeOfTarget() ) {
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
	
	if( scourge->getSession()->getCutscene()->isInMovieMode() ) {
		scourge->getSession()->getCutscene()->drawLetterbox();

		// draw creature talk
		for( int i = 0; i < scourge->getSession()->getCreatureCount(); i++ ) {
			showMovieConversation( scourge->getSession()->getCreature( i ) );
		}
		for( int i = 0; i < scourge->getParty()->getPartySize(); i++ ) {
			showMovieConversation( scourge->getParty()->getParty( i ) );
		}
	}	
}

void ScourgeView::showMovieConversation( Creature *creature ) {
	if( creature->isTalking() ) {
		glDisable( GL_DEPTH_TEST );
		glDisable( GL_CULL_FACE );
		glEnable( GL_TEXTURE_2D );
		
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( 20, scourge->getSession()->getCutscene()->getLetterboxHeight() + 30, 600 );
		creature->drawMoviePortrait( 100, 100 );
		glPopMatrix();

		
		vector<string> *lines = creature->getSpeechWrapped();
		if( lines ) {
			scourge->getSDLHandler()->setFontType( Constants::SCOURGE_LARGE_FONT );
			glPushMatrix();
			glLoadIdentity();
			glDisable(GL_DEPTH_TEST);
			glEnable( GL_TEXTURE_2D );
			glTranslatef( 140, scourge->getSession()->getCutscene()->getLetterboxHeight() + 60, 600 );
			glColor4f( 1, 1, 0.75f, 1 );
			char tmp[3000];
			sprintf( tmp, "%s:", creature->getName() );
			scourge->getSDLHandler()->texPrint( 0, 0, tmp );
			glColor4f( 1, 1, 1, 1 );
			for( unsigned int i = 0; i < lines->size(); i++ ) {
				scourge->getSDLHandler()->texPrint( 0, ( i + 1 ) * 32, (*lines)[ i ].c_str() );
			}
			glPopMatrix();
			scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
		}

		glEnable( GL_DEPTH_TEST );
		glEnable( GL_CULL_FACE );
	}
}

void ScourgeView::drawDraggedItem() {
	if( scourge->getMovingItem() ) {
		glDisable( GL_DEPTH_TEST );
		glPushMatrix();
		glLoadIdentity();
		glTranslatef( scourge->getSDLHandler()->mouseX - 25, scourge->getSDLHandler()->mouseY - 25, 500);
		scourge->drawItemIcon( scourge->getMovingItem(), 32 );
		glPopMatrix();
		glEnable( GL_DEPTH_TEST );
	}
}

#define MIN_RAIN_DROP_COUNT 50
#define MIN_CLOUD_COUNT 10

void ScourgeView::drawWeather() {
	
	#define RAIN_DROP_SPEED 1200
	
	// Draw weather effects only on outdoor maps, when not inside a house
	bool shouldDrawWeather = scourge->getMap()->isHeightMapEnabled() && !scourge->getMap()->getCurrentlyUnderRoof();
	
	Uint32 now = SDL_GetTicks();
	
	int lightningTime = now - lastLightning;
	if ( lastLightningRoll == 0 ) lastLightningRoll = now;
	
	int screenW = scourge->getUserConfiguration()->getW();
	int screenWPlusMore = static_cast<int>( static_cast<float>( screenW ) * 1.25f );
	int screenH = scourge->getUserConfiguration()->getH();
	
	float deltaY;
	float deltaX;
	float cloudDelta;

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	
	glDepthMask(GL_FALSE);
	glEnable( GL_BLEND );
	
	// Draw the fog
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	if ( shouldDrawWeather && scourge->getMap()->getWeather() & WEATHER_FOG ) {
	    glPushMatrix();
	    glLoadIdentity();
	    glTranslatef( 0, 0, 500 );
	    glEnable( GL_TEXTURE_2D );
	    glColor4f( 0.6f, 0.6f, 0.6f, 0.5f );
	    glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getLightningTexture() );
	    glBegin( GL_QUADS );
	    glNormal3f( 0, 0, 1 );
	    glTexCoord2f( 0, 0 );
	    glVertex2i( 0, 0 );
	    glTexCoord2f( 1, 0 );
	    glVertex2i( screenW, 0 );
	    glTexCoord2f( 1, 1 );
	    glVertex2i( screenW, screenH );
	    glTexCoord2f( 0, 1 );
	    glVertex2i( 0, screenH );
	    glEnd();
	    glDisable( GL_TEXTURE_2D );
	
	    glPopMatrix();

	}
	
        if( scourge->getMap()->isHeightMapEnabled() && scourge->getMap()->getCurrentlyUnderRoof() ) {
          Mix_Volume(Constants::RAIN_CHANNEL, 40);
          Mix_Volume(Constants::AMBIENT_CHANNEL, 40);
        } else if( scourge->getMap()->isHeightMapEnabled() ) {
          Mix_Volume(Constants::RAIN_CHANNEL, 128);
          Mix_Volume(Constants::AMBIENT_CHANNEL, 128);
        }

	// Draw the rain drops
        //glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        if ( shouldDrawWeather && scourge->getMap()->getWeather() & WEATHER_RAIN ) {

        	deltaY = static_cast<int>( static_cast<float>( now - lastWeatherUpdate ) * ( static_cast<float>( RAIN_DROP_SPEED ) / 1000 ) );
        	deltaX = deltaY / 4;

        	int rainDropCount = (int)( RAIN_DROP_COUNT * ( 1.0f - scourge->getMap()->getZoomPercent() ) );
        	if( rainDropCount > RAIN_DROP_COUNT ) rainDropCount = RAIN_DROP_COUNT;
        	else if( rainDropCount < MIN_RAIN_DROP_COUNT ) rainDropCount = MIN_RAIN_DROP_COUNT;
        	
        	glPushMatrix();
          glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getRaindropTexture() );

            
	    for ( int i = 0; i < rainDropCount; i++ ) {
	    	if ( lightningTime < 501 && ( scourge->getMap()->getWeather() & WEATHER_THUNDER ) ) {
        	glColor4f( 1, 1, 1, rainDropZ[i] );
        } else {
        	//	      glColor4f( 0, 0.8f, 1, 0.5f );
        	glColor4f( 0, 0.5f, 0.7f, rainDropZ[i] );
        }
                glLoadIdentity();
	        glTranslatef( rainDropX[i], rainDropY[i], 0 );
	        glScalef( scourge->getMap()->getZoom(), scourge->getMap()->getZoom(), scourge->getMap()->getZoom() );
	        glRotatef( 15, 0, 0, 1 );
	        glEnable( GL_TEXTURE_2D );
	        glBegin( GL_QUADS );
	        glNormal3f( 0, 0, 1 );
	        glTexCoord2f( 0, 0 );
	        glVertex2i( 0, 0 );
	        glTexCoord2f( 1, 0 );
	        glVertex2i( RAIN_DROP_SIZE, 0 );
	        glTexCoord2f( 1, 1 );
	        glVertex2i( RAIN_DROP_SIZE, RAIN_DROP_SIZE );
	        glTexCoord2f( 0, 1 );
	        glVertex2i( 0, RAIN_DROP_SIZE );
	        glEnd();
	        glDisable( GL_TEXTURE_2D );
	
	        rainDropY[i] += (deltaY * rainDropZ[i]);
	        rainDropX[i] -= (deltaX * rainDropZ[i]);
	
	        if ( ( rainDropX[i] < -RAIN_DROP_SIZE ) || ( rainDropX[i] > screenWPlusMore ) || ( rainDropY[i] > screenH ) || ( rainDropY[i] < -screenH ) ) {
	            rainDropX[i] = Util::pickOne( -RAIN_DROP_SIZE, screenWPlusMore );
                    // Start new drops somewhere above the screen.
                    // It prevents them sometimes aligning in horizontal "waves".
	            rainDropY[i] = -Util::pickOne( RAIN_DROP_SIZE, screenH );
	        }

            glPopMatrix();

	    }
	}
	
        //Draw the fog clouds
        glBlendFunc( GL_ONE_MINUS_DST_COLOR, GL_ONE );
	if ( shouldDrawWeather && scourge->getMap()->getWeather() & WEATHER_FOG ) {

            glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->cloud );
            glColor4f( 0.6f, 0.6f, 0.4f, 0.5f );
            glPushMatrix();

            int cloudCount = (int)( CLOUD_COUNT * ( 1.0f - scourge->getMap()->getZoomPercent() ) );
            if( cloudCount > CLOUD_COUNT ) cloudCount = CLOUD_COUNT;
            else if( cloudCount < MIN_CLOUD_COUNT ) cloudCount = MIN_CLOUD_COUNT;

            for( int i = 0; i < cloudCount; i++ ) {

              cloudDelta = static_cast<float>( now - lastWeatherUpdate ) * ( static_cast<float>( cloudSpeed[i] ) / 1000 );

              glLoadIdentity();
              glTranslatef( cloudX[i], cloudY[i], 10 );
              glScalef( scourge->getMap()->getZoom(), scourge->getMap()->getZoom(), scourge->getMap()->getZoom() );
              glEnable( GL_TEXTURE_2D );
              glBegin( GL_QUADS );
              glNormal3f( 0.0f, 0.0f, 1.0f );
              glTexCoord2f( 1.0f, 1.0f );
              glVertex2f( cloudSize[i] * 256.0f, cloudSize[i] * 128.0f );
              glTexCoord2f( 1.0f, 0.0f );
              glVertex2f( cloudSize[i] * 256.0f, 0 );
              glTexCoord2f( 0.0f, 0.0f );
              glVertex2f( 0, 0 );
              glTexCoord2f( 0.0f, 1.0f );
              glVertex2f( 0, cloudSize[i] * 128.0f );
              glEnd();
              glDisable( GL_TEXTURE_2D );

	      cloudX[i] -= cloudDelta;
	
	      if ( cloudX[i] < -(256.0f * scourge->getMap()->getZoom() * cloudSize[i]) || cloudX[i] > (screenW * 2) || cloudY[i] < -(128.0f * cloudSize[i]) || cloudY[i] > screenH ) {
                cloudX[i] = Util::pickOne( screenW, screenW * 2 );
                cloudY[i] = Util::pickOne( -(int)( 128.0f * cloudSize[i] ), screenH );
                cloudSize[i] = 3.0f + ( Util::mt_rand() * 9 );
                cloudSpeed[i] = Util::pickOne( 20, 40 );
	      }

            }

        glPopMatrix();

        }

	// Draw the lightning
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	if ( lightningTime < 201 ) {
	
	    float brightness;
	
	    if ( shouldDrawWeather && scourge->getMap()->getWeather() & WEATHER_THUNDER ) {
	        if ( lightningTime < 101 ) {
	            brightness = ( (float)lightningTime / 100 ) * lightningBrightness;
	        } else {
	            brightness = ( ( 201 - (float)lightningTime ) / 100 ) * lightningBrightness;
	        }

	        glPushMatrix();
	        glLoadIdentity();
	        glTranslatef( 0, 0, 0 );
	        glEnable( GL_TEXTURE_2D );
	        glColor4f( 1, 1, 1, brightness );
	        glBindTexture( GL_TEXTURE_2D, scourge->getShapePalette()->getLightningTexture() );
	        glBegin( GL_QUADS );
	        glNormal3f( 0, 0, 1 );
	        glTexCoord2f( 0, 0 );
	        glVertex2i( 0, 0 );
	        glTexCoord2f( 1, 0 );
	        glVertex2i( screenW, 0 );
	        glTexCoord2f( 1, 1 );
	        glVertex2i( screenW, screenH );
	        glTexCoord2f( 0, 1 );
	        glVertex2i( 0, screenH );
	        glEnd();
	        glDisable( GL_TEXTURE_2D );
	
	        glPopMatrix();
	    }
	}
	
	if ( now > ( lastLightningRoll + 500 ) ) {
	    if ( Util::dice( 25 ) == 0 ) {
	        lastLightning = now;
                lightningBrightness = 0.3f + ( Util::mt_rand() * 0.5f );
	        if ( scourge->getMap()->isHeightMapEnabled() && scourge->getMap()->getWeather() & WEATHER_THUNDER ) {
                    int channel;
                    int volume = ( scourge->getMap()->getCurrentlyUnderRoof() ? 40 : 128 );
                    int thunderSound = Util::pickOne( 1, 4 );
	            if ( thunderSound == 1 ) {
	                channel = scourge->getSession()->getSound()->playSound( "thunder1", Util::pickOne( 41, 213 ) );
                        if( channel > -1 ) Mix_Volume( channel, volume );
	            } else if ( thunderSound == 2 ) {
	                channel = scourge->getSession()->getSound()->playSound( "thunder2", Util::pickOne( 41, 213 ) );
                        if( channel > -1 ) Mix_Volume( channel, volume );
	            } else if ( thunderSound == 3 ) {
	                channel = scourge->getSession()->getSound()->playSound( "thunder3", Util::pickOne( 41, 213 ) );
                        if( channel > -1 ) Mix_Volume( channel, volume );
	            } else if ( thunderSound == 4 ) {
	                channel = scourge->getSession()->getSound()->playSound( "thunder4", Util::pickOne( 41, 213 ) );
                        if( channel > -1 ) Mix_Volume( channel, volume );
	            }
	        }
	    }
	    lastLightningRoll = now;
	}
	
	lastWeatherUpdate = now;
	
	glEnable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	
}

void ScourgeView::generateRain() {
    for( int i = 0; i < RAIN_DROP_COUNT; i++ ) {
      rainDropX[i] = Util::pickOne( -RAIN_DROP_SIZE, scourge->getUserConfiguration()->getW() );
      rainDropY[i] = Util::pickOne( -RAIN_DROP_SIZE, scourge->getUserConfiguration()->getH() );
      rainDropZ[i] = Util::roll( 0.25f, 1.0f ); 
    }
}

void ScourgeView::generateClouds() {
    for( int i = 0; i < CLOUD_COUNT; i++ ) {
      cloudSize[i] = 3.0f + ( Util::mt_rand() * 9 );
      cloudSpeed[i] = Util::pickOne( 20, 40 );
      cloudX[i] = Util::pickOne( -(int)( 256.0f * cloudSize[i] ), scourge->getUserConfiguration()->getW() );
      cloudY[i] = Util::pickOne( -(int)( 128.0f * cloudSize[i] ), scourge->getUserConfiguration()->getH() );
    }
}
