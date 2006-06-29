#ifndef DFSPELLS_H
#define DFSPELLS_H

#include "datafile.h"
#include <map>

struct Spell
{
	std::string name, symbol, level, mana, exp, failureRate, action, distance, area, speed, effect, target, icon_x, icon_y, disposition, prerequisite;
	std::string sound;
	std::string notes;
};
struct School
{
	std::string name, deity, skill, resistSkill, r,g,b, symbol;
	std::string deityDescription;
	std::vector<std::string> lowDonation;
	std::vector<std::string> neutralDonation;
	std::vector<std::string> highDonation;

	std::map <std::string,Spell*> spells;

	~School()
	{
		for ( std::map<std::string,Spell*>::iterator itr = spells.begin(); itr != spells.end(); itr++ )
		{
			delete itr->second;
		}
		spells.clear();
	}
};

class DFSpells : public DataFile <School>
{
protected:
	bool LoadSingle(std::ifstream*, School*);
public:
	void Save();
};

#endif // DFSPELLS_H
