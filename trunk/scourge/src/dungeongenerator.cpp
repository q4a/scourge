/***************************************************************************
                          dungeongenerator.cpp  -  description
                             -------------------
    begin                : Thu May 15 2003
    copyright            : (C) 2003 by Gabor Torok
    email                : cctorok@yahoo.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dungeongenerator.h"

/*
width - max 31
height - max 31

curvyness - in %, the lower the more twisty maze
sparseness - (1-5) the higher the more sparse (more empty space)
loopyness - in %, the higher the more loops in the maze

roomcount
rooom max width
room max height

object count
*/
const int DungeonGenerator::levels[][9] = {
  { 10, 20,  90, 5,  1,    2,  4,  4,      5 },  
  { 15, 15,  70, 5, 10,    3,  6,  4,     10 },
  { 15, 15,  50, 4, 20,    4,  5,  5,     15 },
  { 20, 20,  50, 6, 20,    5,  6,  5,     20 },
  { 25, 25,  40, 6, 20,    5,  6,  5,     25 },
  { 30, 25,   5, 6, 25,    6,  6,  6,     30 },        
  { 31, 31,   3, 5, 25,    6,  7,  7,     35 }
};

/**
   Pre-generated maps:
   x,y,w,h,
   starting coord
   roomCount,
   roomDimensions: x,y,w,h,furnish(0,1)
   map: #-room, +-floor, nsew-doors (by facing)
 */
const MapLocation DungeonGenerator::location[] = {
  { 
	0,0,5,12,
	{ {0, 0}, {0, 0}, {0, 0}, {0, 0} },
	false,
	5,
	{
	  {0,0,5,4,0},
	  {0,5,2,3,1}, 
	  {3,5,2,3,1}, 
	  {0,9,2,3,1}, 
	  {3,9,2,3,1}
	},
	{ 
	  "  ## ",
	  "  ## ",
	  "  ## ",
	  "  s# ",
	  "  +  ",
	  "##+##",
	  "#e+w#",
	  "s# #s",
	  "+   +",
	  "n# #n",
	  "#e+w#",
	  "## ##" 
	},
	21, 
	{
	  { Constants::BOARD_INDEX, 2*unitSide+5, unitSide, 0 },

		{ Constants::COLUMN_INDEX, 2*unitSide+3, unitSide+8, 0 },
		{ Constants::COLUMN_INDEX, 2*unitSide+26, unitSide+8, 0 },
		{ Constants::COLUMN_INDEX, 2*unitSide+3, unitSide+16, 0 },
		{ Constants::COLUMN_INDEX, 2*unitSide+26, unitSide+16, 0 },
		{ Constants::COLUMN_INDEX, 2*unitSide+3, unitSide+24, 0 },
		{ Constants::COLUMN_INDEX, 2*unitSide+26, unitSide+24, 0 },
		{ Constants::COLUMN_INDEX, 2*unitSide+3, unitSide+32, 0 },
		{ Constants::COLUMN_INDEX, 2*unitSide+26, unitSide+32, 0 },

	  { Constants::BRAZIER_INDEX, 2*unitSide+7, unitSide+8, 2 },
	  { Constants::BRAZIER_BASE_INDEX, 2*unitSide+7, unitSide+8, 0 },		
	  { Constants::BRAZIER_INDEX, 2*unitSide+22, unitSide+8, 2 },
	  { Constants::BRAZIER_BASE_INDEX, 2*unitSide+22, unitSide+8, 0 },

	  { Constants::BRAZIER_INDEX, 2*unitSide+7, unitSide+16, 2 },
	  { Constants::BRAZIER_BASE_INDEX, 2*unitSide+7, unitSide+16, 0 },
	  { Constants::BRAZIER_INDEX, 2*unitSide+22, unitSide+16, 2 },
	  { Constants::BRAZIER_BASE_INDEX, 2*unitSide+22, unitSide+16, 0 },

	  { Constants::BRAZIER_INDEX, 2*unitSide+7, unitSide+24, 2 },
	  { Constants::BRAZIER_BASE_INDEX, 2*unitSide+7, unitSide+24, 0 },
	  { Constants::BRAZIER_INDEX, 2*unitSide+22, unitSide+24, 2 },
	  { Constants::BRAZIER_BASE_INDEX, 2*unitSide+22, unitSide+24, 0 }


	}
  }
};

DungeonGenerator::DungeonGenerator(Scourge *scourge, int level){
  this->scourge = scourge;
  this->level = level;

  initByLevel();  
  
  this->nodes = (Uint16**)malloc(sizeof(void*) * width);
  for(int i = 0; i < width; i++) {
    nodes[i] = (Uint16*)malloc(sizeof(void*) * height);
  }

  for(int x = 0; x < width; x++) {
    for(int y = 0; y < height; y++) {
      nodes[x][y] = UNVISITED;
    }
  }
  
  notVisitedCount = width * height;
  notVisited = (int*)new int[notVisitedCount];  
  for(int i = 0; i < notVisitedCount; i++) {
    notVisited[i] = i;
  }
  visitedCount = 0;
  visited = (int*)new int[notVisitedCount];
}

DungeonGenerator::~DungeonGenerator(){
  for(int i = 0; i < width; i++) {
    free(nodes[i]);
  }
  free(nodes);
  delete[] notVisited;
  delete[] visited;
}

void DungeonGenerator::initByLevel() {
  if(level < 1) level = 1;
  
  this->width = levels[level - 1][dgWIDTH];
  this->height = levels[level - 1][dgHEIGHT];
  this->curvyness = levels[level - 1][dgCURVYNESS];
  this->sparseness = levels[level - 1][dgSPARSENESS];
  this->loopyness = levels[level - 1][dgLOOPYNESS];
  this->roomCount = levels[level - 1][dgROOMCOUNT];
  this->roomMaxWidth = levels[level - 1][dgROOMMAXWIDTH];
  this->roomMaxHeight = levels[level - 1][dgROOMMAXHEIGHT];
  this->objectCount = levels[level - 1][dgOBJECTCOUNT];
  this->monsters = true;
}

