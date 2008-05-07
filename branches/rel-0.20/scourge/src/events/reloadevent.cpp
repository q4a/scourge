/***************************************************************************
                          reloadevent.cpp  -  description
                             -------------------
    begin                : Thu Apr 8 2004
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
#include "reloadevent.h"
#include "../session.h"
#include "../sqbinding/sqbinding.h"
#include "../party.h"

using namespace std;

ReloadEvent::ReloadEvent(Date currentDate, Date timeOut, int nbExecutionsToDo, Session *session, int mode) : 
Event(currentDate, timeOut, nbExecutionsToDo) {
  this->session = session;
  this->mode = mode;
}

void ReloadEvent::execute(){
	switch( mode ) {
	case MODE_RELOAD_SCRIPTS:
		session->getSquirrel()->reloadScripts(); break;
	case MODE_REGAIN_POINTS:
		session->getParty()->regainMp();
		session->getParty()->applyRecurringSpecialSkills(); 
		break;
	default:
		cerr << "Unknown mode " << mode << " in ReloadEvent." << endl;
  }
}

ReloadEvent::~ReloadEvent(){
}

