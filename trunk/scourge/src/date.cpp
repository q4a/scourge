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

void Date::setDate(int s, int m, int h, int day, int month, int year){
    this->sec = s;      
    this->min = m;    
    this->hour = h;         
    this->day = day;   
    this->month = month;  
    this->year = year;   
}

void Date::reset(){
    setDate(0, 0, 0, 1, 1, 1128);   // 01/01/1128, 00:00:00
}

void Date::addSeconds(int nbSec){
    
#ifdef DEBUG_DATE
    cout << " add " << nbSec << " TO ";    
    print();    
#endif              
    // This algo suppose that nbSec <= 60, otherwise, it doesn't work properly
    if(nbSec > 60) {
        nbSec = 60;
    }
    sec += nbSec;
                
    if(sec > 59){            
        sec -= 60;    
        min ++;
                        
        if(min > 59){                
            min = 0;    
            hour++;
                
            if(hour > 23){                
                hour = 0;
                day ++;                                    
                        
                if(day > dayInMonth[month]){
                    day = 1;
                    month ++;
                                                        
                    if(month > 12){
                        month = 1;
                        year ++;                    
                    }              
                }
            }
        }  
    } 
#ifdef DEBUG_DATE
    cout << " result : "; 
    print();
    cout << endl;
#endif
}

void Date::addDate(Date d){
    int s, m, h, dy, dy2, mth,  y;
    int nbMonth;
    
#ifdef DEBUG_DATE
    cout << "addDate :";
    d.print();
    cout << " TO ";
    this->print(); cout << endl;
#endif
    
    s = d.getSec();
    m = d.getMin();
    h = d.getHour();
    dy = d.getDay();
    mth = d.getMonth();
    y = d.getYear();
        
    s += sec;
    sec = s % 60;
    m += min;    
    min = (m + s/60) % 60;    
    h += hour;
    hour = (h + m/60) %24;
    
    dy += day;  
    dy2 = dy;
    day = dy2;
    nbMonth = 0;    
    while(dy2 -= dayInMonth[nbMonth + 1] > 0){         
        day = dy2;        
        nbMonth++;
    }                
    mth += month - 1;
#ifdef DEBUG_DATE
    cout << "mth =" << mth << " month=" << month << " nbMonth=" << nbMonth << endl; 
#endif
    month = ((mth + nbMonth) % 12) + 1;    
    y += year;
    year = (y + mth/12);
    

#ifdef DEBUG_DATE    
    cout << " = ";
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


