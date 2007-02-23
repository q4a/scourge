#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

#define bool int
#define false 0
#define true 1

typedef struct _ConversationPair {
  string question;
  string answer;
} ConversationPair;
map<string,vector<ConversationPair>*> conversation;

class NpcInfo {
public:
  int x, y, level;
  char *npcName, *npcType, *npcSubType;

  NpcInfo( int x, int y, char *npcName, int level, char *npcType, char *npcSubType ) {
    this->x = x;
    this->y = y;
    this->npcName = npcName;
    this->level = level;
    this->npcType = npcType;
    this->npcSubType = npcSubType;
  }
};
vector<NpcInfo*> npcInfos;


/*
  Read until the EOL (or EOF whichever comes first)
  Put line chars into 'line', excluding EOL chars.
  Return first char after EOL.
 */
int readLine(char *line, FILE *fp) {
  bool reachedEOL = false;
  int lc = 0;
  int n;
  int ret = EOF;
  // read until the end of line
  while ((n = fgetc(fp)) != EOF) {
    bool isEOLchar = (n == '\n' || n == '\r');
    if (reachedEOL) {
      if (!isEOLchar) {
        //line[lc++] = '\0';
        ret = n;
        break;
      }
    } else {
      if (!isEOLchar) line[lc++] = ( n == '\"' ? '\'' : n );
      else reachedEOL = true;
    }
  }
  line[lc++] = '\0';
  // exclude same-line comment
  for ( int i = 0; i < lc; i++ ) {
    if ( line[i] == '#' || line[i] == '%' ) {
      line[i] = '\0';
      break;
    }
  }
  return ret;
}

int readConversationLine( FILE *fp, char *line, int n, vector<ConversationPair> *m ) {

  // find the first comma
  char keyphrase[80], answer[4000];
  char last = line[ strlen( line ) - 1 ];
  char *p = strchr( line, ';' );
	/*
	This happens if the line has bad syntax or in 'general-only' mode when skipping
	other lines.
	*/
	if( !p ) return last;
  strcpy( answer, p + 1 );
  strcpy( keyphrase, strtok( line, ";" ) );

  //cerr << "1: keyphrase=" << keyphrase << endl;

  // Read lines that end with a \.
  int r;
  while( last == '\\' ) {
    r = strlen( answer ) - 1;
    answer[ r ] = ' ';
    answer[ r + 1 ] = n;
    answer[ r + 2 ] = '\0';
    n = readLine(line, fp);
    strcat( answer, line );
    last = line[ strlen( line ) - 1 ];
  }

  ConversationPair cp;
  cp.question = keyphrase;
  cp.answer = answer;
  m->push_back( cp );
  
  return n;
}

void printConversation() {
  int id = 0;
  map<string,int> keys;
  
  cerr << "[map]" << endl;

  for( unsigned int i = 0; i < npcInfos.size(); i++ ) {
    NpcInfo *info = npcInfos[i];
    cerr << "\t[npc]" << endl;
    cerr << "\t\tname=\"" << info->npcName << "\"" << endl;
    cerr << "\t\tdisplay_name=_( \"" << info->npcName << "\" )" << endl;
    cerr << "\t\tposition=\"" << info->x << "," << info->y << "\"" << endl;
    cerr << "\t\tlevel=" << info->level << endl;
    cerr << "\t\ttype=\"" << info->npcType << "\"" << endl;
    if( info->npcSubType ) cerr << "\t\tsubtype=\"" << info->npcSubType << "\"" << endl;
    cerr << "\t[/npc]" << endl;
  }

  for( map<string,vector<ConversationPair>*>::iterator e = conversation.begin();
       e != conversation.end(); ++e ) {
    string section = e->first;
    vector<ConversationPair> *m = e->second;

    map<string,int> counts;
    cerr << "\t[conversation]" << endl;
    cerr << "\t\tname=\"" << section << "\"" << endl;
    for( vector<ConversationPair>::iterator e = m->begin(); e != m->end(); ++e ) {
      ConversationPair cp = *e;

      int count;
      if( counts.find( cp.question ) == counts.end() ) {
        count = 0;
      } else {
        count = counts[ cp.question ];
      }
      counts[ cp.question ] = count + 1;

      int index;
      if( keys.find( cp.question ) == keys.end() ) {
        index = id++;
        keys[cp.question] = index;
        cerr << "\t\tkeyphrase." << index << "=_( \"" << cp.question << "\" )" << endl;
      } else {
        index = keys[ cp.question ];
      }
        
      if( count > 0 ) {
        cerr << "\t\tanswer." << index << "." << count << "=_( \"" << cp.answer << "\" )" << endl;
      } else {
        cerr << "\t\tanswer." << index << "=_( \"" << cp.answer << "\" )" << endl;
      }
      
    }
    cerr << "\t[/conversation]" << endl;
  }
  cerr << "[/map]" << endl;
}

void loadMapDataFile( const char *filename ) {

  FILE *fp = fopen( filename, "r" );
  if( !fp ) return;

  bool inConversation = false;
  char line[1000];
  int x, y, level;
  char npcName[255], npcType[255], npcSubType[1000];
  //Monster *currentNpc = NULL;
	char *currentNpc = NULL;
  int n = fgetc(fp);
  while(n != EOF) {
    if(n == 'G') {
      fgetc(fp);
      n = readLine(line, fp);

      string key = "general";
      vector<ConversationPair> *m;
      if( conversation.find( key ) == conversation.end() ) {
        m = new vector<ConversationPair>();
        conversation[key] = m;
      } else {
        m = conversation[ key ];
      }
      n = readConversationLine( fp, line, n, m );
    } else if( n == 'P' ) {
      fgetc(fp);
      n = readLine(line, fp);

      currentNpc = strdup( line );

    } else if( n == 'V' && currentNpc ) {    
      fgetc(fp);
      n = readLine(line, fp);

      string key = currentNpc;
      vector<ConversationPair> *m;
      if( conversation.find( key ) == conversation.end() ) {
        m = new vector<ConversationPair>();
        conversation[key] = m;
      } else {
        m = conversation[ key ];
      }
      n = readConversationLine( fp, line, n, m );

    } else if( n == 'N' ) { 
      fgetc( fp );
      n = readLine(line, fp);

      x = atoi( strtok( line, "," ) );
      y = atoi( strtok( NULL, "," ) );
      strcpy( npcName, strtok( NULL, "," ) );
      level = atoi( strtok( NULL, "," ) );
      strcpy( npcType, strtok( NULL, "," ) );
      char *p = strtok( NULL, "," );
      strcpy( npcSubType, ( p ? p : "" ) );

      // store npc info
      NpcInfo *npcInfo = 
        new NpcInfo( x, y, 
                     strdup( npcName ), 
                     level, 
                     strdup( npcType ), 
                     ( strlen( npcSubType ) ? 
                       strdup( npcSubType ) : 
                       NULL ) );
      npcInfos.push_back( npcInfo );
    } else {
      n = readLine(line, fp);      
    }
  }
  fclose(fp);

  printConversation();
}

int main( int argc, char *argv[] ) {

  if( argc < 2 ) {
    fprintf( stderr, "Usage: convert mapfile.txt\n" );
    exit( 1 );
  }

  loadMapDataFile( argv[1] );

  return 0;
}
