/***************************************************************************
                        window.cpp  -  Window manager
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
#include "../common/constants.h"
#include "window.h"
#include "../render/texture.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

//#define DEBUG_WINDOWS  
  
#define OPEN_STEPS 10

// should come from theme
#define INSET 8

// #define DEBUG_SCISSOR 1

const char Window::ROLL_OVER_SOUND[80] = "sound/ui/roll.wav";
const char Window::ACTION_SOUND[80] = "sound/ui/press.wav";
const char Window::DROP_SUCCESS[80] = "sound/equip/equip1.wav";
const char Window::DROP_FAILED[80] = "sound/equip/cant_equip.wav";

//#define DEBUG_WINDOWS

int Window::windowCount = 0;
Window *Window::window[MAX_WINDOW];
Window *Window::currentWin = NULL;
Window *Window::mouseLockWindow = NULL;
Widget *Window::mouseLockWidget = NULL;
/**
  *@author Gabor Torok
  */

#define CLOSE_BUTTON_SIZE 10

Window *Window::message_dialog = NULL;
Label *Window::message_label = NULL;
Button *Window::message_button = NULL;
bool Window::windowWasClosed = false;

Window::Window( ScourgeGui *scourgeGui, int x, int y, int w, int h, char const* title, bool hasCloseButton, int type, const char *themeName ) : Widget( x, y, w, h ) {
	theme = GuiTheme::getThemeByName( themeName );
	commonInit( scourgeGui, x, y, w, h, title, hasCloseButton, type );
}

Window::Window( ScourgeGui *scourgeGui, int x, int y, int w, int h, char const* title, Texture const& texture, bool hasCloseButton, int type, Texture const& texture2 ) : Widget( x, y, w, h ) {
	theme = GuiTheme::getThemeByName( GuiTheme::DEFAULT_THEME );
	/*
	this->texture = texture;
	this->texture2 = texture2;
	background.r = 1.0f;
	background.g = 0.85f;
	background.b = 0.5f;
	*/
	commonInit( scourgeGui, x, y, w, h, title, hasCloseButton, type );
}

void Window::commonInit( ScourgeGui *scourgeGui, int x, int y, int w, int h, char const* title, bool hasCloseButton, int type ) {
	this->escapeHandler = NULL;
	this->opening = false;
	this->animation = DEFAULT_ANIMATION;
	this->lastWidget = NULL;
	this->scourgeGui = scourgeGui;
	//  this->title = title;
	setTitle( title );
	this->visible = false;
	this->modal = false;
	this->widgetCount = 0;
	this->dragging = false;
	this->dragX = this->dragY = 0;
	this->gutter = 21 +
	               ( theme->getWindowBorderTexture() ?
	                 theme->getWindowBorderTexture()->width :
	                 0 );
	if ( hasCloseButton ) {
		if ( theme->getButtonHighlight() ) {
			this->closeButton = new Button( 0, 0, CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE,
			                                theme->getButtonHighlight()->texture );
		} else {
			this->closeButton = new Button( 0, 0, CLOSE_BUTTON_SIZE, CLOSE_BUTTON_SIZE, Texture::none() );
		}
	} else closeButton = NULL;
	openHeight = INSET * 2;
	this->type = type;
	this->locked = false;
	this->rawEventHandler = NULL;

	// FIXME: should come from gui.txt
	if ( type == SIMPLE_WINDOW ) {
		setBackgroundTileWidth( TILE_W );
		setBackgroundTileHeight( TILE_H );
	} else {
		setBackgroundTileWidth( 256 );
		setBackgroundTileHeight( 256 );
	}

	// make windows stay on screen
	this->move( x, y );
	currentY = y;
	addWindow( this );
}

Window::~Window() {
	delete closeButton;
	// Delete all widgets, may cause problem if someday we use same widgets for
	// multiple windows. For now, no problem.
	for ( int i = 0; i < widgetCount ; i++ ) {
		scourgeGui->unregisterEventHandler( widget[i] );
		delete this->widget[i];
	}
	removeWindow( this );
	if (message_dialog != NULL) {
		Window *tmp = message_dialog; // to avoid recursion of ~Window
		message_dialog = NULL;
		delete tmp;
	}
}

// convenience method to register the same handler for all the window's widgets
void Window::registerEventHandler( EventHandler *eventHandler ) {
	for ( int i = 0; i < widgetCount ; i++ ) {
		scourgeGui->registerEventHandler( widget[i], eventHandler );
	}
	if( closeButton ) scourgeGui->registerEventHandler( closeButton, eventHandler );
}

void Window::unregisterEventHandler() {
	for ( int i = 0; i < widgetCount ; i++ ) {
		scourgeGui->unregisterEventHandler( widget[i] );
	}
	if( closeButton ) scourgeGui->unregisterEventHandler( closeButton );
}