void DungeonGenerator::makeRooms() {
  int rw, rh, px, py;
  int best, score;

  for(int i = 0; i < roomCount; i++) {
    // create a room
    rw = (int) ((double)(roomMaxWidth / 2) * rand()/RAND_MAX) + (roomMaxWidth / 2);
    rh = (int) ((double)(roomMaxHeight / 2) * rand()/RAND_MAX) + (roomMaxHeight / 2);
    best = -1;
    px = py = -1;
    // find best place for this room
    if(i % 2) {
      for(int x = 0; x < width - rw; x++) {
        for(int y = 0; y < height - rh; y++) {
          score = getScore(x, y, rw, rh);
          if(score > 0 && (best == -1 || score < best)) {
            best = score;
            px = x;
            py = y;
          }
        }
      }
    } else {
      for(int x = width - rw - 1; x >= 0; x--) {
        for(int y = height - rh - 1; y >= 0; y--) {
          score = getScore(x, y, rw, rh);
          if(score > 0 && (best == -1 || score < best)) {
            best = score;
            px = x;
            py = y;
          }
        }        
      }
    }
    // set the room
    if(px > -1 && py > -1) {

	  // save the room info
	  room[i].x = px;
	  room[i].y = py;
	  room[i].w = rw;
	  room[i].h = rh;

      for(int x = px; x < px + rw; x++) {
        for(int y = py; y < py + rh; y++) {
          nodes[x][y] = ROOM + N_PASS + S_PASS + E_PASS + W_PASS;

          // 1. connect the room to the passage
          // 2. put in some doors: after each door, the chance of there being
          //    another door decreases.
          // 3. put in the walls
          if(x == px) {
            if(x > 0 && nodes[x - 1][y] != UNVISITED) {
              nodes[x - 1][y] |= E_PASS;
            } else {
              nodes[x][y] -= W_PASS;
            }
          }
          if(x == px + rw - 1) {
            if(x < width - 1 && nodes[x + 1][y] != UNVISITED) {
              nodes[x + 1][y] |= W_PASS;
            } else {
              nodes[x][y] -= E_PASS;
            }
          }
          if(y == py) {
            if(y > 0 && nodes[x][y - 1] != UNVISITED) {
              nodes[x][y - 1] |= S_PASS;
            } else {
              nodes[x][y] -= N_PASS;
            }
          }
          if(y == py + rh - 1) {
            if(y < height - 1 && nodes[x][y + 1] != UNVISITED) {
              nodes[x][y + 1] |= N_PASS;
            } else {
              nodes[x][y] -= S_PASS;
            }
          }
        }
      }

      // add doors
      for(int x = px; x < px + rw; x++) {
        for(int y = py; y < py + rh; y++) {
          if(x == px) {
            if(x > 0 && nodes[x - 1][y] != UNVISITED && nodes[x - 1][y] < ROOM) {
							nodes[x][y] |= W_DOOR;
            }
          }
          if(x == px + rw - 1) {
            if(x < width - 1 && nodes[x + 1][y] != UNVISITED && nodes[x + 1][y] < ROOM) {
							nodes[x][y] |= E_DOOR;
            }
          }
          if(y == py) {
            if(y > 0 && nodes[x][y - 1] != UNVISITED && nodes[x][y - 1] < ROOM) {
							nodes[x][y] |= N_DOOR;
            }
          }
          if(y == py + rh - 1) {
            if(y < height - 1 && nodes[x][y + 1] != UNVISITED && nodes[x][y + 1] < ROOM) {
							nodes[x][y] |= S_DOOR;
            }
          }
          
        }
      }
    }
    
  }
}

int DungeonGenerator::getScore(int px, int py, int rw, int rh) {
  int score = 0;
  for(int x = px; x < px + rw; x++) {
    for(int y = py; y < py + rh; y++) {
      if(nodes[x][y] == UNVISITED) {
        if(x < width - 1 && nodes[x + 1][y] != UNVISITED) score++;
        if(x > 0 && nodes[x - 1][y] != UNVISITED) score++;
        if(y < height - 1 && nodes[x][y + 1] != UNVISITED) score++;
        if(y > 0 && nodes[x][y - 1] != UNVISITED) score++;

        if(x < width - 1 && nodes[x + 1][y] >= ROOM) score+=100;
        if(x > 0 && nodes[x - 1][y] >= ROOM) score+=100;
        if(y < height - 1 && nodes[x][y + 1] >= ROOM) score+=100;
        if(y > 0 && nodes[x][y - 1] >= ROOM) score+=100;        
      } else if(nodes[x][y] >= ROOM) {
          score += 100;
      } else {
          score += 3;
      }
    }
  }
  return score;
}

void DungeonGenerator::makeLoops() {
  for(int i = 0; i < sparseness; i++) {
    for(int x = 0; x < width; x++) {
      for(int y = 0; y < height; y++) {        
        switch(nodes[x][y]) {
          case N_PASS:
          case S_PASS:
          case E_PASS:
          case W_PASS:
            if((int) (100.0 * rand()/RAND_MAX) <= loopyness)
            generatePassage(x, y, false);
            break;
          default:
            break;
        }
      }
    }
  } 
}

void DungeonGenerator::makeSparse() {
  for(int i = 0; i < sparseness; i++) {
    for(int x = 0; x < width; x++) {
      for(int y = 0; y < height; y++) {
        switch(nodes[x][y]) {
          case N_PASS:
            nodes[x][y] = UNVISITED;
            if(y > 0) nodes[x][y - 1] -= S_PASS;
            break;
          case S_PASS:
            nodes[x][y] = UNVISITED;
            if(y < height - 1) nodes[x][y + 1] -= N_PASS;
            break;
          case E_PASS:
            nodes[x][y] = UNVISITED;
            if(x < width - 1) nodes[x + 1][y] -= W_PASS;
            break;
          case W_PASS:
            nodes[x][y] = UNVISITED;
            if(x > 0) nodes[x - 1][y] -= E_PASS;
            break;
          default:
            break;
        }
      }
    }
  }
}

