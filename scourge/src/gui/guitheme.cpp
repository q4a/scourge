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

using namespace std;

/**
  *@author Gabor Torok
  */

map<string, GuiTheme*> GuiTheme::themes;
char GuiTheme::DEFAULT_THEME[255] = "default";

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
  selectedCharacterBorder = NULL;
  windowBorderTexture = NULL;
}

GuiTheme::~GuiTheme() {
  free( name );
  delete windowBack;
  delete windowTop;
  delete windowBorder;
  delete windowTitleText;
  delete windowText;
  delete buttonBackground;
  delete buttonSelectionBackground;
  delete buttonHighlight;
  delete buttonBorder;
  delete buttonText;
  delete buttonSelectionText;
  delete listBackground;
  delete inputBackground;
  delete inputText;
  delete selectionBackground;
  delete selectedBorder;
  delete selectedCharacterBorder;
  delete windowBorderTexture;
}

void GuiTheme::initThemes( ScourgeGui *scourgeGui ) {
	ConfigLang *config = ConfigLang::load( "config/ui.cfg" );

	vector<ConfigNode*> *v = config->getDocument()->
		getChildrenByName( "themes" );
	vector<ConfigNode*> *vv = (*v)[0]->
		getChildrenByName( "theme" );

	for( unsigned int i = 0; i < vv->size(); i++ ) {
		ConfigNode *node = (*vv)[i];

		//config->setUpdate( "Loading UI themes", i, vv->size() );
  
		const char *name = node->getValueAsString( "name" );
		GuiTheme *theme = new GuiTheme( strdup( name ) );

		ThemeElement *element = parseElement( node->getValueAsString( "window_background" ) );
		if( element ) theme->setWindowBackground( element );
		else cerr << "Gui theme: " << name << " skipping window background" << endl;

		element = parseElement( node->getValueAsString( "window_top" ) );
		if( element ) theme->setWindowTop( element );
		else cerr << "Gui theme: " << name << " skipping window top/bottom" << endl;

		element = parseElement( node->getValueAsString( "window_border" ) );
		if( element ) theme->setWindowBorder( element );
		else cerr << "Gui theme: " << name << " skipping window border" << endl;

		Color *color = parseColor( node->getValueAsString( "window_title" ) );
		if( color ) theme->setWindowTitleText( color );
		else cerr << "Gui theme: " << name << " skipping window title text color" << endl;

		color = parseColor( node->getValueAsString( "window_text" ) );
		if( color ) theme->setWindowText( color );
		else cerr << "Gui theme: " << name << " skipping window text color" << endl;

		element = parseElement( node->getValueAsString( "button_background" ) );
		if( element ) theme->setButtonBackground( element );
		else cerr << "Gui theme: " << name << " skipping button background" << endl;

		element = parseElement( node->getValueAsString( "button_selection" ) );
		if( element ) theme->setButtonSelectionBackground( element );
		else cerr << "Gui theme: " << name << " skipping button selection background" << endl;

		element = parseElement( node->getValueAsString( "button_highlight" ) );
		if( element ) theme->setButtonHighlight( element );
		else cerr << "Gui theme: " << name << " skipping button highlight" << endl;

		element = parseElement( node->getValueAsString( "button_border" ) );
		if( element ) theme->setButtonBorder( element );
		else cerr << "Gui theme: " << name << " skipping button border" << endl;

		color = parseColor( node->getValueAsString( "button_text" ) );
		if( color ) theme->setButtonText( color );
		else cerr << "Gui theme: " << name << " skipping button text" << endl;

		color = parseColor( node->getValueAsString( "button_selection_text" ) );
		if( color ) theme->setButtonSelectionText( color );
		else cerr << "Gui theme: " << name << " skipping button selection text" << endl;

		element = parseElement( node->getValueAsString( "list_background" ) );
		if( element ) theme->setListBackground( element );
		else cerr << "Gui theme: " << name << " skipping list background" << endl;

		element = parseElement( node->getValueAsString( "input_background" ) );
		if( element ) theme->setInputBackground( element );
		else cerr << "Gui theme: " << name << " skipping input background" << endl;

		color = parseColor( node->getValueAsString( "input_text" ) );
		if( color ) theme->setInputText( color );
		else cerr << "Gui theme: " << name << " skipping input text" << endl;

		element = parseElement( node->getValueAsString( "selection_background" ) );
		if( element ) theme->setSelectionBackground( element );
		else cerr << "Gui theme: " << name << " skipping selection background" << endl;

		color = parseColor( node->getValueAsString( "selection_text" ) );
		if( color ) theme->setSelectionText( color );
		else cerr << "Gui theme: " << name << " skipping selection text" << endl;

		element = parseElement( node->getValueAsString( "selected_border" ) );
		if( element ) theme->setSelectedBorder( element );
		else cerr << "Gui theme: " << name << " skipping selected border" << endl;

		element = parseElement( node->getValueAsString( "selected_character_border" ) );
		if( element ) theme->setSelectedCharacterBorder( element );
		else cerr << "Gui theme: " << name << " skipping selected character border" << endl;

		element = parseElement( node->getValueAsString( "textured_window_border" ) );
		if( element ) theme->setWindowBorderTexture( element );
		else cerr << "Gui theme: " << name << " skipping window border texture" << endl;
		
		theme->loadTextures( scourgeGui );
		
		//string s = name;
		themes[name] = theme;
	}

  delete config;
}

