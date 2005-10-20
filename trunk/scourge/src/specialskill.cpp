/***************************************************************************
                          specialskill.cpp  -  description
                             -------------------
    begin                : Sat Oct 19 2005
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
#include "specialskill.h"
#include "session.h"
#include "sqbinding/sqbinding.h"

/**
  *@author Gabor Torok
  */

using namespace std;

vector<SpecialSkill*> SpecialSkill::skills;
map<string,SpecialSkill*> SpecialSkill::skillsByName;

void SpecialSkill::initSkills( Session *session ) {

  // compile the squirrel file for special skills
  char s[200];
  sprintf(s, "%s/world/skills.nut", rootDir);
  if( !session->getSquirrel()->compile( s ) ) {
    cerr << "Error: *** Unable to compile special skills code: " << s << endl;
  } else {
    cerr << "Successfully compiled " << s << endl;
  }

  // Load the special skills
  char errMessage[500];
  sprintf(s, "%s/world/skills.txt", rootDir);
  FILE *fp = fopen(s, "r");
  if(!fp) {        
    sprintf(errMessage, "Unable to find the file: %s!", s);
    cerr << errMessage << endl;
    exit(1);
  }

  int type;
  char name[255], line[255], description[2000], 
    prereq[255], action[255];
  int n = fgetc(fp);
  while(n != EOF) {
    if( n == 'S' ) {
      // skip ':'
      fgetc( fp );
      n = Constants::readLine( name, fp );
      n = Constants::readLine( line, fp );
      strcpy( prereq, line + 2 );
      n = Constants::readLine( line, fp );
      type = atoi( line + 2 );
      n = Constants::readLine( line, fp );
      strcpy( action, line + 2 );
      strcpy( description, "" );
      while( true ) {
        n = Constants::readLine( line, fp );
        if( line[0] == 'D' ) {
          if( strlen( description ) ) strcat( description, " " );
          strcat( description, line + 2 );
        } else {
          break;
        }
      }
      
      cerr << "Storing special skill: " << name << " (" << prereq << "," << action << ")" << endl;
      SpecialSkill *ss = 
        new SpecialSkill( session, 
                          name, 
                          description, 
                          type, 
                          prereq, 
                          action );
      skills.push_back( ss );
      string nameStr = name;
      skillsByName[ nameStr ] = ss;
    } else {
      n = Constants::readLine(line, fp);
    }
  }
  fclose(fp);
}

SpecialSkill::SpecialSkill( Session *session, 
                            const char *name, 
                            const char *description, 
                            int type,
                            const char *squirrelFuncPrereq,
                            const char *squirrelFuncAction ) {
  this->session = session;
  this->name = name;
  this->description = description;
  this->type = type;
  this->squirrelFuncPrereq = squirrelFuncPrereq;
  this->squirrelFuncAction = squirrelFuncAction;
}

SpecialSkill::~SpecialSkill() {
}


