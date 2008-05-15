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

  inMovieMode = false;
  endingMovie = false;
  cameraMoving = false;
}

Cutscene::~Cutscene(){
}

void Cutscene::startMovieMode() {
  fromX = originalX = session->getMap()->getXPos();
  fromY = originalY = session->getMap()->getYPos();
  fromZ = originalZ = session->getMap()->getZPos();

  fromXRot = originalXRot = session->getMap()->getXRot();
  fromYRot = originalYRot = session->getMap()->getYRot();
  fromZRot = originalZRot = session->getMap()->getZRot();

  fromZoom = originalZoom = session->getMap()->getZoom();

  inMovieMode = true;
  endingMovie = false;
  cameraMoving = false;

  cameraStartTime = cameraDuration = letterboxStartTime = letterboxEndTime = 0;

  startLetterbox();
}

void Cutscene::endMovieMode() {
  // Initiate end of the movie. Note that movie mode does
  // not end until the letterbox has faded out completely.
  endingMovie = true;
  endLetterbox();
}

#define LETTERBOX_DURATION 1500

void Cutscene::startLetterbox() {
  letterboxStartTime = SDL_GetTicks();
}

void Cutscene::endLetterbox() {
  letterboxEndTime = SDL_GetTicks();
}

bool Cutscene::isInMovieMode() {
  if( inMovieMode && endingMovie ) {
    Uint32 now = SDL_GetTicks();
      if( now > ( letterboxEndTime + LETTERBOX_DURATION ) ) {
        inMovieMode = false; endingMovie = false;
        //TODO: restore pre-cutscene camera position
      }
  }

  return inMovieMode;
}

int Cutscene::getCurrentLetterboxHeight() {
  Uint32 now = SDL_GetTicks();
  int h;

  if( endingMovie ) {

    if( ( now - letterboxEndTime ) > LETTERBOX_DURATION ) {
      h = letterboxHeight;
    } else {
      float percent = (float)( now - letterboxEndTime ) / LETTERBOX_DURATION;
      h = (float)letterboxHeight * percent;
    }

  } else {

    if( ( now - letterboxStartTime ) > LETTERBOX_DURATION ) {
      h = letterboxHeight;
    } else {
      float percent = (float)( now - letterboxStartTime ) / LETTERBOX_DURATION;
      h = (float)letterboxHeight * ( 1.0f - percent );
    }

  }

  return h;
}

void Cutscene::placeCamera( float x, float y, float z, float xRot, float yRot, float zRot, float zoom ) {
  Uint32 now = SDL_GetTicks();

  fromX = toX = x;
  fromY = toY = y;
  fromZ = toZ = z;

  fromXRot = toXRot = xRot;
  fromYRot = toYRot = yRot;
  fromZRot = toZRot = zRot;

  fromZoom = toZoom = zoom;

  cameraStartTime = now;
  cameraDuration = 0;
  cameraMoving = false;
}

void Cutscene::animateCamera( float targetX, float targetY, float targetZ, float targetXRot, float targetYRot, float targetZRot, float targetZoom, Uint32 duration ) {
  Uint32 now = SDL_GetTicks();

  toX = targetX;
  toY = targetY;
  toZ = targetZ;

  toXRot = targetXRot;
  toYRot = targetYRot;
  toZRot = targetZRot;

  toZoom = targetZoom;

  cameraStartTime = now;
  cameraDuration = duration;
  cameraMoving = true;
}

bool Cutscene::isCameraMoving() {
  Uint32 now = SDL_GetTicks();

  if( cameraMoving ) {
    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
    }
  }

  return cameraMoving;
}

float Cutscene::getCameraX() {
  Uint32 now = SDL_GetTicks();
  float x;

  if( cameraMoving ) {

    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
      x = fromX;
    } else {
      float percent = (float)( now - cameraStartTime ) / cameraDuration;
      x = fromX + ( percent * ( toX - fromX ) );
    }

  } else {
    x = fromX;
  }

  return x;
}

float Cutscene::getCameraY() {
  Uint32 now = SDL_GetTicks();
  float y;

  if( cameraMoving ) {

    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
      y = fromY;
    } else {
      float percent = (float)( now - cameraStartTime ) / cameraDuration;
      y = fromY + ( percent * ( toY - fromY ) );
    }

  } else {
    y = fromY;
  }

  return y;
}

float Cutscene::getCameraZ() {
  Uint32 now = SDL_GetTicks();
  float z;

  if( cameraMoving ) {

    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
      z = fromZ;
    } else {
      float percent = (float)( now - cameraStartTime ) / cameraDuration;
      z = fromZ + ( percent * ( toZ - fromZ ) );
    }

  } else {
    z = fromZ;
  }

  return z;
}

float Cutscene::getCameraXRot() {
  Uint32 now = SDL_GetTicks();
  float r;

  if( cameraMoving ) {

    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
      r = fromXRot;
    } else {
      float percent = (float)( now - cameraStartTime ) / cameraDuration;
      // TODO: calculate the current angle
    }

  } else {
    r = fromXRot;
  }

  return r;
}

float Cutscene::getCameraYRot() {
  Uint32 now = SDL_GetTicks();
  float r;

  if( cameraMoving ) {

    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
      r = fromYRot;
    } else {
      float percent = (float)( now - cameraStartTime ) / cameraDuration;
      // TODO: calculate the current angle
    }

  } else {
    r = fromYRot;
  }

  return r;
}

float Cutscene::getCameraZRot() {
  Uint32 now = SDL_GetTicks();
  float r;

  if( cameraMoving ) {

    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
      r = fromZRot;
    } else {
      float percent = (float)( now - cameraStartTime ) / cameraDuration;
      // TODO: calculate the current angle
    }

  } else {
    r = fromZRot;
  }

  return r;
}

float Cutscene::getCameraZoom() {
  Uint32 now = SDL_GetTicks();
  float m;

  if( cameraMoving ) {

    if( ( now - cameraStartTime ) > cameraDuration ) {
      placeCamera( toX, toY, toZ, toXRot, toYRot, toZRot, toZoom );
      m = fromZoom;
    } else {
      float percent = (float)( now - cameraStartTime ) / cameraDuration;
      m = fromZoom + ( percent * ( toZoom - fromZoom ) );
    }

  } else {
    m = fromZoom;
  }

  return m;
}
