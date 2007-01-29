/***************************************************************************
                          config.h  -  description
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
#ifndef CONFIG_LANG_H
#define CONFIG_LANG_H

#include "common/constants.h"
#include <vector>
#include <map>

class ConfigNode;

class ConfigValue {
public:
	enum TYPE {
		STRING_TYPE=0,
		NUMBER_TYPE
	};

private:
	int type;
	bool translatable;
	std::string valueStr;
	float valueNum;

public:
	ConfigValue( char *value );
	~ConfigValue();

	float getAsFloat();
	std::string getAsString();
};
				
class ConfigNode {
private:
	std::string name;
	std::map<std::string,ConfigValue*> values;
	std::vector<ConfigNode*> children;

public:
	ConfigNode( std::string name );
	~ConfigNode();
	inline void addChild( ConfigNode *node ) { children.push_back( node ); }
	inline void addValue( std::string name, ConfigValue *value ) { values[ name ] = value; }

	inline std::string getName() { return name; }
	inline std::map<std::string, ConfigValue*>* getValues() { return &values; }
	inline std::vector<ConfigNode*>* getChildren() { return &children; }
};

class ConfigLang {
private:
	ConfigNode *document;
	ConfigLang( char *config );
	void parse( char *config );
	std::string cleanText( char *p, int n );
	

public:
	~ConfigLang();
	void debug( ConfigNode *node, std::string indent="" );
	inline void debug() { debug( document ); }

	static ConfigLang *load( char *file );
};

#endif

