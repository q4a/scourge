/***************************************************************************
                          config.cpp  -  description
                             -------------------
    begin                : Thu May 15 2003
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
#include "configlang.h"
#include "session.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

using namespace std;

ConfigValue::ConfigValue( ConfigValue *value ) {
  this->translatable = value->translatable;
  this->translateStr = value->translateStr;
  this->valueStr = value->valueStr;
  this->valueNum = value->valueNum;
  this->type = value->type;
}

ConfigValue::ConfigValue( char *pValue ) {
	char *value = strdup( pValue );
	original = value;
	translatable = false;
	translateStr = "";
	char *start = strchr( value, '\"' );
	if( start ) {
		char *end = strrchr( value, '\"' );
		if( end ) *end = '\0';
		valueStr = start + 1;
		char *p = strchr( value, '_' );
		if( p && p < start ) {
			translatable = true;
			translateStr = _( start + 1 );
#ifdef DEBUG_CONFIG_FILE
			if( translateStr == valueStr ) {
				cerr << "No translation for: " << ( start + 1 ) << endl;
			}
#endif
		}
		type = STRING_TYPE;
		valueNum = 0;
	} else {
		valueStr = value;
		valueNum = atof( value );
		type = NUMBER_TYPE;
	}
	free( value );
}

ConfigValue::~ConfigValue() {
}

float ConfigValue::getAsFloat() {
	return valueNum;
}
	
const char *ConfigValue::getAsString() {
	return( translatable ? translateStr.c_str() : valueStr.c_str() );
}

ConfigNode::ConfigNode( ConfigLang *config, string name ) {
  this->config = config;
	this->name = name;
  this->id = "";
	this->super = NULL;
}

ConfigNode::~ConfigNode() {
  for( map<string, vector<ConfigNode*>*>::iterator i = childrenByName.begin(); 
			 i != childrenByName.end(); ++i ) {
		string name = i->first;
		vector<ConfigNode*> *v = i->second;
    delete v;
  }
	for( vector<ConfigNode*>::iterator i = children.begin(); 
			 i != children.end(); ++i ) {
		ConfigNode *node = *i;
		delete node;
	}
	for( map<string, ConfigValue*>::iterator i = values.begin(); 
			 i != values.end(); ++i ) {
		string name = i->first;
		ConfigValue *value = i->second;
		delete value;
	}
}

void ConfigNode::addChild( ConfigNode *node ) { 
  children.push_back( node ); 
  vector<ConfigNode*> *v;
  if( childrenByName.find( node->getName() ) == childrenByName.end() ) {
    v = new vector<ConfigNode*>();
    childrenByName[ node->getName() ] = v;
  } else {
    v = childrenByName[ node->getName() ];
  }
  v->push_back( node );
}

void ConfigNode::addValue( std::string name, ConfigValue *value ) { 
  values[ name ] = value; 
  if( name == "id" ) {
    this->id = value->getAsString();
    (*(config->getIdMap()))[ id ] = this;  
  }
}

void ConfigNode::copyFromNode( ConfigNode *node ) {
	// copy the values
	for( map<string, ConfigValue*>::iterator i = node->getValues()->begin(); 
			 i != node->getValues()->end(); ++i ) {
		string name = i->first;
		if( name != "id" ) {
			ConfigValue *value = i->second;
			addValue( name, new ConfigValue( value ) );
		}
	}

	// copy the children
	for( vector<ConfigNode*>::iterator i = node->getChildren()->begin();
			 i != node->getChildren()->end(); ++i ) {
		ConfigNode *child = *i;
		ConfigNode *c = new ConfigNode( config, child->getName() );
		addChild( c );
		c->copyFromNode( child );
	}
}

void ConfigNode::extendNode( std::string id ) {
	ConfigNode *super = (*(config->getIdMap()))[id];
	if( super ) {
		// build a stack of parent nodes
		stack<ConfigNode*> hier;
		hier.push( super );
		while( super->getSuper() ) {
			hier.push( super->getSuper() );
			super = super->getSuper();
		}

		// copy into current node in reverse order
		while( !hier.empty() ) {
			super = hier.top();
			hier.pop();
			copyFromNode( super );
		}

		// remember our parent node
		setSuper( super );
	} else {
		cerr << "*** Error: can't find node with id=" << id << endl;
	}
}

void ConfigNode::getKeys( std::set<std::string> *keyset ) {
	for( map<string,ConfigValue*>::iterator e = values.begin(); e != values.end(); ++e ) {
		keyset->insert( e->first );
	}
}

ConfigLang::ConfigLang( char *config ) {
	document = NULL;
	//cerr << config << endl;
	parse( config );
	//debug();
}

ConfigLang::ConfigLang( vector<string> *lines ) {
	document = NULL;
	parse( lines );
}

ConfigLang::~ConfigLang() {
	delete document;
}

void ConfigLang::debug( ConfigNode *node, string indent, ostream &out ) {
	out << indent << "[" << node->getName() << "]" << endl;
	string s = indent + "  ";
	for( map<string, ConfigValue*>::iterator i = node->getValues()->begin(); 
			 i != node->getValues()->end(); ++i ) {
		string name = i->first;
		ConfigValue *value = i->second;
		out << s << name << "=" << value->getOriginal() << endl;
	}
	for( vector<ConfigNode*>::iterator i = node->getChildren()->begin(); 
			 i != node->getChildren()->end(); ++i ) {
		ConfigNode *n = *i;
		debug( n, s, out );
	}
	out << indent << "[/" << node->getName() << "]" << endl;
}

#define WHITESPACE " \t\n\r"
void ConfigLang::parse( vector<string> *lines ) {
	bool inValue = false;
	string name, value, extends;
	ConfigNode *node = NULL;
	stack<ConfigNode*> parents;

	for( unsigned int i = 0; i < lines->size(); i++ ) {
		string s = (*lines)[i];
		if( inValue ) {
			int index = s.find_last_not_of( WHITESPACE );
			if( s[index] == '\\' ) {
				value += s.substr( 0, index );
			} else {
				value += s.substr( 0, index + 1 );
				node->addValue( name, new ConfigValue( (char*)(value.c_str()) ) );
				inValue = false;
			}
		}	else {
			string::size_type first = s.find_first_not_of( WHITESPACE );
			if( first != string::npos ) {
				
				if( s[first] == '#' ) {
					// skip comment
				} else if( s[first] == '[' ) {
					string::size_type last = s.find( ']' );
					if( last != string::npos ) {
						if( s[first + 1] == '/' ) {
							// end tag
							node = parents.top();
							parents.pop();
							assert( node );
						} else {
							// start tag
							string tag = s.substr( first + 1, last - ( first + 1 ) );

							string::size_type n = tag.find( ',' );
							string extends = "";
							if( n != string::npos ) {
								extends = tag.substr( n + 1 );
								tag = tag.substr( 0, n );
							}

							if( !node ) {
								document = node = new ConfigNode( this, tag );
								parents.push( node );
							} else {
								ConfigNode *tmp = new ConfigNode( this, tag );
								node->addChild( tmp );
								parents.push( node );
								node = tmp;
							}
							if( extends != "" ) {
								node->extendNode( extends );
							}
						}
					}
				} else {
					string::size_type eq = s.find( '=', first );
					if( eq != string::npos ) {
						string::size_type nameLast = s.find_last_not_of( WHITESPACE, eq );
						name = s.substr( first, nameLast - first );
						string::size_type valueStart = s.find_first_not_of( WHITESPACE, eq + 1 );
						string::size_type valueEnd = s.find_last_not_of( WHITESPACE );
						if( s[valueEnd] == '\\' ) {
							value = s.substr( valueStart, valueEnd - valueStart );
							inValue = true;
						} else {
							node->addValue( name, new ConfigValue( (char*)(s.substr( valueStart, valueEnd - valueStart + 1 ).c_str()) ) ); 
						}
					}
				}
			}
		}
	}
	assert( parents.empty() );
}

void ConfigLang::parse( char *config ) {
	bool inQuotes = false;
	bool inValue = false;
  bool inComment = false;
	string name;
	int pos = 0;

	stack<ConfigNode*> parents;;
	ConfigNode *node = NULL;

	for( unsigned int i = 0; i < strlen( config ); i++ ) {
		char c = config[i];
		if( !inQuotes && c == '#' ) {
      inComment = true;
    } else if( !( inComment || inQuotes ) && c == '[' ) {
      pos = i + 1;
    } else if( !( inComment || inQuotes ) && c == ']' ) {
      string tag = cleanText( config + pos, i - pos );
      if( tag.at( 0 ) == '/' ) {
        node = parents.top();
        parents.pop();
        assert( node );
      } else {
				string::size_type n = tag.find( ',' );
				string extends = "";
				if( n != string::npos ) {
					extends = tag.substr( n + 1 );
					tag = tag.substr( 0, n );
					//cerr << tag << " extends " << extends << endl;
				}
        if( node == NULL ) {
          document = node = new ConfigNode( this, tag );
          parents.push( node );
        } else {
          ConfigNode *tmp = new ConfigNode( this, tag );
          node->addChild( tmp );
          parents.push( node );
          node = tmp;
        }
				if( extends != "" ) {
					node->extendNode( extends );
				}
      }
      pos = i + 1;
    } else if( !( inComment || inQuotes ) && c == '=' ) {
      name = cleanText( config + pos, i - pos );
      pos = i + 1;
      inValue = true;
    } else if( !inComment && c == '\"' ) {
			inQuotes = ( inQuotes ? false : true );
		} else if( config[ i - 1 ] != '\\' && 
							 ( c == '\n' || c == '\r' ) ) {
      if( inComment ) {
        inComment = false;
				pos = i + 1;
      } else if( inValue ) {
        string value = cleanText( config + pos, i - pos );
        node->addValue( name, new ConfigValue( (char*)(value.c_str()) ) );
        inQuotes = false;
        pos = i + 1;
        inValue = false;
      }
    }
	}
}

string ConfigLang::cleanText( char *p, int n ) {
	// look for the start
	int start = 0;
	for( int i = 0; i < n; i++ ) {
		char c = p[i];
		if( !( c == ' ' || c == '\t' || c == '\n' || c == '\r' ) ) {
			start = i;
			break;
		}
	}

	// look for the end
	int end = n;
	for( int i = n - 1; i >= 0; i-- ) {
		char c = p[i];
		if( !( c == '\n' || c == '\r' || c == ' ' || c == '\t' ) ) {
			end = i + 1;
			break;
		}
	}

	// remove end-of-line \-s
	string s = "";
  bool startEOL = false;
	for( int i = start; i < end; i++ ) {
		char c = *(p + i);
    if( c == '\n' || c == '\r' ) {
      // don't add EOL chars
      startEOL = true;
    } else if( c == '\\' && 
				i + 1 < end && 
				(*(p + i + 1) == '\r' || 
				 *(p + i + 1) == '\n' ) ) {
			// don't add end-of-line markers
		} else {
      if( startEOL ) {
        startEOL = false;
        //s += " ";
      }      
			s += c;
		}
	}
	return s;
	//return string( p + start, end - start );
}

ConfigLang *ConfigLang::fromString( char *str ) {
	return new ConfigLang( str );
}

ConfigLang *ConfigLang::fromString( vector<string> *lines ) {
	return new ConfigLang( lines );
}

#define LINE_PARSER 1

ConfigLang *ConfigLang::load( char *file, bool absolutePath ) {
	string rootDirString;
	if( absolutePath ) rootDirString = "";
	else {
		rootDirString = rootDir;
		rootDirString += "/";
	}
//	cerr << "File:" << ( rootDirString + file ) << endl;
	ifstream in;
	in.open( ( rootDirString + file ).c_str(), ios::in );
	if( !in ) {
		cerr << "Cannot open file: " << file << endl;
		return NULL;
	}

	Uint32 now = SDL_GetTicks();
#if LINE_PARSER == 1
	vector<string> lines;
	while( in.rdstate() == ifstream::goodbit ) {
		string line;
		getline( in, line );
		lines.push_back( line );
	}

//	for( unsigned int i = 0; i < lines.size(); i++ ) {
//		cerr << lines[i] << endl;
//	}
//	cerr << "read " << lines.size() << " lines." << endl;

	ConfigLang *config  = new ConfigLang( &lines );
#else
	std::stringstream ss;
	ss << in.rdbuf();

	ConfigLang *config = fromString( (char*)( ss.str().c_str() ) );
#endif

	cerr << "Parsed " << file << " in " << ( SDL_GetTicks() - now ) << " millis." << endl;
	in.close();
	return config;
	
	return NULL;
}

void ConfigLang::setUpdate( char *message, int n, int total ) {
	Session::instance->getGameAdapter()->setUpdate( message, n, total );
}

void ConfigLang::save( char *file, bool absolutePath ) {
	string rootDirString;
	if( absolutePath ) rootDirString = "";
	else {
		rootDirString = rootDir;
		rootDirString += "/";
	}
	ofstream out;
	out.open( ( rootDirString + file ).c_str(), ios::out );
	if( !out ) {
		cerr << "Cannot open file: " << file << endl;
	} else {
		Uint32 now = SDL_GetTicks();
		debug( getDocument(), "", out );
		out.close();
		cerr << "Saved " << file << " in " << ( SDL_GetTicks() - now ) << " millis." << endl;
	}
}
