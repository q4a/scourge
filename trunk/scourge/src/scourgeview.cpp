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
  textEffect = NULL;
  textEffectTimer = 0;
  needToCheckDropLocation = true;
  targetWidth = 0.0f;
  targetWidthDelta = 0.05f / DIV;
  lastTargetTick = SDL_GetTicks();
}

void ScourgeView::initUI() {
quadric = gluNewQuadric();
  turnProgress = 
    new Progress( scourge->getSDLHandler(),
                  scourge->getSession()->getShapePalette()->getProgressTexture(),
                  scourge->getSession()->getShapePalette()->getProgressHighlightTexture(),
                  10, false, true, false );
}

ScourgeView::~ScourgeView() {
}

void ScourgeView::drawView() {
  // move inventory window with party window
  scourge->getInventory()->positionWindow();

  // make a move (player, monsters, etc.)
  scourge->playRound();

  scourge->updatePartyUI();

  checkForDropTarget();
  checkForInfo();

  // in TB combat, center map when monster's turn
  centerOnMonsterInTB();  

  scourge->getMap()->draw();
  scourge->getMiniMap()->drawMap();

  // the boards outside the map
  drawOutsideMap();
  
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_TEXTURE_2D );
  
  drawMapInfos();

  drawDescriptions( scourge->getMessageList() );

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_TEXTURE_2D );

  drawBorder();

  drawTextEffect();  

  // Hack: A container window may have been closed by hitting the Esc. button.
  if(Window::windowWasClosed) {
    scourge->removeClosedContainerGuis();
  }
}

