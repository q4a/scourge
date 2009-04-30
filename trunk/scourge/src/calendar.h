/***************************************************************************
                   calendar.cpp  -  Schedules ingame events
                             -------------------
    begin                : Wed April 7 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CALENDAR_H
#define CALENDAR_H
#pragma once

#include <iostream>
#include <vector>    // STL for Vector
#include "date.h"
#include "events/event.h"

#define MAX_SCHEDULED_EVENTS 500

/**
  *@author Daroth-U
  */


class Event;
class Date;

/// Manages ingame events, like potions expiring and characters getting hungry.
class Calendar {
private:

	// moved inside getInstance(): static Calendar *instance;

	Date currentDate;
	int timeMultiplicator;
	GLint lastTick;
	bool timeFrozen;
	char nextResetDate[40];

	std::vector<Event*> scheduledEvents;

	// private, since calendar is singleton
	Calendar();
	~Calendar();
	// undefine default copying
	Calendar( Calendar const& that );
	Calendar& operator=( Calendar const& that );


public:

	static Calendar * getInstance();
	bool update( int gameSpeed );
	void setPause( bool mustPause );
	void setTimeMultiplicator( int t );
	void scheduleEvent( Event *e );
	void reset( bool resetTime = true );
	void cancelEvent( Event *e );
	void cancelEventsForCreature( Creature *creature );

	// next time you reset, set the date to this
	inline void setNextResetDate( char *s ) {
		strcpy( nextResetDate, s );
	}

	// return date by value to avoid modification by other classes
	inline Date getCurrentDate() {
		return currentDate;
	}
	inline void addAnHour() {
		currentDate.setDate( currentDate.getSec(), currentDate.getMin(), currentDate.getHour() + 1, currentDate.getDay(),
		                     currentDate.getMonth(), currentDate.getYear() );
	}
	inline void addADay() {
		currentDate.setDate( currentDate.getSec(), currentDate.getMin(), currentDate.getHour(), currentDate.getDay() + 1,
		                     currentDate.getMonth(), currentDate.getYear() );
	}
	inline void addAMonth() {
		currentDate.setDate( currentDate.getSec(), currentDate.getMin(), currentDate.getHour(), currentDate.getDay(),
		                     currentDate.getMonth() + 1 , currentDate.getYear() );
	}


protected:
};

#endif