void DungeonGenerator::generateMaze() {
  int x, y;

  //fprintf(stderr, "Starting maze w=%d h=%d\n", width, height);
  nextNotVisited(&x, &y);
  while(notVisitedCount > 0) {

    // draw the passage
    generatePassage(x, y, true);

    // select a starting point
    nextVisited(&x, &y);
    if(x == -1) break;
  }
}

void DungeonGenerator::generatePassage(const int sx, const int sy, const bool stopAtVisited) {
  //char line[80];

  int x = sx;
  int y = sy;
  
  //fprintf(stderr, "\tnotVisitedCount=%d x=%d y=%d\n", notVisitedCount, x, y);
  int nx = x;
  int ny = y;

  int dir = initDirections();
  // keep going while you can
  int stepCount = 0;
  bool reachedVisited = false;
  bool inMap = false;
  while(true) {
    // take a step in the selected direction
    switch(dir) {
      case DIR_N: ny--; break;
      case DIR_S: ny++; break;
      case DIR_W: nx--; break;
      case DIR_E: nx++; break;
      default: fprintf(stderr, "ERROR: unknown direction selected: %d\n", dir);
    }

    // off the map or location already visited
    inMap = (nx >= 0 && nx < width && ny >= 0 && ny < height);

    if(!inMap ||
       nodes[nx][ny] != UNVISITED ||
       stepCount++ >= width / 2 ||
       (curvyness > 1 && curvyness < 100 &&
       (int) ((double)curvyness * rand()/RAND_MAX) == 0)) {

      if(!stopAtVisited && inMap && nodes[nx][ny] != UNVISITED) {
        reachedVisited = true;
      } else {
        // step back
        nx = x;
        ny = y;
        // pick another direction
        dir = nextDirection();
        if(dir == -1) {
          break;
        }
        stepCount = 0;        
        continue;
      }
    }

    // connect to the previous cell
    switch(dir) {
      case DIR_N:
        nodes[x][y] |= N_PASS;
        if(inMap) nodes[nx][ny] |= S_PASS;
        break;
      case DIR_S:
        nodes[x][y] |= S_PASS;
        if(inMap) nodes[nx][ny] |= N_PASS;
        break;
      case DIR_W:
        nodes[x][y] |= W_PASS;
        if(inMap) nodes[nx][ny] |= E_PASS;
        break;
      case DIR_E:
        nodes[x][y] |= E_PASS;
        if(inMap) nodes[nx][ny] |= W_PASS;
        break;
    }

    if(reachedVisited) {
      reachedVisited = false;
      // step back
      nx = x;
      ny = y;
      // pick another direction
      dir = nextDirection();
      if(dir == -1) {
        break;
      }
      stepCount = 0;      
      continue;
    }
    

    // mark the cell visited
    markVisited(nx, ny);

    // save last position
    x = nx;
    y = ny;

    // debug
    //printMaze();
    //gets(line);
  }
}

void DungeonGenerator::markVisited(int x, int y) {
  // add to visited
  int n = (y * width) + x;
  // has it already been visited?
  for(int i = 0; i < visitedCount; i++) {
    if(visited[i] == n) return;
  }
  visited[visitedCount++] = n;
  // remove from not visited
  for(int i = 0; i < notVisitedCount; i++) {
    if(notVisited[i] == n) {
      notVisitedCount--;
      for(int t = i; t < notVisitedCount; t++) {
        notVisited[t] = notVisited[t + 1];
      }
      return;
    }
  }
}

bool DungeonGenerator::isVisited(int x, int y) {
  int n = (y * width) + x;
  for(int i = 0; i < visitedCount; i++) {
    if(visited[i] == n) return true;
  }
  return false;
}

void DungeonGenerator::nextNotVisited(int *x, int *y) {
  // are there no more unvisited locations?
  if(notVisitedCount <= 0) {
    *x = *y = -1;
    return;
  }
  // get a random location
  int index = (int) ((double)notVisitedCount * rand()/RAND_MAX);
  int n = notVisited[index];
  // remove from visited areas
  notVisitedCount--;
  for(int i = index; i < notVisitedCount; i++) {
    notVisited[i] = notVisited[i + 1];
  }
  // break up into x,y coordinates
  *y = n / width;
  *x = n % width;
  // add it to visited
  visited[visitedCount++] = n;
}

void DungeonGenerator::nextVisited(int *x, int *y) {
  // are there no visited locations?
  if(visitedCount <= 0) {
    *x = *y = -1;
    return;
  }
  // get a random location
  int index = (int) ((double)visitedCount * rand()/RAND_MAX);
  int n = visited[index];
  // break up into x,y coordinates
  *y = n / width;
  *x = n % width;
}

int DungeonGenerator::initDirections() {
  // init all available directions
  dirCount = DIR_COUNT;
  for(int i = 0; i < dirCount; i++) {
    dirs[i] = i;
  }
  return dirs[(int) ((double)dirCount * rand()/RAND_MAX)];
}

int DungeonGenerator::nextDirection() {
  if(dirCount <= 0) return -1;
  int index = (int) ((double)dirCount * rand()/RAND_MAX);
  int dir = dirs[index];
  dirCount--;
  for(int i = index; i < dirCount; i++) {
    dirs[i] = dirs[i + 1];
  }
  return dir;
}

void DungeonGenerator::printMaze() {
  printf("---------------------------------------\n");
  int c = 0;
  for(int y = 0; y < height; y++) {    
    for(int i = 0; i < 3; i++) {
      for(int x = 0; x < width; x++) {

          switch(i) {
            case 0: // top row
              if((nodes[x][y] & N_PASS)) {
                printf(" | ");
              } else {
                printf("   ");
              }
              break;                   
            case 1:
              if((nodes[x][y] & W_PASS)) {
                printf("-");              
              } else {
                printf(" ");
              }
              if(nodes[x][y] == UNVISITED)
                printf(" ");
              else if(nodes[x][y] & ROOM)
                printf("*");
              else
                printf("O");          
              if((nodes[x][y] & E_PASS)) {
                printf("-");
              } else {
                printf(" ");
              }
              break;
            case 2: // bottom row
              if((nodes[x][y] & S_PASS)) {
                printf(" | ");
              } else {
                printf("   ");
              }
              break;
          }        
        c++;
      }
      printf("\n");
    }
    c++;    
  }
  printf("---------------------------------------\n");
}

