#include "dfrpg.h"
#include "../common/constants.h"

/*DFRpg::DFRpg()
{
	//ctor
}

DFRpg::~DFRpg()
{
	//dtor
}*/

bool DFRpg::LoadSingle(std::ifstream *fin, Group *group)
{
	char buffer[256];
	fin->getline(buffer, 256, '\n');

	group->name = strtok(buffer+2, ",");
	group->description = strtok(0, "\n\t");

	char key = fin->peek();
	while ( key == 'S' )
	{
		ParseSkill(fin, group);
		key = fin->peek();
	}

	return true;
}

bool DFRpg::ParseSkill(std::ifstream *fin, Group *group)
{
	char buffer[256];
	fin->getline(buffer, 256, '\n');

	char *skillName = strtok(buffer+2, ",");
	group->skills.push_back( new Skill );
	Skill *skill = (Skill*)group->skills[ group->skills.size()-1 ];
	skill->name = skillName;

	skill->symbol = strtok(0, ",");
	skill->description = strtok(0, "\n\t");

	char key = fin->peek();
	if ( key != 'P')		// no multiplier data
		return true;

	fin->getline(buffer, 256, '\n');

	skill->multiplier = strtok(buffer+2, ",");
	char *p = strtok(0, ",");
	while ( p )
	{
		skill->statNames.push_back( p );
		p = strtok(0, ",");
	}

	return true;
}

void DFRpg::Save()
{
	std::ofstream fout( GetDataPath("%s/world/rpgTEST"), std::ios::binary );

	fout << "# Key:\n"
		 << "# G: group name, descriptive name\n"
		 << "# S: skill name, useCount, symbol, description\n"
		 << "# P: multiplier, stat-name [ more than 1 ]\n"
		 << "\n"
		 << "# skill groups\n"
		 << "# ===================================================================\n"
		 << "# Skills: name,symbol,descriptions (symbol is used to name enchanted items)\n"
		 << "# Note: the group and skill names cannot be changed (referenced in code)\n"
		 << "#\n"
		 << "\n"
		 << "#\n"
		 << "# Note: the basic group is stats (1-20) not skills really\n"
		 << "# Skill name has to be less than 50 chars.\n"
		 << "#\n\n";

	std::vector<Group*>::iterator itr;
	for ( itr = data.begin(); itr != data.end(); itr++ )
	{
		fout << "G:" << (*itr)->name << "," << (*itr)->description;
		for ( unsigned int i = 0; i < (*itr)->skills.size(); i++ )
			SaveSkill( fout, (Skill*)(*itr)->skills[i] );

		fout << "\n\n";
	}

	fout.close();
}

void DFRpg::SaveSkill(std::ofstream &fout, Skill *skill)
{
	fout << "\nS:" << skill->name << "," << skill->symbol << "," << skill->description;
	if ( skill->multiplier != "" && skill->statNames.size() > 0 )
	{
		fout << "\nP:" << skill->multiplier;
		for ( unsigned int i = 0; i < skill->statNames.size(); i++ )
			fout << "," << skill->statNames[i];
	}
}