void Window::addWindow( Window *win ) {
	if ( windowCount < MAX_WINDOW ) {
		win->setZ( 50 + windowCount * 10 );
		window[windowCount++] = win;
	}
}

void Window::removeWindow( Window *win ) {
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i] == win ) {
			for ( int t = i; t < windowCount - 1; t++ ) {
				window[t] = window[t + 1];
			}
			windowCount--;
			return;
		}
	}
}

void Window::drawVisibleWindows() {
	//  glDisable(GL_CULL_FACE);
	glDisable( GL_DEPTH_TEST );
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i]->isVisible() ) {
			window[i]->drawWidget( NULL );
		}
	}
	glEnable( GL_DEPTH_TEST );
}

Widget *Window::delegateEvent( SDL_Event *event, int x, int y, Window **selectedWindow ) {

	if ( mouseLockWindow ) {
		return mouseLockWindow->handleWindowEvent( event, x, y );
	}

	// find the topmost window
	Window *win = NULL;
	int maxz = 0;
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i]->isVisible() ) {
			if ( window[i]->isModal() ) {
				win = window[i];
				break;
			} else if ( event->type == SDL_KEYUP ||
			            event->type == SDL_KEYDOWN ) {
				if ( window[i] == currentWin ) {
					win = window[i];
					break;
				}
			} else if ( window[i]->isInside( x, y ) ) {
				if ( window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_NORMAL ||
				        window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_ATTACK ||
				        window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_RANGED ||
				        window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_MOVE ||
				        window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_TALK )
					window[i]->getScourgeGui()->setCursorMode( Constants::CURSOR_NORMAL );
				if ( maxz < window[i]->getZ() ) {
					win = window[i];
					maxz = win->getZ();
				}
			}
		}
	}
	// find the active widget
	Widget *widget = NULL;
	if ( win ) {
#ifdef DEBUG_WINDOWS
		cerr << "handled by window: " << win->getZ() << endl;
#endif		
		widget = win->handleWindowEvent( event, x, y );
	}

	// tell the other windows that the mouse is elsewhere
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i] != win ) {
			for ( int t = 0; t < window[i]->widgetCount; t++ ) {
				window[i]->widget[t]->removeEffects( window[i] );
			}
		}
	}
	
	if( selectedWindow ) {
		*selectedWindow = win;
	}
	return widget;
}

Widget *Window::handleWindowEvent( SDL_Event *event, int x, int y ) {

	if ( mouseLockWidget ) {
		mouseLockWidget->
		handleEvent( mouseLockWindow, event,
		             x - getX(),
		             y - getY() - gutter );
		return mouseLockWidget;
	}

	if ( dragging ) {
		handleEvent( NULL, event, x, y );
		return this;
	}

	// handle some special key strokes
	Widget *w = NULL;
	bool systemKeyPressed = false;
	if ( event->type == SDL_KEYUP ) {
		switch ( event->key.keysym.sym ) {
		case SDLK_ESCAPE:
			// select an open, non-locked window with a close button to close
			if ( !currentWin || currentWin->isLocked() || !( currentWin->closeButton ) ) {
				for ( int i = 0; i < windowCount; i++ ) {
					if ( !window[i]->isLocked() && window[i]->closeButton && window[i]->isVisible() ) {
						currentWin = window[i];
						currentWin->toTop();
						break;
					}
				}
			}
			if( currentWin && currentWin->getEscapeHandler() ) {
				w = currentWin->getEscapeHandler();
				setFocus( w );
			} else {
				systemKeyPressed = true;
			}
			break;
		case SDLK_TAB:
			systemKeyPressed = true; break;
		default:
			break;
		}
	}
	
	if ( !systemKeyPressed ) {
		// handled by a component?
		bool insideWidget = false;
		if( !w ) {
			for ( int t = 0; t < widgetCount; t++ ) {
				if ( this->widget[t]->isVisible() && this->widget[t]->isEnabled() ) {
					if ( !insideWidget ) {
						if ( insideWidget = this->widget[t]->isInside( x - getX(), y - getY() - gutter ) ) {
							if ( ( event->type == SDL_MOUSEBUTTONUP ||
							        event->type == SDL_MOUSEBUTTONDOWN ) && event->button.button == SDL_BUTTON_LEFT ) {
								currentWin = this;
								setFocus( this->widget[t] );
							}
						}
					}
					if ( this->widget[t]->handleEvent( this,
					                                   event,
					                                   x - getX(),
					                                   y - getY() - gutter ) )
						w = this->widget[t];
				}
			}
		}
		
		// special handling
		if ( message_button != NULL && w == message_button && message_dialog != NULL ) {
			message_dialog->setVisible( false );
		}			
		
		if ( w ) {
			if ( w->hasSound() ) scourgeGui->playSound( Window::ACTION_SOUND, 127 );
			return w;
		}

		// handled by closebutton
		if ( closeButton ) {
			if ( !insideWidget ) {

				// w - 10 - ( closeButton->getWidth() ), topY + 8

				insideWidget =
				  closeButton->isInside( x - ( getX() + ( getWidth() - 10 - closeButton->getWidth() ) ),
				                         y - ( getY() + 8 ) );
			}
			if ( closeButton->handleEvent( this, event,
			                               x - ( getX() + ( getWidth() - 10 - closeButton->getWidth() ) ),
			                               y - ( getY() + 8 ) ) ) {
				scourgeGui->playSound( Window::ACTION_SOUND, 127 );
				return closeButton;
			}
		}

		if ( insideWidget &&
		        !( event->type == SDL_KEYUP ||
		           event->type == SDL_KEYDOWN ) ) {
			return this;
		}
	}
	
	// see if the window wants it
	if ( handleEvent( NULL, event, x, y ) ) {
		return this;
	}

	// swallow event if in a modal window
	return( isModal() ? this : NULL );
}

