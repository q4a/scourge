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
#include "label.h"
#include "window.h"

/**
  *@author Gabor Torok
  */

Label::Label(int x, int y, char *text, int lineWidth) : 
  Widget(x, y, 0, 0) {
  this->text = text;
	this->lineWidth = lineWidth;
}

Label::~Label() {
}


void Label::drawWidget(Widget *parent) {
	if(text) {
		applyColor();
		if(lineWidth <= 1 || strlen(text) < lineWidth) {
			// draw a single-line label
			((Window*)parent)->getSDLHandler()->texPrint(0, 0, text);
		} else {
			// draw multi-line label
			char *p = (char*)malloc((lineWidth + 3) * sizeof(char));
			int len = strlen(text);
			int start = 0;
			int y = 0;
			//cerr << "len=" << len << endl;
			//cerr << "text=" << text << endl;
			while(start < len) {
				int end = start + lineWidth;
				if(end > len) end = len;
				else {
					// find a space (if any)
					int n = end;
					while(n > start && *(text + n) != ' ') n--;
					if(n > start) end = n + 1;
				}

				//cerr << "\tstart=" << start << endl;
				//cerr << "\tend=" << end << endl;
				//cerr << "\tend-start=" << (end-start) << endl;

				strncpy(p, text + start, end - start);
				*(p + end - start) = 0;
				//cerr << "p=" << p << endl;

				((Window*)parent)->getSDLHandler()->texPrint(0, y, p);
				start = end;
				y+=15;
			}
			free(p);
		}
	}
}