void GuiTheme::loadTextures( ScourgeGui *scourgeGui ) {
//  cerr << "----------------------------------------" << endl;
//  cerr << "Loading gui theme: " << name << endl;
  if( windowBack ) windowBack->loadTextures( scourgeGui );
  if( windowTop ) windowTop->loadTextures( scourgeGui );
  if( windowBorder ) windowBorder->loadTextures( scourgeGui );
  if( buttonBackground ) buttonBackground->loadTextures( scourgeGui );
  if( buttonSelectionBackground ) buttonSelectionBackground->loadTextures( scourgeGui );
  if( buttonHighlight ) buttonHighlight->loadTextures( scourgeGui );
  if( buttonBorder ) buttonBorder->loadTextures( scourgeGui );
  if( listBackground ) listBackground->loadTextures( scourgeGui );
  if( inputBackground ) inputBackground->loadTextures( scourgeGui );
  if( windowBorderTexture ) windowBorderTexture->loadTextures( scourgeGui );
//  cerr << "Done loading gui theme: " << name << endl;
//  cerr << "----------------------------------------" << endl;
}

ThemeElement *GuiTheme::parseElement( const char *s ) {
//  cerr << "parseElement: line=" << line << endl;

	// need to copy incoming string so we don't modify shared string memory (std::string)
	std::vector<char> line(strlen(s)+1);
	strcpy( &line[0], s );


  char *p = strtok( &line[0], "," );
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
    if( p && strcmp(p,"none") ) {
	  element->rep_h = atoi( p );
    } else element->rep_h = 1;
	p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
	  element->rep_v = atoi( p );
    } else element->rep_v = 1;
    p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->north, p );
      strcat( element->north, ".bmp" );
    } else strcpy( element->north, "" );
    p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->south, p );
      strcat( element->south, ".bmp" );
    } else strcpy( element->south, "" );
    p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->east, p );
      strcat( element->east, ".bmp" );
    } else strcpy( element->east, "" );    
    p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->west, p );
      strcat( element->west, ".bmp" );
    } else strcpy( element->west, "" );
    p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->nw, p );
      strcat( element->nw, ".bmp" );
    } else strcpy( element->nw, "" );
    p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->ne, p );
      strcat( element->ne, ".bmp" );
    } else strcpy( element->ne, "" );
        p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->sw, p );
      strcat( element->sw, ".bmp" );
    } else strcpy( element->sw, "" );
    p = strtok( NULL, "," );
    if( p && strcmp(p,"none") ) {
      strcpy( element->se, p );
      strcat( element->se, ".bmp" );
    } else strcpy( element->se, "" );
    return element;
  } else {
    return NULL;
  }
}

Color *GuiTheme::parseColor( const char *s ) {
//  cerr << "parseColor: line=" << line << endl;

	// need to copy incoming string so we don't modify shared string memory (std::string)
	std::vector<char> line(strlen(s)+1);
	strcpy( &line[0], s );


	char *p = strtok( &line[0], "," );
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

void ThemeElement::loadTextures( ScourgeGui *scourgeGui ) {
  texture = scourgeGui->loadSystemTexture( textureFileName );
  if( strlen( north ) ) tex_north = scourgeGui->loadSystemTexture( north );
  if( strlen( south ) ) tex_south = scourgeGui->loadSystemTexture( south );
  if( strlen( east ) ) tex_east = scourgeGui->loadSystemTexture( east );
  if( strlen( west ) ) tex_west = scourgeGui->loadSystemTexture( west );
  if( strlen( nw ) ) tex_nw = scourgeGui->loadSystemTexture( nw );
  if( strlen( ne ) ) tex_ne = scourgeGui->loadSystemTexture( ne );
  if( strlen( sw ) ) tex_sw = scourgeGui->loadSystemTexture( sw );
  if( strlen( se ) ) tex_se = scourgeGui->loadSystemTexture( se );
}

