#include "conversation.h"
#include "configlang.h"
#include "util.h"
#include "creature.h"

using namespace std;

// ###### MS Visual C++ specific ###### 
#if defined(_MSC_VER) && defined(_DEBUG)
# define new DEBUG_NEW
# undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif 

std::map<std::string, Conversation*> Conversation::cachedConversations;
std::map<Conversation*, std::set<Creature*>*> Conversation::conversationRefs;

void Conversation::unref( Creature *creature, Conversation *conversation ) {
	if( conversationRefs.find( conversation ) != conversationRefs.end() ) {
		set<Creature*> *creatures = conversationRefs[ conversation ];
		creatures->erase( creature );
		if( creatures->size() == 0 ) {
			delete conversation;
		}
	}
}

Conversation *Conversation::ref( Creature *creature, std::string filename, GameAdapter *adapter ) {
	Conversation *conversation;
	bool keepGoing = true;
	while( keepGoing ) {
		if( cachedConversations.find( filename ) == cachedConversations.end() ) {
			conversation = new Conversation( filename, adapter );
		} else {
			conversation = cachedConversations[ filename ];
		}
		set<Creature*> *creatures = conversationRefs[ conversation ];
		creatures->insert( creature );
		
		// if nothing was read read from hq.cfg
		keepGoing = false;
		if ( conversation->empty() && filename != "general" ) {
			unref( creature, conversation );
			filename = "general";
			keepGoing = true;				
		}
	}
	return conversation;
}

// -------------------------------------
// initialization
//
Conversation::Conversation( string filename, GameAdapter *adapter ) {
	this->filename = filename;
	this->adapter = adapter;
	cachedConversations[filename] = this;
	conversationRefs[this] = new set<Creature*>();
	char tmp[3000];
	sprintf( tmp, "/maps/%s.cfg", filename.c_str() );
	string s = tmp;
	cerr << "Loading conversation file: " << s << endl;
	ConfigLang *config = ConfigLang::load( s );
	if( config ) {
		initConversations( config );
		delete config;
	}
}

Conversation::~Conversation() {
	conversationRefs.erase( this );
	cachedConversations.erase( getFilename() );
}

string Conversation::decodeKeyValue( string key ) {
	string::size_type n = key.find( "." );
	if ( n == string::npos ) {
		cerr << "Bad answer string name: " << key << endl;
		return "";
	} else {
		string k;
		string::size_type m = key.find( ".", n + 1 );
		return( m == string::npos ? key.substr( n + 1 ) : key.substr( n + 1, m - n - 1 ) );
	}
}

