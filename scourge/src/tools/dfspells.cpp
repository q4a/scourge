#include "dfspells.h"
#include "common.h"
#include "../common/constants.h"

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

	char key = fin->peek();
	while ( key == 'G' )
	{
		fin->getline(buffer, 256, '\n');
		school->deityDescription += (buffer+2); school->deityDescription += ' ';

		key = fin->peek();
	}
	school->deityDescription.erase( school->deityDescription.size()-1);		// Remove trailing space

	key = fin->peek();
	while ( key == 'L' )
	{
		fin->getline(buffer, 256, '\n');
		school->lowDonation.push_back(buffer+2);

		key = fin->peek();
	}
	while ( key == 'N' )
	{
		fin->getline(buffer, 256, '\n');
		school->neutralDonation.push_back(buffer+2);

		key = fin->peek();
	}
	while ( key == 'H' )
	{
		fin->getline(buffer, 256, '\n');
		school->highDonation.push_back(buffer+2);

		key = fin->peek();
	}

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
	std::ofstream fout( GetDataPath("%s/world/spellsTEST"), std::ios::binary );

	fout << "#\n"
			"# key: \n"
			"# S:school, provider deity, skill, resistSkill,r,g,b,symbol\n"
			"# G:deity description (multiple lines)\n"
			"# L:Message from deity when donation is too low\n"
			"# N:Message from deity when donation is too low\n"
			"# H:Message from deity when donation is too low\n"
			"# P:spell, symbol, level, mana, exp, failure rate, action, distance, single/group target, speed, effect, CLIP, icontileX, icontileY, F|H[ ,pre-requisite ]\n"
			"# W:sound\n"
			"# D:notes\n"
			"#\n"
			"# Symbol (for spells) is used to modify item name if item contains the spell. (e.g.: staffs, rings, etc.)\n"
			"#\n"
			"# CLIP=C-creature target, L-location target, I-item target, P-target is the party\n"
			"# F|H=F-Friendly, H-Hostile (it's how monsters use spells)\n"
			"# pre-requisite:\n"
			"# for Friendly spells: (reverse for hostile: high hp, etc.)\n"
			"# \thp - low hp\n"
			"#\tac - low ac\n"
			"#\tcursed, etc. - having bad state-mod\n"
			"# \tblessed, etc. - not having good state-mod\n"
			"\n"
			"# \n"
			"# stat-mods:\n"
			"# \n"
			"# good:\n"
			"# \tblessed=0, \n"
			"# \tempowered, \n"
			"# \tenraged, \n"
			"# \tac_protected, \n"
			"# \tmagic_protected,\n"
			"# \tinvisible,\n"
			"# \n"
			"# bad:\n"
			"# \tdrunk, \n"
			"# \tpoisoned, \n"
			"# \tcursed, \n"
			"# \tpossessed, \n"
			"# \tblinded, \n"
			"# \tcharmed, \n"
			"# \tdead,\n"
			"# \n"
			"# neutral:\n"
			"# \toverloaded,\n"
			"# \tleveled\n"
			"#\n\n\n";

	std::vector<School*>::iterator itr;
	for ( itr = data.begin(); itr != data.end(); itr++ )
	{
		SaveSchool(fout, *itr);
	}

	fout.close();
}

void DFSpells::SaveSchool(std::ofstream &fout, School *school)
{
	char buffer[256];
	std::vector <std::string> lines;

	fout << "\n\n#---------------------------------------------\n";

	sprintf(buffer, "S:%s,%s,%s,%s,%s,%s,%s,%s", school->name.c_str(),school->deity.c_str(),school->skill.c_str(),school->resistSkill.c_str(),
			school->r.c_str(),school->g.c_str(),school->b.c_str(),school->symbol.c_str());
	fout << buffer;

	SplitLine( school->deityDescription, lines);
	for ( int i = 0; i < lines.size(); i++ )
		fout << "\nG:" << lines[i];

	// donations
	for ( int i = 0; i < school->lowDonation.size(); i++ )
		fout << "\nL:" << school->lowDonation[i];
	for ( int i = 0; i < school->neutralDonation.size(); i++ )
		fout << "\nN:" << school->neutralDonation[i];
	for ( int i = 0; i < school->highDonation.size(); i++ )
		fout << "\nH:" << school->highDonation[i];

	fout << "\n#---------------------------------------------\n\n";

	std::vector<Spell*>::iterator itr;
	for ( itr = school->spells.begin(); itr != school->spells.end(); itr++ )
	{
		SaveSpell(fout, *itr);
	}
}
void DFSpells::SaveSpell(std::ofstream &fout, Spell *spell)
{
	char buffer[256];
	std::vector <std::string> lines;

	sprintf(buffer, "P:%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s",
			spell->name.c_str(), spell->symbol.c_str(), spell->level.c_str(), spell->mana.c_str(), spell->exp.c_str(),
			spell->failureRate.c_str(), spell->action.c_str(), spell->distance.c_str(), spell->area.c_str(),
			spell->speed.c_str(), spell->effect.c_str(), spell->target.c_str(), spell->icon_x.c_str(), spell->icon_y.c_str(),
			spell->disposition.c_str(), spell->prerequisite.c_str());
	fout << buffer;

	fout << "\nW:" << spell->sound;

	SplitLine(spell->notes, lines);
	for ( int i = 0; i < lines.size(); i++ )
		fout << "\nD:" << lines[i];

	fout << "\n\n";
}
