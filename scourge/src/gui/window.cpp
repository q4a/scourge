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

int Window::windowCount = 0;
Window *Window::window[MAX_WINDOW];

/**
  *@author Gabor Torok
  */
  
Window::Window(SDLHandler *sdlHandler, 
			   int x, int y, int w, int h, 
			   const char *title, GLuint texture) {
  this->sdlHandler = sdlHandler;
  this->x = x;
  this->y = y;
  this->h = h;
  this->w = w;
  this->title = strdup(title);
  this->texture = texture;
  this->visible = false;
  this->widgetCount = 0;
  this->dragging = false;
  this->dragX = this->dragY = 0;
  addWindow(this);
}

Window::~Window() {
  free(title);
  removeWindow(this);
}

void Window::addWindow(Window *win) {
  if(windowCount < MAX_WINDOW) window[windowCount++] = win;
}

void Window::removeWindow(Window *win) {
  if(windowCount) --windowCount;
}

void Window::drawVisibleWindows() {
  for(int i = 0; i < windowCount; i++) {
	if(window[i]->isVisible()) window[i]->draw();
  }
}

void Window::delegateEvent(SDL_Event *event, int x, int y) {
  for(int i = 0; i < windowCount; i++) {
	if(window[i]->isVisible()) {
	  window[i]->handleWindowEvent(event, x, y);
	}
  }
}

void Window::handleWindowEvent(SDL_Event *event, int x, int y) {
  // handled by a component?
  bool handled = false;
  for(int t = 0; t < widgetCount; t++) {
	if(widget[t]->canHandle(sdlHandler, event, x - getX(), y - (getY() + TOP_HEIGHT))) {
	  widget[t]->handleEvent(sdlHandler, event, x - getX(), y - (getY() + TOP_HEIGHT));
	  handled = true;
	}
  }
  // see if the window wants it
  if(!handled && canHandle(event, x, y))
	handleEvent(event, x, y);
}

bool Window::canHandle(SDL_Event *event, int x, int y) {
  return(dragging || 
  		 (x >= getX() && x < getX() + w &&
  		  y >= getY() && y < getY() + h));
}

void Window::handleEvent(SDL_Event *event, int x, int y) {
  switch(event->type) {
  case SDL_MOUSEMOTION:
	if(dragging) move(x - dragX, y - dragY);
	break;
  case SDL_MOUSEBUTTONUP:
	dragging = false;
	break;
  case SDL_MOUSEBUTTONDOWN:
	dragging = true;
	dragX = x - getX();
	dragY = y - getY();
	break;
  }
  // fire a dummy event, so the event it's not used by the map
  sdlHandler->fireEvent(NULL, event);
}

void Window::addWidget(Widget *widget) {
  if(widgetCount < MAX_WIDGET) this->widget[widgetCount++] = widget;
}

/*
void Window::removeWidget(Widget *widget) {
  for(int i = 0; i < widgetCount; i++) {
	if(widget[i] == widget) {
	  for(int t = i; t < widgetCount - 1; t++) {
		widget[t] = widget[t + 1];
	  }
	  widgetCount--;
	  return;
	}
  }
}
*/

void Window::draw() {

  glDisable( GL_DEPTH_TEST );

  glPushMatrix();
  glLoadIdentity( );
  glEnable( GL_TEXTURE_2D );
  // tile the background
  glColor3f(1.0f, 0.6f, 0.3f);
  glTranslated(x, y, 0);
  glBindTexture( GL_TEXTURE_2D, texture );
  glBegin (GL_QUADS);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, 0);
  glTexCoord2f (0.0f, TOP_HEIGHT/(float)TILE_H);
  glVertex2i (0, TOP_HEIGHT);
  glTexCoord2f (w/(float)TILE_W, TOP_HEIGHT/(float)TILE_H);
  glVertex2i (w, TOP_HEIGHT);
  glTexCoord2f (w/(float)TILE_W, 0.0f);      
  glVertex2i (w, 0);
  glEnd ();

  glBegin (GL_QUADS);
  glTexCoord2f (0.0f, 0.0f);
  glVertex2i (0, h - BOTTOM_HEIGHT);
  glTexCoord2f (0.0f, BOTTOM_HEIGHT/(float)TILE_H);
  glVertex2i (0, h);
  glTexCoord2f (w/(float)TILE_W, BOTTOM_HEIGHT/(float)TILE_H);
  glVertex2i (w, h);
  glTexCoord2f (w/(float)TILE_W, 0.0f);      
  glVertex2i (w, h - BOTTOM_HEIGHT);
  glEnd ();

  glDisable( GL_TEXTURE_2D );

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  applyBackgroundColor();
  glBegin (GL_QUADS);
  glVertex2i (0, TOP_HEIGHT);
  glVertex2i (0, h - BOTTOM_HEIGHT);
  glVertex2i (w, h - BOTTOM_HEIGHT);
  glVertex2i (w, TOP_HEIGHT);
  glEnd();
  glDisable( GL_BLEND );

  // add a border
  applyBorderColor();
  glBegin(GL_LINES);
  glVertex2d(w, h);
  glVertex2d(0, h);
  glVertex2d(0, 0);
  glVertex2d(w, 0);
  glVertex2d(0, 0);
  glVertex2d(0, h);
  glVertex2d(w, 0);
  glVertex2d(w, h);		            
  glVertex2i (0, TOP_HEIGHT);
  glVertex2i (w, TOP_HEIGHT);
  glVertex2i (0, h - BOTTOM_HEIGHT);
  glVertex2i (w, h - BOTTOM_HEIGHT);
  glEnd();

  // print title
  glColor3f( 1, 1, 1 );
  sdlHandler->texPrint(10, 13, "%s", title);

  // draw widgets
  for(int i = 0; i < widgetCount; i++) {
  	glPushMatrix();
  	glLoadIdentity();


	// if this is modified, also change handleWindowEvent
  	glTranslated(x, y + TOP_HEIGHT, 0);


	widget[i]->draw(this);
  	glPopMatrix();
  }

  glEnable( GL_TEXTURE_2D );
  glPopMatrix();

  glEnable( GL_DEPTH_TEST );
}

void Window::applyBorderColor() { 
  glColor3f(0.8f, 0.5f, 0.2f); 
}

void Window::applyBackgroundColor(bool opaque) { 
  glColor4f( 1, 0.75f, 0.45f, (opaque ? 1.0f : 0.85f) ); 
}
