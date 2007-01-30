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
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

using namespace std;

ConfigValue::ConfigValue( char *value ) {
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
}

ConfigValue::~ConfigValue() {
}

float ConfigValue::getAsFloat() {
	return valueNum;
}
	
const char *ConfigValue::getAsString() {
	return( translatable ? translateStr.c_str() : valueStr.c_str() );
}

ConfigNode::ConfigNode( string name ) {
	this->name = name;
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

ConfigLang::ConfigLang( char *config ) {
	document = NULL;
	//cerr << config << endl;
	parse( config );
	//debug();
}

ConfigLang::~ConfigLang() {
	delete document;
}

void ConfigLang::debug( ConfigNode *node, string indent ) {
	cerr << indent << node->getName() << endl;
	string s = indent + "  ";
	for( map<string, ConfigValue*>::iterator i = node->getValues()->begin(); 
			 i != node->getValues()->end(); ++i ) {
		string name = i->first;
		ConfigValue *value = i->second;
		cerr << s << name << "=" << value->getAsString() << endl;
	}
	for( vector<ConfigNode*>::iterator i = node->getChildren()->begin(); 
			 i != node->getChildren()->end(); ++i ) {
		ConfigNode *n = *i;
		debug( n, s );
	}
}

void ConfigLang::parse( char *config ) {
	bool inQuotes = false;
	bool inValue = false;
	string name;
	int pos = 0;

	stack<ConfigNode*> parents;;
	ConfigNode *node = NULL;

	for( unsigned int i = 0; i < strlen( config ); i++ ) {
		char c = config[i];
		if( c == '[' ) {
			pos = i + 1;
		} else if( c == ']' ) {
			string tag = cleanText( config + pos, i - pos );
			if( tag.at( 0 ) == '/' ) {
				node = parents.top();
				parents.pop();
				assert( node );
			} else {
				if( node == NULL ) {
					document = node = new ConfigNode( tag );
					parents.push( node );
				} else {
					ConfigNode *tmp = new ConfigNode( tag );
					node->addChild( tmp );
					parents.push( node );
					node = tmp;
				}
			}
			pos = i + 1;
		} else if( c == '=' ) {
			name = cleanText( config + pos, i - pos );
			pos = i + 1;
			inValue = true;
		} else if( c == '\"' ) {
			inQuotes = ( inQuotes ? false : true );
		} else if( config[ i - 1 ] != '\\' && 
							 inValue && 
							 ( c == '\n' || c == '\r' ) ) {
			string value = cleanText( config + pos, i - pos );

			node->addValue( name, new ConfigValue( (char*)(value.c_str()) ) );

			inQuotes = false;
			pos = i + 1;
			inValue = false;
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
    } else if( c == '\t' ) {
      s += " ";
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

ConfigLang *ConfigLang::load( char *file ) {
	string rootDirString = rootDir;
	ifstream in;
	in.open( ( rootDirString + "/" + file ).c_str(), ios::in );
	if( !in ) {
		cerr << "Cannot open file: " << file << endl;
		return NULL;
	}
	std::stringstream ss;
	ss << in.rdbuf();
	ConfigLang *config = new ConfigLang( (char*)( ss.str().c_str() ) );
	in.close();
	return config;
}

