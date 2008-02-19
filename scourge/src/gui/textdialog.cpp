/***************************************************************************
                          textdialog.cpp  -  description
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

#include "textdialog.h"
#include "window.h"
#include "button.h"
#include "scrollinglabel.h"

#define TEXT_DIALOG_WIDTH 400
#define TEXT_DIALOG_HEIGHT 270

TextDialog::TextDialog( ScourgeGui *scourgeGui, char const* title ) {
  win = new Window( scourgeGui,
										( scourgeGui->getScreenWidth() / 2 ) - ( TEXT_DIALOG_WIDTH / 2 ),
										( scourgeGui->getScreenHeight() / 2 ) - ( TEXT_DIALOG_HEIGHT / 2 ),
										TEXT_DIALOG_WIDTH, TEXT_DIALOG_HEIGHT,
					(char*)( title ? title : _("Information") ),
										scourgeGui->getGuiTexture(), true, 
										Window::BASIC_WINDOW,
										scourgeGui->getGuiTexture2() );
  int mx = TEXT_DIALOG_WIDTH / 2;
  okButton = new Button( mx - 30, TEXT_DIALOG_HEIGHT - 55, 
  											 mx + 30, TEXT_DIALOG_HEIGHT - 35, 
  											 scourgeGui->getHighlightTexture(), "Ok" );
  win->addWidget( (Widget*)okButton );
  
  label = new ScrollingLabel( 8, 0, 
  												TEXT_DIALOG_WIDTH - 18, TEXT_DIALOG_HEIGHT - 60, 
  												"" );  
  win->addWidget( (Widget*)label );
}

TextDialog::~TextDialog() {
	delete win;
}
	
void TextDialog::setText( char *text ) {
	label->setText( text );
}

void TextDialog::setVisible( bool b ) {
	win->setVisible( b );
}

bool TextDialog::isVisible() {
	return win->isVisible();
}