bool Window::isInside( int x, int y ) {
	return( dragging || Widget::isInside( x, y ) );
}

bool Window::handleEvent( Widget *parent, SDL_Event *event, int x, int y ) {
	switch ( event->type ) {
	case SDL_KEYDOWN:
		return false;
	case SDL_KEYUP:
		if ( event->key.keysym.sym == SDLK_TAB ) {
			if ( SDL_GetModState() & KMOD_CTRL ) {
				if ( SDL_GetModState() & KMOD_SHIFT ) {
					prevWindowToTop( this );
				} else {
					nextWindowToTop( this );
				}
			} else if ( SDL_GetModState() & KMOD_SHIFT ) {
				prevFocus();
			} else {
				nextFocus();
			}
			return true;
		} else if ( event->key.keysym.sym == SDLK_ESCAPE && ( closeButton || message_button ) && !isLocked() ) {
			scourgeGui->blockEvent();
			setVisible( false );
			// raise the next unlocked window
			currentWin = NULL;
			nextWindowToTop( NULL, false );
			return true;
		} else {
			return false;
		}
	case SDL_MOUSEMOTION:
		if ( dragging ) move( x - dragX, y - dragY );
		break;
	case SDL_MOUSEBUTTONUP:
		dragging = false;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if ( event->button.button != SDL_BUTTON_LEFT ) return false;
		toTop();
		if ( !isLocked() ) {
			dragging = ( isInside( x, y ) );
			dragX = x - getX();
			dragY = y - getY();
		}
		break;
	}
	return isInside( x, y );
}

void Window::setFocus( Widget *w ) {
	bool focusSet = false;
	for ( int i = 0; i < widgetCount; i++ ) {
		bool b = ( w->canGetFocus() && widget[i] == w ) ||
		         ( !w->canGetFocus() && widget[i]->canGetFocus() && !focusSet );
		if ( !focusSet ) focusSet = b;
		widget[i]->setFocus( b );
	}
}

void Window::nextFocus() {
	bool setFocus = false;
	for ( int t = 0; t < 2; t++ ) {
		for ( int i = 0; i < widgetCount; i++ ) {
			if ( widget[i]->hasFocus() ) {
				widget[i]->setFocus( false );
				setFocus = true;
			} else if ( setFocus && widget[i]->canGetFocus() ) {
				widget[i]->setFocus( true );
				return;
			}
		}
	}
}

void Window::prevFocus() {
	bool setFocus = false;
	for ( int t = 0; t < 2; t++ ) {
		for ( int i = widgetCount - 1; i >= 0; i-- ) {
			if ( widget[i]->hasFocus() ) {
				widget[i]->setFocus( false );
				setFocus = true;
			} else if ( setFocus && widget[i]->canGetFocus() ) {
				widget[i]->setFocus( true );
				return;
			}
		}
	}
}

void Window::addWidget( Widget *widget ) {
	if ( widgetCount < MAX_WIDGET ) {
		this->widget[widgetCount++] = widget;

		// apply the window's color scheme
		if ( !theme ) {
			widget->setColor( getColor() );
			widget->setBackground( getBackgroundColor() );
			widget->setSelectionColor( getSelectionColor() );
			widget->setBorderColor( getBorderColor() );
		}
		setFocus( widget );
	} else {
		cerr << "Gui/Window.cpp : max widget limit reached!" << endl;
	}
}

void Window::removeWidget( Widget *widget ) {
	for ( int i = 0; i < widgetCount; i++ ) {
		if ( this->widget[i] == widget ) {
			for ( int t = i; t < widgetCount - 1; t++ ) {
				this->widget[t] = this->widget[t + 1];
			}
			widgetCount--;
			return;
		}
	}
}

