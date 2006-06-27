#include "dfmissions.h"

/*DFMissions::DFMissions()
{
	//ctor
}

DFMissions::~DFMissions()
{
	for ( std::vector<Mission*>::iterator itr = missions.begin(); itr != missions.end(); itr++ )
	{
		delete (*itr);
	}
	missions.clear();
}

bool DFMissions::Load()
{
	std::ifstream fin("../../share/scourge_data/world/missions.txt", std::ios::binary);
	std::string line;
	char buffer[512];
	Mission *mission;
	char c;

	while ( !fin.eof() )
	{
		fin.get(c);

		if ( c == 'M' )
		{
			mission = new Mission;
			LoadMission(&fin, mission);
			if ( mission->description == "" )
			{
				delete mission;
				continue;
			}

			missions.push_back(mission);
		}
		else
			fin.getline(buffer, 512, '\n');
	}

	fin.close();
}*/

bool DFMissions::LoadSingle(std::ifstream *fin, Mission *mission)
{
	std::string line ="blank";
	char buffer[512];
	bool readyForSpecial = false;

	while ( line[0] != 0 )
	{
		fin->getline(buffer, 512, '\n');	line = buffer;

		if ( line[0] == 'M' )
		{
			mission->type = line[2];
			mission->name = line.substr(line.find_first_of(',')+1);
		}
		else if ( line[0] == 'T' )		// Title
		{
			mission->storyline = true;
			mission->name = line.substr(2);
		}
		else if ( line[0] == 'S' )		// level,stories[,mapname]
		{
			line = line.substr(2);		// remove the "S:"
			if ( readyForSpecial )
			{
				mission->special = line;
				continue;
			}
			else
				readyForSpecial = true;

			mission->level = line.substr(0, line.find_first_of(','));
			line = line.substr(line.find_first_of(',')+1);
			mission->stories = line.substr(0, line.find_first_of(','));
			if ( line.find_first_of(',') == std::string::npos )
				continue;
			line = line.substr(line.find_first_of(',')+1);
			mission->mapname = line;
		}
		else if ( line[0] == 'D' )
		{
			mission->description += line.substr(2);		mission->description += ' ';
		}
		else if ( line[0] == 'I' )		// items
		{
			mission->items.push_back( line.substr(2) );
		}
		else if ( line[0] == 'C' )		// creatures
		{
			mission->creatures.push_back( line.substr(2) );
		}
		else if ( line[0] == 'Y' )
		{
			mission->success += line.substr(2);		mission->success += ' ';
		}
		else if ( line[0] == 'N' )
		{
			mission->failure += line.substr(2);	mission->failure += ' ';
		}
	}
	if ( mission->name == "" || mission->description == "" )
	{
		std::cerr << mission->name << mission->type << mission->description << mission->success << mission->failure;
		return false;
	}
	mission->description.erase(mission->description.size()-1);
	mission->success.erase(mission->success.size()-1);
	mission->failure.erase(mission->failure.size()-1);

	return true;
}

void SplitLine(std::string &line, std::vector<std::string> &lines)
{
	// Make sure the vector is clear
	lines.clear();

	std::string::size_type strLength;
	std::string str;
	while ( true )
	{
		int i = 70;
		if ( line.size() <= i )
		{
			lines.push_back(line);
			break;
		}
		str = line.substr(0,i);
		strLength = str.find_last_of(' ');
		str = str.substr(0, strLength);
		lines.push_back(str);

		line = line.substr(strLength+1);		// Skip the space
	}
}

void DFMissions::Save()
{
	std::vector <Mission*> generalMissions, storylineMissions;
	std::vector <Mission*>::iterator itr;
	std::vector<std::string> lines;

	for ( itr = data.begin(); itr != data.end(); itr++ )
	{
		if ( (*itr)->storyline )
			storylineMissions.push_back( *itr );
		else
			generalMissions.push_back( *itr );
	}
	std::ofstream fout("../../share/scourge_data/world/missionsTEST", std::ios::binary);

	fout << "##########################################################\n"
		 << "# Templated missions\n# Key:\n# M:type,mission template name\n"
		 << "# D:description (multi-line)\n# where \"type\" is a char from locations.txt\n#\n\n";
	for ( itr = generalMissions.begin(); itr != generalMissions.end(); itr++ )
	{
		fout << "M:" << (*itr)->type << ',' << (*itr)->name;

		SplitLine((*itr)->description,lines);
		for ( int i = 0; i < lines.size(); i++ )
			fout << "\nD:" << lines[i];

		SplitLine((*itr)->success,lines);
		for ( int i = 0; i < lines.size(); i++ )
			fout << "\nY:" << lines[i];

		SplitLine((*itr)->failure,lines);
		for ( int i = 0; i < lines.size(); i++ )
			fout << "\nN:" << lines[i];

		fout << "\n\n";
	}

	fout << "##########################################################\n"
		 << "# Storyline missions\n#\n# Key:\n#   T:Mission title\n"
		 << "#   S:level,stories,[ mapname ]\n#   D:description (multi-line)\n"
		 << "#   I:required items in level (multi-line)\n#   C:required creatures in level (multi-line)\n"
		 << "#   Y:success text\n#   N:failure text\n#\n"
		 << "# Note: edited map names must be of the format: name[1,2,...n] where level 0 is always 'name', level 1 is 'name1', etc.\n"
		 << "#\n# ***************************************************************\n"
		 << "# ***************************************************************\n"
		 << "# Remember to update BOOKS.TXT if the name of a mission changes!!\n"
		 << "# ***************************************************************\n"
		 << "# ***************************************************************\n";
	for ( itr = storylineMissions.begin(); itr != storylineMissions.end(); itr++ )
	{
		fout << "T:" << ',' << (*itr)->name;
		fout << "\nS:" << (*itr)->level << ',' << (*itr)->stories;
		if ( (*itr)->mapname != "" )
			fout << "," << (*itr)->mapname;

		SplitLine((*itr)->description,lines);
		for ( int i = 0; i < lines.size(); i++ )
			fout << "\nD:" << lines[i];

		SplitLine((*itr)->success,lines);
		for ( int i = 0; i < lines.size(); i++ )
			fout << "\nY:" << lines[i];

		SplitLine((*itr)->failure,lines);
		for ( int i = 0; i < lines.size(); i++ )
			fout << "\nN:" << lines[i];

		for ( int i = 0; i < (*itr)->items.size(); i++ )
			fout << "\nI:" << (*itr)->items[i];
		for ( int i = 0; i << (*itr)->creatures.size(); i++ )
			fout << "\nC:" << (*itr)->creatures[i];

		if ( (*itr)->special != "" )
			fout << "\nS:" << (*itr)->special;
		fout << "\n\n";
	}

	fout.close();
}
