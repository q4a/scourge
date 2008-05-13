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

Cutscene::Cutscene( Session *session ){
  this->session = session;
}

Cutscene::~Cutscene(){
}

void Cutscene::startCutsceneMode() {
  originalX = session->getMap()->getXPos();
  originalY = session->getMap()->getYPos();
  originalZ = session->getMap()->getZPos();

  originalXRot = session->getMap()->getXRot();
  originalYRot = session->getMap()->getYRot();
  originalZRot = session->getMap()->getZRot();

  originalZoom = session->getMap()->getZoom();

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

#define LETTERBOX_DURATION 3000

void Cutscene::updateCutscene() {
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

  if( ( letterboxStartTime > 0 ) && ( ( now - letterboxStartTime ) > LETTERBOX_DURATION ) ) {
    cutsceneFinished = true;
    letterboxStartTime = 0;
  } else {
    // TODO: calculate letterbox height
  }

  if( ( letterboxEndTime > 0 ) && ( ( now - letterboxEndTime ) > LETTERBOX_DURATION ) ) {
    cutsceneFinished = true;
    cutscenePlaying = false;
    letterboxEndTime = 0;
  } else {
    // TODO: calculate letterbox height
  }
}

void Cutscene::startLetterbox() {
  letterboxStartTime = SDL_GetTicks();
}

void Cutscene::endLetterbox() {
  letterboxEndTime = SDL_GetTicks();
}
