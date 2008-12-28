/***************************************************************************
                     guievent.h  -  GuiEvent and helper templates
                             -------------------
    created              : Sun Dec 28 2008
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
#ifndef GUIEVENT_H
#define GUIEVENT_H
#pragma once


// ***** Under construction STARTS ****
/// GUI Event Type
typedef char const* GuiEvent;

/// base class for event listener info  
class Subscription { 
public:
	Subscription() {}
	virtual ~Subscription() {}
	virtual bool fireIt( Widget* w ) = 0;
};

/// real listener info (can contain callback of any subscribers)
template < typename T >
class RealSubscription : public Subscription {
public:
	typedef bool( T::*Callback )( Widget* );
	RealSubscription( Callback callback, T* host )
		: Subscription()
		, _callback( callback )
		, _host( host ) {
	}
	virtual ~RealSubscription() {}
	virtual bool fireIt( Widget* w ) {
		return (_host->*_callback)( w );
	}
private:
	//Callback _callback;
	Callback _callback;
	T* _host;
};

// ***** Under construction ENDS ****
#endif
