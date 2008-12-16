#ifndef DFGUI_H
#define DFGUI_H

#include "../common/constants.h"
#include "datafile.h"
#include <map>

struct Element
{
	std::string texture, north, south, east, west;
	Color color;		// color
	int lineWidth;
	void Clear() { texture = north = south = east = west = ""; color.Clear(); lineWidth = 1; }
};
struct TexturedBorder : public Element
{
	std::string nw,ne,sw,se;
	void Clear() { texture = north=south=east=west = nw=ne=sw=se = ""; color.Clear(); lineWidth = 1; }
};
struct Theme
{
	std::string name;
	std::map <std::string,Element*> elements;
	std::map <std::string,Color*> colors;
	TexturedBorder *texturedBorder;

	Theme()
	{
		elements["windowBack"] = new Element;
		elements["windowTop"] = new Element;
		elements["windowBorder"] = new Element;
		colors["windowTitleText"] = new Color;
		colors["windowText"] = new Color;
		elements["buttonBackground"] = new Element;
		elements["buttonSelectionBackground"] = new Element;
		elements["buttonHighlight"] = new Element;
		elements["buttonBorder"] = new Element;
		colors["buttonText"] = new Color;
		colors["buttonSelectionText"] = new Color;
		elements["listBackground"] = new Element;
		elements["inputBackground"] = new Element;
		colors["inputText"] = new Color;
		elements["selectionBackground"] = new Element;
		colors["selectionText"] = new Color;
		elements["selectedBorder"] = new Element;
		elements["selectedCharacterBorder"] = new Element;
		texturedBorder = new TexturedBorder;
	}
	~Theme()
	{
		for ( std::map<std::string,Element*>::iterator itr = elements.begin(); itr != elements.end(); itr++ )
			delete itr->second;
		elements.clear();

		for ( std::map<std::string,Color*>::iterator itr = colors.begin(); itr != colors.end(); itr++ )
			delete itr->second;
		colors.clear();

		delete texturedBorder;
	}
};

class DFGui : public DataFile<Theme>
{
protected:
	bool LoadSingle(std::ifstream*, Theme*);

	bool ParseElement(std::ifstream*,Element*);
	bool ParseColor(std::ifstream*,Color*);
	bool ParseTexturedBorder(std::ifstream*,TexturedBorder*);

	void SaveElement(std::ofstream&,Theme*,char*);
	void SaveColor(std::ofstream&,Theme*,char*);
	void SaveTexturedBorder(std::ofstream&,TexturedBorder*);

public:
	void Save();

};

#endif // DFGUI_H
