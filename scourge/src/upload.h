/***************************************************************************
                          upload.h  -  description
                             -------------------
    begin                : Sat May 3 2003
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

#ifndef UPLOAD
#define UPLOAD

class Upload {
public:
	enum { RESULT_SIZE = 300 }; 
	typedef char RESULT[ RESULT_SIZE ];
	static int uploadScoreToWeb( char *score, RESULT& result );
};

#endif
