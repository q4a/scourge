#include "dfrpg.h"
#include "../common/constants.h"

/*DFRpg::DFRpg()
{
	//ctor
}*/

DFRpg::~DFRpg()
{
	for ( std::vector<Group*>::iterator itr = data.begin(); itr != data.end(); itr++ )
		delete (*itr);
	data.clear();
	for ( std::vector<ItemTag*>::iterator itr = itemTags.begin(); itr != itemTags.end(); itr++ )
		delete (*itr);
	itemTags.clear();
}

bool DFRpg::LoadSingle(std::ifstream *fin, Group *group)
{
	char buffer[256];	char key;
	key = fin->peek();
	if ( key == 'T' )
	{
		LoadItemTags(fin);
		return false;		// HACK - stops new Groups being added, while allowing item tags to be loaded
	}
	else if ( key == 'F' )
	{
		LoadSyllables(fin);
		return false;
	}

	fin->getline(buffer, 256, '\n');

	group->name = strtok(buffer+2, ",");
	group->description = strtok(0, "\n\t");

	key = fin->peek();
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

void DFRpg::LoadItemTags(std::ifstream *fin)
{
	char buffer[256];
	char key = fin->peek();

	while ( key == 'T' )
	{
		fin->getline(buffer, 256, '\n');

		ItemTag *itemTag = new ItemTag;
		itemTags.push_back(itemTag);

		itemTag->name = strtok(buffer+2, ",");
		itemTag->description = strtok(0, "\n\t");

		key = fin->peek();
	}
}

void DFRpg::LoadSyllables(std::ifstream *fin)
{
	char buffer[256]; char *p;
	SyllableLine line;
	char key = fin->peek();

	while ( key == 'F' )
	{
		fin->getline(buffer, 256, '\n');
		syllables.first.push_back( line );

		p = strtok(buffer+2, ",");
		while ( p )
		{
			syllables.first[ syllables.first.size()-1 ].push_back( p );

			p = strtok(0, ",\n\t");
		}

		key = fin->peek();
	}
	while ( key == 'M' )
	{
		fin->getline(buffer, 256, '\n');
		syllables.mid.push_back( line );

		p = strtok(buffer+2, ",");
		while ( p )
		{
			syllables.mid[ syllables.mid.size()-1 ].push_back( p );

			p = strtok(0, ",\n\t");
		}

		key = fin->peek();
	}
	while ( key == 'E' )
	{
		fin->getline(buffer, 256, '\n');
		syllables.end.push_back( line );

		p = strtok(buffer+2, ",");
		while ( p )
		{
			syllables.end[ syllables.end.size()-1 ].push_back( p );

			p = strtok(0, ",\n\t");
		}

		key = fin->peek();
	}
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

	SaveItemTags(fout);
	SaveSyllables(fout);

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

void DFRpg::SaveItemTags(std::ofstream &fout)
{
	fout << "# ===================================================================\n"
		 << "# Item tag descriptions. This is optional, \n"
		 << "# it is used to display a profession's item-use limitations.\n"
		 << "#\n"
		 << "# Key: T:tag-name,tag-description ($$ will be substituted with 'weapons' or 'armor')\n"
		 << "#";		// trailing \n not needed

	std::vector<ItemTag*>::iterator itr;
	for ( itr = itemTags.begin(); itr != itemTags.end(); itr++ )
		fout << "\nT:" << (*itr)->name << "," << (*itr)->description;

	fout << "\n\n";
}

void DFRpg::SaveSyllables(std::ofstream &fout)
{
	fout << "# ===================================================================\n"
		 << "# Syllables used for NPC name generation.\n"
		 << "#\n"
		 << "# F:first-syl\n"
		 << "# M:mid-syl\n"
		 << "# E:end-syl\n"
		 << "#";

	std::vector<SyllableLine>::iterator lineItr;
	std::vector<std::string>::iterator itr;
	for ( lineItr = syllables.first.begin(); lineItr != syllables.first.end(); lineItr++ )
	{
		fout << "\nF:" << (*(*lineItr).begin());
		for ( itr = ++(*lineItr).begin(); itr != (*lineItr).end(); itr++ )
			fout << "," << (*itr);
	}
	for ( lineItr = syllables.mid.begin(); lineItr != syllables.mid.end(); lineItr++ )
	{
		fout << "\nM:" << (*(*lineItr).begin());
		for ( itr = ++(*lineItr).begin(); itr != (*lineItr).end(); itr++ )
			fout << "," << (*itr);
	}
	for ( lineItr = syllables.end.begin(); lineItr != syllables.end.end(); lineItr++ )
	{
		fout << "\nE:" << (*(*lineItr).begin());
		for ( itr = ++(*lineItr).begin(); itr != (*lineItr).end(); itr++ )
			fout << "," << (*itr);
	}
	fout << "\n\n";
}
