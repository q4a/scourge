#ifndef DFRPG_H
#define DFRPG_H

#include "datafile.h"
#include <map>
#include "common.h"

struct Skill : public Named
{
	std::string /*name*/ symbol, description;
	std::string multiplier;
	std::vector<std::string> statNames;
};
struct Group
{
	std::string name, description;
	std::vector<Named*> skills;
};

class DFRpg : public DataFile<Group>
{
protected:
	bool LoadSingle(std::ifstream*, Group*);

	bool ParseSkill(std::ifstream*, Group*);
public:
	void Save();

	void SaveSkill(std::ofstream&, Skill*);
};

#endif // DFRPG_H
