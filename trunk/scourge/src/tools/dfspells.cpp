#include "dfspells.h"

/*DFSpells::DFSpells()
{
	//ctor
}

DFSpells::~DFSpells()
{
	//dtor
}*/

bool DFSpells::LoadSingle(std::ifstream *fin, School *school)
{
	char buffer[256];
	fin->getline(buffer, 256, '\n');

	school->name = strtok(buffer+2, ",");
	school->deity = strtok(0, ",");
	school->skill = strtok(0, ",");
	school->resistSkill = strtok(0, ",");
	school->r = strtok(0, ",");
	school->g = strtok(0, ",");
	school->b = strtok(0, ",");
	school->symbol = strtok(0, "\n \t");

	std::cerr << school->name <<"\n"<< school->deity <<"\n"<< school->skill <<"\n"<< school->resistSkill <<"\n";
	std::cerr << school->r <<"\n"<< school->g <<"\n"<< school->b <<"\n"<< school->symbol <<"\n\n";

	char key = fin->peek();		std::cerr<<"1) key = "<<key<<"\n";
	while ( key == 'G' )
	{
		fin->getline(buffer, 256, '\n');
		school->deityDescription += (buffer+2); school->deityDescription += ' ';

		key = fin->peek();		std::cerr<<"2) key = "<<key<<"\n";
	}
	school->deityDescription.erase( school->deityDescription.size()-1);		// Remove trailing space

	key = fin->peek();		std::cerr<<"3) key = "<<key<<"\n";
	while ( key == 'L' )
	{
		fin->getline(buffer, 256, '\n');
		school->lowDonation.push_back(buffer);

		key = fin->peek();
	}
	while ( key == 'N' )
	{
		fin->getline(buffer, 256, '\n');
		school->neutralDonation.push_back(buffer);

		key = fin->peek();
	}
	while ( key == 'H' )
	{
		fin->getline(buffer, 256, '\n');
		school->highDonation.push_back(buffer);

		key = fin->peek();
	}

std::cerr<<"4) key = "<<key<<"\n";
	Spell *spell;
	while ( key != 0 && key != 'S' && !fin->eof())
	{
		fin->getline(buffer, 512, '\n');

		if ( buffer[0] == 'P' )
		{
			spell = new Spell;
			// Load 'P' line
			spell->name = strtok(buffer+2, ",");		std::cerr<<"spell->name = "<<spell->name<<"\n";
			spell->symbol = strtok(0, ",");
			spell->level = strtok(0, ",");
			spell->mana = strtok(0, ",");
			spell->exp = strtok(0, ",");
			spell->failureRate = strtok(0, ",");
			spell->action = strtok(0, ",");
			spell->distance = strtok(0, ",");
			spell->area = strtok(0, ",");
			spell->speed = strtok(0, ",");
			spell->effect = strtok(0, ",");
			spell->target = strtok(0, ",");
			spell->icon_x = strtok(0, ",");
			spell->icon_y = strtok(0, ",");
			spell->disposition = strtok(0, ",");
			char *p = strtok(0, ",");
			if ( p )
				spell->prerequisite = p;

			school->spells.push_back(spell);
		}
		else if ( buffer[0] == 'W' )
		{
			spell->sound = (buffer+2);
		}
		else if ( buffer[0] == 'D' )
		{
			spell->notes += (buffer+2);		spell->notes += ' ';
		}

		key = fin->peek();
	}
	if ( spell->notes.size() > 1 )
		spell->notes.erase(spell->notes.size()-1);		// remove trailing space

	return true;
}

void DFSpells::Save()
{
}