void Conversation::initConversations( ConfigLang *config ) {
	map<string, string> keyphrases;
	map<string, map<string, vector<string>*>*> answers;

	char const* currentNpc;
	vector<ConfigNode*> *v = config->getDocument()->getChildrenByName( "conversation" );
	for ( unsigned int i = 0; v && i < v->size(); i++ ) {
		ConfigNode *node = ( *v )[i];

		char const* name = node->getValueAsString( "name" );

		// if it's NOT general
		currentNpc = NULL;
		if ( strcmp( name, "general" ) ) {

			Monster *m = Monster::getMonsterByName( name );
			if ( m ) {
				currentNpc = m->getType();
			} else {
				Creature *c = adapter->getSession()->getCreatureByName( name );
				if ( c ) currentNpc = c->getName();
				else {
					cerr << "*** Error: can't find creature named: " << name << endl;
				}
			}
		}

		for ( map<string, ConfigValue*>::iterator e = node->getValues()->begin();
		        e != node->getValues()->end(); ++e ) {
			string key = e->first;
			ConfigValue *value = e->second;

			if ( key == "name" ) {
				// do nothing
			} else if ( key.substr( 0, 3 ) == "key" ) {
				string k = decodeKeyValue( key );
				if ( k != "" ) {
					keyphrases[k] = value->getAsString();
				}
			} else {
				string k = decodeKeyValue( key );
				if ( k != "" ) {
					string currentName = ( currentNpc == NULL ? "general" : currentNpc );
					map<string, vector<string>*> *m;
					if ( answers.find( currentName ) == answers.end() ) {
						m = new map<string, vector<string>*>();
						answers[currentName] = m;
					} else {
						m = answers[currentName];
					}
					vector<string>* v;
					if ( m->find( k ) == m->end() ) {
						v = new vector<string>();
						( *m )[k] = v;
					} else {
						v = ( *m )[k];
					}
					v->push_back( value->getAsString() );
				}
			}
		}
	}

	for ( map<string, map<string, vector<string>*>*>::iterator e = answers.begin();
	        e != answers.end(); ++e ) {
		string phase = e->first;
		map<string, vector<string>*>* m = e->second;

		for ( map<string, vector<string>*>::iterator e2 = m->begin();
		        e2 != m->end(); ++e2 ) {
			string key = e2->first;
			vector<string> *v = e2->second;
			if ( keyphrases.find( key ) == keyphrases.end() ) {
				cerr << "*** Error: can't find conversation keyphrase id=" << key << endl;
			} else {
				string keyphrase = keyphrases[ key ];
				for ( unsigned int i = 0; i < v->size(); i++ ) {
					string answer = ( *v )[i];
					if ( phase == "general" ) {
						setGeneralConversationLine( keyphrase, answer );
					} else {
						setConversationLine( phase, keyphrase, answer );
					}
				}
			}
		}
	}

	// cleanup
	for ( map<string, map<string, vector<string>*>*>::iterator e = answers.begin();
	        e != answers.end(); ++e ) {
		//string phase = e->first;
		map<string, vector<string>*>* m = e->second;
		for ( map<string, vector<string>*>::iterator e2 = m->begin();
		        e2 != m->end(); ++e2 ) {
			//string key = e2->first;
			vector<string> *v = e2->second;
			delete v;
		}
		delete m;
	}
}

void Conversation::setGeneralConversationLine( string keyphrase, string answer ) {
	storeConversationLine( keyphrase,
	                       answer,
	                       &intros,
	                       &unknownPhrases,
	                       &conversations,
	                       &firstKeyPhrase,
	                       &answers );
}

void Conversation::setConversationLine( string npc, string keyphrase, string answer ) {
	NpcConversation *npcConv;
	if ( npcConversations.find( npc ) == npcConversations.end() ) {
		npcConv = new NpcConversation();
		npcConversations[ npc ] = npcConv;
	} else {
		npcConv = npcConversations[ npc ];
	}

	storeConversationLine( keyphrase,
	                       answer,
	                       &npcConv->npc_intros,
	                       &npcConv->npc_unknownPhrases,
	                       &npcConv->npc_conversations,
	                       &npcConv->npc_firstKeyPhrase,
	                       &npcConv->npc_answers );
}

void Conversation::storeConversationLine( string keyphrase,
                                          string answer,
                                          vector<string> *intros,
                                          vector<string> *unknownPhrases,
                                          map<string, int> *conversations,
                                          map<string, string> *firstKeyPhrase,
                                          vector<string> *answers ) {
	if ( keyphrase == INTRO_PHRASE ) {
		intros->push_back( answer );
	} else if ( keyphrase == UNKNOWN_PHRASE ) {
		unknownPhrases->push_back( answer );
	} else {
		char line[300];
		strcpy( line, keyphrase.c_str() );

		char tmp[80];
		char *p = strtok( line, "," );
		string first = p;
		while ( p ) {
			strcpy( tmp, p );
			string lower = Util::toLowerCase( tmp );
			( *firstKeyPhrase )[ lower ] = first;
			( *conversations )[ lower ] = answers->size();
			p = strtok( NULL, "," );
		}
		answers->push_back( answer );
	}
}

