/***************************************************************************
                          date.cpp  -  description
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

#include "date.h"

using namespace std;

int Date::dayInMonth[13] = {
    0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

const char * Date::monthName[13] = {
    NULL, 
    "January", "February", 
    "March", "April", 
    "May", "June", 
    "July", "August", 
    "September", "October", 
    "November", "December"
};

const char * Date::dayName[8] = {
    NULL, "Monday", "Tuesday", "Wednesday", "Thursday", 
    "Friday", "Saturday", "Sunday"
};
 
Date::Date(){    
    reset();
}

Date::Date(int sec, int min, int hour, int day, int month, int year){           
    setDate(sec, min, hour, day, month, year);           
}

Date::Date( char *shortString ) {
	setDate( shortString );
}

char *Date::getShortString() {
  sprintf( shortString, "%d/%d/%d/%d/%d/%d", year, month, day, hour, min, sec );
  return shortString;
}

void Date::setDate(int s, int m, int h, int day, int month, int year){
	this->sec = s;      
	this->min = m;    
	this->hour = h;         
	this->day = day;   
	this->month = month;  
	this->year = year;   
}

void Date::setDate( char *shortString ) {
  char *p = strdup( shortString );
  
  char *q = strtok( p, "/" );
  this->year = atoi( q );

  q = strtok( NULL, "/" );
  this->month = atoi( q );

  q = strtok( NULL, "/" );
  this->day = atoi( q );

  q = strtok( NULL, "/" );
  this->hour = atoi( q );

  q = strtok( NULL, "/" );
  this->min = atoi( q );

  q = strtok( NULL, "/" );
  this->sec = atoi( q );

  free( p );
}

void Date::reset( char *shortString ){
	if( shortString && strlen( shortString ) ) {
		setDate( shortString );
	} else {
		setDate( 0, 0, 0, 1, 1, 1128 );   // 01/01/1128, 00:00:00
	}
}

void Date::addSec( int n ) {
	sec += n;
	if( sec > 59 ) {
		sec -= 60;
		addMin( 1 );
	}
}

void Date::addMin( int n ) {
	min += n;
	if( min > 59 ) {
		min -= 60;
		addHour( 1 );
	}
}

void Date::addHour( int n ) {
	hour += n;
	if( hour > 23 ) {
		hour -= 24;
		addDay( 1 );
	}
}

void Date::addDay( int n ) {
	day += n;
	if( day >= dayInMonth[ month + 1 ] ) {
		day -= dayInMonth[ month + 1 ];
		addMonth( 1 );
	}
}

void Date::addMonth( int n ) {
	month += n;
	if( month > 11 ) {
		month -= 12;		
		addYear( 1 );
	}
	// FIXME?
	if( day >= dayInMonth[ month + 1 ] ) {
		day = dayInMonth[ month + 1 ] - 1;
	}
}

void Date::addYear( int n ) {
	year += n;
}

void Date::addDate( Date d ) {
#ifdef DEBUG_DATE
	cout << "addDate :";
	d.print();
	cout << " TO ";
	this->print(); 
	cout << endl;
#endif

	addSec( d.getSec() );
	addMin( d.getMin() );
	addHour( d.getHour() );
	addDay( d.getDay() );
	addMonth( d.getMonth() );
	addYear( d.getYear() );
	
#ifdef DEBUG_DATE
	cout << "final RESULT: ";
	this->print(); 
	cout << endl;
#endif
}


bool Date::isInferiorTo(Date d){    
    
    if(d.isEqualTo(*this)) return false;       
    
    if(year < d.getYear()) return true;
    else if(month < d.getMonth()) return true;            
    else if(month > d.getMonth()) return false;
    else if(day < d.getDay()) return true;
    else if(day > d.getDay()) return false;
    else if(hour < d.getHour()) return true;
    else if(hour > d.getHour()) return false;
    else if(min < d.getMin()) return true;
    else if(min > d.getMin()) return false;
    else if(sec < d.getSec()) return true;
    else if(sec > d.getSec()) return false;
    return false;
}

bool Date::isEqualTo(Date d){
    return(
            year == d.getYear()
        &&  month == d.getMonth()
        &&  day  == d.getDay()
        &&  hour == d.getHour()
        &&  min  == d.getMin()
        &&  sec  == d.getSec()
    );
}


void Date::print(){    
    cout << day << "/" << month << "/" << year << " "<< hour << "h" << min << ":" << sec << " ";
}


void Date::buildDateString(){
    char buff[10];
                
    buff[0] = monthName[month][0];
    buff[1] = monthName[month][1];
    buff[2] = monthName[month][2];
    buff[3] = ' ';
    buff[4] = '\0';    
    
    sprintf(dateString, "%s %.2d %.2d:%.2d:%.2d", 
            buff, (int)day, (int)hour, (int)min, (int)sec); 
             
}

Date::~Date(){

}

bool Date::isADayLater(Date date) {
  return(date.getYear() < getYear() ||
         (date.getYear() == getYear() && date.getMonth() < getMonth()) ||
         (date.getYear() == getYear() && date.getMonth() == getMonth() && date.getDay() < getDay()));
}

bool Date::isAnHourLater(Date date) {
  return(date.getYear() < getYear() ||
         (date.getYear() == getYear() && date.getMonth() < getMonth()) ||
         (date.getYear() == getYear() && date.getMonth() == getMonth() && date.getDay() < getDay()) ||
				 (date.getYear() == getYear() && date.getMonth() == getMonth() && date.getDay() == getDay() && date.getHour() < getHour()));
}