void Window::drawWidget( Widget *parent ) {
	GLint t = SDL_GetTicks();

	GLint topY;
	if ( animation == SLIDE_UP ) {
		if ( y > currentY ) {
			if ( t - lastTick > 10 ) {
				lastTick = t;
				y -= ( h / OPEN_STEPS );
				if ( y < currentY ) y = currentY;
			}
		}
		opening = ( y > currentY );
		topY = y - currentY;
		openHeight = h;

		// slide-up scissor
		//    glScissor(x, sdlHandler->getScreen()->h - (currentY + h), w, h);
		//    glEnable( GL_SCISSOR_TEST );
		scissorToWindow( false );
	} else {
		if ( openHeight < h ) {
			if ( t - lastTick > 10 ) {
				lastTick = t;
				openHeight += ( h / OPEN_STEPS ); // always open in the same number of steps
				if ( openHeight >= h )
					openHeight = h;
			}
		}
		topY = ( h / 2 ) - ( openHeight / 2 );
		opening = ( openHeight < h );
	}





	glPushMatrix();
	glLoadIdentity( );


	if ( type != INVISIBLE_WINDOW ) {
		glEnable( GL_TEXTURE_2D );
		// tile the background

		//  if(isLocked()) {
		//    glColor3f(0.65f, 0.6f, 0.55f);
		//  } else
		if ( theme->getWindowTop() ) {
			glColor4f( theme->getWindowTop()->color.r,
			           theme->getWindowTop()->color.g,
			           theme->getWindowTop()->color.b,
			           theme->getWindowTop()->color.a );
		} else {
			glColor3f( 1.0f, 0.6f, 0.3f );
		}

		glTranslated( x, y, z );

		// HACK: blend window if top color's a < 1.0f
		if ( !isModal() ) {
			if ( theme->getWindowTop() &&
			        theme->getWindowTop()->color.a < 1.0f ) {
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			}
		}

		drawBackground( topY, openHeight );

		glDisable( GL_BLEND );
		glDisable( GL_TEXTURE_2D );

		// draw drop-shadow
		if ( !isLocked() ) drawDropShadow( topY, openHeight );

		// top bar
		if ( title || ( closeButton && !isLocked() ) ) {
			glColor4f( 0, 0, 0, 0.5f );
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			glBegin( GL_TRIANGLE_STRIP );
			glVertex2f( 0, topY );
			glVertex2f( getWidth(), topY );
			glVertex2f( 0, topY + TITLE_HEIGHT );
			glVertex2f( getWidth(), topY + TITLE_HEIGHT );
			glEnd();
			glDisable( GL_BLEND );
		}

		if ( type == BASIC_WINDOW &&
		        theme->getWindowBorderTexture() ) {
			drawBorder( topY, openHeight );
		} else {
			drawLineBorder( topY, openHeight );
		}

		// print title
		if ( title ) drawTitle( topY, openHeight );

		// draw the close button
		if ( closeButton && !isLocked() ) drawCloseButton( topY, openHeight );
	}
	glDisable( GL_SCISSOR_TEST );

	// draw widgets
	bool tmp = isOpening();
	if ( tmp ) {
		scissorToWindow();
	}
	for ( int i = 0; i < widgetCount; i++ ) {
		if ( widget[i]->isVisible() ) {

			glPushMatrix();
			glLoadIdentity();


			// if this is modified, also change handleWindowEvent
			//glTranslated(x, y + topY + TOP_HEIGHT, z + 5);
			glTranslated( x, y + topY + gutter, z + 5 );

			widget[i]->draw( this );
			glPopMatrix();
		}
	}
	if ( tmp ) {
		glDisable( GL_SCISSOR_TEST );
	}

	for ( int i = 0; i < widgetCount; i++ ) {
		if ( widget[i]->isVisible() ) {
			widget[i]->drawTooltip( this );
		}
	}

	glEnable( GL_TEXTURE_2D );
	glPopMatrix();

	//glEnable( GL_DEPTH_TEST );
}

