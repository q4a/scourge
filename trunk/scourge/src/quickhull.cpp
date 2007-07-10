/***************************************************************************
                          quickhull.cpp  -  description
                             -------------------
    begin                : Sun July 8 2007
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
/**
This quickhull alg. was described here:
http://www.cse.yorku.ca/~aaw/Hang/quick_hull/Algorithm.html
*/
#include "quickhull.h"

using namespace std;

// from: http://www.ahristov.com/tutorial/geometry-games/convex-hull.html
// find which side of the segment AB is P on
int pointLocation( CVector2 *A, CVector2 *B, CVector2 *P ) {
	float cp1 = ( B->x - A->x ) * ( P->y - A->y ) - ( B->y - A->y ) * ( P->x - A->x );
	return ( cp1 > 0 ) ? 1 : -1;
}

// from: http://www.ahristov.com/tutorial/geometry-games/convex-hull.html
// find the 'distance' of C from the segment AB
float distance( CVector2 *A, CVector2 *B, CVector2 *C ) {
	float ABx = B->x - A->x;
	float ABy = B->y - A->y;
	float num = ABx * ( A->y - C->y ) - ABy * ( A->x - C->x );
	if( num < 0 ) num = -num;
	return num;
}

// Find points on convex hull from the set Sk of points
// that are on the right side of the oriented line from P to Q
void findHull( set<CVector2*> *sk, CVector2 *p, CVector2 *q, vector<CVector2*> *out ) {
	if( sk == NULL || sk->size() == 0 ) return;

	// find the point C which is the farthest from the segment pq
	float d = -1;
	CVector2 *c;
	for( set<CVector2*>::iterator e = sk->begin(); e != sk->end(); ++e ) {
		CVector2 *point = *e;
		float dist = distance( p, q, point );
		if( d == -1 || dist > d ) {
			d = dist;
			c = point;
		}
	}

	// add c to the hull between p and q
	for( vector<CVector2*>::iterator e = out->begin(); e != out->end(); ++e ) {
		if( *e == p ) {
			out->insert( e, c );
			break;
		}
	}

	// partition the rest of the points into two new sets: right of pc and right of cq
	set<CVector2*> s1, s2;
	for( set<CVector2*>::iterator e = sk->begin(); e != sk->end(); ++e ) {
		CVector2 *point = *e;
		if( point != c ) {
			if( pointLocation( p, c, point ) < 0 ) s1.insert( point );
			else if( pointLocation( c, q, point ) < 0 ) s2.insert( point );
		}
	}

	// recursively find rest of hull points
	findHull( &s1, p, c, out );
	findHull( &s2, c, q, out );
}

void QuickHull::findConvexHull( vector<CVector2*> *in, vector<CVector2*> *out ) {
	out->clear();
	if( !in || !out || in->size() < 2 ) return;
	// find the right- and left-most points
	CVector2 *a, *b;
	a = b = in->at( 0 );
	for( unsigned i = 0; i < in->size(); i++ ) {
    CVector2 *p = in->at( i );
		if( p->x < a->x ) a = p;
		else if( p->x > b->x ) b = p;
	}
	cerr << "Quickhull: min=" << a->x << "," << a->y << " max=" << b->x << "," << b->y << endl;

	out->push_back( a );
	out->push_back( b );

	// find points to the right of ab and to the right of ba
	set<CVector2*> s1, s2;
	for( unsigned i = 0; i < in->size(); i++ ) {
		CVector2 *p = in->at( i );
		if( p != a && p != b ) {
			if( pointLocation( a, b, p ) < 0 ) s1.insert( p );
			else if( pointLocation( b, a, p ) < 0 ) s2.insert( p );
		}
	}

	findHull( &s1, a, b, out );
	findHull( &s2, b, a, out );
	cerr << "\tpoint count, in=" << in->size() << " out=" << out->size() << endl;
}

