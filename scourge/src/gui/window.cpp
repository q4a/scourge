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

#define CLOSE_BUTTON_SIZE 10

Window *Window::message_dialog = NULL;
Label *Window::message_label = NULL;
Button *Window::message_button = NULL;
  
Window::Window(SDLHandler *sdlHandler, 
			   int x, int y, int w, int h, 
			   char *title, GLuint texture,
			   bool hasCloseButton) :
Widget(x, y, w, h) {
  this->sdlHandler = sdlHandler;
  this->title = title;
  this->texture = texture;
  this->visible = false;
  this->modal = false;
  this->widgetCount = 0;
  this->dragging = false;
  this->dragX = this->dragY = 0;
	if(hasCloseButton) this->closeButton = new Button(0, 0, CLOSE_BUTTON_SIZE, TOP_HEIGHT - 6);
	else closeButton = NULL;
  openHeight = 0;
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
  // find the topmost window
  Window *win = NULL;
  int maxz = 0;
  for (int i = 0; i < windowCount; i++) {
	if(window[i]->isVisible()) {
	  if(window[i]->isModal()) {
		win = window[i];
		break;
	  } else if(window[i]->isInside(x, y)) {
		if(maxz < window[i]->getZ()) {
		  win = window[i];
		  maxz = win->getZ();
		}
	  }
	}
  }
  // find the active widget
  Widget *widget = NULL;
  if (win) {
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

  // special handling
  if(message_button && w == message_button) {
	message_dialog->setVisible(false);
  }

  if(w)	return w;
  
  // handled by closebutton
  if(closeButton) {
	if(!insideWidget) {
	  insideWidget = closeButton->isInside(x - (getX() + (getWidth() - (closeButton->getWidth() + 3))), 
										   y - (getY() + 3));
	}
	if(closeButton->handleEvent(this, event, 
								x - (getX() + (getWidth() - (closeButton->getWidth() + 3))), 
								y - (getY() + 3))) {
	  return closeButton;
	}
  }
  
  if(insideWidget) {
	return this;
  }
  
  // see if the window wants it
  if(handleEvent(NULL, event, x, y)) {
	return this;
  }

  // swallow event if in a modal window
  return (isModal() ? this : NULL);
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
		toTop();
		dragging = isInside(x, y);
		dragX = x - getX();
		dragY = y - getY();
		break;
	}
	return isInside(x, y);
}

void Window::addWidget(Widget *widget) {
  if(widgetCount < MAX_WIDGET){
    this->widget[widgetCount++] = widget;
  }
  else{
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

  if(!isModal()) {
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  applyBackgroundColor();
  glBegin (GL_QUADS);
  glVertex2i (0, topY + TOP_HEIGHT);
  glVertex2i (0, topY + TOP_HEIGHT + openHeight);
  glVertex2i (w, topY + TOP_HEIGHT + openHeight);
  glVertex2i (w, topY + TOP_HEIGHT);
  glEnd();
  if(!isModal()) {
	glDisable( GL_BLEND );
  }
  
  // draw drop-shadow
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

  // add a border
  applyBorderColor();

  glLineWidth( isModal() ? 3.0f : 1.0f );
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

  glBegin(GL_LINES);
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

	// draw the close button
	if(closeButton) {
		glPushMatrix();	
		//glLoadIdentity();
		glTranslated(w - (closeButton->getWidth() + 3), topY + 3, z + 5);
		closeButton->draw(this);
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


Button *Window::createButton(int x1, int y1, int x2, int y2, char *label, bool toggle){
    if(widgetCount < MAX_WIDGET){
        Button * theButton;
        theButton = new Button(x1, y1, x2, y2, strdup(label));
    	theButton->setToggle(toggle);	   	
    	addWidget((Widget *)theButton);
    	return theButton;
	}
	else{
	   cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
	   return NULL;
	}
} 

Label * Window::createLabel(int x1, int x2, char * label, int color){
    if(widgetCount < MAX_WIDGET){
        Label * theLabel;
        theLabel = new Label(x1, x2, label);  
          
        // set new color or keep default color (black)
        if(color == Constants::RED_COLOR){        
            theLabel->setColor( 0.8f, 0.2f, 0.0f, 1.0f );            
        }
        else if(color == Constants::BLUE_COLOR){
            theLabel->setColor( 0.0f, 0.3f, 0.9f, 1.0f  );
        }               
        addWidget((Widget *)theLabel);     
        return theLabel;
    }
	else{
	   cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
	   return NULL;
	}
} 

Checkbox * Window::createCheckbox(int x1, int y1, int x2, int y2, char *label){
    if(widgetCount < MAX_WIDGET){
        Checkbox * theCheckbox;
        theCheckbox = new Checkbox(x1, y1, x2, y2, strdup(label));    
        addWidget((Widget *)theCheckbox);      
        return theCheckbox;
    }
    else{
	   cerr<<"Gui/Window.cpp : max widget limit reached!" << endl;
	   return NULL;
	}    
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
  toTop();
  Widget::setVisible(b);
  if(b) openHeight = 0;
}

void Window::toTop() {
  toTop(this);
}

void Window::toTop(Window *win) {
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

void Window::showMessageDialog(SDLHandler *sdlHandler, 
							   int x, int y, int w, int h, 
							   char *title, GLuint texture,
							   char *message, 
							   char *buttonLabel) {
  if(message_dialog && message_dialog->isVisible()) {
	cerr << "*** Warning: Unable to display second message dialog: " << message << endl;
	return;
  }
  if(!message_dialog) {
	message_dialog = new Window( sdlHandler,
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
	message_button->getLabel()->setText(buttonLabel);
  }
  message_dialog->setVisible(true);
}