void Window::drawBackground( int topY, int openHeight ) {
	if ( theme->getWindowBackground() && !theme->getWindowBackground()->rep_h ) {
		theme->getWindowBackground()->texture.glBind();
		glBegin ( GL_TRIANGLE_STRIP );
		glTexCoord2f ( 0.0f, 0.0f );
		glVertex2i ( 0, topY );
		glTexCoord2f ( 1, 0 );
		glVertex2i ( w, topY );
		glTexCoord2f ( 0, 1 );
		glVertex2i ( 0, topY + openHeight );
		glTexCoord2f ( 1, 1 );
		glVertex2i ( w, topY + openHeight );
		glEnd ();
	} else if ( type == SIMPLE_WINDOW ) {
		if ( theme->getWindowBackground() && theme->getWindowBackground()->texture.isSpecified() ) {
			theme->getWindowBackground()->texture.glBind();
		}
		glBegin ( GL_TRIANGLE_STRIP );
		glTexCoord2f ( 0.0f, 0.0f );
		glVertex2i ( 0, topY );
		glTexCoord2f ( 1, 0 );
		glVertex2i ( w, topY );
		glTexCoord2f ( 0, openHeight / static_cast<float>( tileHeight ) );
		glVertex2i ( 0, topY + openHeight );
		glTexCoord2f ( 1, openHeight / static_cast<float>( tileHeight ) );
		glVertex2i ( w, topY + openHeight );
		glEnd ();
	} else if ( type == BASIC_WINDOW ) {
		/*
		if(!isModal()) {
		  glEnable( GL_BLEND );
		  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		}
		*/
		if ( theme->getWindowBackground()
		        && theme->getWindowBackground()->texture.isSpecified() ) {
			theme->getWindowBackground()->texture.glBind();
		} else {
			glDisable( GL_TEXTURE_2D );
		}

		//applyBackgroundColor();
		if ( theme->getWindowBackground() ) {
			glColor4f( theme->getWindowBackground()->color.r, theme->getWindowBackground()->color.g,
			           theme->getWindowBackground()->color.b, theme->getWindowBackground()->color.a );
		}

		glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2i( 0, topY );
		glTexCoord2f( 1, 0.0f );
		//glTexCoord2d( w/static_cast<float>(tileWidth), 0 );
		glVertex2i( w, topY );
		glTexCoord2f( 0.0f, ( openHeight ) / static_cast<float>( tileHeight ) );
		glVertex2i( 0, topY + openHeight );
		//glTexCoord2f( w/static_cast<float>(tileWidth), ( openHeight ) /static_cast<float>(tileHeight) );
		glTexCoord2f( 1, ( openHeight ) / static_cast<float>( tileHeight ) );
		glVertex2i( w, topY + openHeight );
		glEnd();
	}
}

void Window::drawDropShadow( int topY, int openHeight ) {
	glEnable( GL_BLEND );
	//  glBlendFunc( GL_SRC_ALPHA, GL_DST_COLOR );
	glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );
	int n = 10;
	glColor4f( 0.15f, 0.15f, 0.15f, 0.25f );
	glBegin( GL_QUADS );
	glVertex2i ( n, topY + openHeight );
	glVertex2i ( n, topY + openHeight + n );
	glVertex2i ( w + n, topY + openHeight + n );
	glVertex2i ( w + n, topY + openHeight );

	glVertex2i ( w, topY + n );
	glVertex2i ( w, topY + openHeight );
	glVertex2i ( w + n, topY + openHeight );
	glVertex2i ( w + n, topY + n );
	glEnd();
	glDisable( GL_BLEND );
}

void Window::drawCloseButton( int topY, int openHeight ) {
	// apply the window's color scheme
	/*
	   closeButton->setColor( getColor() );
	   closeButton->setBackground( getBackgroundColor() );
	   closeButton->setSelectionColor( getSelectionColor() );
	   closeButton->setBorderColor( getBorderColor() );
	*/

	if ( theme->getButtonText() ) closeButton->setColor( theme->getButtonText() );
	if ( theme->getButtonBackground() ) closeButton->setBackground( &( theme->getButtonBackground()->color ) );
	if ( theme->getButtonHighlight() ) closeButton->setSelectionColor( &( theme->getButtonHighlight()->color ) );
	if ( theme->getButtonBorder() ) closeButton->setBorderColor( &( theme->getButtonBorder()->color ) );


	glPushMatrix();
	//glLoadIdentity();
	glTranslated( w - 10 - ( closeButton->getWidth() ), topY + 8, z + 5 );
	closeButton->draw( this );
	glPopMatrix();
}

void Window::drawTitle( int topY, int openHeight ) {
	glPushMatrix();
	glTranslated( 0, 0, 5 );
	if ( theme->getWindowTitleText() ) {
		glColor4f( theme->getWindowTitleText()->r,
		           theme->getWindowTitleText()->g,
		           theme->getWindowTitleText()->b,
		           theme->getWindowTitleText()->a );
	} else {
		glColor3f( 1, 1, 1 );
	}
	scourgeGui->setFontType( Constants::SCOURGE_UI_FONT );
#ifdef DEBUG_WINDOWS
	scourgeGui->texPrint( 8, topY + TITLE_HEIGHT - 5, "%s (%d)", title, getZ() );
#else
	scourgeGui->texPrint( 8, topY + TITLE_HEIGHT - 5, "%s", title );
#endif
	scourgeGui->setFontType( Constants::SCOURGE_DEFAULT_FONT );
	glPopMatrix();
}

void Window::setTopWindowBorderColor() {
	if ( theme->getSelectedBorder() ) {
		glColor4f( theme->getSelectedBorder()->color.r,
		           theme->getSelectedBorder()->color.g,
		           theme->getSelectedBorder()->color.b,
		           theme->getSelectedBorder()->color.a );
	} else {
		applyHighlightedBorderColor();
	}
}

