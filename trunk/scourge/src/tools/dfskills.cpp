#include "dfskills.h"
#include "common.h"
#include "../common/constants.h"

/*DFSkills::DFSkills()
{
	//ctor
}

DFSkills::~DFSkills()
{
	//dtor
}*/

bool DFSkills::LoadSingle(std::ifstream *fin, SpecialSkill *skill)
{
	std::string line ="blank";
	char buffer[512]; char *p;

	while ( line[0] != 0 )
	{
		fin->getline(buffer, 512, '\n');	line = buffer;
		p = buffer+2;

		if ( line[0] == 'S' )
		{
			skill->name = strtok(p, ",");
			skill->squirrelName = strtok(0, ",");
			skill->type = strtok(0,",");
			if ( skill->type == "A" )
				skill->event = strtok(0, ",");
			skill->icon_x = strtok(0, ",");
			skill->icon_y = strtok(0, ",");
		}
		else if ( line[0] == 'D' )
		{
			skill->description += line.substr(2);		skill->description += ' ';
		}
	}
	skill->description.erase(skill->description.size()-1);		// Remove trailing space

	return true;
}

void DFSkills::Save()
{
	std::ofstream fout( GetDataPath("%s/world/skillsTEST"), std::ios::binary);

	/**
	  * Possible improvement: set out skills in class sections as already done in skills.txt
	  * To do this class data will probably need to be saved, or comments read to determine what
	  * section a skill belongs in
	  * (Probably the latter)
	  */
	fout << "# Special skills\n"
		 << "# Key:\n"
		 << "# S:name,squirrel name,type[,event],icon_x,icon_y\n"
		 << "# where:\n"
		 << "# squirrel name is the postfix part of a two Squirrel functions:\n"
		 << "# prereq<squirrel name> is called for the prerequisite it should return a true/false value\n"
		 << "# action<squirrel name> is called to performt the action,when activating the skill\n"
		 << "# icon tile x,y are from spells.bmp\n"
		 << "# D:Description of skill (multiple lines)\n"
		 << "#\n"
		 << "# type is: (M-manual,A-automatic)\n"
		 << "# event is: (A-armor)\n"
		 << "#\n"
		 << "# The prereq function can be slow-ish, but the action should execute\n"
		 << "# fast.\n"
		 << "#\n"
		 << "# TODO: add prereq special skills here to create a rough tree of what\n"
		 << "# sages show. This also helps with not running too many prereq functions.\n\n";

	std::vector <std::string> lines;
	for ( std::vector<SpecialSkill*>::iterator itr = data.begin(); itr != data.end(); itr++ )
	{
		fout << "S:" << (*itr)->name << ',' << (*itr)->squirrelName << ','
			 << (*itr)->type << ',';
		if ( (*itr)->type == "A" )
			fout << (*itr)->event << ',';
		fout << (*itr)->icon_x << ',' << (*itr)->icon_y;

		SplitLine((*itr)->description,lines);
		for ( int i = 0; i < lines.size(); i++ )
			fout << "\nD:" << lines[i];

		fout << "\n\n";
	}

	fout.close();
}
