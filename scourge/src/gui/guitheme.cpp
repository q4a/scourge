/***************************************************************************
                          guitheme.cpp  -  description
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

#include "guitheme.h"
#include "../shapepalette.h"

/**
  *@author Gabor Torok
  */

map<string, GuiTheme*> GuiTheme::themes;
const char *GuiTheme::DEFAULT_THEME = "default";

GuiTheme::GuiTheme( char *name ) {
  this->name = name;
  windowBack = NULL;
  windowTop = NULL;
  windowBorder = NULL;
  windowTitleText = NULL;
  windowText = NULL;
  buttonBackground = NULL;
  buttonSelectionBackground = NULL;
  buttonHighlight = NULL;
  buttonBorder = NULL;
  buttonText = NULL;
  buttonSelectionText = NULL;
  listBackground = NULL;
  inputBackground = NULL;
  inputText = NULL;
  selectionBackground = NULL;
  selectedBorder = NULL;
}

GuiTheme::~GuiTheme() {
  free( name );
  if( windowBack ) delete windowBack;
  if( windowTop ) delete windowTop;
  if( windowBorder ) delete windowBorder;
  if( windowTitleText ) delete windowTitleText;
  if( windowText ) delete windowText;
  if( buttonBackground ) delete buttonBackground;
  if( buttonSelectionBackground ) delete buttonSelectionBackground;
  if( buttonHighlight ) delete buttonHighlight;
  if( buttonBorder ) delete buttonBorder;
  if( buttonText ) delete buttonText;
  if( buttonSelectionText ) delete buttonSelectionText;
  if( listBackground ) delete listBackground;
  if( inputBackground ) delete inputBackground;
  if( inputText ) delete inputText;
  if( selectionBackground ) delete selectionBackground;
  if( selectedBorder ) delete selectedBorder;
}

