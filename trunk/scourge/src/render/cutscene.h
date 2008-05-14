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

#ifndef CUTSCENE_H
#define CUTSCENE_H

#include "../session.h"

class Session;

class Cutscene {
private:
  Session *session;

  Uint32 cameraStartTime, cameraDuration;
  Uint32 dialogLineStartTime, dialogLineDuration;
  Uint32 doNothingStartTime, doNothingDuration;
  Uint32 letterboxStartTime, letterboxEndTime;

  int letterboxHeight, currentLetterboxHeight;

  void startLetterbox();
  void endLetterbox();

  // camera state
  float originalX, fromX, currentX, toX;
  float originalY, fromY, currentY, toY;
  float originalZ, fromZ, currentZ, toZ;

  float originalXRot, fromXRot, currentXRot, toXRot;
  float originalYRot, fromYRot, currentYRot, toYRot;
  float originalZRot, fromZRot, currentZRot, toZRot;

  float originalZoom, fromZoom, currentZoom, toZoom;

  // true if we are in cutscene mode (black bars visible)
  bool cutscenePlaying;
  // false if some cutscene action is still playing
  bool cutsceneFinished;
  bool endingCutscene;

public:
  Cutscene( Session *session );
  ~Cutscene();

  // scene setup
  void startCutsceneMode();
  void placeActor( Creature *actor, int x, int y, int facingDirection );
  void placeCamera( float x, float y, float z, float xRot, float yRot, float zRot, float zoom );
  void endCutsceneMode();

  // choreography
  void animateCamera( float targetX, float targetY, float targetZ, float targetXRot, float targetYRot, float targetZRot, float targetZoom, Uint32 duration );
  void startDialogLine( Creature *actor, std::string *text, Uint32 duration ); //TODO: requires a "floating text" widget that takes a creature as a parameter
  void doNothing( Uint32 duration );

  // cuts
  void fade( Uint32 duration );

  // scene information
  void updateCutscene(); // update all information about the scene
  inline int getCurrentLetterboxHeight() { return currentLetterboxHeight; }
  inline bool isCutscenePlaying() { return cutscenePlaying; }
  inline bool isCutsceneFinished() { return cutsceneFinished; }
  inline float getCurrentX() { return currentX; }
  inline float getCurrentY() { return currentY; }
  inline float getCurrentZ() { return currentZ; }
  inline float getCurrentXRot() { return currentXRot; }
  inline float getCurrentYRot() { return currentYRot; }
  inline float getCurrentZRot() { return currentZRot; }
  inline float getCurrentZoom() { return currentZoom; }
};

#endif
