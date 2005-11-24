/***************************************************************************
                          window.cpp  -  description
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
#include "window.h"

using namespace std;

#define OPEN_STEPS 10

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

Window::Window( ScourgeGui *scourgeGui, int x, int y, int w, int h, char *title, bool hasCloseButton, int type, const char *themeName ) : Widget(x, y, w, h) {
  theme = GuiTheme::getThemeByName( themeName );
  commonInit( scourgeGui, x, y, w, h, title, hasCloseButton, type );
}

Window::Window( ScourgeGui *scourgeGui, int x, int y, int w, int h, char *title, GLuint texture, bool hasCloseButton, int type, GLuint texture2) : Widget(x, y, w, h) {
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

void Window::commonInit( ScourgeGui *scourgeGui, int x, int y, int w, int h, char *title, bool hasCloseButton, int type) {
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
  if(hasCloseButton) {
    if( theme->getButtonHighlight() ) {
      this->closeButton = new Button(0, 0, CLOSE_BUTTON_SIZE, TOP_HEIGHT - 6, 
                                     theme->getButtonHighlight()->texture);
    } else {
      this->closeButton = new Button(0, 0, CLOSE_BUTTON_SIZE, TOP_HEIGHT - 6, 0 );
    }
  } else closeButton = NULL;
  openHeight = 0;
  this->type = type;
  this->locked = false;
  setBackgroundTileWidth(TILE_W);
  setBackgroundTileHeight(TILE_H);
  // make windows stay on screen
  this->move(x, y);
  currentY = y;
  addWindow(this);
}

Window::~Window() {
  delete closeButton;   
  // Delete all widgets, may cause problem if someday we use same widgets for 
  // multiple windows. For now, no problem.
  for(int i = 0; i < widgetCount ; i++){
    if(this->widget[i]) delete this->widget[i];
  }
  removeWindow(this);
}

void Window::addWindow(Window *win) {
  if(windowCount < MAX_WINDOW) {
    win->setZ(50 + windowCount * 10);
    window[windowCount++] = win;
  }
}

void Window::removeWindow(Window *win) {
  for(int i = 0; i < windowCount; i++) {
    if(window[i] == win) {
      for(int t = i; t < windowCount - 1; t++) {
        window[t] = window[t + 1];
      }
      windowCount--;
      return;
    }
  }
}

void Window::drawVisibleWindows() {
  //  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  for(int i = 0; i < windowCount; i++) {
    if(window[i]->isVisible()) window[i]->drawWidget(NULL);
  }
  glEnable(GL_DEPTH_TEST);
}

Widget *Window::delegateEvent(SDL_Event *event, int x, int y) {

  if( mouseLockWindow ) {
    return mouseLockWindow->handleWindowEvent( event, x, y );
  }

  // find the topmost window
  Window *win = NULL;
  int maxz = 0;
  for(int i = 0; i < windowCount; i++) {
    if(window[i]->isVisible()) {
      if(window[i]->isModal()) {
        win = window[i];
        break;
      } else if(event->type == SDL_KEYUP || 
                event->type == SDL_KEYDOWN) {
        if(window[i] == currentWin) {
          win = window[i];
          break;
        }
      } else if(window[i]->isInside(x, y)) {
        if( window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_NORMAL || 
            window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_ATTACK ||
            window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_RANGED ||
            window[i]->getScourgeGui()->getCursorMode() == Constants::CURSOR_TALK )
          window[i]->getScourgeGui()->setCursorMode( Constants::CURSOR_NORMAL );
        if(maxz < window[i]->getZ()) {
          win = window[i];
          maxz = win->getZ();
        }
      }
    }
  }
  // find the active widget
  Widget *widget = NULL;
  if(win) {
    widget = win->handleWindowEvent(event, x, y);
  } 

  // tell the other windows that the mouse is elsewhere
  for(int i = 0; i < windowCount; i++) {
    if(window[i] != win) {
      for(int t = 0; t < window[i]->widgetCount; t++) {
        window[i]->widget[t]->removeEffects(window[i]);
      }
    }
  }
  return widget;
}

Widget *Window::handleWindowEvent(SDL_Event *event, int x, int y) {

  if( mouseLockWidget ) {
    mouseLockWidget->
      handleEvent( mouseLockWindow, event, 
                   x - getX(), 
                   y - ( getY() + TOP_HEIGHT ) );
    return mouseLockWidget;
  }

  if(dragging) {
    handleEvent(NULL, event, x, y);
    return this;
  }

  // handle some special key strokes
  bool systemKeyPressed = false;
  if(event->type == SDL_KEYUP || event->type == SDL_KEYDOWN) {
    switch(event->key.keysym.sym) {
    case SDLK_ESCAPE: 
    // select an open, non-locked window with a close button to close
    if( !currentWin || currentWin->isLocked() || !(currentWin->closeButton) ) {
      for( int i = 0; i < windowCount; i++ ) {
        if( !window[i]->isLocked() && window[i]->closeButton && window[i]->isVisible() ) {
          currentWin = window[i];
          currentWin->toTop();
          break;
        }
      }
    }
    systemKeyPressed = true; break;
    case SDLK_TAB:
        systemKeyPressed = true; break;
    default:
        break;
    }
  }

  if(!systemKeyPressed) {
    // handled by a component?
    bool insideWidget = false;
    Widget *w = NULL;
    for(int t = 0; t < widgetCount; t++) {
      if(this->widget[t]->isVisible()) {
        if(!insideWidget) {
          if(insideWidget = this->widget[t]->isInside(x - getX(), y - (getY() + TOP_HEIGHT))) {
            if(event->type == SDL_MOUSEBUTTONUP || 
               event->type == SDL_MOUSEBUTTONDOWN) {
              currentWin = this;
              setFocus(this->widget[t]);
            }
          }
        } 
        if(this->widget[t]->handleEvent(this, event, x - getX(), y - (getY() + TOP_HEIGHT)))
          w = this->widget[t];
      }
    }
    
    // special handling
    if(message_button && w == message_button) {
      message_dialog->setVisible(false);
    }
    if(w) {
      if(w->hasSound()) scourgeGui->playSound(Window::ACTION_SOUND);
      return w;
    }
    
    // handled by closebutton
    if(closeButton) {
      if(!insideWidget) {
        insideWidget = closeButton->isInside(x - (getX() + (getWidth() - (closeButton->getWidth() + 3))), 
                                             y - (getY() + 3));
      }
      if(closeButton->handleEvent(this, event, 
                                  x - (getX() + (getWidth() - (closeButton->getWidth() + 3))), 
                                  y - (getY() + 3))) {
        scourgeGui->playSound(Window::ACTION_SOUND);
        return closeButton;
      }
    }
    
    if(insideWidget && 
       !(event->type == SDL_KEYUP || 
         event->type == SDL_KEYDOWN)) {
      return this;
    }
  }
  
  // see if the window wants it
  if(handleEvent(NULL, event, x, y)) {
    return this;
  }

  // swallow event if in a modal window
  return(isModal() ? this : NULL);
}

bool Window::isInside(int x, int y) {
  return(dragging || Widget::isInside(x, y));
}

bool Window::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  switch(event->type) {
  case SDL_KEYDOWN:
    return false;
  case SDL_KEYUP:
  if(event->key.keysym.sym == SDLK_TAB) {
    if(SDL_GetModState() & KMOD_CTRL) {
      if(SDL_GetModState() & KMOD_SHIFT) {
        prevWindowToTop();
      } else {
        nextWindowToTop();
      }
    } else if(SDL_GetModState() & KMOD_SHIFT) {
      prevFocus();
    } else {
      nextFocus();
    }
    return true;
  } else if(event->key.keysym.sym == SDLK_ESCAPE && closeButton && !isLocked()) {
    setVisible(false);
    // raise the next unlocked window
    currentWin = NULL;
    nextWindowToTop( false );
    return true;
  } else {
    return false;
  }
  case SDL_MOUSEMOTION:
  if(dragging) move(x - dragX, y - dragY);
  break;
  case SDL_MOUSEBUTTONUP:
  dragging = false;
  break;
  case SDL_MOUSEBUTTONDOWN:
  toTop();
  if(!isLocked()) {
    dragging = isInside(x, y);
    dragX = x - getX();
    dragY = y - getY();
  }
  break;
  }
  return isInside(x, y);
}

void Window::setFocus(Widget *w) {
  bool focusSet = false;
  for(int i = 0; i < widgetCount; i++) {
    bool b = (w->canGetFocus() && widget[i] == w) ||
      (!w->canGetFocus() && widget[i]->canGetFocus() && !focusSet);
    if(!focusSet) focusSet = b;
    widget[i]->setFocus(b);
  }
}

void Window::nextFocus() {
  bool setFocus = false;
  for(int t = 0; t < 2; t++) {
    for(int i = 0; i < widgetCount; i++) {
      if(widget[i]->hasFocus()) {
        widget[i]->setFocus(false);
        setFocus = true;
      } else if(setFocus && widget[i]->canGetFocus()) {
        widget[i]->setFocus(true);
        return;
      }
    }
  }
}

void Window::prevFocus() {
  bool setFocus = false;
  for(int t = 0; t < 2; t++) {
    for(int i = widgetCount - 1; i >= 0; i--) {
      if(widget[i]->hasFocus()) {
        widget[i]->setFocus(false);
        setFocus = true;
      } else if(setFocus && widget[i]->canGetFocus()) {
        widget[i]->setFocus(true);
        return;
      }
    }
  }
}

void Window::addWidget(Widget *widget) {
  if(widgetCount < MAX_WIDGET){
    this->widget[widgetCount++] = widget;

    // apply the window's color scheme
	if( !theme ) {
	  widget->setColor( getColor() );
	  widget->setBackground( getBackgroundColor() );
	  widget->setSelectionColor( getSelectionColor() );
	  widget->setBorderColor( getBorderColor() );
	}
    setFocus(widget);
  } else{
    cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
  }
}

void Window::removeWidget(Widget *widget) {
  for(int i = 0; i < widgetCount; i++) {
    if(this->widget[i] == widget) {
      for(int t = i; t < widgetCount - 1; t++) {
        this->widget[t] = this->widget[t + 1];
      }
      widgetCount--;
      return;
    }
  }
}

#define TOP_HEIGHT 20
#define BOTTOM_HEIGHT 5

void Window::drawWidget(Widget *parent) {
  GLint t = SDL_GetTicks();

  GLint topY;
  if( animation == SLIDE_UP ) {
    if( y > currentY ) {
      if( t - lastTick > 10 ) {
        lastTick = t;      
        y -= ( h / OPEN_STEPS );
        if( y < currentY ) y = currentY;
      }
    }
    opening = ( y > currentY );
    topY = y - currentY;
    openHeight = (h - (TOP_HEIGHT + BOTTOM_HEIGHT));

    // slide-up scissor
    //    glScissor(x, sdlHandler->getScreen()->h - (currentY + h), w, h);  
    //    glEnable( GL_SCISSOR_TEST );
    scissorToWindow( false );
  } else {
    if(openHeight < (h - (TOP_HEIGHT + BOTTOM_HEIGHT))) {
      if( t - lastTick > 10 ) {
        lastTick = t;
        openHeight += ( h / OPEN_STEPS ); // always open in the same number of steps
        if(openHeight >= (h - (TOP_HEIGHT + BOTTOM_HEIGHT)))
          openHeight = (h - (TOP_HEIGHT + BOTTOM_HEIGHT));
      }
    }
    topY = ((h - (TOP_HEIGHT + BOTTOM_HEIGHT)) / 2) - (openHeight / 2);
    opening = ( openHeight < (h - (TOP_HEIGHT + BOTTOM_HEIGHT)) );
  }

  glPushMatrix();
  glLoadIdentity( );
  glEnable( GL_TEXTURE_2D );
  // tile the background

//  if(isLocked()) {
//    glColor3f(0.65f, 0.6f, 0.55f);
//  } else 
  if( theme->getWindowTop() ) {
	glColor4f( theme->getWindowTop()->color.r, 
			   theme->getWindowTop()->color.g, 
			   theme->getWindowTop()->color.b, 
			   theme->getWindowTop()->color.a );
  } else {
    glColor3f(1.0f, 0.6f, 0.3f);
  }

  glTranslated(x, y, z);
  /*
  if(texture)
    glBindTexture( GL_TEXTURE_2D, texture );
  */
  if(type == BASIC_WINDOW) {
    if( theme->getWindowTop() && theme->getWindowTop()->texture ) 
      glBindTexture( GL_TEXTURE_2D, theme->getWindowTop()->texture );

    glBegin (GL_QUADS);
    glTexCoord2f (0.0f, 0.0f);
    glVertex2i (0, topY);
    //glTexCoord2f (0.0f, TOP_HEIGHT/(float)tileHeight);
    glTexCoord2f (0.0f, 1);
    glVertex2i (0, topY + TOP_HEIGHT);
    //glTexCoord2f (w/(float)tileWidth, TOP_HEIGHT/(float)tileHeight);
    glTexCoord2f (1, 1);
    glVertex2i (w, topY + TOP_HEIGHT);
    //glTexCoord2f (w/(float)tileWidth, 0.0f);      
    glTexCoord2f (1, 0.0f);      
    glVertex2i (w, topY);
    glEnd ();

    glBegin (GL_QUADS);
    glTexCoord2f (0.0f, 0.0f);
    glVertex2i (0, topY + TOP_HEIGHT + openHeight);
    //glTexCoord2f (0.0f, BOTTOM_HEIGHT/(float)tileHeight);
    glTexCoord2f (0.0f, 1);
    glVertex2i (0, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    //glTexCoord2f (w/(float)tileWidth, BOTTOM_HEIGHT/(float)tileHeight);
    glTexCoord2f (1, 1);
    glVertex2i (w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    //glTexCoord2f (w/(float)tileWidth, 0.0f);      
    glTexCoord2f (1, 0.0f);      
    glVertex2i (w, topY + TOP_HEIGHT + openHeight);
    glEnd ();

  } 

  // HACK: blend window if top color's a < 1.0f
  if(!isModal()) {
    if( theme->getWindowTop() && 
        theme->getWindowTop()->color.a < 1.0f ) {
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
  } 

  if(type == SIMPLE_WINDOW) {
    if( theme->getWindowBackground() && theme->getWindowBackground()->texture ) {
      glBindTexture( GL_TEXTURE_2D, theme->getWindowBackground()->texture );
    }
    glBegin (GL_QUADS);
    glTexCoord2f (0.0f, 0.0f);
    glVertex2i (0, topY);
    glTexCoord2f (0, (TOP_HEIGHT + BOTTOM_HEIGHT + openHeight) / (float)tileHeight);
    glVertex2i (0, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    glTexCoord2f (1, (TOP_HEIGHT + BOTTOM_HEIGHT + openHeight) / (float)tileHeight);
    glVertex2i (w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    glTexCoord2f (1, 0);      
    glVertex2i (w, topY);
    glEnd ();
  }

  if(type == BASIC_WINDOW) {
    /*
    if(!isModal()) {
      glEnable( GL_BLEND );
      glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    }
    */
    if( theme->getWindowBackground() && theme->getWindowBackground()->texture ) {
      glBindTexture( GL_TEXTURE_2D, theme->getWindowBackground()->texture );
    } else {
      glDisable( GL_TEXTURE_2D );
    }

    //applyBackgroundColor();
    if( theme->getWindowBackground() ) {
      glColor4f( theme->getWindowBackground()->color.r,
                 theme->getWindowBackground()->color.g,
                 theme->getWindowBackground()->color.b,
                 theme->getWindowBackground()->color.a );
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2i(0, topY + TOP_HEIGHT);
    glTexCoord2f(0.0f, ( openHeight )/(float)tileHeight);
    glVertex2i(0, topY + TOP_HEIGHT + openHeight);
    //    glTexCoord2f (w/(float)tileWidth, ( openHeight ) /(float)tileHeight);
    glTexCoord2f( 1, ( openHeight ) /(float)tileHeight);
    glVertex2i(w, topY + TOP_HEIGHT + openHeight);
    glTexCoord2f( 1, 0.0f);      
    glVertex2i(w, topY + TOP_HEIGHT);
    glEnd();
  }

  glDisable( GL_BLEND );
  glDisable( GL_TEXTURE_2D );

  // draw drop-shadow
  if(!isLocked()) {
    glEnable( GL_BLEND );
    //  glBlendFunc( GL_SRC_ALPHA, GL_DST_COLOR );
    glBlendFunc( GL_SRC_COLOR, GL_DST_COLOR );
    int n = 10;
    glColor4f( 0.15f, 0.15f, 0.15f, 0.25f );
    glBegin(GL_QUADS);
    glVertex2i (n, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    glVertex2i (n, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT + n);
    glVertex2i (w + n, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT + n);
    glVertex2i (w + n, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    
    glVertex2i (w, topY + n);
    glVertex2i (w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    glVertex2i (w + n, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
    glVertex2i (w + n, topY + n);
    glEnd();
    glDisable( GL_BLEND );
  }

  // add a border
  if(currentWin == this) {
    if( theme->getSelectedBorder() ) {
      glColor4f( theme->getSelectedBorder()->color.r,
                 theme->getSelectedBorder()->color.g,
                 theme->getSelectedBorder()->color.b,
                 theme->getSelectedBorder()->color.a );
    } else {
      applyHighlightedBorderColor();
    }
  } else {
//  } else if(isLocked()) {
//    glColor3f(0.5f, 0.3f, 0.2f);
//  } else 
    if( theme->getWindowBorder() ) {
    glColor4f( theme->getWindowBorder()->color.r, 
               theme->getWindowBorder()->color.g,
               theme->getWindowBorder()->color.b,
               theme->getWindowBorder()->color.a );
    } else {
      applyBorderColor();
    }
  }
  
  if( this == currentWin || isLocked() || isModal() ) {
    glLineWidth( 3.0f );
  } else if( theme->getWindowBorder() ) {
    glLineWidth( theme->getWindowBorder()->width );
  } else {
    glLineWidth( 2.0f );
  }
  glBegin(GL_LINES);
  glVertex2d(w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glVertex2d(0, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glVertex2d(0, topY);
  glVertex2d(w, topY);
  glVertex2d(0, topY);
  glVertex2d(0, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glVertex2d(w, topY);
  glVertex2d(w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glEnd();
  glLineWidth( 1.0f );

  if(type == BASIC_WINDOW) {
    glBegin(GL_LINES);
    glVertex2i (0, topY + TOP_HEIGHT);
    glVertex2i (w, topY + TOP_HEIGHT);
    glVertex2i (0, topY + TOP_HEIGHT + openHeight);
    glVertex2i (w, topY + TOP_HEIGHT + openHeight);
    glEnd();
  }

  // print title
  if(title) {
    glPushMatrix();
    glTranslated( 0, 0, 5 );
	if( theme->getWindowTitleText() ) {
	  glColor4f( theme->getWindowTitleText()->r, 
				 theme->getWindowTitleText()->g,
				 theme->getWindowTitleText()->b,
				 theme->getWindowTitleText()->a );
	} else {
	  glColor3f( 1, 1, 1 );
	}
#ifdef DEBUG_WINDOWS
    scourgeGui->texPrint(10, topY + 13, "%s (%d)", title, getZ());
#else
    scourgeGui->texPrint(10, topY + 13, "%s", title);
#endif
    glPopMatrix();
  }

  // draw the close button
  if(closeButton && !isLocked()) {

    // apply the window's color scheme
	/*
    closeButton->setColor( getColor() );
    closeButton->setBackground( getBackgroundColor() );
    closeButton->setSelectionColor( getSelectionColor() );
    closeButton->setBorderColor( getBorderColor() );
	*/

    if( theme->getButtonText() ) closeButton->setColor( theme->getButtonText() );
    if( theme->getButtonBackground() ) closeButton->setBackground( &(theme->getButtonBackground()->color) );
    if( theme->getButtonHighlight() ) closeButton->setSelectionColor( &(theme->getButtonHighlight()->color) );
    if( theme->getButtonBorder() ) closeButton->setBorderColor( &(theme->getButtonBorder()->color) );


    glPushMatrix(); 
    //glLoadIdentity();
    glTranslated(w - (closeButton->getWidth() + 3), topY + 3, z + 5);
    closeButton->draw(this);
    glPopMatrix();
  }

  glDisable( GL_SCISSOR_TEST );

  // draw widgets
  bool tmp = isOpening();
  if(tmp) {  
    scissorToWindow();
  }
  for(int i = 0; i < widgetCount; i++) {                  
    if(widget[i]->isVisible()) {

      glPushMatrix();
      glLoadIdentity();


      // if this is modified, also change handleWindowEvent
      glTranslated(x, y + topY + TOP_HEIGHT, z + 5);

      widget[i]->draw(this);
      glPopMatrix();
    }
  }  
  if(tmp) {  
    glDisable( GL_SCISSOR_TEST );
  }

  for(int i = 0; i < widgetCount; i++) {                  
    if(widget[i]->isVisible()) {
      widget[i]->drawTooltip( this );
    }
  }

  glEnable( GL_TEXTURE_2D );
  glPopMatrix();

  //glEnable( GL_DEPTH_TEST );
}


Button *Window::createButton(int x1, int y1, int x2, int y2, char *label, bool toggle){
  if(widgetCount < MAX_WIDGET){
    Button * theButton;
    theButton = new Button(x1, y1, x2, y2, scourgeGui->getHighlightTexture(), label);
    theButton->setToggle(toggle);     
    addWidget((Widget *)theButton);
    return theButton;
  } else{
    cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
    return NULL;
  }
} 

Label * Window::createLabel(int x1, int x2, char * label, int color){
  if(widgetCount < MAX_WIDGET){
    Label * theLabel;
    theLabel = new Label(x1, x2, label);  

    // addwidget call must come before setColor...
    addWidget((Widget *)theLabel);     

    // set new color or keep default color (black)
    if(color == Constants::RED_COLOR){        
      theLabel->setColor( 0.8f, 0.2f, 0.0f, 1.0f );            
    } else if(color == Constants::BLUE_COLOR){
      theLabel->setColor( 0.0f, 0.3f, 0.9f, 1.0f  );
    }
    return theLabel;
  } else{
    cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
    return NULL;
  }
} 

Checkbox * Window::createCheckbox(int x1, int y1, int x2, int y2, char *label){
  if(widgetCount < MAX_WIDGET){
    Checkbox * theCheckbox;
    theCheckbox = new Checkbox(x1, y1, x2, y2, scourgeGui->getHighlightTexture(), label);    
    addWidget((Widget *)theCheckbox);      
    return theCheckbox;
  } else{
    cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
    return NULL;
  }    
} 

TextField *Window::createTextField(int x, int y, int numChars) {
  if(widgetCount < MAX_WIDGET){
    TextField *tf;
    tf = new TextField(x, y, numChars);    
    addWidget((Widget *)tf);
    return tf;
  } else {
    cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
    return NULL;
  }    
}

// scissor test: y screen coordinate is reversed, rectangle is 
// specified by lower-left corner. sheesh!
void Window::scissorToWindow( bool insideOnly ) {
  GLint topY = ((h - (TOP_HEIGHT + BOTTOM_HEIGHT)) / 2) - (openHeight / 2);
  if( insideOnly ) {
    glScissor(x, scourgeGui->getScreenHeight() - (currentY + topY + openHeight + TOP_HEIGHT), 
              w, openHeight);  
  } else {
    glScissor(x, scourgeGui->getScreenHeight() - (currentY + topY + openHeight + 
                                               TOP_HEIGHT + BOTTOM_HEIGHT), 
              w, openHeight + (TOP_HEIGHT + BOTTOM_HEIGHT));  
  }
  glEnable( GL_SCISSOR_TEST );
}

void Window::setVisible(bool b, bool animate) {
  toTop();
  Widget::setVisible(b);
  if(b) {
    lastTick = 0;
    opening = ( !isLocked() );
    currentY = y;
    if( animate ) {
      if( animation == SLIDE_UP ) {
        openHeight = getHeight();
        y = getHeight();
      } else {
        openHeight = 0;
      }
    } else {
      openHeight = getHeight() - (TOP_HEIGHT + BOTTOM_HEIGHT);
    }
  } else {
    y = currentY;
    windowWasClosed = true;
    nextWindowToTop();

    // Any windows open?
    if( !anyFloatingWindowsOpen() ) scourgeGui->allWindowsClosed();
  }
}

bool Window::anyFloatingWindowsOpen() {
  bool found = false;
  for( int i = 0; i < Window::windowCount; i++ ) {
    if( Window::window[i]->isVisible() && 
        !Window::window[i]->isLocked() ) {
      found = true;
      break;
    }
  }
  return found;
}

void Window::toTop() {
  toTop(this);
}

void Window::toTop(Window *win) {
  currentWin = win;
  if(win->isLocked()) return;
  for(int i = 0; i < windowCount; i++) {
    if(window[i] == win) {
      for(int t = i; t < windowCount - 1; t++) {
        window[t] = window[t + 1];    
        window[t]->setZ(window[t]->getZ() - 10);
      }
      window[windowCount - 1] = win;
      win->setZ(50 + (windowCount * 10));
      break;
    }
  }
}

void Window::toBottom() {
  toBottom(this);
}

void Window::toBottom(Window *win) {
  if(win->isLocked()) return;
  for(int i = 0; i < windowCount; i++) {
    if(window[i] == win) {
      for(int t = i - 1; t >= 0; t--) {
        window[t] = window[t - 1];    
        window[t]->setZ(window[t]->getZ() + 10);
      }
      window[0] = win;
      win->setZ(50);
      break;
    }
  }
}

void Window::nextWindowToTop( bool includeLocked ) {
  bool next = false;
  
  for(int i = 0; i < windowCount; i++) {
    if(window[i]->isVisible() && window[i]->isModal()) {
      currentWin = window[i];
      currentWin->toTop();
      return;
    }
  }

  for(int t = 0; t < 2; t++) {
    for(int i = 0; i < windowCount; i++) {
      if( window[i]->isVisible() && next ) {
        currentWin = window[i];
        currentWin->toTop();
        return;
      } else if( !currentWin || currentWin == window[i]) {
        next = true;
      }
    }
  }
}

void Window::prevWindowToTop() {
  // FIXME: implement me; harder than nextWindowToTop() b/c toTop reorders windows
}

void Window::showMessageDialog(ScourgeGui *scourgeGui, 
                               int x, int y, int w, int h, 
                               char *title, GLuint texture,
                               char *message, 
                               char *buttonLabel) {
  if(message_dialog && message_dialog->isVisible()) {
    cerr << "*** Warning: Unable to display second message dialog: " << message << endl;
    return;
  }
  if(!message_dialog) {
    message_dialog = new Window( scourgeGui,
                                 x, y, w, h, 
                                 title, 
                                 texture, false );
    message_label = message_dialog->createLabel(10, 30, message);
    message_button = message_dialog->createButton((w / 2) - 50, h - (40 + TOP_HEIGHT), 
                                                  (w / 2) + 50, h - (10 + TOP_HEIGHT), buttonLabel);
    message_dialog->setModal(true);
  } else {
    message_dialog->move(x, y);
    message_dialog->resize(w, h);
    message_label->setText(message);
    message_button->setLabel(buttonLabel);
  }
  message_dialog->setVisible(true);
}

// overridden so windows stay on screen and moving/rotating still works
void Window::move(int x, int y) {
  this->x = x; 
  if(x< SCREEN_GUTTER) this->x = SCREEN_GUTTER;
  if(x >= scourgeGui->getScreenWidth() - (w + SCREEN_GUTTER)) this->x = scourgeGui->getScreenWidth() - (w + SCREEN_GUTTER + 1);
    
  int newY = y;
  if(y < SCREEN_GUTTER) newY = SCREEN_GUTTER;
  if(y >= scourgeGui->getScreenHeight() - (h + SCREEN_GUTTER)) newY = scourgeGui->getScreenHeight() - (h + SCREEN_GUTTER + 1);

  int diffY = newY - this->currentY;
  this->currentY = newY;
  if( animation == SLIDE_UP && this->y > this->currentY ) {
    this->y -= diffY;
  } else {
    this->y = newY;
  }
}

void Window::setLastWidget(Widget *w) {
  if(w != lastWidget) {
    lastWidget = w;
    scourgeGui->playSound(Window::ROLL_OVER_SOUND);
  }
}

void Window::setMouseLock( Widget *widget ) {
  if( widget ) {
    mouseLockWindow = this;
    mouseLockWidget = widget;
    getScourgeGui()->lockMouse( this );
  } else {
    mouseLockWindow = NULL;
    mouseLockWidget = NULL;
    getScourgeGui()->unlockMouse();
  }
}