// -------------------------------------
// lookup
//
char const* Conversation::getIntro() {
	if ( !intros.empty() ) {
		return intros[ Util::dice( intros.size() ) ].c_str();
	} else {
		cerr << "Error: Mission has 0 intros" << endl;
		return "";
	}
}

char const* Conversation::getAnswer( char const* keyphrase ) {
	string ks = keyphrase;
	if ( conversations.find( ks ) != conversations.end() ) {
		return  answers[ conversations[ ks ] ].c_str();
	} else {
		cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
		return unknownPhrases[ Util::dice( unknownPhrases.size() ) ].c_str();
	}
}

char const* Conversation::getFirstKeyPhrase( char const* keyphrase ) {
	string ks = keyphrase;
	if ( firstKeyPhrase.find( ks ) != firstKeyPhrase.end() ) {
		return firstKeyPhrase[ ks ].c_str();
	} else {
		cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
		return unknownPhrases[ Util::dice( unknownPhrases.size() ) ].c_str();
	}
}

char const* Conversation::getIntro( char const* npc ) {
	string s = npc;
	if ( npcConversations.find( s ) == npcConversations.end() ) {
		//cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
		return NULL;
	}
	NpcConversation *nc = npcConversations[ s ];
	return nc->npc_intros[ Util::dice( nc->npc_intros.size() ) ].c_str();
}

bool Conversation::setIntro( Creature *s, char const* keyphrase ) {
	NpcConversation *nc = NULL;
	string npc = s->getMonster()->getType();
	if ( npcConversations.find( npc ) != npcConversations.end() ) {
		nc = npcConversations[ npc ];
	}
	if ( !nc ) {
		npc = npc = s->getName();
		if ( npcConversations.find( npc ) != npcConversations.end() ) {
			nc = npcConversations[ npc ];
		}
	}
	if ( !nc ) {
		cerr << "*** Error: can't find conversation with: " << s << endl;
		return false;
	}

	string ks = keyphrase;
	Util::toLowerCase( ks );
	if ( nc->npc_conversations.find( ks ) != nc->npc_conversations.end() ) {
		nc->npc_intros.clear();
		nc->npc_intros.push_back( nc->npc_answers[ nc->npc_conversations[ ks ] ] );
	} else {
		cerr << "------------------------------------" << endl;
		for ( map<string, int>::iterator i = nc->npc_conversations.begin(); i != nc->npc_conversations.end(); ++i ) {
			cerr << i->first << "=" << i->second << endl;
		}
		cerr << "------------------------------------" << endl;
		cerr << "Can't find " << keyphrase << " in npc conversation for creature: " << s->getName() << endl;
		return false;
	}
	return true;
}

char const* Conversation::getAnswer( char const* npc, char const* keyphrase ) {
	string s( npc );
	if ( npcConversations.find( s ) == npcConversations.end() ) {
		//cerr << "Can't find npc conversation for creature: " << npc->getType() << endl;
		return NULL;
	}
	NpcConversation *nc = npcConversations[ s ];

	string ks = keyphrase;
	if ( nc->npc_conversations.find( ks ) != nc->npc_conversations.end() ) {
		return  nc->npc_answers[ nc->npc_conversations[ ks ] ].c_str();
	} else {
		cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
		return nc->npc_unknownPhrases[ Util::dice( nc->npc_unknownPhrases.size() ) ].c_str();
	}
}

char const* Conversation::getFirstKeyPhrase( char const* npc, char const* keyphrase ) {
	string s( npc );
	if ( npcConversations.find( s ) == npcConversations.end() ) {
		cerr << "Can't find npc conversation for creature: " << npc << endl;
		return NULL;
	}
	NpcConversation *nc = npcConversations[ s ];

	string ks = keyphrase;
	if ( nc->npc_firstKeyPhrase.find( ks ) != nc->npc_firstKeyPhrase.end() ) {
		return nc->npc_firstKeyPhrase[ ks ].c_str();
	} else {
		cerr << "*** Warning: Unknown phrase: " << keyphrase << endl;
		return NULL;
	}
}