// draw a pre-rendered location on the map
void DungeonGenerator::constructMaze(int locationIndex) {
  // create the rooms
  roomCount = location[locationIndex].roomCount;
  for(int i = 0; i < location[locationIndex].roomCount; i++) {
	room[i].x = location[locationIndex].roomDimension[i][0];
	room[i].y = location[locationIndex].roomDimension[i][1];
	room[i].w = location[locationIndex].roomDimension[i][2];
	room[i].h = location[locationIndex].roomDimension[i][3];
  }

  // turn location into nodes
  for(int y = 0; y < location[locationIndex].h; y++) {
	for(int x = 0; x < location[locationIndex].w; x++) {
	  char c = location[locationIndex].map[y][x];
	  int nx = location[locationIndex].x + x;
	  int ny = location[locationIndex].y + y;
	  if(nx >= width || ny >= height) {
		cerr << "Warning: location doesn't fit on map! location:" << 
		  location[locationIndex].w << "," << location[locationIndex].h << 
		  " map:" << width << "," << height << endl;
		continue;
	  }
	  switch(c) {
	  case '#': case 'n': case 's': case 'e': case 'w': nodes[nx][ny] = ROOM; break;
	  case '+': nodes[nx][ny] = PASSAGE; break;
	  default: nodes[nx][ny] = UNVISITED;
	  }
	  // open every side for now
	  if(nodes[nx][ny] != UNVISITED) nodes[nx][ny] |= (N_PASS | S_PASS | W_PASS | E_PASS);
	  // add door
	}
  }

  // build walls
  for(int y = 0; y < location[locationIndex].h; y++) {
	for(int x = 0; x < location[locationIndex].w; x++) {
	  char c = location[locationIndex].map[y][x];
	  int nx = location[locationIndex].x + x;
	  int ny = location[locationIndex].y + y;
	  if(nx >= width || ny >= height) {
		cerr << "Warning: location doesn't fit on map! location:" << 
		  location[locationIndex].w << "," << location[locationIndex].h << 
		  " map:" << width << "," << height << endl;
		continue;
	  }
	  switch(c) {
	  case '#': case '+':
		break;
	  case 'n': 
		nodes[nx][ny] |= N_DOOR; 
		break;
	  case 's': 
		nodes[nx][ny] |= S_DOOR; 
		break;
	  case 'e': 
		nodes[nx][ny] |= E_DOOR; 
		break;
	  case 'w': 
		nodes[nx][ny] |= W_DOOR; 
		break;
	  }
	}
  }

  // seal off some walls
  for(int y = 0; y < location[locationIndex].h; y++) {
	for(int x = 0; x < location[locationIndex].w; x++) {
	  char c = location[locationIndex].map[y][x];
	  int nx = location[locationIndex].x + x;
	  int ny = location[locationIndex].y + y;
	  if(nx >= width || ny >= height) {
		cerr << "Warning: location doesn't fit on map! location:" << 
		  location[locationIndex].w << "," << location[locationIndex].h << 
		  " map:" << width << "," << height << endl;
		continue;
	  }
	  if(nodes[nx][ny] != UNVISITED) {
		if(!(nodes[nx][ny] & W_DOOR) &&
		   (!nx || nodes[nx - 1][ny] == UNVISITED || 
			((nodes[nx][ny] & ROOM) && !(nodes[nx - 1][ny] & ROOM)) || 
			(!(nodes[nx][ny] & ROOM) && (nodes[nx - 1][ny] & ROOM) && !(nodes[nx - 1][ny] & E_DOOR)))) {
		  nodes[nx][ny] &= (0xffff - W_PASS);
		}
		if(!(nodes[nx][ny] & E_DOOR) && 
		   (nx >= width - 1 || nodes[nx + 1][ny] == UNVISITED || 
			((nodes[nx][ny] & ROOM) && !(nodes[nx + 1][ny] & ROOM)) || 
			(!(nodes[nx][ny] & ROOM) && (nodes[nx + 1][ny] & ROOM) && !(nodes[nx + 1][ny] & W_DOOR)))) {
		  nodes[nx][ny] &= (0xffff - E_PASS);
		}
		if(!(nodes[nx][ny] & N_DOOR) && 
		   (!ny || nodes[nx][ny - 1] == UNVISITED ||
			((nodes[nx][ny] & ROOM) && !(nodes[nx][ny - 1] & ROOM)) || 
			(!(nodes[nx][ny] & ROOM) && (nodes[nx][ny - 1] & ROOM) && !(nodes[nx][ny - 1] & S_DOOR)))) {
		  nodes[nx][ny] &= (0xffff - N_PASS);
		}
		if(!(nodes[nx][ny] & S_DOOR) && 
		   (ny >= height - 1 || nodes[nx][ny + 1] == UNVISITED || 
			((nodes[nx][ny] & ROOM) && !(nodes[nx][ny + 1] & ROOM)) || 
			(!(nodes[nx][ny] & ROOM) && (nodes[nx][ny + 1] & ROOM) && !(nodes[nx][ny + 1] & N_DOOR)))) {
		  nodes[nx][ny] &= (0xffff - S_PASS);
		}
	  }
	}
  }

  // other settings
  monsters = location[locationIndex].monsters;
}

void DungeonGenerator::toMap(Map *map, ShapePalette *shapePal, int locationIndex) {	 
  bool preGenerated = (locationIndex);
  locationIndex--;

  // generate the maze
  if(!preGenerated) {
		generateMaze();
		//  printMaze();  
		
		makeSparse();
		//  printMaze();
		
		makeLoops();
		//  printMaze();
		
		makeRooms();
		//  printMaze();
	} else {
		constructMaze(locationIndex);
	}
	
	// draw the nodes on the map
	drawNodesOnMap(map, shapePal, preGenerated, locationIndex);
}

