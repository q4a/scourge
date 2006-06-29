#ifndef DFSKILLS_H
#define DFSKILLS_H

#include "datafile.h"

struct SpecialSkill : public Data
{
	std::string name, squirrelName;
	std::string type, event;
	std::string icon_x, icon_y;
	std::string description;
	SpecialSkill() : name(""),squirrelName(""), type(""),event(""), icon_x(""),icon_y(""), description("") {}
};

class DFSkills : public DataFile <SpecialSkill>
{
protected:
	bool LoadSingle(std::ifstream*, SpecialSkill*);
public:
	void Save();
};

#endif // DFSKILLS_H
