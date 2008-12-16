#include "dfclasses.h"

/*DFClasses::DFClasses()
{
	//ctor
}

DFClasses::~DFClasses()
{
	//dtor
}*/

bool DFClasses::LoadSingle(std::ifstream *fin, Class *c)
{
	char buffer[256], nameBuf[64], shortNameBuf[16]; char *p = buffer+2;
	fin->getline(buffer, 256, '\n');

	p = strtok(p, ",");		c->name = p;
	p = strtok(0,",");		c->hpBonus = p;
	p = strtok(0,",");		c->mpBonus = p;
	p = strtok(0,",");		c->skillBonus = p;
	p = strtok(0,",");		c->levelProgression = p;
	p = strtok(0,",");		c->shortName = p;
	p = strtok(0,",");		c->attackBonus = p;
	p = strtok(0,",");		c->additionalAttackLevel = p;

	char key = fin->peek();
	while ( key == 'D' )
	{
		fin->getline(buffer, 256, '\n');

		c->description += (buffer+2); c->description += ' ';

		key = fin->peek();
	}
	c->description.erase(c->description.size()-1);		// remove trailing space

	ParseSkill(fin, c->skills["SPEED"]);
	ParseSkill(fin, c->skills["COORDINATION"]);
	ParseSkill(fin, c->skills["POWER"]);
	ParseSkill(fin, c->skills["IQ"]);
	ParseSkill(fin, c->skills["LEADERSHIP"]);
	ParseSkill(fin, c->skills["LUCK"]);
	ParseSkill(fin, c->skills["PIETY"]);
	ParseSkill(fin, c->skills["LORE"]);

	ParseSkill(fin, c->skills["SWORD_WEAPON"]);
	ParseSkill(fin, c->skills["AXE_WEAPON"]);
	ParseSkill(fin, c->skills["BOW_WEAPON"]);
	ParseSkill(fin, c->skills["MACE_WEAPON"]);
	ParseSkill(fin, c->skills["HAND_TO_HAND_COMBAT"]);

	ParseSkill(fin, c->skills["SHIELD_DEFEND"]);
	ParseSkill(fin, c->skills["ARMOR_DEFEND"]);
	ParseSkill(fin, c->skills["WEAPON_DEFEND"]);
	ParseSkill(fin, c->skills["HAND_DEFEND"]);

	ParseSkill(fin, c->skills["NATURE_MAGIC"]);
	ParseSkill(fin, c->skills["AWARENESS_MAGIC"]);
	ParseSkill(fin, c->skills["LIFE_AND_DEATH_MAGIC"]);
	ParseSkill(fin, c->skills["HISTORY_MAGIC"]);
	ParseSkill(fin, c->skills["DECEIT_MAGIC"]);
	ParseSkill(fin, c->skills["CONFRONTATION_MAGIC"]);

	ParseSkill(fin, c->skills["OPEN_LOCK"]);
	ParseSkill(fin, c->skills["FIND_TRAP"]);
	ParseSkill(fin, c->skills["MOVE_UNDETECTED"]);
	ParseSkill(fin, c->skills["STEALING"]);

	std::cerr << "\n\n\n\t--- CLASSES ---\n";
	std::cerr << "name = " << c->name;
	std::cerr << "\ndescription = " << c->description;

	return true;
}

void DFClasses::ParseSkill(std::ifstream *fin, Skill *skill)
{
	char buffer[256]; char *p = buffer+2;
	fin->getline(buffer, 256, '\n');

	p = strtok(p, ",");
		skill->name = p;
	p = strtok(0,",");
		skill->min = p;
	p = strtok(0,",");
		skill->max = p;
}

void DFClasses::Save()
{
}