void Window::setWindowBorderColor() {
	//  } else if(isLocked()) {
	//    glColor3f(0.5f, 0.3f, 0.2f);
	//  } else
	if ( theme->getWindowBorder() ) {
		glColor4f( theme->getWindowBorder()->color.r,
		           theme->getWindowBorder()->color.g,
		           theme->getWindowBorder()->color.b,
		           theme->getWindowBorder()->color.a );
	} else {
		applyBorderColor();
	}
}

void Window::drawLineBorder( int topY, int openHeight ) {
	// add a border
	if ( currentWin == this ) {
		setTopWindowBorderColor();
	} else {
		setWindowBorderColor();
	}

	if ( this == currentWin || isLocked() || isModal() ) {
		glLineWidth( 3.0f );
	} else if ( theme->getWindowBorder() ) {
		glLineWidth( theme->getWindowBorder()->width );
	} else {
		glLineWidth( 2.0f );
	}
	glBegin( GL_LINES );
	glVertex2d( w, topY + openHeight );
	glVertex2d( 0, topY + openHeight );
	glVertex2d( 0, topY );
	glVertex2d( w, topY );
	glVertex2d( 0, topY );
	glVertex2d( 0, topY + openHeight );
	glVertex2d( w, topY );
	glVertex2d( w, topY + openHeight );
	glEnd();
	glLineWidth( 1.0f );
}

