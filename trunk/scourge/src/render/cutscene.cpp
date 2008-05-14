/***************************************************************************
                          cutscene.h  -  description
                             -------------------
    begin                : Tue May 13 2008
    copyright            : (C) 2008 by Dennis Murczak
    email                : dmurczak@versanet.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cutscene.h"
#include "map.h"

using namespace std;

Cutscene::Cutscene( Session *session ){
  this->session = session;

  letterboxHeight = (int)( (float)session->getPreferences()->getH() / 8 );

  cutscenePlaying = false;
  cutsceneFinished = false;
  endingCutscene = false;
}

Cutscene::~Cutscene(){
}

void Cutscene::startCutsceneMode() {
  fromX = originalX = session->getMap()->getXPos();
  fromY = originalY = session->getMap()->getYPos();
  fromZ = originalZ = session->getMap()->getZPos();

  fromXRot = originalXRot = session->getMap()->getXRot();
  fromYRot = originalYRot = session->getMap()->getYRot();
  fromZRot = originalZRot = session->getMap()->getZRot();

  fromZoom = originalZoom = session->getMap()->getZoom();

  cutscenePlaying = true;
  cutsceneFinished = false;
  endingCutscene = false;

  cameraStartTime = cameraDuration = dialogLineStartTime = dialogLineDuration = doNothingStartTime = doNothingDuration = letterboxStartTime = letterboxEndTime = 0;

  startLetterbox();
}

void Cutscene::endCutsceneMode() {
  //TODO: restore pre-cutscene camera position

  endingCutscene = true;
  endLetterbox();
}

#define LETTERBOX_DURATION 1500

void Cutscene::updateCutscene() {
  if(!cutscenePlaying) return;

  Uint32 now = SDL_GetTicks();

  cutsceneFinished = false;

  if( ( cameraStartTime > 0 ) && ( cameraDuration > 0 ) && ( ( now - cameraStartTime ) > cameraDuration ) ) {
    cutsceneFinished = true;
    cameraStartTime = cameraDuration = 0;
  } else {
    // TODO: calculate camera position
  }

  if( ( dialogLineStartTime > 0 ) && ( dialogLineDuration > 0 ) && ( ( now - dialogLineStartTime ) > dialogLineDuration ) ) {
    cutsceneFinished = true;
    dialogLineStartTime = dialogLineDuration = 0;
  } else {
    // TODO: draw a floating text above the talking character
  }

  if( ( doNothingStartTime > 0 ) && ( doNothingDuration > 0 ) && ( ( now - doNothingStartTime ) > doNothingDuration ) ) {
    cutsceneFinished = true;
    doNothingStartTime = doNothingDuration = 0;
  }

  if( ( !endingCutscene ) && ( letterboxStartTime > 0 ) && ( ( now - letterboxStartTime ) > LETTERBOX_DURATION ) ) {
    cutsceneFinished = true;
    letterboxStartTime = 0;
  } else if( !endingCutscene ) {
    float percent = (float)( now - letterboxStartTime ) / LETTERBOX_DURATION;
    if( percent > 1.0f ) percent = 1.0f;
    currentLetterboxHeight = (int)( (float)letterboxHeight * percent );
  }

  if( ( endingCutscene ) && ( letterboxEndTime > 0 ) && ( ( now - letterboxEndTime ) > LETTERBOX_DURATION ) ) {
    cutsceneFinished = true;
    cutscenePlaying = false;
    endingCutscene = false;
    letterboxEndTime = 0;
  } else if( endingCutscene ) {
    float percent = (float)( now - letterboxEndTime ) / LETTERBOX_DURATION;
    if( percent > 1.0f ) percent = 1.0f;
    currentLetterboxHeight = (int)( (float)letterboxHeight * ( 1.0f - percent ) );
  }
}

void Cutscene::startLetterbox() {
  letterboxStartTime = SDL_GetTicks();
}

void Cutscene::endLetterbox() {
  letterboxEndTime = SDL_GetTicks();
}
