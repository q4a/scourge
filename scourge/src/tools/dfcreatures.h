#ifndef DFCREATURES_H
#define DFCREATURES_H

#include "datafile.h"
#include <map>

struct Creature
{
	std::string name, portrait;
	std::string md2, skin, level, hp, mp, armor, rareness, speed, scale, npc,npcStartX,npcStartY;
	std::vector <std::string> inventory;
	std::vector <std::string> spells;
	std::map <std::string, std::string> skills;
	// portrait is optional
	// scale, npc, npcStartX, npcStartY are optional
};

class DFCreatures : public DataFile<Creature>
{
protected:
	bool LoadSingle(std::ifstream*, Creature*);

public:
	void Save();
};

#endif // DFCREATURES_H
