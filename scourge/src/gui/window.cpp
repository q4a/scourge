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

#define OPEN_STEPS 10

int Window::windowCount = 0;
Window *Window::window[MAX_WINDOW];

/**
  *@author Gabor Torok
  */
  
Window::Window(SDLHandler *sdlHandler, 
			   int x, int y, int w, int h, 
			   char *title, GLuint texture) :
Widget(x, y, w, h) {
  this->sdlHandler = sdlHandler;
  this->title = title;
  this->texture = texture;
  this->visible = false;
  this->widgetCount = 0;
  this->dragging = false;
  this->dragX = this->dragY = 0;
  openHeight = 0;
  addWindow(this);
}

Window::~Window() {
  delete[] widget;
  removeWindow(this);
}

void Window::addWindow(Window *win) {
  if(windowCount < MAX_WINDOW) {
	win->setZ(500 + windowCount * 100);
	window[windowCount++] = win;
  }
}

void Window::removeWindow(Window *win) {
  if(windowCount) --windowCount;
}

void Window::drawVisibleWindows() {
  //  glDisable(GL_CULL_FACE);
  for(int i = 0; i < windowCount; i++) {
	if(window[i]->isVisible()) window[i]->drawWidget(NULL);
  }
}

Widget *Window::delegateEvent(SDL_Event *event, int x, int y) {
  // find the topmost window
  Window *win = NULL;
  int maxz = 0;
  for(int i = 0; i < windowCount; i++) {
	if(window[i]->isVisible() && window[i]->isInside(x, y)) {
	  if(maxz < window[i]->getZ()) {
		win = window[i];
		maxz = win->getZ();
	  }
	}
  }
  // find the active widget
  Widget *widget = NULL;
  if(win) {
	widget = win->handleWindowEvent(event, x, y);
  }
  return widget;
}

Widget *Window::handleWindowEvent(SDL_Event *event, int x, int y) {
  if(dragging) {
	handleEvent(NULL, event, x, y);
	return this;
  }

  // handled by a component?
  bool insideWidget = false;
  Widget *w = NULL;
  for(int t = 0; t < widgetCount; t++) {
	if(this->widget[t]->isVisible()) {
	  if(!insideWidget) 
		insideWidget = this->widget[t]->isInside(x - getX(), y - (getY() + TOP_HEIGHT));
	  if(this->widget[t]->handleEvent(this, event, x - getX(), y - (getY() + TOP_HEIGHT)))
		w = this->widget[t];
	}
  }
  if(w) return w;
  if(insideWidget) {
	return this;
  }

  // see if the window wants it
  if(handleEvent(NULL, event, x, y)) {
	  return this;
  }
  return NULL;
}

bool Window::isInside(int x, int y) {
  return(dragging || Widget::isInside(x, y));
}

bool Window::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
  switch(event->type) {
  case SDL_MOUSEMOTION:
	if(dragging) move(x - dragX, y - dragY);
	break;
  case SDL_MOUSEBUTTONUP:
	dragging = false;
	break;
  case SDL_MOUSEBUTTONDOWN:
	dragging = isInside(x, y);
	dragX = x - getX();
	dragY = y - getY();
	break;
  }
  return isInside(x, y);
}

