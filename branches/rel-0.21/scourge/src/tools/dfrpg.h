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

struct ItemTag
{
	std::string name, description;
};

typedef std::vector<std::string> SyllableLine;
struct Syllables
{
	std::vector<SyllableLine> first, mid, end;
};

class DFRpg : public DataFile<Group>
{
public:
	std::vector<ItemTag*> itemTags;
	Syllables syllables;

protected:
	bool LoadSingle(std::ifstream*, Group*);

	bool ParseSkill(std::ifstream*, Group*);

	void LoadItemTags(std::ifstream*);
	void LoadSyllables(std::ifstream*);
public:
	~DFRpg();

	void Save();

	void SaveSkill(std::ofstream&, Skill*);

	void SaveItemTags(std::ofstream&);
	void SaveSyllables(std::ofstream&);
};

#endif // DFRPG_H
