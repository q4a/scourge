/***************************************************************************
                          guitheme.h  -  description
                             -------------------
    begin                : Thu Aug 28 2003
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

#ifndef GUI_THEME_H
#define GUI_THEME_H

#include "../constants.h"
#include "widget.h"
#include <map>

using namespace std;

class ShapePalette;

/**
  *@author Gabor Torok
  */

class ThemeElement {
 public:
  char textureFileName[40], north[40], south[40], east[40], west[40];
  GLuint texture, tex_north, tex_south, tex_east, tex_west;
  Color color;
  int width;

  ThemeElement() {}
  ~ThemeElement() {}

  void loadTextures( ShapePalette *shapePal );
};

class GuiTheme {
private:
  char *name;
  ThemeElement *windowBack;
  ThemeElement *windowTop;
  ThemeElement *windowBorder;
  Color *windowTitleText;
  Color *windowText;
  ThemeElement *buttonBackground;
  ThemeElement *buttonHighlight;
  ThemeElement *buttonBorder;
  Color *buttonText;
  ThemeElement *listBackground;
  ThemeElement *inputBackground;
  Color *inputText;
  ThemeElement *selectionBackground;
  Color *selectionText;

  static map<string, GuiTheme*> themes;

public: 
  GuiTheme( char *name );
  ~GuiTheme();

  static const char *DEFAULT_THEME;

  static void initThemes( ShapePalette *shapePal );
  static inline GuiTheme *getThemeByName( const char *name ) { 
	string s = name; 
	if( themes.find( s ) != themes.end() ) return themes[ s ]; 
	else {
	  cerr << "*** error: can't find theme: " << s << endl;
	  return NULL; 
	}
  }

  inline ThemeElement *getWindowBackground() { return windowBack; }
  inline ThemeElement *getWindowTop() { return windowTop; }
  inline ThemeElement *getWindowBorder() { return windowBorder; }
  inline Color *getWindowTitleText() { return windowTitleText; }
  inline Color *getWindowText() { return windowText; }  
  inline ThemeElement *getButtonBackground() { return buttonBackground; }
  inline ThemeElement *getButtonHighlight() { return buttonHighlight; }
  inline ThemeElement *getButtonBorder() { return buttonBorder; }
  inline Color *getButtonText() { return buttonText; }
  inline ThemeElement *getListBackground() { return listBackground; }
  inline ThemeElement *getInputBackground() { return inputBackground; }
  inline Color *getInputText() { return inputText; }
  inline ThemeElement *getSelectionBackground() { return selectionBackground; }
  inline Color *getSelectionText() { return selectionText; }


 protected:
  static ThemeElement *parseElement( char *line );
  static Color *parseColor( char *line );

  void loadTextures( ShapePalette *shapePal );
  
  inline void setWindowBackground( ThemeElement *element ) { this->windowBack = element; }
  inline void setWindowTop( ThemeElement *element ) { this->windowTop = element; }
  inline void setWindowBorder( ThemeElement *element ) { this->windowBorder = element; }
  inline void setWindowTitleText( Color *color ) { this->windowTitleText = color; }
  inline void setWindowText( Color *color ) { this->windowText = color; }
  inline void setButtonBackground( ThemeElement *element ) { this->buttonBackground = element; }
  inline void setButtonHighlight( ThemeElement *element ) { this->buttonHighlight = element; }
  inline void setButtonBorder( ThemeElement *element ) { this->buttonBorder = element; }
  inline void setButtonText( Color *color ) { this->buttonText = color; }
  inline void setListBackground( ThemeElement *element ) { this->listBackground = element; }
  inline void setInputBackground( ThemeElement *element ) { this->inputBackground = element; }
  inline void setInputText( Color *color ) { this->inputText = color; }
  inline void setSelectionBackground( ThemeElement *element ) { selectionBackground = element; }
  inline void setSelectionText( Color *color ) { this->selectionText = color; }
};

#endif

