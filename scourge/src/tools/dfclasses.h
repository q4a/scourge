#ifndef DFCLASSES_H
#define DFCLASSES_H

#include "datafile.h"
#include <map>

struct Skill
{
	std::string name;
	std::string min, max;		// * for no limit

	Skill(): name(""), min("0"),max("100") {}

	void Clear()
	{
		*this = Skill();
	}
};
struct Class
{
	std::string name, shortName, description;
	std::string hpBonus, mpBonus, skillBonus, levelProgression, attackBonus, additionalAttackLevel;

	std::map<std::string,Skill*> skills;

	Class(): name(""),shortName(""),description(""),
			 hpBonus(""),mpBonus(""),skillBonus(""),levelProgression(""),attackBonus(""),additionalAttackLevel("")
	{
		// Attributes
		skills["SPEED"] = new Skill;
		skills["COORDINATION"] = new Skill;
		skills["POWER"] = new Skill;
		skills["IQ"] = new Skill;
		skills["LEADERSHIP"] = new Skill;
		skills["LUCK"] = new Skill;
		skills["PIETY"] = new Skill;
		skills["LORE"] = new Skill;

		// Weapon skills
		skills["SWORD_WEAPON"] = new Skill;
		skills["AXE_WEAPON"] = new Skill;
		skills["BOW_WEAPON"] = new Skill;
		skills["MACE_WEAPON"] = new Skill;
		skills["HAND_TO_HAND_COMBAT"] = new Skill;

		// Defensive skills
		skills["SHIELD_DEFEND"] = new Skill;
		skills["ARMOR_DEFEND"] = new Skill;
		skills["WEAPON_DEFEND"] = new Skill;
		skills["HAND_DEFEND"] = new Skill;

		// Magic skills
		skills["NATURE_MAGIC"] = new Skill;
		skills["AWARENESS_MAGIC"] = new Skill;
		skills["LIFE_AND_DEATH_MAGIC"] = new Skill;
		skills["HISTORY_MAGIC"] = new Skill;
		skills["DECEIT_MAGIC"] = new Skill;
		skills["CONFRONTATION_MAGIC"] = new Skill;

		// Thieving skills
		skills["OPEN_LOCK"] = new Skill;
		skills["FIND_TRAP"] = new Skill;
		skills["MOVE_UNDETECTED"] = new Skill;
		skills["STEALING"] = new Skill;
	}
	~Class()
	{
		for ( std::map<std::string,Skill*>::iterator itr = skills.begin(); itr != skills.end(); itr++ )
			delete itr->second;
		skills.clear();
	}

	void Clear()
	{
		name = shortName = description = "";
		hpBonus = mpBonus = skillBonus = levelProgression = attackBonus = additionalAttackLevel = "";
		for ( std::map<std::string,Skill*>::iterator itr = skills.begin(); itr != skills.end(); itr++ )
		{
			itr->second->Clear();
		}
	}
};

class DFClasses : public DataFile <Class>
{
protected:
	bool LoadSingle(std::ifstream*,Class*);

	void ParseSkill(std::ifstream*,Skill*);

public:
	void Save();
};

#endif // DFCLASSES_H
