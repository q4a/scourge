/***************************************************************************
                          constants.cpp  -  description
                             -------------------
    begin                : Sun Oct 12 2003
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

#include "constants.h"

char rootDir[300];

//sprintf(s, "Welcome to Scourge version %7.2f", SCOURGE_VERSION);
char *Constants::messages[][80] = {
  { 
	"Infamy awaits in the dungeons of Scourge!", 
	"Another day, another sewer! Welcome to Scourge!", 
	"Happy hunting; welcome to Scourge!" },
  { "That item is out of your reach", 
	"You can't touch that", 
	"You have to be closer to get that", 
	"You are too far to reach it" },
{ "The door is blocked",
  "Something is blocking that door",
  "You can't use that door; something is in the way" }
};

int Constants::messageCount[] = {
  3, 4, 3
};

// opengl extension routines
PFNGLACTIVETEXTUREARBPROC glSDLActiveTextureARB = NULL;
PFNGLMULTITEXCOORD2FARBPROC glSDLMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2IARBPROC glSDLMultiTexCoord2iARB = NULL;

const char *Constants::SKILL_NAMES[] = {
  "SWORD_WEAPON",
  "AXE_WEAPON",
  "BOW_WEAPON",

  "SPEED",
  "COORDINATION",
  "POWER",
  "IQ",
  "LEADERSHIP",
  "LUCK",
  "PIETY",
  "LORE",

  "SHIELD_DEFEND",
  "ARMOR_DEFEND",
  "WEAPON_DEFEND",

  "MATERIAL_SPELL",
  "ILLUSION_SPELL",
  "PSYCHIC_SPELL",

  "OPEN_LOCK",
  "FIND_TRAP",
  "MOVE_UNDETECTED",

  "SKILL_0", "SKILL_1", "SKILL_2", "SKILL_3", "SKILL_4", "SKILL_5", "SKILL_6", "SKILL_7", "SKILL_8", "SKILL_9"
};

const char *Constants::STATE_NAMES[] = {
  "blessed", "empowered", "enraged", "ac_protected", "magic_protected",
  "drunk", "poisoned", "cursed", "possessed", "blinded", "charmed", "changed"
};


Constants::Constants(){
}

Constants::~Constants(){
}

char *Constants::getMessage(int index) {
  int n = (int)((float)messageCount[index] * rand() / RAND_MAX);
  return messages[index][n];
}