void DungeonGenerator::drawNodesOnMap(Map *map, ShapePalette *shapePal, 
																			bool preGenerated, int locationIndex) {
  // add shapes to map
  Sint16 mapx, mapy;
  for(Sint16 x = 0; x < width; x++) {    
	for(Sint16 y = 0; y < height; y++) {
				
	  mapx = x * unitSide + offset;
	  mapy = y * unitSide + offset;
	  if(nodes[x][y] != UNVISITED) {
					 
		if(nodes[x][y] >= ROOM) {
		  map->setFloorPosition(mapx, mapy + unitSide, 
								shapePal->getShape(Constants::ROOM_FLOOR_TILE_INDEX));
		} else {
		  map->setFloorPosition(mapx, mapy + unitSide, 
								shapePal->getShape(Constants::FLOOR_TILE_INDEX));
		}
					 
		// init the free space
		if(nodes[x][y] & E_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, E_DOOR);
		} else if(nodes[x][y] & W_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, W_DOOR);
		} else if(nodes[x][y] & N_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, N_DOOR);
		} else if(nodes[x][y] & S_DOOR) {
		  drawDoor(map, shapePal, mapx, mapy, S_DOOR);
		}

		// random doors
		if((nodes[x][y] & W_PASS) &&
		   !(nodes[x][y] & N_PASS) &&
		   !(nodes[x][y] & S_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, W_DOOR);
		}
		if((nodes[x][y] & E_PASS) &&
		   !(nodes[x][y] & N_PASS) &&
		   !(nodes[x][y] & S_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, E_DOOR);
		}
		if((nodes[x][y] & S_PASS) &&
		   !(nodes[x][y] & W_PASS) &&
		   !(nodes[x][y] & E_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, S_DOOR);
		}
		if((nodes[x][y] & N_PASS) &&
		   !(nodes[x][y] & W_PASS) &&
		   !(nodes[x][y] & E_PASS)) {
		  if((int)(100.0 * rand()/RAND_MAX) <= randomDoors)
			drawDoor(map, shapePal, mapx, mapy, N_DOOR);
		}
					 
		if(!(nodes[x][y] & W_PASS)) {
		  if(nodes[x][y] & N_PASS && nodes[x][y] & S_PASS) {
			map->setPosition(mapx, mapy + unitSide, 
							 0, shapePal->getShape(Constants::EW_WALL_TWO_EXTRAS_INDEX));								
		  } else if(nodes[x][y] & N_PASS) {
			map->setPosition(mapx, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(Constants::EW_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & S_PASS) {
			map->setPosition(mapx, mapy + unitSide, 
							 0, shapePal->getShape(Constants::EW_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(Constants::EW_WALL_INDEX));								
		  }						  
		  if((int) (100.0 * rand()/RAND_MAX) <= torches) {
			map->setPosition(mapx + unitOffset, mapy + unitSide - 4, 
							 6, shapePal->getShape(Constants::LAMP_WEST_INDEX));
			map->setPosition(mapx + unitOffset, mapy + unitSide - 4, 
							 4, shapePal->getShape(Constants::LAMP_BASE_INDEX));
		  }          
		}
		if(!(nodes[x][y] & E_PASS)) {
		  if(nodes[x][y] & N_PASS && nodes[x][y] & S_PASS) {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 
							 0, shapePal->getShape(Constants::EW_WALL_TWO_EXTRAS_INDEX));								
		  } else if(nodes[x][y] & N_PASS) {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(Constants::EW_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & S_PASS) {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 
							 0, shapePal->getShape(Constants::EW_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
							 0, shapePal->getShape(Constants::EW_WALL_INDEX));
		  }
		  if((int) (100.0 * rand()/RAND_MAX) <= torches) {
			map->setPosition(mapx + unitSide - (unitOffset + 1), mapy + unitSide - 4, 
							 6, shapePal->getShape(Constants::LAMP_EAST_INDEX));
			map->setPosition(mapx + unitSide - (unitOffset + 1), mapy + unitSide - 4, 
							 4, shapePal->getShape(Constants::LAMP_BASE_INDEX));
		  }          
		}
		if(!(nodes[x][y] & N_PASS)) {
		  if(nodes[x][y] & W_PASS && nodes[x][y] & E_PASS) {
			map->setPosition(mapx, mapy + unitOffset, 0, 
							 shapePal->getShape(Constants::NS_WALL_TWO_EXTRAS_INDEX));
		  } else if(nodes[x][y] & W_PASS) {
			map->setPosition(mapx, mapy + unitOffset, 0, 
							 shapePal->getShape(Constants::NS_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & E_PASS) {
			map->setPosition(mapx + unitOffset, mapy + unitOffset, 0, 
							 shapePal->getShape(Constants::NS_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx + unitOffset, mapy + unitOffset, 0, 
							 shapePal->getShape(Constants::NS_WALL_INDEX));
		  }
		  if((int) (100.0 * rand()/RAND_MAX) <= torches) {
			map->setPosition(mapx + 4, mapy + unitOffset + 1, 6, 
							 shapePal->getShape(Constants::LAMP_NORTH_INDEX));
			map->setPosition(mapx + 4, mapy + unitOffset + 1, 4, 
							 shapePal->getShape(Constants::LAMP_BASE_INDEX));
		  }            
		}
		if(!(nodes[x][y] & S_PASS)) {
		  if(nodes[x][y] & W_PASS && nodes[x][y] & E_PASS) {
			map->setPosition(mapx, mapy + unitSide, 0, 
							 shapePal->getShape(Constants::NS_WALL_TWO_EXTRAS_INDEX));
		  } else if(nodes[x][y] & W_PASS) {
			map->setPosition(mapx, mapy + unitSide, 0, 
							 shapePal->getShape(Constants::NS_WALL_EXTRA_INDEX));
		  } else if(nodes[x][y] & E_PASS) {
			map->setPosition(mapx + unitOffset, mapy + unitSide, 0, 
							 shapePal->getShape(Constants::NS_WALL_EXTRA_INDEX));
		  } else {
			map->setPosition(mapx + unitOffset, mapy + unitSide, 0, 
							 shapePal->getShape(Constants::NS_WALL_INDEX));
		  }
		}
					 
		if(nodes[x][y] & N_PASS && nodes[x][y] & W_PASS) {
		  map->setPosition(mapx, mapy + unitOffset, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX));
		}
		if(nodes[x][y] & N_PASS && nodes[x][y] & E_PASS) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX));
		}
		if(nodes[x][y] & S_PASS && nodes[x][y] & W_PASS) {
		  map->setPosition(mapx, mapy + unitSide, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX));
		}
		if(nodes[x][y] & S_PASS && nodes[x][y] & E_PASS) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX));
		}
		if(!(nodes[x][y] & N_PASS) && !(nodes[x][y] & W_PASS)) {
		  map->setPosition(mapx, mapy + unitOffset, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX));
		}
		if(!(nodes[x][y] & N_PASS) && !(nodes[x][y] & E_PASS)) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX)); 
		}
		if(!(nodes[x][y] & S_PASS) && !(nodes[x][y] & W_PASS)) {
		  map->setPosition(mapx, mapy + unitSide, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX)); 
		}
		if(!(nodes[x][y] & S_PASS) && !(nodes[x][y] & E_PASS)) {
		  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 0, 
						   shapePal->getShape(Constants::CORNER_INDEX)); 
		}					 
	  }
	}
  }
	 
  // Remove 'columns' from rooms
  for(int x = 0; x < MAP_WIDTH - unitSide; x++) {
	for(int y = 0; y < MAP_DEPTH - (unitSide * 2); y++) {
				
	  if(map->getFloorPosition(x, y + unitSide) ==
		 shapePal->getShape(Constants::ROOM_FLOOR_TILE_INDEX) && 
		 map->getFloorPosition(x + unitSide, y + unitSide) ==
		 shapePal->getShape(Constants::ROOM_FLOOR_TILE_INDEX) &&
		 map->getFloorPosition(x, y + unitSide + unitSide) ==
		 shapePal->getShape(Constants::ROOM_FLOOR_TILE_INDEX) &&         
		 map->getFloorPosition(x + unitSide, y + unitSide + unitSide) ==
		 shapePal->getShape(Constants::ROOM_FLOOR_TILE_INDEX)) {
					 
		//        map->setFloorPosition(x, y + unitSide, shapePal->getShape(Constants::FLOOR_TILE_INDEX));
		map->removePosition(x + unitSide - unitOffset, y + unitSide, 0);
		map->removePosition(x + unitSide - unitOffset, y + unitSide + unitOffset, 0);
		map->removePosition(x + unitSide, y + unitSide, 0);
		map->removePosition(x + unitSide, y + unitSide + unitOffset, 0);                
	  }
	}    
  }

	int x, y;
	RpgItem *rpgItem;
	// add the containers
	for(int i = 0; i < roomCount; i++) {
		if(preGenerated && location[locationIndex].roomDimension[i][4] == 0) continue;
		for(int pos = unitOffset; pos < room[i].h * unitSide; pos++) {
			rpgItem = RpgItem::getRandomContainer();
			if(rpgItem) {
				// WEST side
				x = (room[i].x * unitSide) + unitOffset + offset;
				y = (room[i].y * unitSide) + pos + offset;
				Shape *shape = scourge->getShapePalette()->getItemShape(rpgItem->getShapeIndex());
				if(map->shapeFits(shape, x, y, 0) && 
					 !coversDoor(map, shapePal, shape, x, y)) {
					addItem(map, NULL, scourge->newItem(rpgItem), NULL, x, y);
				}
			}
			rpgItem = RpgItem::getRandomContainer();
			if(rpgItem) {
				// EAST side
				x = ((room[i].x + room[i].w - 1) * unitSide) + unitSide - (unitOffset * 2) + offset;
				Shape *shape = scourge->getShapePalette()->getItemShape(rpgItem->getShapeIndex());
				if(map->shapeFits(shape, x, y, 0) && 
					 !coversDoor(map, shapePal, shape, x, y)) {
					addItem(map, NULL, scourge->newItem(rpgItem), NULL, x, y);
				}
			}
		}
		for(int pos = unitOffset; pos < room[i].w * unitSide; pos++) {
			rpgItem = RpgItem::getRandomContainerNS();
			if(rpgItem) {
				// NORTH side
				x = (room[i].x * unitSide) + pos + offset;
				y = (room[i].y * unitSide) + (unitOffset * 2) + offset;
				Shape *shape = scourge->getShapePalette()->getItemShape(rpgItem->getShapeIndex());
				if(map->shapeFits(shape, x, y, 0) && 
					 !coversDoor(map, shapePal, shape, x, y)) {
					addItem(map, NULL, scourge->newItem(rpgItem), NULL, x, y);
				}
			}
			rpgItem = RpgItem::getRandomContainerNS();
			if(rpgItem) {
				// SOUTH side
				y = ((room[i].y + room[i].h - 1) * unitSide) + unitSide - unitOffset + offset;
				Shape *shape = scourge->getShapePalette()->getItemShape(rpgItem->getShapeIndex());
				if(map->shapeFits(shape, x, y, 0) && 
					 !coversDoor(map, shapePal, shape, x, y)) {
					addItem(map, NULL, scourge->newItem(rpgItem), NULL, x, y);
				}
			}
		}
	}	
  
  // Collapse the free space and put objects in the available spots
	ff = (Sint16*)malloc( 2 * sizeof(Sint16) * MAP_WIDTH * MAP_DEPTH );
	if(!ff) {
		fprintf(stderr, "out of mem\n");
		exit(0);    
	}
	ffCount = 0;
	for(int fx = offset; fx < MAP_WIDTH; fx+=unitSide) {
		for(int fy = offset; fy < MAP_DEPTH; fy+=unitSide) {
			if(map->getFloorPosition(fx, fy + unitSide)) {
				for(int ffx = 0; ffx < unitSide; ffx++) {
					for(int ffy = unitSide; ffy > 0; ffy--) {
						if(!map->getLocation(fx + ffx, fy + ffy, 0)) {
							*(ff + ffCount * 2) = fx + ffx;
							*(ff + ffCount * 2 + 1) = fy + ffy;
							ffCount++;
						}
					}
				}
			}
		}
	} 

  // add pre-generated shapes first
	if(preGenerated) {
		for(int i = 0; i < location[locationIndex].shapeCount; i++) {
			int mapx = location[locationIndex].shapePosition[i][1] + offset;
			int mapy = location[locationIndex].shapePosition[i][2] + offset;
			map->setPosition(mapx, mapy, location[locationIndex].shapePosition[i][3], 
											 shapePal->getShape(location[locationIndex].shapePosition[i][0]));
			// find and remove this location from ff list (replace w. last entry and decr. counter)
			for(int n = 0; n < ffCount; n++) {
				if(mapx == ff[n * 2] && mapy == ff[n * 2 + 1]) {
					ff[n * 2] = ff[(ffCount - 1) * 2];
					ff[n * 2 + 1] = ff[(ffCount - 1) * 2 + 1];
					ffCount--;
					break;
				}
			}
		}
	}

	if(!preGenerated) {
		// add the items
		for(int i = 0; i < objectCount; i++) {
			RpgItem *rpgItem = RpgItem::getRandomItem(level);
			if(!rpgItem) {
				cerr << "Warning: no items defined for level: " << level << endl;
				break;
			}
			Item *item = scourge->newItem(rpgItem);
			getRandomLocation(map, item->getShape(), &x, &y);
			addItem(map, NULL, item, NULL, x, y);
		}
	}

  // add monsters in every room
	if(monsters) {
		int totalLevel = 0;
		for(int i = 0; i < 4; i++) totalLevel += scourge->getParty(i)->getLevel();
		//fprintf(stderr, "creating monsters for total player level: %d\n", totalLevel);
		for(int i = 0; i < roomCount; i++) {
			int levelSum = 0;
			while(levelSum < totalLevel) {
				Monster *monster = Monster::getRandomMonster(level - 1);
				//fprintf(stderr, "Trying to add %s to room %d\n", monster->getType(), i);
				if(!monster) {
					cerr << "Warning: no monsters defined for level: " << level << endl;
					break;
				}
				bool fits = 
					getLocationInRoom(map, 
														i,
														scourge->getShapePalette()->getCreatureShape(monster->getShapeIndex()), 
														&x, &y);
				if(fits) {
					//fprintf(stderr, "\tmonster fits at %d,%d.\n", x, y);
					Creature *creature = scourge->newCreature(monster);
					addItem(map, creature, NULL, NULL, x, y);
					creature->moveTo(x, y, 0);
					levelSum += level;
				} else {
					//fprintf(stderr, "\tmonster DOESN'T fit.\n");
					break;
				}
			}
		}
	
		// add a few misc. monsters in the corridors (use objectCount to approx. number of wandering monsters)
		for(int i = 0; i < objectCount * 2; i++) {
			Monster *monster = Monster::getRandomMonster(level - 1);
			if(!monster) {
				cerr << "Warning: no monsters defined for level: " << level << endl;
				break;
			}	
			Creature *creature = scourge->newCreature(monster);
			getRandomLocation(map, creature->getShape(), &x, &y);
			addItem(map, creature, NULL, NULL, x, y);
			creature->moveTo(x, y, 0);
		}
	}

	// add tables, chairs, etc.
	addItemsInRoom(RpgItem::items[RpgItem::TABLE], 1, preGenerated, locationIndex);
	addItemsInRoom(RpgItem::items[RpgItem::CHAIR], 2, preGenerated, locationIndex);	
		
	// add the party in the first room
	// FIXME: what happens if the party doesn't fit in the room?
	//  for(int i = 0; i < roomCount; i++) {
	for(int t = 0; t < 4; t++) {
		if(scourge->getParty(t)->getStateMod(Constants::dead)) continue;
		bool fits = 
			getLocationInRoom(map, 
												0,
												scourge->getParty(t)->getShape(), 
												&x, &y,
												true);
		if(fits) {
			addItem(map, scourge->getParty(t), NULL, NULL, x, y);
			scourge->getParty(t)->moveTo(x, y, 0);
		}
	}
	//}
	
  // free empty space container
  free(ff);  
}