void Window::addWidget(Widget *widget) {
  if(widgetCount < MAX_WIDGET) this->widget[widgetCount++] = widget;
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

void Window::drawWidget(Widget *parent) {
  
  GLint t = SDL_GetTicks();
  //if(openHeight < (h - (TOP_HEIGHT + BOTTOM_HEIGHT)) && (lastTick == 0 || t - lastTick > 15)) {
  if(openHeight < (h - (TOP_HEIGHT + BOTTOM_HEIGHT))) {
	lastTick = t;
	openHeight += ( h / OPEN_STEPS ); // always open in the same number of steps
	if(openHeight >= (h - (TOP_HEIGHT + BOTTOM_HEIGHT))) 
      openHeight = (h - (TOP_HEIGHT + BOTTOM_HEIGHT));
  }
  GLint topY = ((h - (TOP_HEIGHT + BOTTOM_HEIGHT)) / 2) - (openHeight / 2);  

  glPushMatrix();
  glLoadIdentity( );
  glEnable( GL_TEXTURE_2D );
  // tile the background
  glColor3f(1.0f, 0.6f, 0.3f);
  glTranslated(x, y, z);
  if(texture)
	glBindTexture( GL_TEXTURE_2D, texture );
  glBegin (GL_QUADS);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, topY);
  glTexCoord2f (0.0f, TOP_HEIGHT/(float)TILE_H);
  glVertex2i (0, topY + TOP_HEIGHT);
  glTexCoord2f (w/(float)TILE_W, TOP_HEIGHT/(float)TILE_H);
  glVertex2i (w, topY + TOP_HEIGHT);
  glTexCoord2f (w/(float)TILE_W, 0.0f);      
  glVertex2i (w, topY);
  glEnd ();

  glBegin (GL_QUADS);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, topY + TOP_HEIGHT + openHeight);
  glTexCoord2f (0.0f, BOTTOM_HEIGHT/(float)TILE_H);
  glVertex2i (0, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glTexCoord2f (w/(float)TILE_W, BOTTOM_HEIGHT/(float)TILE_H);
  glVertex2i (w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glTexCoord2f (w/(float)TILE_W, 0.0f);      
  glVertex2i (w, topY + TOP_HEIGHT + openHeight);
  glEnd ();

  glDisable( GL_TEXTURE_2D );

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  applyBackgroundColor();
  glBegin (GL_QUADS);
  glVertex2i (0, topY + TOP_HEIGHT);
  glVertex2i (0, topY + TOP_HEIGHT + openHeight);
  glVertex2i (w, topY + TOP_HEIGHT + openHeight);
  glVertex2i (w, topY + TOP_HEIGHT);
  glEnd();
  glDisable( GL_BLEND );

  // add a border
  applyBorderColor();
  glBegin(GL_LINES);
  glVertex2d(w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glVertex2d(0, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glVertex2d(0, topY);
  glVertex2d(w, topY);
  glVertex2d(0, topY);
  glVertex2d(0, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glVertex2d(w, topY);
  glVertex2d(w, topY + TOP_HEIGHT + openHeight + BOTTOM_HEIGHT);
  glVertex2i (0, topY + TOP_HEIGHT);
  glVertex2i (w, topY + TOP_HEIGHT);
  glVertex2i (0, topY + TOP_HEIGHT + openHeight);
  glVertex2i (w, topY + TOP_HEIGHT + openHeight);
  glEnd();

  // print title
  if(title) {
	glPushMatrix();
	glTranslated( 0, 0, 5 );
	glColor3f( 1, 1, 1 );
	sdlHandler->texPrint(10, topY + 13, "%s", title);
	glPopMatrix();
  }

  // draw widgets
  if(isOpening()) {  
	scissorToWindow();
  }
  for(int i = 0; i < widgetCount; i++) {                  
	if(widget[i]->isVisible()) {
	  glPushMatrix();
	  glLoadIdentity();
	
	
	  // if this is modified, also change handleWindowEvent
	  glTranslated(x, y + TOP_HEIGHT, z + 5);
	
	
	  widget[i]->draw(this);
	  glPopMatrix();
	}
  }  
  if(isOpening()) {  
    glDisable( GL_SCISSOR_TEST );
  }
  
  glEnable( GL_TEXTURE_2D );
  glPopMatrix();
  
  //glEnable( GL_DEPTH_TEST );
}

void Window::scissorToWindow() {
  GLint topY = ((h - (TOP_HEIGHT + BOTTOM_HEIGHT)) / 2) - (openHeight / 2);
  // scissor test: y screen coordinate is reversed, rectangle is 
  // specified by lower-left corner. sheesh!
  glScissor(x, sdlHandler->getScreen()->h - (y + topY + TOP_HEIGHT + openHeight), 
			w, openHeight);  
  glEnable( GL_SCISSOR_TEST );
}

void Window::setVisible(bool b) {
  Widget::setVisible(b);
  if(b) openHeight = 0;
}
