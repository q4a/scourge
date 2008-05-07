#include "dfbooks.h"
#include "../common/constants.h"

bool DFBooks::LoadSingle(std::ifstream *fin, Book *book)
{
	std::string line ="blank";
	char buffer[512];

	while ( line[0] != 0 )
	{
		fin->getline(buffer, 512, '\n');	line = buffer;

		if ( line[0] == 'B' )
		{
			line = line.substr(2);		// remove 'T:'
			book->name = line.substr(0, line.find_last_of(','));
			book->rareness = line.substr(line.find_last_of(',')+1);
		}
		else if ( line[0] == 'M' )		// Title
		{
			book->missionSpecific = true;
			book->missionName = line.substr(2);
		}
		else if ( line[0] == 'T' )
		{
			book->text += line.substr(2);		book->text += ' ';
		}
	}
	if ( book->name == "" || book->text == "" )
	{
		std::cerr << book->name << book->rareness << book->text;
		return false;
	}
	book->text.erase(book->text.size()-1);

	return true;
}

void DFBooks::Save()
{
	std::vector<Book*> generalBooks;
	std::vector<Book*> missionBooks;

	for ( unsigned int i = 0; i < data.size(); i++ )
	{
		if ( data[i]->missionSpecific )
			missionBooks.push_back(data[i]);
		else
			generalBooks.push_back(data[i]);
	}

	std::ofstream fout( GetDataPath("%s/world/booksTEST"), std::ios::binary);

	fout << "# Random texts found in books and notes.\n#\n# Key:\n# B: Book Name, rareness"
		 << "# M: Mission name (optional: from missions.txt)\n# T: multi-line text\n# \n\n"
		 << "#################################\n# General texts\n#\n";
	for ( unsigned int i = 0; i < generalBooks.size(); i++ )
	{
		fout << "B:" << generalBooks[i]->name << ',' << generalBooks[i]->rareness;
		fout << "\nT:" << generalBooks[i]->text;
		fout << "\n\n";
	}

	fout << "#################################\n# Mission-specific texts\n#\n";
	for ( unsigned int i = 0; i < missionBooks.size(); i++ )
	{
		fout << "B:" << missionBooks[i]->name << ',' << missionBooks[i]->rareness;
		fout << "\nM:" << missionBooks[i]->missionName;
		fout << "\nT:" << missionBooks[i]->text;
		fout << "\n\n";
	}
}
