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

  int type, event, iconTileX, iconTileY;
  char name[255], line[255], description[2000], 
    prereq[255], action[255];
  int n = fgetc(fp);
  while(n != EOF) {
    if( n == 'S' ) {
      // skip ':'
      fgetc( fp );
      n = Constants::readLine( name, fp );
      n = Constants::readLine( line, fp );
      strcpy( prereq, line + 1 );

      type = SpecialSkill::SKILL_TYPE_AUTOMATIC;
      event = SpecialSkill::SKILL_EVENT_DEFENSE;
      n = Constants::readLine( line, fp );
      char *p = strtok( line + 1, "," );
      if( p ) {
        switch(*p) {
        case 'A': type = SpecialSkill::SKILL_TYPE_AUTOMATIC; break;
        case 'M': type = SpecialSkill::SKILL_TYPE_MANUAL; break;
        default: cerr << "Unknown special skill type: " << (*p) << endl;
                type = SpecialSkill::SKILL_TYPE_MANUAL;
        }
        p = strtok( NULL, "," );
        if( p ) {
          switch(*p) {
          case 'D': event = SpecialSkill::SKILL_EVENT_DEFENSE; break;
          case 'T': event = SpecialSkill::SKILL_EVENT_TO_HIT; break;
          case 'A': event = SpecialSkill::SKILL_EVENT_DAMAGE; break;
          case 'S': event = SpecialSkill::SKILL_EVENT_STATE_MOD; break;
          default: cerr << "Unknown special skill event: " << (*p) << endl;
          event = SpecialSkill::SKILL_EVENT_DAMAGE;
          }
        }
      }

      n = Constants::readLine( line, fp );
      strcpy( action, line + 1 );

      iconTileX = iconTileY = 0;
      n = Constants::readLine( line, fp );      
      p = strtok( line + 1, "," );
      if( p ) {
        iconTileX = atoi( p ) - 1;
        if( iconTileX < 0 ) iconTileX = 0;
        p = strtok( NULL, "," );
        if( p ) iconTileY = atoi( p ) - 1;
        if( iconTileY < 0 ) iconTileY = 0;
      }


      strcpy( description, "" );
      while( n == 'D' ) {
        n = Constants::readLine( line, fp );
        if( strlen( description ) ) strcat( description, " " );
        strcat( description, line + 1 );
      }
      
      cerr << "Storing special skill: " << name << " (" << prereq << "," << action << ")" << endl;
      SpecialSkill *ss = 
        new SpecialSkill( session, 
                          strdup( name ), 
                          strdup( description ), 
                          type, 
                          event,
                          strdup( prereq ), 
                          strdup( action ),
                          iconTileX,
                          iconTileY );
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
                            int event,
                            const char *squirrelFuncPrereq,
                            const char *squirrelFuncAction,
                            int iconTileX,
                            int iconTileY ) {
  this->session = session;
  this->name = name;
  this->description = description;
  this->type = type;
  this->event = event;
  this->squirrelFuncPrereq = squirrelFuncPrereq;
  this->squirrelFuncAction = squirrelFuncAction;
  this->iconTileX = iconTileX;
  this->iconTileY = iconTileY;
}

SpecialSkill::~SpecialSkill() {
}


