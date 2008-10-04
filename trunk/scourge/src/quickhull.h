/***************************************************************************
                    quickhull.h  -  Quick Hull Algorithm
                             -------------------
    begin                : Sun July 8 2007
    copyright            : (C) 2007 by Gabor Torok
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
/**
This quickhull alg. was described here:
http://www.cse.yorku.ca/~aaw/Hang/quick_hull/Algorithm.html
*/
#ifndef QUICK_HULL_H
#define QUICK_HULL_H
#pragma once

#include <vector>
#include <set>

/// Quick Hull algorithm.
class QuickHull {
public:
	static void findConvexHull( std::vector<CVector2*> *in, std::vector<CVector2*> *out );
};

#endif

