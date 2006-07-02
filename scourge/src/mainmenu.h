/***************************************************************************
                          mainmenu.h  -  description
                             -------------------
    begin                : Tue Aug 12 2003
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

#ifndef MAINMENU_H
#define MAINMENU_H

#include "common/constants.h"
#include "sdleventhandler.h"
#include "sdlscreenview.h"
#include <vector>

/**
  *@author Gabor Torok
  */

class RenderedCreature;
class Creature;
class Scourge;
class PartyEditor;
class Window;
class Label;
class Button;
class ScrollingLabel;

typedef struct _MenuItemParticle {
  int life;
  float x, y;
  int r, g, b;
  float dir, step;
  float zoom;
} MenuItemParticle;

typedef struct _MenuItem {
  char text[80];
  GLuint texture[1];
  unsigned char *textureInMemory;
  int x;
  int y;
  bool active;
  int value;
  MenuItemParticle particle[100];
} MenuItem;

class MainMenu : public SDLEventHandler,SDLScreenView {
private:
  Scourge *scourge;
  int value;
  float logoRot, logoRotDelta;
  GLint logoTicks;
  GLint logoTicksDelta;
  int top, openingTop;
  Uint32 lastTick, lastTickMenu;
  int candleFlameX, candleFlameY;
  PartyEditor *partyEditor;
  bool initTextures;
  Window *aboutDialog;
  ScrollingLabel *aboutText;
  Button *aboutOK;

#define MAX_LOGOS 100
  typedef struct _LogoSprite {
	float x, y, angle, rot;
	int quadrant, steps;
  } LogoSprite;
  int logoSpriteCount;
  LogoSprite logoSprite[MAX_LOGOS];


  typedef struct _Cloud {
    int x, y, w, h, speed;
  } Cloud;
  Cloud cloud[100];
  int cloudCount;
  int starCount;
  typedef struct _Star {
    int x, y;
  } Star;
  Star star[500];

  Window *mainWin;
  Button *newGameButton;
  Button *continueButton;
  Button *multiplayer;
  Button *optionsButton;
  Button *aboutButton;
  Button *quitButton;

  Window *newGameConfirm;
  Button *newGameConfirmOK, *newGameConfirmCancel;

  static const char *menuText[];
  static const int values[];
  std::vector< MenuItem* > menuItemList;
  
public:
#define NEW_GAME 1
#define CONTINUE_GAME 2
#define OPTIONS 3
#define ABOUT 4
#define QUIT 5
#define MULTIPLAYER 6
#define MULTIPLAYER_START 7
#define NEW_GAME_START 8
#define EDITOR 9

  MainMenu(Scourge *scourge);
  ~MainMenu();

  void drawView();
  void drawAfter();
  bool handleEvent(SDL_Event *event);
  bool handleEvent(Widget *widget, SDL_Event *event);
  int getValue();
  void showNewGameConfirmationDialog();

  void show();
  void hide();
  bool isVisible();
  void showPartyEditor();
  void createParty( Creature **pc, int *partySize );
	RenderedCreature *createWanderingHero( int level );

 protected:
  void drawClouds(bool moveClouds, bool flipped);
  void drawWater();
  void drawLogo();
  void drawMenu();
  void drawActiveMenuItem( float divisor, int count );
  void buildTextures();
  void addLogoSprite();
  void drawLogoSprites();
  void moveLogoSprites();
  void drawParticles();
  void drawStars();
};

#endif