void DungeonGenerator::drawDoor(Map *map, ShapePalette *shapePal, 
								Sint16 mapx, Sint16 mapy, int doorType) {
  switch(doorType) {
  case E_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(Constants::EW_DOOR_INDEX), 
				   mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2)) {
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
					   wallHeight - 2, shapePal->getShape(Constants::EW_DOOR_TOP_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset +  2, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset * 2 +  2, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2, 
					   0, shapePal->getShape(Constants::EW_DOOR_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));

	}
	break;
  case W_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(Constants::EW_DOOR_INDEX), 
				   mapx + 1, mapy + unitSide - unitOffset - 2)) {
	  map->setPosition(mapx, mapy + unitSide - unitOffset, 
					   wallHeight - 2, shapePal->getShape(Constants::EW_DOOR_TOP_INDEX));
	  map->setPosition(mapx, mapy + unitOffset +  2, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx, mapy + unitOffset * 2 +  2, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + 1, mapy + unitSide - unitOffset - 2, 
					   0, shapePal->getShape(Constants::EW_DOOR_INDEX));
	  map->setPosition(mapx, mapy + unitSide - unitOffset, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	}
	break;
  case N_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(Constants::NS_DOOR_INDEX), 
				   mapx + unitOffset * 2, mapy + unitOffset - 1)) {
	  map->setPosition(mapx + unitOffset, mapy + unitOffset, 
					   wallHeight - 2, shapePal->getShape(Constants::NS_DOOR_TOP_INDEX));
	  map->setPosition(mapx + unitOffset, mapy + unitOffset, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitOffset * 2, mapy + unitOffset - 1, 
					   0, shapePal->getShape(Constants::NS_DOOR_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset * 2, mapy + unitOffset, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset * 3, mapy + unitOffset, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));

	}
	break;
  case S_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->getShape(Constants::NS_DOOR_INDEX), 
				   mapx + unitOffset * 2, mapy + unitSide - 1)) {
	  map->setPosition(mapx + unitOffset, mapy + unitSide, 
					   wallHeight - 2, shapePal->getShape(Constants::NS_DOOR_TOP_INDEX));
	  map->setPosition(mapx + unitOffset, mapy + unitSide, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitOffset * 2, mapy + unitSide - 1, 
					   0, shapePal->getShape(Constants::NS_DOOR_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset * 2, mapy + unitSide, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	  map->setPosition(mapx + unitSide - unitOffset * 3, mapy + unitSide, 
					   0, shapePal->getShape(Constants::DOOR_SIDE_INDEX));
	}
	break;
  }
}

