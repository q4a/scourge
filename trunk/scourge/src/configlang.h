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
#include <set>

class ConfigValue;
class ConfigNode;
class ConfigLang;

class ConfigValue {
public:
	enum TYPE {
		STRING_TYPE=0,
		NUMBER_TYPE
	};

private:
	int type;
	bool translatable;
	std::string valueStr, translateStr;
	float valueNum;

public:
	ConfigValue( char *value );
  ConfigValue( ConfigValue *value );
	~ConfigValue();

	float getAsFloat();
	const char *getAsString();
};

class ConfigNode {
private:
	ConfigNode *super;
  ConfigLang *config;
	std::string name;
  std::string id;
	std::map<std::string,ConfigValue*> values;
	std::vector<ConfigNode*> children;
  std::map<std::string, std::vector<ConfigNode*>*> childrenByName;

public:
	ConfigNode( ConfigLang *config, std::string name );
	~ConfigNode();
	void addChild( ConfigNode *node );
	void addValue( std::string name, ConfigValue *value );

  inline ConfigLang *getConfig() { return config; }
	inline std::string getName() { return name; }
  inline std::string getId() { return id; }
	inline std::map<std::string, ConfigValue*>* getValues() { return &values; }
	inline std::vector<ConfigNode*>* getChildren() { return &children; }
	inline ConfigNode *getSuper() { return super; }
	inline void setSuper( ConfigNode *super ) { this->super = super; }

  // convenience methods
  inline const char *getValueAsString( std::string name ) {
    if( values.find( name ) == values.end() ) {
      return "";
    } else {
      return values[ name ]->getAsString();
    }
  }

  inline float getValueAsFloat( std::string name ) {
    if( values.find( name ) == values.end() ) {
      return 0;
    } else {
      return values[ name ]->getAsFloat();
    }
  }

	inline int getValueAsInt( std::string name ) {
    return toint( getValueAsFloat( name ) );
  }

	inline bool getValueAsBool( std::string name ) {
    if( values.find( name ) == values.end() ) {
      return false;
    } else {
      return( !strcasecmp( values[ name ]->getAsString(), "true" ) ||
							!strcasecmp( values[ name ]->getAsString(), "yes" ) ||
							!strcasecmp( values[ name ]->getAsString(), "on" ) ||
							values[ name ]->getAsFloat() > 0 );
    }
  }

	void getKeys( std::set<std::string> *keyset );
	

  inline bool hasValue( std::string name ) {
    return( values.find( name ) != values.end() );
  }

  inline std::vector<ConfigNode*> *getChildrenByName( std::string name ) {
    if( childrenByName.find( name ) == childrenByName.end() ) {
      return NULL;
    } else {
      return childrenByName[ name ];
    }
  }

  void extendNode( std::string id );

protected:
	// copy node's values and children into this node
	void copyFromNode( ConfigNode *node );

};

class ConfigLang {
private:
  std::map<std::string, ConfigNode*> idmap;
	ConfigNode *document;
	ConfigLang( char *config );
	void parse( char *config );
	std::string cleanText( char *p, int n );

public:
	~ConfigLang();
	void debug( ConfigNode *node, std::string indent="" );
	inline void debug() { debug( document ); }
  inline ConfigNode *getDocument() { return document; }
  inline std::map<std::string, ConfigNode*> *getIdMap() { return &idmap; }

	void setUpdate( char *message, int n=-1, int total=-1 );

	static ConfigLang *load( char *file );
};

#endif

