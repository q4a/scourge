#include "dfcreatures.h"

/*DFCreatures::DFCreatures()
{
	//ctor
}

DFCreatures::~DFCreatures()
{
	//dtor
}*/

bool DFCreatures::LoadSingle(std::ifstream *fin, Creature *creature)
{
	char buffer[256];	char *p;
	fin->getline(buffer, 256, '\n');

	creature->name = strtok(buffer+2, ",");
	if ( p = strtok(0, ",") )
		creature->portrait = p;

	fin->getline(buffer, 256, '\n');
	creature->md2 = strtok(buffer+2, ",");
	creature->skin = strtok(0,",");
	creature->level = strtok(0,",");
	creature->hp = strtok(0,",");
	creature->mp = strtok(0,",");
	creature->armor = strtok(0,",");
	creature->rareness = strtok(0,",");
	creature->speed = strtok(0,",");
	if ( p = strtok(0, ",") )
		creature->scale = p;
	if ( p = strtok(0, ",") )
		creature->npc = p;
	if ( p = strtok(0, ",") )
		creature->npcStartX = p;
	if ( p = strtok(0, ",") )
		creature->npcStartY = p;

	char key = fin->peek();
	while ( key == 'I' )
	{
		fin->getline(buffer, 256, '\n');
		creature->inventory.push_back(buffer+2);

		key = fin->peek();
	}
	while ( key == 'S' )
	{
		fin->getline(buffer, 256, '\n' );
		creature->spells.push_back(buffer+2);

		key = fin->peek();
	}
	while ( key == 'P' )
	{
		fin->getline(buffer, 256, '\n');
		strtok(buffer,",");		// skip skill name
		creature->skills[ buffer+2 ] = atoi( strtok(buffer,"\n\t ") );

		key = fin->peek();
	}

	return true;
}

void DFCreatures::Save()
{
}
