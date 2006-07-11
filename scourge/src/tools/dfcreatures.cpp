#include "dfcreatures.h"
#include "../common/constants.h"

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
		creature->skills[ buffer+2 ] = strtok(0,"\n\t ");

		key = fin->peek();
	}

	return true;
}

void DFCreatures::Save()
{
	std::ofstream fout( GetDataPath("%s/world/creaturesTEST"), std::ios::binary );

	fout << "# Creatures\n"
		 << "# \n"
		 << "# Key: (the old-way)\n"
		 << "# M:monster\n"
		 << "#   name[,(descriptive type for monsters | portrait for npcs)]\n"
		 << "#   md2 model, skin, level,hp,mp,natural armor,rareness,speed[,scale(overrides shapes.txt's scale),npc[,startX,startY]]\n"
		 << "# I:inventory (1 line per item ref. by name from items.txt)\n"
		 << "# S:spell\n"
		 << "# P:skill_name,percentage (skill level; used mainly for magic resistance)\n"
		 << "#\n"
		 << "# Notes: \n"
		 << "#       hp,mp are multiplied by the level when the monster is created (natural armor isn't)\n"
		 << "# 	rareness=1-10 where 1 is most and 10 is least rare (for given level)\n"
		 << "#	speed=1-10 where 1 is the fastest and 10 is the slowest\n"
		 << "#       descriptiveType is only needed once per model\n"
		 << "#\n"
		 << "# Magic schools:\n"
		 << "#  RESIST_NATURE_MAGIC,\n"
		 << "#  RESIST_AWARENESS_MAGIC,\n"
		 << "#  RESIST_LIFE_AND_DEATH_MAGIC,\n"
		 << "#  RESIST_HISTORY_MAGIC,\n"
		 << "#  RESIST_DECEIT_MAGIC,\n"
		 << "#  RESIST_CONFRONTATION_MAGIC\n\n";

	std::vector <Creature*>::iterator itr = data.begin();
	for ( ; itr != data.end(); itr++ )
	{
		fout << "M:" << (*itr)->name;
		if ( (*itr)->portrait != "" )
			fout << "," << (*itr)->portrait;

		fout << "\nM:" << (*itr)->md2 << "," << (*itr)->skin << "," << (*itr)->level << "," << (*itr)->hp << "," << (*itr)->mp
			 << "," << (*itr)->armor << "," << (*itr)->rareness << "," << (*itr)->speed;
		if ( (*itr)->scale != "" )
			fout << "," << (*itr)->scale;
		if ( (*itr)->npc != "" )
			fout << "," << (*itr)->npc;
		if ( (*itr)->npcStartX != "-1" )
			fout << "," << (*itr)->npcStartX << "," << (*itr)->npcStartY;

		std::vector<std::string>::iterator strVecItr;
		for ( strVecItr = (*itr)->inventory.begin(); strVecItr != (*itr)->inventory.end(); strVecItr++ )
			fout << "\nI:" << *strVecItr;
		for ( strVecItr = (*itr)->spells.begin(); strVecItr != (*itr)->spells.end(); strVecItr++ )
			fout << "\nS:" << *strVecItr;

		std::map<std::string,std::string>::iterator strMapItr;
		for ( strMapItr = (*itr)->skills.begin(); strMapItr != (*itr)->skills.end(); strMapItr++ )
			fout << "\nP:" << strMapItr->first << "," << strMapItr->second;

		fout << "\n\n";
	}

	fout.close();
}