void Window::drawBorder( int topY, int openHeight ) {
	int n = 16; // FIXME: compute when loading textures

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glEnable( GL_TEXTURE_2D );

	glColor4f( theme->getWindowBorderTexture()->color.r,
	           theme->getWindowBorderTexture()->color.g,
	           theme->getWindowBorderTexture()->color.b,
	           theme->getWindowBorderTexture()->color.a );
	glPushMatrix();
	theme->getWindowBorderTexture()->tex_nw.glBind();
	glTranslatef( theme->getWindowBorderTexture()->width,
	              topY + theme->getWindowBorderTexture()->width,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( n, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, n );
	glTexCoord2f( 1, 1 );
	glVertex2i( n, n );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	theme->getWindowBorderTexture()->tex_ne.glBind();
	glTranslatef( getWidth() - n - theme->getWindowBorderTexture()->width,
	              topY + theme->getWindowBorderTexture()->width,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( n, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, n );
	glTexCoord2f( 1, 1 );
	glVertex2i( n, n );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	theme->getWindowBorderTexture()->tex_se.glBind();
	glTranslatef( getWidth() - n - theme->getWindowBorderTexture()->width,
	              topY + openHeight - n - theme->getWindowBorderTexture()->width,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( n, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, n );
	glTexCoord2f( 1, 1 );
	glVertex2i( n, n );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	theme->getWindowBorderTexture()->tex_sw.glBind();
	glTranslatef( theme->getWindowBorderTexture()->width,
	              topY + openHeight - n - theme->getWindowBorderTexture()->width,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( n, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, n );
	glTexCoord2f( 1, 1 );
	glVertex2i( n, n );
	glEnd();
	glPopMatrix();

	int h = openHeight - 2 * ( n + theme->getWindowBorderTexture()->width );
	glPushMatrix();
	theme->getWindowBorderTexture()->tex_west.glBind();
	glTranslatef( theme->getWindowBorderTexture()->width,
	              topY + theme->getWindowBorderTexture()->width + n,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( n, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, h );
	glTexCoord2f( 1, 1 );
	glVertex2i( n, h );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	theme->getWindowBorderTexture()->tex_east.glBind();
	glTranslatef( getWidth() - n - theme->getWindowBorderTexture()->width,
	              topY + theme->getWindowBorderTexture()->width + n,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( n, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, h );
	glTexCoord2f( 1, 1 );
	glVertex2i( n, h );
	glEnd();
	glPopMatrix();

	int w = getWidth() - 2 * ( n + theme->getWindowBorderTexture()->width );
	glPushMatrix();
	theme->getWindowBorderTexture()->tex_north.glBind();
	glTranslatef( n + theme->getWindowBorderTexture()->width,
	              topY + theme->getWindowBorderTexture()->width,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( w, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, n );
	glTexCoord2f( 1, 1 );
	glVertex2i( w, n );
	glEnd();
	glPopMatrix();

	glPushMatrix();
	theme->getWindowBorderTexture()->tex_south.glBind();
	glTranslatef( n + theme->getWindowBorderTexture()->width,
	              topY + openHeight - n - theme->getWindowBorderTexture()->width,
	              0 );
	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0, 0 );
	glVertex2i( 0, 0 );
	glTexCoord2f( 1, 0 );
	glVertex2i( w, 0 );
	glTexCoord2f( 0, 1 );
	glVertex2i( 0, n );
	glTexCoord2f( 1, 1 );
	glVertex2i( w, n );
	glEnd();
	glPopMatrix();

	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
}


Button *Window::createButton( int x1, int y1, int x2, int y2, char *label, bool toggle, Texture const& texture ) {
	if ( widgetCount < MAX_WIDGET ) {
		Button * theButton;
		theButton = new Button( x1, y1, x2, y2, scourgeGui->getHighlightTexture(), label, texture );
		theButton->setToggle( toggle );
		addWidget( ( Widget * )theButton );
		return theButton;
	} else {
		cerr << "Gui/Window.cpp : max widget limit reached!" << endl;
		return NULL;
	}
}

Label * Window::createLabel( int x1, int x2, char const* label, int color ) {
	if ( widgetCount < MAX_WIDGET ) {
		Label * theLabel;
		theLabel = new Label( x1, x2, label );

		// addwidget call must come before setColor...
		addWidget( ( Widget * )theLabel );

		// set new color or keep default color (black)
		if ( color == Constants::RED_COLOR ) {
			theLabel->setColor( 0.8f, 0.2f, 0.0f, 1.0f );
		} else if ( color == Constants::BLUE_COLOR ) {
			theLabel->setColor( 0.0f, 0.3f, 0.9f, 1.0f  );
		}
		return theLabel;
	} else {
		cerr << "Gui/Window.cpp : max widget limit reached!" << endl;
		return NULL;
	}
}

Checkbox * Window::createCheckbox( int x1, int y1, int x2, int y2, char *label ) {
	if ( widgetCount < MAX_WIDGET ) {
		Checkbox * theCheckbox;
		theCheckbox = new Checkbox( x1, y1, x2, y2, scourgeGui->getHighlightTexture(), label );
		addWidget( ( Widget * )theCheckbox );
		return theCheckbox;
	} else {
		cerr << "Gui/Window.cpp : max widget limit reached!" << endl;
		return NULL;
	}
}

TextField *Window::createTextField( int x, int y, int numChars ) {
	if ( widgetCount < MAX_WIDGET ) {
		TextField *tf;
		tf = new TextField( x, y, numChars );
		addWidget( ( Widget * )tf );
		return tf;
	} else {
		cerr << "Gui/Window.cpp : max widget limit reached!" << endl;
		return NULL;
	}
}

// scissor test: y screen coordinate is reversed, rectangle is
// specified by lower-left corner. sheesh!
void Window::scissorToWindow( bool insideOnly ) {
	GLint topY = ( h / 2 ) - ( openHeight / 2 );

	int n = 8;
	int sx, sy, sw, sh;

	if ( insideOnly ) {
		sx = x + n;
		sy = currentY + topY + openHeight - n;
		sw = w - n * 2;
		sh = openHeight - n * 2;
	} else {
		sx = x;
		sy = currentY + topY + openHeight;
		sw = w;
		sh = openHeight;
	}

#ifdef DEBUG_SCISSOR
	glPushMatrix();
	glTranslatef( -x, -y, 0 );
	glDisable( GL_TEXTURE_2D );
	if ( insideOnly ) {
		glColor4f( 1, 1, 1, 1 );
	} else {
		glColor4f( 1, 0, 0, 1 );
	}
	glBegin( GL_LINE_LOOP );
	glVertex2f( sx, sy - sh );
	glVertex2f( sx + sw, sy - sh );
	glVertex2f( sx + sw, sy  );
	glVertex2f( sx, sy  );
	glEnd();
	glEnable( GL_TEXTURE_2D );
	glPopMatrix();
#endif

	glScissor( sx, scourgeGui->getScreenHeight() - sy, sw, sh );

	glEnable( GL_SCISSOR_TEST );
}

void Window::setVisible( bool b, bool animate ) {
	toTop();
	Widget::setVisible( b );
	if ( b ) {
		lastTick = 0;
		opening = ( !isLocked() );
		currentY = y;
		if ( animate ) {
			if ( animation == SLIDE_UP ) {
				openHeight = getHeight();
				y = getHeight();
			} else {
				openHeight = INSET * 2;
			}
		} else {
			openHeight = getHeight();
		}
		toTop();
	} else {
		for ( unsigned int i = 0; i < listeners.size(); i++ ) {
			listeners[i]->windowClosing();
		}
		y = currentY;
		windowWasClosed = true;
		if( currentWin == this ) currentWin = NULL;
		nextWindowToTop( NULL, false );

		// Any windows open?
		if ( !anyFloatingWindowsOpen() ) scourgeGui->allWindowsClosed();
	}
}

bool Window::anyFloatingWindowsOpen() {
	bool found = false;
	for ( int i = 0; i < Window::windowCount; i++ ) {
		if ( Window::window[i]->isVisible() &&
		        !Window::window[i]->isLocked() ) {
			found = true;
			break;
		}
	}
	return found;
}

void Window::toTop() {
	toTop( this );
}

void Window::toTop( Window *win ) {
	currentWin = win;
	if ( win->isLocked() ) return;
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i] == win ) {
			for ( int t = i; t < windowCount - 1; t++ ) {
				window[t] = window[t + 1];
				window[t]->setZ( window[t]->getZ() - 10 );
			}
			window[windowCount - 1] = win;
			win->setZ( 50 + ( windowCount * 10 ) );
			break;
		}
	}
}

void Window::toBottom() {
	toBottom( this );
}

void Window::toBottom( Window *win ) {
	if ( win->isLocked() ) return;
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i] == win ) {
			for ( int t = i; t > 0; t-- ) {
				window[t] = window[t - 1];
				window[t]->setZ( window[t]->getZ() + 10 );
			}
			window[0] = win;
			win->setZ( 50 );
			break;
		}
	}
}