// FIXME: low likelyhood of infinite loop
void DungeonGenerator::getRandomLocation(Map *map, Shape *shape, int *xpos, int *ypos) {
  int x, y;
  while(1) {
	// get a random location
	int n = (int)((float)ffCount * rand()/RAND_MAX);
	x = ff[n * 2];
	y = ff[n * 2 + 1];
	
	// can it fit?
	bool fits = map->shapeFits(shape, x, y, 0);
	// doesn't fit? try again (could be inf. loop)
	if(fits) {
	  // remove from ff list
	  for(int i = n + 1; i < ffCount - 1; i++) {
		ff[i * 2] = ff[i * 2 + 2];
		ff[i * 2 + 1] = ff[ i * 2 + 3];
	  }
	  ffCount--;
	  // return result
	  *xpos = x;
	  *ypos = y;
	  return;
	}
  }	
}

void DungeonGenerator::addItemsInRoom(RpgItem *rpgItem, int n, 
																			bool preGenerated, int locationIndex) {
	int x, y;
	for(int i = 0; i < roomCount; i++) {
		if(preGenerated && !location[locationIndex].roomDimension[i][4]) continue;
		for(int r = 0; r < n; r++) {
			for(int t = 0; t < 5; t++) { // 5 tries
				Shape *shape = scourge->getShapePalette()->getItemShape(rpgItem->getShapeIndex());
				bool fits = getLocationInRoom(scourge->getMap(), i, shape, &x, &y);
				if(fits && !coversDoor(scourge->getMap(), scourge->getShapePalette(), shape, x, y)) {
					Item *item = scourge->newItem(rpgItem);
					addItem(scourge->getMap(), NULL, item, NULL, x, y);
					break;
				}
			}
		}
	}
}

