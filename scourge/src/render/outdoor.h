#ifndef OUTDOOR_H_
#define OUTDOOR_H_

class Map;
class Surface;
class Location;
#include "render.h"

class Outdoor {
private:
	Map *map;
	
public:
	Outdoor( Map *map ) { this->map = map; }
	~Outdoor() {}
	
	void draw();

	/// Draws creature effects and damage counters.
	void drawEffects();
	
	/// Draws the roofs on outdoor levels, including the fading.
	void drawRoofs();
};

#endif /*OUTDOOR_H_*/
