#ifndef DFLOCATIONS_H
#define DFLOCATIONS_H

#include "datafile.h"
#include <map>

struct Location
{
	std::string name, x,y, type;
	bool random;

	Location(): random(false) {}
};

class DFLocations : public DataFile<Location>
{
protected:
	bool LoadSingle(std::ifstream*, Location*);

public:
	void Save();
};

#endif // DFLOCATIONS_H