void Window::nextWindowToTop( Window *win, bool includeLocked ) {
	int nextWindow = -1;
	int nextZ;
	
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i]->isVisible() && ( !win || window[i]->getZ() < win->getZ() ) && ( nextWindow == -1 || window[i]->getZ() > nextZ ) ) {
			if ( !includeLocked && window[i]->isLocked() ) continue;
			nextWindow = i;
			nextZ = window[i]->getZ();
		}
	}
	if ( nextWindow == -1 ) {
		return;
	}
	currentWin = window[nextWindow];
	currentWin->toTop();
}

void Window::prevWindowToTop( Window *win, bool includeLocked ) {
	int prevWindow = -1;
	int prevZ;

	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i]->isVisible() && ( window[i]->getZ() > win->getZ() ) && ( prevWindow == -1 || window[i]->getZ() < prevZ ) ) {
			if ( !includeLocked && window[i]->isLocked() ) continue;
			prevWindow = i;
			prevZ = window[i]->getZ();
		}
	}
	if ( prevWindow == -1 ) return;
	currentWin = window[prevWindow];
	currentWin->toTop();
}

void Window::showMessageDialog( ScourgeGui *scourgeGui,
                                int x, int y, int w, int h,
                                char *title, Texture texture,
                                char const* message,
                                char *buttonLabel ) {
	if ( message_dialog && message_dialog->isVisible() ) {
		cerr << "*** Warning: Unable to display second message dialog: " << message << endl;
		return;
	}
	if ( !message_dialog ) {
		message_dialog = new Window( scourgeGui,
		                             x, y, w, h,
		                             title,
		                             texture, false );
		int textWidth = scourgeGui->textWidth( message );
		message_label = message_dialog->createLabel( ( w - textWidth ) / 2, 30, message );
		message_button = message_dialog->createButton( ( w / 2 ) - 50,
		                                               h - 30 - message_dialog->getGutter() - 5,
		                                               ( w / 2 ) + 50,
		                                               h - 10 - message_dialog->getGutter() - 5,
		                                               buttonLabel );
		message_dialog->setModal( true );
		message_dialog->setEscapeHandler( message_button );
	} else {
		message_dialog->move( x, y );
		message_dialog->resize( w, h );
		message_label->setText( message );
		int textWidth = scourgeGui->textWidth( message );
		message_label->move( ( w - textWidth ) / 2, 30 );
		message_button->setLabel( buttonLabel );
	}
	message_dialog->setVisible( true );
}

// overridden so windows stay on screen and moving/rotating still works
void Window::move( int x, int y ) {
	this->x = x;
	if ( x < SCREEN_GUTTER ) this->x = SCREEN_GUTTER;
	if ( x >= scourgeGui->getScreenWidth() - ( w + SCREEN_GUTTER ) ) this->x = scourgeGui->getScreenWidth() - ( w + SCREEN_GUTTER + 1 );

	int newY = y;
	if ( y < SCREEN_GUTTER ) newY = SCREEN_GUTTER;
	if ( y >= scourgeGui->getScreenHeight() - ( h + SCREEN_GUTTER ) ) newY = scourgeGui->getScreenHeight() - ( h + SCREEN_GUTTER + 1 );

	int diffY = newY - this->currentY;
	this->currentY = newY;
	if ( animation == SLIDE_UP && this->y > this->currentY ) {
		this->y -= diffY;
	} else {
		this->y = newY;
	}
}

void Window::setLastWidget( Widget *w ) {
	if ( w != lastWidget ) {
		lastWidget = w;
		scourgeGui->playSound( Window::ROLL_OVER_SOUND, 127 );
	}
}

void Window::setMouseLock( Widget *widget ) {
	if ( widget ) {
		mouseLockWindow = this;
		mouseLockWidget = widget;
		getScourgeGui()->lockMouse( this );
	} else {
		mouseLockWindow = NULL;
		mouseLockWidget = NULL;
		getScourgeGui()->unlockMouse();
	}
}

Window *Window::getTopWindow() {
	return currentWin;
}
