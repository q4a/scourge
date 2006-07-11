#include "dflocations.h"
#include "../common/constants.h"

/*DFLocations::DFLocations()
{
	//ctor
}

DFLocations::~DFLocations()
{
	//dtor
}*/

bool DFLocations::LoadSingle(std::ifstream *fin, Location *location)
{
	char buffer[256];
	fin->getline(buffer, 256, '\n');

	location->name = strtok(buffer+2, ",");
	location->x = strtok(0, ",");
	location->y = strtok(0, ",");
	location->type = strtok(0, ",");
	if ( location->type.find("R") != std::string::npos )
		location->random = true;

	return true;
}

void DFLocations::Save()
{
	std::ofstream fout( GetDataPath("%s/world/locationsTEST"), std::ios::binary);

	fout << "# Locations of relevant places on the scourge map.\n"
		 << "# Key:\n"
		 << "# L:name,x,y,type[R]\n"
		 << "#\n"
		 << "# where \"type\" is arbitrary, it's used by mission templates to map to a \n"
		 << "# type of map location. Some examples are:\n"
		 << "# C - city\n"
		 << "# F - forest\n"
		 << "# M - mountains\n"
		 << "# P - plains\n"
		 << "# W - ocean/water\n"
		 << "# H - hills\n"
		 << "# D - dungeon\n"
		 << "# An 'R' is added if the place can be used for random missions\n"
		 << "#\n"
		 << "# TODO:Add more locations!\n\n";

	std::vector<Location*>::iterator itr = data.begin();
	for ( ; itr != data.end(); itr++ )
	{
		fout << "\nL:" << (*itr)->name << "," << (*itr)->x << "," << (*itr)->y << "," << (*itr)->type;
	}

	fout.close();
}
