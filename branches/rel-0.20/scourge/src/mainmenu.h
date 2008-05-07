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
class SavegameDialog;
class Progress;
class TextEffect;

struct MenuItemParticle {
  int life;
  float x, y;
  int r, g, b;
  float dir, step;
  float zoom;
};

struct MenuItem {
  char text[80];
  GLuint texture[1];
  unsigned char *textureInMemory;
  int x;
  int y;
  bool active;
  int value;
  MenuItemParticle particle[100];
};

class MainMenu : public SDLEventHandler,SDLScreenView {
private:
  Scourge *scourge;
  int value;
  float logoRot, logoRotDelta;
  GLint logoTicks;
  GLint logoTicksDelta;
  int top, openingTop;
	bool musicStarted;
  Uint32 lastTick, lastTickMenu;
  int candleFlameX, candleFlameY;
  PartyEditor *partyEditor;
  bool initTextures;
  Window *aboutDialog;
  ScrollingLabel *aboutText;
  Button *aboutOK;
  int lastMenuTick; // this is when the program last drew the main menu (to make the sleep a bit more sensible) 

#define MAX_LOGOS 100
  struct LogoSprite {
	float x, y, angle, rot;
	int quadrant, steps;
  };
  int logoSpriteCount;
  LogoSprite logoSprite[MAX_LOGOS];


  struct Cloud {
    int x, y, w, h, speed;
  };
  Cloud cloud[100];
  int cloudCount;
  int starCount;
  struct Star {
    int x, y;
  };
  Star star[500];

	SavegameDialog *savegameDialog;

  Window *newGameConfirm;
  Button *newGameConfirmOK, *newGameConfirmCancel;

  static const char *menuText[];
  static const int values[];
  std::vector< MenuItem* > menuItemList;

	Progress *progress;

	std::vector<TextEffect*> textEffects;
  
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
	void showSavegameDialog( bool inSaveMode=true );

  void show();
  void hide();
  void showPartyEditor();
  void createParty( Creature **pc, int *partySize );
	RenderedCreature *createWanderingHero( int level );

 protected:
  void drawClouds(bool moveClouds, bool flipped);
  void drawWater();
  void drawLogo();
  void drawMenu();
  void drawStars();
};

#endif
