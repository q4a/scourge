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
class Notifier { 
public:
	Notifier() {}
	virtual ~Notifier() {}
	virtual bool notify( Widget* w ) = 0;
};

/// real listener info (can contain callback of any subscribers)
template < typename T >
class ObserverInfo : public Notifier {
public:
	typedef bool( T::*Callback )( Widget* );
	ObserverInfo( Callback update, T* observer )
		: Notifier()
		, _update( update )
		, _observer( observer ) {
	}
	virtual ~ObserverInfo() {}
	virtual bool notify( Widget* w ) {
		return (_observer->*_update)( w );
	}
private:
	Callback _update;
	T* _observer;
};

// ***** Under construction ENDS ****
#endif