// return false if the creature won't fit in the room
bool DungeonGenerator::getLocationInRoom(Map *map, int roomIndex, Shape *shape, 
										 int *xpos, int *ypos,
										 bool startMiddle) {

  int startx = offset + room[roomIndex].x * unitSide + unitOffset;
  int endx = offset + (room[roomIndex].x + room[roomIndex].w) * unitSide;
  int starty = offset + room[roomIndex].y * unitSide + unitOffset;
  int endy = offset + (room[roomIndex].y + room[roomIndex].h) * unitSide;

  Sint16* fff = (Sint16*)malloc( 2 * sizeof(Sint16) * (endx - startx) * (endy - starty) );  

  int count = 0;
  for(int n = 0; n < ffCount; n++) {
	if(ff[n * 2] >= startx && ff[n * 2] < endx &&
	   ff[n * 2 + 1] >=starty && ff[n * 2 + 1] < endy) {
	  fff[count * 2] = ff[n * 2];
	  fff[count * 2 + 1] = ff[n * 2 + 1];
	  count++;
	}
  }

  bool fits = false;
  while(count > 0) {
	int pos = (int)((float)count * rand() / RAND_MAX);
	int x = fff[pos * 2];
	int y = fff[pos * 2 + 1];
	fits = map->shapeFits(shape, x, y, 0);
	if(fits) {
	  // find this location in ff list
	  for(int n = 0; n < ffCount; n++) {
		if(x == ff[n * 2] && y == ff[n * 2 + 1]) {
		  ff[n * 2] = ff[(ffCount - 1) * 2];
		  ff[n * 2 + 1] = ff[(ffCount - 1) * 2 + 1];
		  /*
		  // remove from ff list
		  for(int i = n + 1; i < ffCount - 1; i++) {
			ff[i * 2] = ff[i * 2 + 2];
			ff[i * 2 + 1] = ff[ i * 2 + 3];
		  }
		  */
		  ffCount--;
		  break;
		}
	  }
	  *xpos = x;
	  *ypos = y;
	  break;	  
	} else {
	  // "remove" this from fff (replace w. last element and decrement counter)
	  fff[pos * 2] = fff[(count - 1) * 2];
	  fff[pos * 2 + 1] = fff[(count - 1) * 2 + 1];
	  count--;
	}
  }

  free(fff);
  return fits;
}

bool DungeonGenerator::coversDoor(Map *map, ShapePalette *shapePal, 
								  Shape *shape, int x, int y) {
  for(int ty = y - shape->getDepth() - 3; ty < y + 3; ty++) {
	for(int tx = x - 3; tx < x + shape->getWidth() + 3; tx++) {
	  if(isDoor(map, shapePal, tx, ty)) return true;
	}
  }
  return false;
}

bool DungeonGenerator::isDoor(Map *map, ShapePalette *shapePal, int tx, int ty) {
  if(tx >= 0 && tx < MAP_WIDTH && 
	 ty >= 0 && ty < MAP_DEPTH) {
	Location *loc = map->getLocation(tx, ty, 0);
	if(loc && 
	   (loc->shape == shapePal->getShape(Constants::EW_DOOR_INDEX) ||
		loc->shape == shapePal->getShape(Constants::NS_DOOR_INDEX))) {
	  return true;
	}
  }
  return false;
}

void DungeonGenerator::addItem(Map *map, Creature *creature, Item *item, Shape *shape, int x, int y) {
  if(creature) map->setCreature(x, y, 0, creature);
  else if(item) map->setItem(x, y, 0, item);
  else map->setPosition(x, y, 0, shape);
  // populate containers
  if(item && item->getRpgItem()->getType() == RpgItem::CONTAINER) {
	int n = (int)(3.0f * rand() / RAND_MAX);
	for(int i = 0; i < n; i++) {
	  item->addContainedItem(scourge->newItem(RpgItem::getRandomItem(level)));
	}
  }
}
