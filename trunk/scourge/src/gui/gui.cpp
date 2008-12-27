/***************************************************************************
                     gui.cpp  -  Non-abstract part of ScourgeGui Class.
                             -------------------
    created              : Sat Dec 27 2008
    author               : Vambola Kotkas
    email                : vambola.kotkas@proekspert.ee
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
#include "gui.h"
#include "window.h"
using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

ScourgeGui::ScourgeGui() 
	: windowCount( 0 )
	, message_dialog( NULL )
	, message_label( NULL )
	, currentWin( NULL )
	, mouseLockWindow( NULL )
	, mouseLockWidget( NULL )
	, windowWasClosed( false )
	, message_button( NULL ) {
}

void ScourgeGui::addWindow( Window* win ) {
	if ( windowCount < MAX_WINDOW ) {
		win->setZ( 50 + windowCount * 10 );
		window[windowCount++] = win;
	}
}

void ScourgeGui::removeWindow( Window* win ) {
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

void ScourgeGui::drawVisibleWindows() {
	//  glDisable(GL_CULL_FACE);
	glDisable( GL_DEPTH_TEST );
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i]->isVisible() ) {
			window[i]->drawWidget( NULL );
		}
	}
	glEnable( GL_DEPTH_TEST );
}

Widget* ScourgeGui::delegateEvent( SDL_Event* event, int x, int y, Window** selectedWindow ) {

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
				if ( getCursorMode() == Constants::CURSOR_ATTACK 
				   || getCursorMode() == Constants::CURSOR_RANGED 
				   || getCursorMode() == Constants::CURSOR_MOVE 
				   || getCursorMode() == Constants::CURSOR_TALK ) {
					setCursorMode( Constants::CURSOR_NORMAL );
				}
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
			window[i]->removeEffects();
		}
	}
	
	if( selectedWindow ) {
		*selectedWindow = win;
	}
	return widget;
}

Widget* ScourgeGui::selectCurrentEscapeHandler() {
	Widget* w( NULL ); 
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
		currentWin->setFocus( w );
	}
	return w;
}

bool ScourgeGui::anyFloatingWindowsOpen() {
	for ( int i = 0; i < windowCount; i++ ) {
		if ( window[i]->isVisible() && !window[i]->isLocked() ) {
			return true;
		}
	}
	return false;
}

void ScourgeGui::toTop( Window *win ) {
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


void ScourgeGui::toBottom( Window *win ) {
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

void ScourgeGui::nextWindowToTop( Window *win, bool includeLocked ) {
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

void ScourgeGui::prevWindowToTop( Window *win, bool includeLocked ) {
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

void ScourgeGui::showMessageDialog( int x, int y, int w, int h,
                                    char const* title, Texture texture,
                                    char const* message,
                                    char const* buttonLabel ) {
	if ( message_dialog && message_dialog->isVisible() ) {
		cerr << "*** Warning: Unable to display second message dialog: " << message << endl;
		return;
	}
	if ( !message_dialog ) {
		message_dialog = new Window( this,
		                             x, y, w, h,
		                             title,
		                             texture, false );
		message_label = message_dialog->createLabel( ( w - textWidth( message ) ) / 2, 30, message );
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
		message_label->move( ( w - textWidth( message ) ) / 2, 30 );
		message_button->setLabel( buttonLabel );
	}
	message_dialog->setVisible( true );
}

