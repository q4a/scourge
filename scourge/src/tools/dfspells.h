#ifndef DFSPELLS_H
#define DFSPELLS_H

#include "datafile.h"
#include <vector>
#include <map>

struct Spell
{
	std::string name, symbol, level, mana, exp, failureRate, action, distance, area, speed, effect, target, icon_x, icon_y, disposition, prerequisite;
	std::string sound;
	std::string notes;

	Spell(): name("spell"), area("single"), icon_x("1"),icon_y("1") {}
};
struct School
{
	std::string name, deity, skill, resistSkill, r,g,b, symbol;
	std::string deityDescription;
	std::vector<std::string> lowDonation;
	std::vector<std::string> neutralDonation;
	std::vector<std::string> highDonation;

//	std::map <std::string,Spell*> spells;
	std::vector <Spell*> spells;


	~School()
	{
		for ( std::vector<Spell*>::iterator itr = spells.begin(); itr != spells.end(); itr++ )
		{
			delete (*itr);
		}
		spells.clear();
	}
	Spell* NewSpell()
	{
		spells.push_back(new Spell);
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
