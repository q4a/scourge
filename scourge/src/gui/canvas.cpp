/***************************************************************************
                          label.h  -  description
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
#include "canvas.h"
#include "window.h"

/**
  *@author Gabor Torok
  */

Canvas::Canvas(int x, int y, int x2, int y2, WidgetView *view, 
               DragAndDropHandler *dragAndDropHandler,
               bool highlightOnMouseOver) : 
  Widget(x, y, x2 - x, y2 - y) {
	this->view = view;
  this->dragAndDropHandler = dragAndDropHandler;
	this->x2 = x2;
	this->y2 = y2;
  this->dragging = false;
  this->highlightOnMouseOver = highlightOnMouseOver;
  this->inside = false;
  this->glowing = false;
  highlightBorders = false;
	drawBorders = true;
}

Canvas::~Canvas() {
}

void Canvas::drawWidget(Widget *parent) {
  GuiTheme *theme = ((Window*)parent)->getTheme();
  if( highlightOnMouseOver ) {
    drawButton( parent, 0, 0, x2 - x, y2 - y, 
                false, false, dragging, glowing, inside );
  }

  if(view && !((Window*)parent)->isOpening()) {
    glScissor(((Window*)parent)->getX() + x, 
              ((Window*)parent)->getScourgeGui()->getScreenHeight() - 
              (((Window*)parent)->getY() + ((Window*)parent)->getGutter() + y + getHeight()), 
              w, getHeight());  
    glEnable( GL_SCISSOR_TEST );
    
    glPushMatrix();
    view->drawWidgetContents(this);
    glPopMatrix();
    
    glDisable( GL_SCISSOR_TEST );
  }

  // draw the border
	if( drawBorders ) {
		if(highlightBorders) {
			glLineWidth( 3.0f );
		}
		if( theme->getButtonBorder() ) {
			glColor4f( theme->getButtonBorder()->color.r,
								 theme->getButtonBorder()->color.g,
								 theme->getButtonBorder()->color.b,
								 theme->getButtonBorder()->color.a );
		} else {
			applyBorderColor();
		}  
		glBegin( GL_LINE_LOOP );
		glVertex2d(0, 0);
		glVertex2d(0, y2 - y);
		glVertex2d(x2 - x, y2 - y);
		glVertex2d(x2 - x, 0);
		glEnd();
		glLineWidth( 1.0f );
	}
}

bool Canvas::handleEvent(Widget *parent, SDL_Event *event, int x, int y) {
	inside = (x >= getX() && x < x2 && y >= getY() && y < y2);
	switch(event->type) {
	case SDL_MOUSEMOTION:
  if(dragging &&
     (abs(dragX - x) > DragAndDropHandler::DRAG_START_DISTANCE ||
      abs(dragY - y) > DragAndDropHandler::DRAG_START_DISTANCE) &&
     dragAndDropHandler) {
    if(!dragAndDropHandler->startDrag(this, dragX, dragY)) {
      // cancel drag
      dragging = false;
    }
  }
  highlightBorders = (isInside(x, y) && dragAndDropHandler);  
  break;
  case SDL_MOUSEBUTTONUP:
  //if(dragAndDropHandler) dragAndDropHandler->receive(this);
  if( inside && dragAndDropHandler ) dragAndDropHandler->receive(this);
  dragging = false;
  return inside;
	case SDL_MOUSEBUTTONDOWN:
		if( event->button.button == SDL_BUTTON_LEFT ) {
			dragging = inside;
			dragX = x - getX();
			dragY = y - getY();
		}
		break;
  }
  return false;
}

void Canvas::removeEffects(Widget *parent) {
  highlightBorders = false;
}

ImageCanvas::ImageCanvas( int x, int y, int x2, int y2, GLuint image ) : Canvas( x, y, x2, y2, this, NULL, false ) {
	this->image = image;
}
  
ImageCanvas::~ImageCanvas() {
}

void ImageCanvas::drawWidgetContents( Widget *w ) {
	glEnable( GL_ALPHA_TEST );
	//glAlphaFunc( GL_EQUAL, 0xff );
	glAlphaFunc( GL_NOTEQUAL, 0 );
	glEnable(GL_TEXTURE_2D);
	glPushMatrix();
	glBindTexture( GL_TEXTURE_2D, image );
	glColor4f(1, 1, 1, 1);
	glBegin( GL_QUADS );
	glNormal3f( 0, 0, 1 );
	glTexCoord2f( 0, 0 );
	glVertex3f( 0, 0, 0 );
	glTexCoord2f( 0, 1 );
	glVertex3f( 0, getHeight(), 0 );
	glTexCoord2f( 1, 1 );
	glVertex3f( getWidth(), getHeight(), 0 );
	glTexCoord2f( 1, 0 );
	glVertex3f( getWidth(), 0, 0 );
	glEnd();
	glPopMatrix();	
	glDisable( GL_ALPHA_TEST );
	glDisable(GL_TEXTURE_2D);
}