void GuiTheme::initThemes( ShapePalette *shapePal ) {
  char errMessage[500];
  char s[200];
  sprintf(s, "%s/world/gui.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  //int itemCount = 0;
  char line[255];
  char name[255];
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'T') {
      // skip ':'
      fgetc(fp);
      // read the rest of the line
      n = Constants::readLine(name, fp);
      
      GuiTheme *theme = new GuiTheme( strdup( name ) );
      Color *color;
      ThemeElement *element;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setWindowBackground( element );
      else cerr << "Gui theme: " << name << " skipping window background" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setWindowTop( element );
      else cerr << "Gui theme: " << name << " skipping window top/bottom" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setWindowBorder( element );
      else cerr << "Gui theme: " << name << " skipping window border" << endl;
      
      n = Constants::readLine( line, fp );
      color = parseColor( line + 1 );
      if( color ) theme->setWindowTitleText( color );
      else cerr << "Gui theme: " << name << " skipping window title text color" << endl;
      
      n = Constants::readLine( line, fp );
      color = parseColor( line + 1 );
      if( color ) theme->setWindowText( color );
      else cerr << "Gui theme: " << name << " skipping window text color" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setButtonBackground( element );
      else cerr << "Gui theme: " << name << " skipping button background" << endl;

      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setButtonSelectionBackground( element );
      else cerr << "Gui theme: " << name << " skipping button selection background" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setButtonHighlight( element );
      else cerr << "Gui theme: " << name << " skipping button highlight" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setButtonBorder( element );
      else cerr << "Gui theme: " << name << " skipping button border" << endl;
      
      n = Constants::readLine( line, fp );
      color = parseColor( line + 1 );
      if( color ) theme->setButtonText( color );
      else cerr << "Gui theme: " << name << " skipping button text" << endl;

      n = Constants::readLine( line, fp );
      color = parseColor( line + 1 );
      if( color ) theme->setButtonSelectionText( color );
      else cerr << "Gui theme: " << name << " skipping button selection text" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setListBackground( element );
      else cerr << "Gui theme: " << name << " skipping list background" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setInputBackground( element );
      else cerr << "Gui theme: " << name << " skipping input background" << endl;
      
      n = Constants::readLine( line, fp );
      color = parseColor( line + 1 );
      if( color ) theme->setInputText( color );
      else cerr << "Gui theme: " << name << " skipping input text" << endl;
      
      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setSelectionBackground( element );
      else cerr << "Gui theme: " << name << " skipping selection background" << endl;
      
      n = Constants::readLine( line, fp );
      color = parseColor( line + 1 );
      if( color ) theme->setSelectionText( color );
      else cerr << "Gui theme: " << name << " skipping selection text" << endl;

      n = Constants::readLine( line, fp );
      element = parseElement( line + 1 );
      if( element ) theme->setSelectedBorder( element );
      else cerr << "Gui theme: " << name << " skipping selected border" << endl;

      theme->loadTextures( shapePal );
      
      string s = name;
      themes[name] = theme;
      
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

void GuiTheme::loadTextures( ShapePalette *shapePal ) {
  cerr << "----------------------------------------" << endl;
  cerr << "Loading gui theme: " << name << endl;
  if( windowBack ) windowBack->loadTextures( shapePal );
  if( windowTop ) windowTop->loadTextures( shapePal );
  if( windowBorder ) windowBorder->loadTextures( shapePal );
  if( buttonBackground ) buttonBackground->loadTextures( shapePal );
  if( buttonSelectionBackground ) buttonSelectionBackground->loadTextures( shapePal );
  if( buttonHighlight ) buttonHighlight->loadTextures( shapePal );
  if( buttonBorder ) buttonBorder->loadTextures( shapePal );
  if( listBackground ) listBackground->loadTextures( shapePal );
  if( inputBackground ) inputBackground->loadTextures( shapePal );
  cerr << "Done loading gui theme: " << name << endl;
  cerr << "----------------------------------------" << endl;
}

ThemeElement *GuiTheme::parseElement( char *line ) {
  cerr << "parseElement: line=" << line << endl;
  char *p = strtok( line, "," );
  if( p ) {
    ThemeElement *element = new ThemeElement();
    strcpy( element->textureFileName, p );
    strcat( element->textureFileName, ".bmp" );
    element->color.r = atof( strtok( NULL, "," ) );
    element->color.g = atof( strtok( NULL, "," ) );
    element->color.b = atof( strtok( NULL, "," ) );
    element->color.a = atof( strtok( NULL, "," ) );
    element->width = atoi( strtok( NULL, "," ) );
    p = strtok( NULL, "," );
    if( p ) {
      strcpy( element->north, p );
      strcat( element->north, ".bmp" );
    } else strcpy( element->north, "" );
    p = strtok( NULL, "," );
    if( p ) {
      strcpy( element->south, p );
      strcat( element->south, ".bmp" );
    } else strcpy( element->south, "" );
    p = strtok( NULL, "," );
    if( p ) {
      strcpy( element->east, p );
      strcat( element->east, ".bmp" );
    } else strcpy( element->east, "" );
    p = strtok( NULL, "," );
    if( p ) {
      strcpy( element->west, p );
      strcat( element->west, ".bmp" );
    } else strcpy( element->west, "" );
    return element;
  } else {
    return NULL;
  }
}

Color *GuiTheme::parseColor( char *line ) {
  cerr << "parseColor: line=" << line << endl;
  char *p = strtok( line, "," );
  if( p ) {
	Color *color = new Color();
	color->r = atof( p );
	color->g = atof( strtok( NULL, "," ) );
	color->b = atof( strtok( NULL, "," ) );
	color->a = atof( strtok( NULL, "," ) );
	return color;
  } else {
	return NULL;
  }
}

void ThemeElement::loadTextures( ShapePalette *shapePal ) {
  texture = shapePal->loadSystemTexture( textureFileName );
  if( strlen( north ) ) tex_north = shapePal->loadSystemTexture( north );
  if( strlen( south ) ) tex_south = shapePal->loadSystemTexture( south );
  if( strlen( east ) ) tex_east = shapePal->loadSystemTexture( east );
  if( strlen( west ) ) tex_west = shapePal->loadSystemTexture( west );
}   