void ScourgeView::centerOnMonsterInTB() {
  scourge->getMap()->setMapCenterCreature( NULL );
  if( scourge->inTurnBasedCombat() ) {
    Battle *battle = scourge->getCurrentBattle();
    Creature *c = battle->getCreature();
    if( c->isMonster() || c->getStateMod( Constants::possessed ) ) {
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
				if( !pos ) pos = scourge->getMap()->getItemLocation( mapx, mapy );
        if( pos ) {
          int cursor;
          if( pos->creature &&
              scourge->getParty()->getPlayer()->canAttack( pos->creature, &cursor ) ) {
            scourge->getSDLHandler()->setCursorMode( ((Creature*)(pos->creature))->isMonster() && ((Creature*)(pos->creature))->getMonster()->isNpc() ?
                                            Constants::CURSOR_TALK :
                                            cursor );
                                            //Constants::CURSOR_ATTACK );
            handled = true;
          } else if( getOutlineColor( pos ) ) {
            scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_USE );
            handled = true;
          }
        }
      }
      if( !handled ) scourge->getSDLHandler()->setCursorMode( Constants::CURSOR_NORMAL );
    }
  }

  if( scourge->getUserConfiguration()->getTooltipEnabled() &&
      SDL_GetTicks() - scourge->getSDLHandler()->lastMouseMoveTime >
      (Uint32)( scourge->getUserConfiguration()->getTooltipInterval() * 10) ) {
    if(needToCheckInfo) {
      needToCheckInfo = false;

      // check location
      Uint16 mapx = scourge->getMap()->getCursorMapX();
      Uint16 mapy = scourge->getMap()->getCursorMapY();
      Uint16 mapz = scourge->getMap()->getCursorMapZ();
      if(mapx < MAP_WIDTH) {
        Location *pos = scourge->getMap()->getLocation(mapx, mapy, mapz);
				if( !pos ) pos = scourge->getMap()->getItemLocation( mapx, mapy );
        if( pos ) {
          char s[300];
          void *obj = NULL;
          if( pos->creature ) {
            obj = pos->creature;
            ((Creature*)(pos->creature))->getDetailedDescription(s);
          } else if( pos->item ) {
            obj = pos->item;
            ((Item*)(pos->item))->getDetailedDescription(s);
          } else if( pos->shape ) {
            obj = pos->shape;
            strcpy( s, scourge->getSession()->getShapePalette()->getRandomDescription( pos->shape->getDescriptionGroup() ) );
          }
          if( obj ) {
            bool found = false;
            // Don't show info about the same object twice
            // FIXME: use lookup table
            for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {
              InfoMessage *message = i->first;
              if( message->obj == obj ||
                  ( message->x == pos->x &&
                    message->y == pos->y &&
                    message->z == pos->z ) ) {
                found = true;
                break;
              }
            }
            if( !found ) {
              InfoMessage *message =
                new InfoMessage( s, obj,
                                 pos->x + pos->shape->getWidth() / 2,
                                 pos->y - 1 - pos->shape->getDepth() / 2,
                                 pos->z + pos->shape->getHeight() / 2 );
                                 //pos->z + pos->shape->getHeight() );
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
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ) {
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
  glDisable( GL_CULL_FACE );
  glDisable( GL_SCISSOR_TEST );
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
    showCreatureInfo( scourge->getParty()->getParty(i),
                      player,
                      ( scourge->getMap()->getSelectedDropTarget() &&
                        scourge->getMap()->getSelectedDropTarget()->creature == scourge->getParty()->getParty(i) ),
                      !scourge->getParty()->isPlayerOnly(),
											false );
  }
}

void ScourgeView::drawInfos() {
  float xpos2, ypos2, zpos2;
  for (map<InfoMessage *, Uint32>::iterator i=infos.begin(); i!=infos.end(); ++i) {

    InfoMessage *message = i->first;
    xpos2 = ((float)(message->x - scourge->getMap()->getX()) / DIV);
    ypos2 = ((float)(message->y - scourge->getMap()->getY()) / DIV);
    zpos2 = ((float)(message->z) / DIV);

    scourge->getSDLHandler()->drawTooltip( xpos2, ypos2, zpos2,
																					 -( scourge->getMap()->getZRot() ),
																					 -( scourge->getMap()->getYRot() ),
																					 message->message,
																					 0, 0.15f, 0.05f,
																					 1.0f / scourge->getSession()->getMap()->getZoom() );
  }
}

void ScourgeView::checkForDropTarget() {
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
          if(!(dropTarget &&
               (dropTarget->creature ||
                (dropTarget->item &&
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

void ScourgeView::drawDescriptions(ScrollingList *list) {
  if( scourge->getMap()->didDescriptionsChange()) {
    scourge->getMap()->setDescriptionsChanged( false );
    list->setLines( scourge->getMap()->getDescriptionCount(),
                    scourge->getMap()->getDesriptions(),
                    scourge->getMap()->getDesriptionColors() );
    list->setSelectedLine( scourge->getMap()->getDescriptionCount() - 1);
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
  glVertex2i ((int)TILE_W, h);
  glTexCoord2f (TILE_W/TILE_W, 0.0f);
  glVertex2i ((int)TILE_W, 0);

  // right
  int gutter = 5;
  glTexCoord2f (TILE_W/TILE_W, 0.0f);
  glVertex2i (w - (int)TILE_W + gutter, 0);
  glTexCoord2f (TILE_W/TILE_W, h/TILE_H);
  glVertex2i (w - (int)TILE_W + gutter, h);
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
  glVertex2i (0, (int)TILE_H);
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (w, (int)TILE_H);
  glTexCoord2f (w/TILE_W, 0.0f);
  glVertex2i (w, 0);

  // bottom
  glTexCoord2f (w/TILE_W, TILE_H/TILE_H);
  glVertex2i (0, h - (int)TILE_H + gutter);
  glTexCoord2f (w/TILE_W, 0.0f);
  glVertex2i (0, h + gutter);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (w, h + gutter);
  glTexCoord2f (0.0f, TILE_H/TILE_H);
  glVertex2i (w, h - (int)TILE_H + gutter);
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
  // draw the current text effect
  if( textEffect ) {
    if( SDL_GetTicks() - textEffectTimer < 5000 ) {
      textEffect->draw();
    } else {
      delete textEffect;
      textEffect = NULL;
    }
  }
}

// check for interactive items.
Color *ScourgeView::getOutlineColor( Location *pos ) {
  Color *ret = NULL;
  if( pos->item || pos->creature || pos->shape->isInteractive() ) {
    ret = outlineColor;
  } else if( scourge->getMap()->isSecretDoor( pos ) ) {
    // try to detect the secret door
    if( pos->z > 0 || scourge->getMap()->isSecretDoorDetected( pos ) ) {
      ret = outlineColor;
    } else if( scourge->getParty()->getPlayer()->rollSecretDoor( pos ) ) {
      scourge->getMap()->setSecretDoorDetected( pos );
      ret = outlineColor;
    }
  }
  return ret;
}

void ScourgeView::drawDisk( float w, float diff ) {
	float n = w * 2;
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
  glPushMatrix();
  //showInfoAtMapPos(creature->getX(), creature->getY(), creature->getZ(), creature->getName());

  glEnable( GL_DEPTH_TEST );
  glDepthMask(GL_FALSE);
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glDisable( GL_CULL_FACE );

  // draw circle
  double w = ((double)(creature->getShape()->getWidth()) / 2.0f) / DIV;
  //double w = (((double)(creature->getShape()->getWidth()) / 2.0f) + 1.0f) / DIV;
  double s = 0.15f / DIV;

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
    if( ( targetWidthDelta < 0 && targetWidth < -s ) ||
				( targetWidthDelta > 0 && targetWidth >= s ) )
      targetWidthDelta *= -1.0f;

    lastTargetTick = t;
  }

  // show path
  if( !creature->getStateMod( Constants::dead ) &&
			( ( PATH_DEBUG && !creature->isMonster() ) ||
				( player && scourge->inTurnBasedCombat() ) ) ) {
    for( int i = creature->getPathIndex();
         i < (int)creature->getPath()->size(); i++) {
      Location pos = (*(creature->getPath()))[i];

			if( player ) {
				glColor4f(1, 0.4f, 0.0f, 0.5f);
			} else {
				glColor4f(0, 0.4f, 1, 0.5f);
			}
      xpos2 = ((float)(pos.x - scourge->getMap()->getX()) / DIV);
      ypos2 = ((float)(pos.y - scourge->getMap()->getY()) / DIV);
      zpos2 = 0.0f / DIV;
      glPushMatrix();
      glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
      gluDisk(quadric, 0, 4, 12, 1);
      glPopMatrix();
    }
  }

  // Yellow for move creature target
  if( !creature->getStateMod( Constants::dead ) &&
      player && creature->getSelX() > -1 &&
      !creature->getTargetCreature() &&
      !(creature->getSelX() == toint(creature->getX()) &&
        creature->getSelY() == toint(creature->getY())) ) {
    // draw target
    glColor4f(1.0f, 0.75f, 0.0f, 0.5f);
    xpos2 = ((float)(creature->getSelX() - scourge->getMap()->getX()) / DIV);
    ypos2 = ((float)(creature->getSelY() - scourge->getMap()->getY()) / DIV);
    zpos2 = 0.0f / DIV;
    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    //glTranslatef( xpos2 + w, ypos2 - w, zpos2 + 5);
		glTranslatef( xpos2, ypos2 - w * 2 - 1 / DIV, zpos2 + 5);
    //gluDisk(quadric, w - targetWidth, w, 12, 1);
		drawDisk( w, targetWidth );
		

    // in TB mode and paused?
    if( scourge->inTurnBasedCombat() && !( scourge->getParty()->isRealTimeMode() ) ) {
      char cost[40];
      sprintf( cost, "Move: %d", (int)(creature->getPath()->size()) );
      scourge->getSDLHandler()->drawTooltip( 0, 0, 0,
                                             -( scourge->getMap()->getZRot() ),
                                             -( scourge->getMap()->getYRot() ),
                                             cost,
                                             0.5f, 0.2f, 0.0f,
																						 1.0f / scourge->getMap()->getZoom() );
    }
    glPopMatrix();
  }

  // red for attack target
  if( !creature->getStateMod( Constants::dead ) &&
      player &&
      creature->getTargetCreature() &&
      !creature->getTargetCreature()->getStateMod( Constants::dead ) ) {
    double tw = ((double)creature->getTargetCreature()->getShape()->getWidth() / 2.0f) / DIV;
    //double td = (((double)(creature->getTargetCreature()->getShape()->getWidth()) / 2.0f) + 1.0f) / DIV;
    //double td = ((double)(creature->getTargetCreature()->getShape()->getDepth())) / DIV;
    glColor4f(1.0f, 0.15f, 0.0f, 0.5f);
    xpos2 = ((float)(creature->getTargetCreature()->getX() - scourge->getMap()->getX()) / DIV);
    ypos2 = ((float)(creature->getTargetCreature()->getY() - scourge->getMap()->getY()) / DIV);
    zpos2 = 0.0f / DIV;
    glPushMatrix();
    //glTranslatef( xpos2 + tw, ypos2 - tw * 2, zpos2 + 5);
    //glTranslatef( xpos2 + tw, ypos2 - td, zpos2 + 5);
		glTranslatef( xpos2, ypos2 - tw * 2 - 1 / DIV, zpos2 + 5);
    //gluDisk(quadric, tw - targetWidth, tw, 12, 1);
		drawDisk( tw, targetWidth );

    glPopMatrix();
  }

  xpos2 = (creature->getX() - (float)(scourge->getMap()->getX())) / DIV;
  ypos2 = (creature->getY() - (float)(scourge->getMap()->getY())) / DIV;
  zpos2 = creature->getZ() / DIV;

  if(creature->getAction() != Constants::ACTION_NO_ACTION) {
    glColor4f(0, 0.7, 1, 0.5f);
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

  // draw state mods
  if( !creature->getStateMod( Constants::dead ) &&
      ( groupMode || player || creature->isMonster() || wanderingHero )) {
    glEnable(GL_TEXTURE_2D);
    int n = 16;
    int count = 0;
    for(int i = 0; i < Constants::STATE_MOD_COUNT + 2; i++) {

			GLuint icon = 255;
			if( !creature->isMonster() &&
					!wanderingHero &&
					i == Constants::STATE_MOD_COUNT && 
					creature->getThirst() <= 5 ) {
				icon = scourge->getSession()->getShapePalette()->getThirstIcon();
			} else if( !creature->isMonster() && 
								 !wanderingHero &&
								 i == Constants::STATE_MOD_COUNT + 1 &&
								 creature->getHunger() <= 5 ) {
				icon = scourge->getSession()->getShapePalette()->getHungerIcon();
			} else if( creature->getStateMod( i ) && i != Constants::dead ) {
				icon = scourge->getSession()->getShapePalette()->getStatModIcon( i );
			}

			if( icon < 255 ) {
        glPushMatrix();
        glTranslatef( xpos2 + w,
                      ypos2 - ( w * 2.0f ) - ( 1.0f / DIV ) + w,
                      zpos2 + 5);
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

  if( !creature->getStateMod( Constants::dead ) ) {

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

    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    //glTranslatef( xpos2 + w, ypos2 - d, zpos2 + 5);
		glTranslatef( xpos2, ypos2 - w * 2 - 1 / DIV, zpos2 + 5);
    if( groupMode || player || creature->isMonster() || wanderingHero ) {
      //gluDisk(quadric, w - s, w, 12, 1);
			drawDisk( w, 0 );

      // in TB mode, player's turn and paused?
      if( scourge->inTurnBasedCombatPlayerTurn() && !( scourge->getParty()->isRealTimeMode() ) ) {

        // draw the range
        if( player ) {
          //glDisable( GL_DEPTH_TEST );

          float range = scourge->getParty()->getPlayer()->getBattle()->getRange();
          float n = ( ( MIN_DISTANCE + range + creature->getShape()->getWidth() ) * 2.0f ) / DIV;

          glPushMatrix();

          Uint32 t = SDL_GetTicks();
          if( areaTicks == 0 || t - areaTicks >= AREA_SPEED ) {
            areaRot += AREA_ROT_DELTA;
            if( areaRot >= 360.0f ) areaRot -= 360.0f;
          }
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
        }

        char cost[40];
        Color color;
        if( scourge->getParty()->getPlayer()->getBattle()->describeAttack( creature, cost, &color, player ) ) {
          glDisable( GL_DEPTH_TEST );
          scourge->getSDLHandler()->drawTooltip( 0, 0, 0,
                                                 -( scourge->getMap()->getZRot() ),
                                                 -( scourge->getMap()->getYRot() ),
                                                 cost,
                                                 color.r, color.g, color.b,
																								 1.0f / scourge->getMap()->getZoom() );
          glEnable( GL_DEPTH_TEST );
        }
      }
    }
    glPopMatrix();
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
    xpos2 = ((float)( creature->getX() +
                      creature->getShape()->getWidth() / 2 -
                      scourge->getMap()->getX()) / DIV);
    ypos2 = ((float)( creature->getY() -
                      creature->getShape()->getDepth() / 2 -
                      scourge->getMap()->getY()) / DIV);
    zpos2 = ( (float)(creature->getShape()->getHeight() * 1.25f) + dp->pos ) / DIV;
    glPushMatrix();
    //glTranslatef( xpos2 + w, ypos2 - w * 2, zpos2 + 5);
    glTranslatef( xpos2, ypos2, zpos2 );
    // rotate each particle to face viewer
    glRotatef( -( scourge->getMap()->getZRot() ), 0.0f, 0.0f, 1.0f);
    glRotatef( -scourge->getMap()->getYRot(), 1.0f, 0.0f, 0.0f);

    float alpha = (float)( maxPos - dp->pos ) / ( maxPos * 0.75f );
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
  glDisable(GL_TEXTURE_2D);
  scourge->getSDLHandler()->setFontType( Constants::SCOURGE_DEFAULT_FONT );
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
    char msg[80];
    if( c->getPath()->size() > 0 && !c->getBattle()->isInRangeOfTarget() ) {
      sprintf( msg, "%s %d/%d (cost %d)",
               c->getName(),
               c->getBattle()->getAP(),
               c->getBattle()->getStartingAP(),
               (int)( c->getPath()->size() ) );
    } else {
      sprintf( msg, "%s %d/%d",
               c->getName(),
               c->getBattle()->getAP(),
               c->getBattle()->getStartingAP() );
    }
    turnProgress->updateStatus( msg, false,
                                c->getBattle()->getAP(),
                                c->getBattle()->getStartingAP(),
                                c->getPath()->size() );
    glPopMatrix();
    //glPushAttrib(GL_ENABLE_BIT);
  }
}

void ScourgeView::drawDraggedItem() {
  if( scourge->getMovingItem() ) {
    glDisable( GL_DEPTH_TEST );
    glPushMatrix();
    glLoadIdentity();
    glTranslatef( scourge->getSDLHandler()->mouseX - 25, 
                  scourge->getSDLHandler()->mouseY - 25, 
                  500);
    scourge->drawItemIcon( scourge->getMovingItem(), 32 );
    glPopMatrix();
    glEnable( GL_DEPTH_TEST );
  }
}
