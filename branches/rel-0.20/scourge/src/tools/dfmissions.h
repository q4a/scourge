#ifndef DFMISSIONS_H
#define DFMISSIONS_H

#include "datafile.h"

struct Mission : public Data
{
	bool storyline;
	char type;
	std::string name;
	std::string level, stories, mapname;
	std::string description;
	std::vector <std::string> items;
	std::vector <std::string> creatures;
	std::string success;
	std::string failure;
	std::string special;
	Mission() : storyline(false), type('D'), name(""),
			level(""),stories(""),mapname(""),
			description(""), success(""), failure(""), special("") {}
};

class DFMissions : public DataFile<Mission>
{
protected:
	bool LoadSingle(std::ifstream*, Mission*);

public:
	void Save();

};

#endif // DFMISSIONS_H
