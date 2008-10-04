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

#include "common/constants.h"
#include "calendar.h"
#include "debug.h"
#include "creature.h"

using namespace std;

Calendar *Calendar::instance = NULL;


Calendar *Calendar::getInstance() {
	if ( !Calendar::instance ) {
		Calendar::instance = new Calendar();
	}
	return Calendar::instance;
}

Calendar::Calendar() {
	//currentDate = new Date();
	timeMultiplicator = 6;
	lastTick = 0;
	timeFrozen = false;
	strcpy( nextResetDate, "" );
}

void Calendar::reset( bool resetTime ) {
	lastTick = 0;
	if ( resetTime ) {
		currentDate.reset( nextResetDate );
		strcpy( nextResetDate, "" );
		if ( !scheduledEvents.empty() )
			scheduledEvents.clear();
	}
	timeFrozen = false;
}

/// Inserts an event into the queue.

void Calendar::scheduleEvent( Event *e ) {
	if ( scheduledEvents.size() < MAX_SCHEDULED_EVENTS ) {
		scheduledEvents.push_back( e );
	}
}

// FIXME: O(n)
void Calendar::cancelEvent( Event *e ) {
	for ( int i = 0; i < static_cast<int>( scheduledEvents.size() ); i++ ) {
		if ( scheduledEvents[i] == e ) {
			e->scheduleDeleteEvent();
			break;
		}
	}
}

// FIXME: O(n)
void Calendar::cancelEventsForCreature( Creature *creature ) {
	//cerr << "Cancelling events for creature: " << creature->getName() << endl;
	for ( int i = 0; i < static_cast<int>( scheduledEvents.size() ); i++ ) {
		if ( scheduledEvents[i]->doesReferenceCreature( creature ) ) {
			//cerr << "\t+++ Canceling event " << scheduledEvents[i]->getName() << " for " << creature->getName() << endl;
			scheduledEvents[i]->scheduleDeleteEvent();
		}
	}
}

void Calendar::setPause( bool mustPause ) {
	if ( !mustPause ) {
		// game is unfreezed, so starting time has changed
		lastTick = SDL_GetTicks();
	}
	timeFrozen = mustPause;
}

bool Calendar::update( int gameSpeed ) {
	Event * e;

	// no time update if time is frozen
	if ( timeFrozen ) return false;

	GLint t = SDL_GetTicks();

	if ( t - lastTick >= 1000 ) {
		lastTick = t;
		setTimeMultiplicator( 24 ); // 1 game day = 1 hr
		currentDate.addSec( timeMultiplicator );
		if ( CALENDAR_DEBUG ) cerr << "time: " << currentDate.getDateString() << endl;
	}

	// look for scheduled events
	if ( CALENDAR_DEBUG ) cout << "nbScheduled events: " <<  scheduledEvents.size() << endl;
	for ( int i = 0 ; i < static_cast<int>( scheduledEvents.size() ); i++ ) {
		if ( CALENDAR_DEBUG ) {
			currentDate.print();
			cout << " >= ";
			scheduledEvents[i]->getEventDate().print();
			cout << " ? ";
			cout << "\tEvent:" << scheduledEvents[i]->getName() << endl;

			// this will cause exception once level is left. (but useful for debugging.)
			//" creature=" << ( scheduledEvents[i]->getCreature() ? scheduledEvents[i]->getCreature()->getName() : "null" ) << endl;
		}
		// eventDate >= currentDate ?
		if ( !( currentDate.isInferiorTo( scheduledEvents[i]->getEventDate() ) ) ) {
			if ( CALENDAR_DEBUG ) cout << " Yes " << endl;
			if ( !scheduledEvents[i]->isCancelEventSet() )
				scheduledEvents[i]->execute();

			// remove this event as it has been executed
			e = scheduledEvents[i];
			scheduledEvents[i] = scheduledEvents[scheduledEvents.size()-1];
			scheduledEvents[scheduledEvents.size()-1] = e;
			scheduledEvents.pop_back();
			e -> increaseNbExecutions();
			if ( CALENDAR_DEBUG ) cout << "NbExecutions : " << e->getNbExecutions() << "/" << e->getNbExecutionsToDo() << endl;

			// and re-adds it if needed
			if ( CALENDAR_DEBUG ) cout << " readd ? ";
			if ( e->getNbExecutionsToDo() == Event::INFINITE_EXECUTIONS || e->getNbExecutions() < e->getNbExecutionsToDo() ) {

				Date d;
				//d = e->getEventDate();
				d.setDate( currentDate.getShortString() );
				d.addDate( e->getTimeOut() );

				if ( CALENDAR_DEBUG ) {
					cout << " Yes" << endl;
					cout << "Readding event at date: " << endl;
					d.print();
					cout << endl;
				}

				e->setEventDate( d );
				scheduleEvent( e );
			} else {
				// Don't need this event anymore
				if ( CALENDAR_DEBUG ) cout << " No, deleting this event." << endl;
				// call the last execution to do cleanup in event if any
				if ( !scheduledEvents[i]->isCancelEventSet() )
					e->executeBeforeDelete();
				delete e;
				if ( CALENDAR_DEBUG ) cout << " Ok, event deleted." << endl;
			}
		} else {
			if ( CALENDAR_DEBUG ) cout << " No" << endl;
		}
	}
	return true;
}

void Calendar::setTimeMultiplicator( int t ) {
	if ( t > 0 && t <= 60 ) {
		timeMultiplicator = t;
	}
}

Calendar::~Calendar() {
	Calendar::instance = NULL;
	//if(currentDate) delete currentDate;
}
