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

const char DungeonGenerator::MESSAGE[] = "Assembling Dungeon Level";

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
	{ 
		{2*unitSide+16, unitSide+6}, 
	  {2*unitSide+13, unitSide+9}, 
		{2*unitSide+19, unitSide+9}, 
		{2*unitSide+16, unitSide+12} 
	},
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
	25, 
	{
	  { "BOARD", 2*unitSide+11, unitSide + 1, 0 },


		{ "COLUMN", 2*unitSide+3, unitSide, 0 },
		{ "COLUMN", 2*unitSide+26, unitSide, 0 },
		{ "COLUMN", 2*unitSide+3, unitSide+8, 0 },
		{ "COLUMN", 2*unitSide+26, unitSide+8, 0 },
		{ "COLUMN", 2*unitSide+3, unitSide+16, 0 },
		{ "COLUMN", 2*unitSide+26, unitSide+16, 0 },
		{ "COLUMN", 2*unitSide+3, unitSide+24, 0 },
		{ "COLUMN", 2*unitSide+26, unitSide+24, 0 },
		{ "COLUMN", 2*unitSide+3, unitSide+32, 0 },
		{ "COLUMN", 2*unitSide+26, unitSide+32, 0 },
		{ "COLUMN", 2*unitSide+3, unitSide+40, 0 },
		{ "COLUMN", 2*unitSide+26, unitSide+40, 0 },

	  { "BRAZIER", 2*unitSide+7, unitSide+8, 2 },
	  { "BRAZIER_BASE", 2*unitSide+7, unitSide+8, 0 },		
	  { "BRAZIER", 2*unitSide+22, unitSide+8, 2 },
	  { "BRAZIER_BASE", 2*unitSide+22, unitSide+8, 0 },

	  { "BRAZIER", 2*unitSide+7, unitSide+16, 2 },
	  { "BRAZIER_BASE", 2*unitSide+7, unitSide+16, 0 },
	  { "BRAZIER", 2*unitSide+22, unitSide+16, 2 },
	  { "BRAZIER_BASE", 2*unitSide+22, unitSide+16, 0 },

	  { "BRAZIER", 2*unitSide+7, unitSide+24, 2 },
	  { "BRAZIER_BASE", 2*unitSide+7, unitSide+24, 0 },
	  { "BRAZIER", 2*unitSide+22, unitSide+24, 2 },
	  { "BRAZIER_BASE", 2*unitSide+22, unitSide+24, 0 }


	}
  }
};

DungeonGenerator::DungeonGenerator(Scourge *scourge, int level, bool stairsDown, 
								   bool stairsUp, Mission *mission){
  this->scourge = scourge;
  this->level = level;
  this->stairsUp = stairsUp;
  this->stairsDown = stairsDown;
  this->mission = mission;

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

  doorCount = 0;

  progress = new Progress(scourge, 16);
}

DungeonGenerator::~DungeonGenerator(){
  delete progress;

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
            if(y > 0) {
			  nodes[x][y - 1] -= S_PASS;
			}
            break;
          case S_PASS:
            nodes[x][y] = UNVISITED;
            if(y < height - 1) {
			  nodes[x][y + 1] -= N_PASS;
			}
            break;
          case E_PASS:
            nodes[x][y] = UNVISITED;
            if(x < width - 1) {
			  nodes[x + 1][y] -= W_PASS;
			}
            break;
          case W_PASS:
            nodes[x][y] = UNVISITED;
            if(x > 0) {
			  nodes[x - 1][y] -= E_PASS;
			}
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
	  //char c = location[locationIndex].map[y][x];
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

  progress->updateStatus(MESSAGE);
  //scourge->getSDLHandler()->setHandlers((SDLEventHandler *)this, (SDLScreenView *)this);

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
  progress->updateStatus(MESSAGE);
    
  // draw the nodes on the map
  drawNodesOnMap(map, shapePal, preGenerated, locationIndex);
  
  // hack: draw diff. floor tiles in HQ
  if(preGenerated) {
	for(Sint16 x = 0; x < MAP_WIDTH; x++) {
	  for(Sint16 y = 0; y < MAP_DEPTH; y++) {
		if(map->getFloorPosition(x,y) == shapePal->findShapeByName("ROOM_FLOOR_TILE")) {
		  map->setFloorPosition(x,y,shapePal->findShapeByName("ROOM2_FLOOR_TILE"));
		}
	  }
	}
  }
  progress->updateStatus(MESSAGE);
}

void DungeonGenerator::drawBasics(Map *map, ShapePalette *shapePal, 
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
                                shapePal->findShapeByName("ROOM_FLOOR_TILE"));
        } else {
          map->setFloorPosition(mapx, mapy + unitSide, 
                                shapePal->findShapeByName("FLOOR_TILE"));
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
          if((int)(100.0 * rand()/RAND_MAX) <= randomDoors) {
            drawDoor(map, shapePal, mapx, mapy, W_DOOR);
          }
        }
        if((nodes[x][y] & E_PASS) &&
           !(nodes[x][y] & N_PASS) &&
           !(nodes[x][y] & S_PASS)) {
          if((int)(100.0 * rand()/RAND_MAX) <= randomDoors) {
            drawDoor(map, shapePal, mapx, mapy, E_DOOR);
          }
        }
        if((nodes[x][y] & S_PASS) &&
           !(nodes[x][y] & W_PASS) &&
           !(nodes[x][y] & E_PASS)) {
          if((int)(100.0 * rand()/RAND_MAX) <= randomDoors) {
            drawDoor(map, shapePal, mapx, mapy, S_DOOR);
          }
        }
        if((nodes[x][y] & N_PASS) &&
           !(nodes[x][y] & W_PASS) &&
           !(nodes[x][y] & E_PASS)) {
          if((int)(100.0 * rand()/RAND_MAX) <= randomDoors) {
            drawDoor(map, shapePal, mapx, mapy, N_DOOR);
          }
        }

        if(!(nodes[x][y] & W_PASS)) {
          if(nodes[x][y] & N_PASS && nodes[x][y] & S_PASS) {
            map->setPosition(mapx, mapy + unitSide, 
                             0, shapePal->findShapeByName("EW_WALL_TWO_EXTRAS"));               
          } else if(nodes[x][y] & N_PASS) {
            map->setPosition(mapx, mapy + unitSide - unitOffset, 
                             0, shapePal->findShapeByName("EW_WALL_EXTRA"));
          } else if(nodes[x][y] & S_PASS) {
            map->setPosition(mapx, mapy + unitSide, 
                             0, shapePal->findShapeByName("EW_WALL_EXTRA"));
          } else {
            map->setPosition(mapx, mapy + unitSide - unitOffset, 
                             0, shapePal->findShapeByName("EW_WALL"));                
          }             
          if((int) (100.0 * rand()/RAND_MAX) <= torches) {
            map->setPosition(mapx + unitOffset, mapy + unitSide - 4, 
                             6, shapePal->findShapeByName("LAMP_WEST"));
            map->setPosition(mapx + unitOffset, mapy + unitSide - 4, 
                             4, shapePal->findShapeByName("LAMP_BASE"));
          }
        }
        if(!(nodes[x][y] & E_PASS)) {
          if(nodes[x][y] & N_PASS && nodes[x][y] & S_PASS) {
            map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 
                             0, shapePal->findShapeByName("EW_WALL_TWO_EXTRAS"));               
          } else if(nodes[x][y] & N_PASS) {
            map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
                             0, shapePal->findShapeByName("EW_WALL_EXTRA"));
          } else if(nodes[x][y] & S_PASS) {
            map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 
                             0, shapePal->findShapeByName("EW_WALL_EXTRA"));
          } else {
            map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
                             0, shapePal->findShapeByName("EW_WALL"));
          }
          if((int) (100.0 * rand()/RAND_MAX) <= torches) {
            map->setPosition(mapx + unitSide - (unitOffset + 1), mapy + unitSide - 4, 
                             6, shapePal->findShapeByName("LAMP_EAST"));
            map->setPosition(mapx + unitSide - (unitOffset + 1), mapy + unitSide - 4, 
                             4, shapePal->findShapeByName("LAMP_BASE"));
          }
        }
        if(!(nodes[x][y] & N_PASS)) {
          if(nodes[x][y] & W_PASS && nodes[x][y] & E_PASS) {
            map->setPosition(mapx, mapy + unitOffset, 0, 
                             shapePal->findShapeByName("NS_WALL_TWO_EXTRAS"));
          } else if(nodes[x][y] & W_PASS) {
            map->setPosition(mapx, mapy + unitOffset, 0, 
                             shapePal->findShapeByName("NS_WALL_EXTRA"));
          } else if(nodes[x][y] & E_PASS) {
            map->setPosition(mapx + unitOffset, mapy + unitOffset, 0, 
                             shapePal->findShapeByName("NS_WALL_EXTRA"));
          } else {
            map->setPosition(mapx + unitOffset, mapy + unitOffset, 0, 
                             shapePal->findShapeByName("NS_WALL"));
          }
          if((int) (100.0 * rand()/RAND_MAX) <= torches) {
            map->setPosition(mapx + 4, mapy + unitOffset + 1, 6, 
                             shapePal->findShapeByName("LAMP_NORTH"));
            map->setPosition(mapx + 4, mapy + unitOffset + 1, 4, 
                             shapePal->findShapeByName("LAMP_BASE"));
          }
        }
        if(!(nodes[x][y] & S_PASS)) {
          if(nodes[x][y] & W_PASS && nodes[x][y] & E_PASS) {
            map->setPosition(mapx, mapy + unitSide, 0, 
                             shapePal->findShapeByName("NS_WALL_TWO_EXTRAS"));
          } else if(nodes[x][y] & W_PASS) {
            map->setPosition(mapx, mapy + unitSide, 0, 
                             shapePal->findShapeByName("NS_WALL_EXTRA"));
          } else if(nodes[x][y] & E_PASS) {
            map->setPosition(mapx + unitOffset, mapy + unitSide, 0, 
                             shapePal->findShapeByName("NS_WALL_EXTRA"));
          } else {
            map->setPosition(mapx + unitOffset, mapy + unitSide, 0, 
                             shapePal->findShapeByName("NS_WALL"));
          }
        }

        if(nodes[x][y] & N_PASS && nodes[x][y] & W_PASS) {
          map->setPosition(mapx, mapy + unitOffset, 0, 
                           shapePal->findShapeByName("CORNER"));
        }
        if(nodes[x][y] & N_PASS && nodes[x][y] & E_PASS) {
          map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset, 0, 
                           shapePal->findShapeByName("CORNER"));
        }
        if(nodes[x][y] & S_PASS && nodes[x][y] & W_PASS) {
          map->setPosition(mapx, mapy + unitSide, 0, 
                           shapePal->findShapeByName("CORNER"));
        }
        if(nodes[x][y] & S_PASS && nodes[x][y] & E_PASS) {
          map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 0, 
                           shapePal->findShapeByName("CORNER"));
        }
        if(!(nodes[x][y] & N_PASS) && !(nodes[x][y] & W_PASS)) {
          map->setPosition(mapx, mapy + unitOffset, 0, 
                           shapePal->findShapeByName("CORNER"));
        }
        if(!(nodes[x][y] & N_PASS) && !(nodes[x][y] & E_PASS)) {
          map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset, 0, 
                           shapePal->findShapeByName("CORNER")); 
        }
        if(!(nodes[x][y] & S_PASS) && !(nodes[x][y] & W_PASS)) {
          map->setPosition(mapx, mapy + unitSide, 0, 
                           shapePal->findShapeByName("CORNER")); 
        }
        if(!(nodes[x][y] & S_PASS) && !(nodes[x][y] & E_PASS)) {
          map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide, 0, 
                           shapePal->findShapeByName("CORNER")); 
        }
      }
    }
  }
}

void DungeonGenerator::removeColumns(Map *map, ShapePalette *shapePal, 
                                     bool preGenerated, int locationIndex) {
  // Remove 'columns' from rooms
  for(int x = 0; x < MAP_WIDTH - unitSide; x++) {
    for(int y = 0; y < MAP_DEPTH - (unitSide * 2); y++) {

      if(map->getFloorPosition(x, y + unitSide) ==
         shapePal->findShapeByName("ROOM_FLOOR_TILE") && 
         map->getFloorPosition(x + unitSide, y + unitSide) ==
         shapePal->findShapeByName("ROOM_FLOOR_TILE") &&
         map->getFloorPosition(x, y + unitSide + unitSide) ==
         shapePal->findShapeByName("ROOM_FLOOR_TILE") &&         
         map->getFloorPosition(x + unitSide, y + unitSide + unitSide) ==
         shapePal->findShapeByName("ROOM_FLOOR_TILE")) {

        //        map->setFloorPosition(x, y + unitSide, shapePal->findShapeByName("FLOOR_TILE"));
        map->removePosition(x + unitSide - unitOffset, y + unitSide, 0);
        map->removePosition(x + unitSide - unitOffset, y + unitSide + unitOffset, 0);
        map->removePosition(x + unitSide, y + unitSide, 0);
        map->removePosition(x + unitSide, y + unitSide + unitOffset, 0);                
      }
    }
  }
}

void DungeonGenerator::addContainers(Map *map, ShapePalette *shapePal, 
                                     bool preGenerated, int locationIndex) {
  int x = 0;
  int y = 0;
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
        Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
        if(map->shapeFits(shape, x, y, 0) && 
           !coversDoor(map, shapePal, shape, x, y)) {
          addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
        }
      }
      rpgItem = RpgItem::getRandomContainer();
      if(rpgItem) {
        // EAST side
        x = ((room[i].x + room[i].w - 1) * unitSide) + unitSide - (unitOffset * 2) + offset;
        Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
        if(map->shapeFits(shape, x, y, 0) && 
           !coversDoor(map, shapePal, shape, x, y)) {
          addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
        }
      }
    }
    for(int pos = unitOffset; pos < room[i].w * unitSide; pos++) {
      rpgItem = RpgItem::getRandomContainerNS();
      if(rpgItem) {
        // NORTH side
        x = (room[i].x * unitSide) + pos + offset;
        y = (room[i].y * unitSide) + (unitOffset * 2) + offset;
        Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
        if(map->shapeFits(shape, x, y, 0) && 
           !coversDoor(map, shapePal, shape, x, y)) {
          addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
        }
      }
      rpgItem = RpgItem::getRandomContainerNS();
      if(rpgItem) {
        // SOUTH side
        y = ((room[i].y + room[i].h - 1) * unitSide) + unitSide - unitOffset + offset;
        Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
        if(map->shapeFits(shape, x, y, 0) && 
           !coversDoor(map, shapePal, shape, x, y)) {
          addItem(map, NULL, scourge->getSession()->newItem(rpgItem), NULL, x, y);
        }
      }
    }
  }
}

void DungeonGenerator::addStairs(Map *map, ShapePalette *shapePal, 
                                 bool preGenerated, int locationIndex) {
  // add stairs for multi-level missions
  if(stairsUp) {
    bool done = false;
    for(int i = 0; i < 10; i++) {
      Shape *shape = scourge->getShapePalette()->findShapeByName("GATE_UP");
      int x, y;
      bool fits = getLocationInRoom(map, i, shape, &x, &y);
      if(fits && !coversDoor(map, scourge->getShapePalette(), shape, x, y)) {
        addItem(map, NULL, NULL, shape, x, y);
        done = true;
        break;
      }
    }
    if(!done) {
      cerr << "Error: couldn't add up stairs." << endl;
      exit(1);
    }
  }
  if(stairsDown) {
    bool done = false;
    for(int i = 0; i < 10; i++) {
      Shape *shape = scourge->getShapePalette()->findShapeByName("GATE_DOWN");
      int x, y;
      bool fits = getLocationInRoom(map, i, shape, &x, &y);
      if(fits && !coversDoor(map, scourge->getShapePalette(), shape, x, y)) {
        addItem(map, NULL, NULL, shape, x, y);
        done = true;
        break;
      }
    }
    if(!done) {
      cerr << "Error: couldn't add down stairs." << endl;
      exit(1);
    }
  }
}

void DungeonGenerator::addPregeneratedShapes(Map *map, ShapePalette *shapePal, 
                                             bool preGenerated, int locationIndex) {
  // add pre-generated shapes first
  for(int i = 0; i < location[locationIndex].shapeCount; i++) {
    int mapx = location[locationIndex].shapePosition[i].x + offset;
    int mapy = location[locationIndex].shapePosition[i].y + offset;
    map->setPosition(mapx, mapy, location[locationIndex].shapePosition[i].z, 
                     shapePal->findShapeByName(location[locationIndex].shapePosition[i].name));
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

void DungeonGenerator::addItems(Map *map, ShapePalette *shapePal,
                                bool preGenerated, int locationIndex) {
  // add the items
  for(int i = 0; i < objectCount; i++) {
    RpgItem *rpgItem = RpgItem::getRandomItem(level);
    if(!rpgItem) {
      cerr << "Warning: no items defined for level: " << level << endl;
      break;
    }
    Item *item = scourge->getSession()->newItem(rpgItem);
    int x, y;
    getRandomLocation(map, item->getShape(), &x, &y);
    addItem(map, NULL, item, NULL, x, y);
  }

  // add some scrolls with spells
  for(int i = 0; i < objectCount / 4; i++) {
    Spell *spell = MagicSchool::getRandomSpell(level);
    if(!spell) {
      cerr << "Warning: no spells defined for level: " << level << endl;
      break;
    }
    Item *item = scourge->getSession()->newItem(RpgItem::getItemByName("Scroll"), spell);
    int x, y;
    getRandomLocation(map, item->getShape(), &x, &y);
    addItem(map, NULL, item, NULL, x, y);
  }

  // populate containers
  for(int i = 0; i < containers.size(); i++) {
    Item *item = containers[i];
    // some items
    int n = (int)(3.0f * rand() / RAND_MAX);
    for(int i = 0; i < n; i++) {
      RpgItem *containedItem = RpgItem::getRandomItem(level);
      if(containedItem) item->addContainedItem(scourge->getSession()->newItem(containedItem));
    }
    // some spells
    if(!((int)(25.0f * rand() / RAND_MAX))) {
      int n = (int)(2.0f * rand() / RAND_MAX) + 1;
      for(int i = 0; i < n; i++) {
        Spell *spell = MagicSchool::getRandomSpell(level);
        if(spell) {
          Item *scroll = scourge->getSession()->newItem(RpgItem::getItemByName("Scroll"), spell);
          item->addContainedItem(scroll);
        }
      }
    }
  }
}

void DungeonGenerator::addMissionObjectives(Map *map, ShapePalette *shapePal, 
                                            bool preGenerated, int locationIndex) {
  // add mission objects 
  if(mission && mission->getObjective() && !stairsDown) {

    // mission objects are on a pedestal
    // and make them blocking so creatures can't get them
    for(int i = 0; i < mission->getObjective()->itemCount; i++) {
      Item *item = scourge->getSession()->newItem(mission->getObjective()->item[i]);
      item->setBlocking(true); // don't let monsters pick this up
      Item *pedestal = scourge->getSession()->newItem(RpgItem::getItemByName("Pedestal"));
      int x, y;
      getRandomLocation(map, pedestal->getShape(), &x, &y);
      addItem(map, NULL, pedestal, NULL, x, y);
      addItem(map, NULL, item, NULL, 
              x + (pedestal->getShape()->getWidth()/2) - (item->getShape()->getWidth()/2), 
              y - (pedestal->getShape()->getDepth()/2) + (item->getShape()->getDepth()/2), 
              pedestal->getShape()->getHeight());
      cerr << "*** Added mission item: " << item->getItemName() << endl;
    }

    // add mission creatures
    for(int i = 0; i < mission->getObjective()->monsterCount; i++) {
      GLShape *shape = 
      scourge->getShapePalette()->
      getCreatureBlockShape(mission->getObjective()->monster[i]->getModelName());
      int x, y;
      getRandomLocation(map, shape, &x, &y);    
      Creature *creature = scourge->getSession()->newCreature(mission->getObjective()->monster[i]);
      addItem(map, creature, NULL, NULL, x, y);
      creature->moveTo(x, y, 0);
      cerr << "*** Added mission monster: " << creature->getMonster()->getType() << endl;
    }
  }
}

void DungeonGenerator::addMonsters(Map *map, ShapePalette *shapePal, 
                                   bool preGenerated, int locationIndex) {
  // add monsters in every room
  if(monsters) {
    //int totalLevel = scourge->getParty()->getTotalLevel();
    //fprintf(stderr, "creating monsters for total player level: %d\n", totalLevel);
    for(int i = 0; i < roomCount; i++) {
      int areaCovered = 0;
      // don't crowd the rooms
      int roomAreaUsed = (int)(room[i].w * room[i].h * unitSide * 0.33f);
      while(areaCovered < roomAreaUsed) {
        Monster *monster = Monster::getRandomMonster(level);
        //fprintf(stderr, "Trying to add %s to room %d\n", monster->getType(), i);
        if(!monster) {
          cerr << "Warning: no monsters defined for level: " << level << endl;
          break;
        }

        // use the creature's block shape to see if it would fit
        GLShape *shape = 
        scourge->getShapePalette()->
        getCreatureBlockShape(monster->getModelName());
        int x, y;
        bool fits = getLocationInRoom(map, i, shape, &x, &y);

        if(fits) {
          //fprintf(stderr, "\tmonster fits at %d,%d.\n", x, y);
          Creature *creature = scourge->getSession()->newCreature(monster);
          addItem(map, creature, NULL, NULL, x, y);
          creature->moveTo(x, y, 0);
          areaCovered += (creature->getShape()->getWidth() * 
                          creature->getShape()->getDepth());
        } else {
          //fprintf(stderr, "\tmonster DOESN'T fit.\n");
          break;
        }
      }
    }

    // add a few misc. monsters in the corridors (use objectCount to approx. number of wandering monsters)
    for(int i = 0; i < objectCount * 2; i++) {
      Monster *monster = Monster::getRandomMonster(level);
      if(!monster) {
        cerr << "Warning: no monsters defined for level: " << level << endl;
        break;
      }
      Creature *creature = scourge->getSession()->newCreature(monster);
      int x, y;
      getRandomLocation(map, creature->getShape(), &x, &y);
      addItem(map, creature, NULL, NULL, x, y);
      creature->moveTo(x, y, 0);
    }
  }
}

void DungeonGenerator::addFurniture(Map *map, ShapePalette *shapePal, 
                                    bool preGenerated, int locationIndex) {
  // add tables, chairs, etc.
  addItemsInRoom(RpgItem::getItemByName("Table"), 1, preGenerated, locationIndex);
  addItemsInRoom(RpgItem::getItemByName("Chair"), 2, preGenerated, locationIndex);  
}

void DungeonGenerator::addTeleporters(Map *map, ShapePalette *shapePal, 
                                      bool preGenerated, int locationIndex) {
  int teleportersAdded = 0;
  for(int teleporterCount = 0; teleporterCount < 3; teleporterCount++) {
    int x, y;
    getRandomLocation(map, scourge->getShapePalette()->findShapeByName("TELEPORTER"), &x, &y);
    if( x < MAP_WIDTH ) {
      cerr << "teleporter at " << x << "," << y << endl;
      addItem(scourge->getMap(), NULL, NULL, 
              scourge->getShapePalette()->findShapeByName("TELEPORTER"), 
              x, y, 1);
      addItem(scourge->getMap(), NULL, NULL, 
              scourge->getShapePalette()->findShapeByName("TELEPORTER_BASE"), 
              x, y);
      teleportersAdded++;
    } else {
      cerr << "ERROR: couldn't add teleporter!!! #" << teleporterCount << endl;
    }
  }
  if(teleportersAdded == 0) exit(0);
}

void DungeonGenerator::addParty(Map *map, ShapePalette *shapePal, 
                                bool preGenerated, int locationIndex) {
  // add the party in the first room
  // FIXME: what happens if the party doesn't fit in the room?
  //  for(int i = 0; i < roomCount; i++) {
  int x, y;
  for(int t = 0; t < scourge->getParty()->getPartySize(); t++) {
    if(scourge->getParty()->getParty(t)->getStateMod(Constants::dead)) continue;
    bool fits;
    if(preGenerated && location[locationIndex].start[t][0] > 0) {
      fits = true;
      x = location[locationIndex].start[t][0] + offset;
      y = location[locationIndex].start[t][1] + offset;
    } else {
      fits = 
      getLocationInRoom(map, 
                        0,
                        scourge->getParty()->getParty(t)->getShape(), 
                        &x, &y,
                        true);
    }
    if(fits) {
      addItem(map, scourge->getParty()->getParty(t), NULL, NULL, x, y);
      scourge->getParty()->getParty(t)->moveTo(x, y, 0);
      scourge->getParty()->getParty(t)->setSelXY(-1,-1);
    }
  }
}

void DungeonGenerator::lockDoors(Map *map, ShapePalette *shapePal, 
                                 bool preGenerated, int locationIndex) {
  // lock some doors
  //cerr << "*** Locking doors, count=" << doorCount << endl;
  for(int i = 0; i < doorCount; i++) {
    Sint16 mapx = door[i][0];
    Sint16 mapy = door[i][1];
    if((int)(10.0f * rand() / RAND_MAX) == 0) {
      //cerr << "\t*** Locking door: " << mapx << "," << mapy << endl;
      // lock the door
      map->setLocked(mapx, mapy, 0, true);
      // find an accessible location for the switch
      int nx, ny;
      Shape *lever = scourge->getShapePalette()->findShapeByName("SWITCH_OFF");
      Uint32 start = SDL_GetTicks();
      getRandomLocation(map, lever, &nx, &ny, true, 
                        scourge->getParty()->getPlayer()->getX(), 
                        scourge->getParty()->getPlayer()->getY());
      //cerr << "\t*** Location for lever search: " << (SDL_GetTicks() - start) << 
      //  " millis. Result=" << ( nx < MAP_WIDTH ) << endl;
      if( nx < MAP_WIDTH ) {
        //cerr << "\t\t*** Lever at: " << nx << "," << ny << endl;
        // place the switch
        addItem(scourge->getMap(), NULL, NULL, lever, nx, ny, 0);
        // connect the switch and the door
        map->setKeyLocation(mapx, mapy, 0, nx, ny, 0);
      } else {
        // if none found, unlock the door
        map->removeLocked(mapx, mapy, 0);
      }
    }
  }
  //cerr << "*** Done locking doors" << endl;
}

void DungeonGenerator::createFreeSpaceMap(Map *map, ShapePalette *shapePal, 
                                          bool preGenerated, int locationIndex) {
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
}

void DungeonGenerator::deleteFreeSpaceMap(Map *map, ShapePalette *shapePal, 
                                          bool preGenerated, int locationIndex) {
  // free empty space container
  free(ff);  
}

void DungeonGenerator::drawNodesOnMap(Map *map, ShapePalette *shapePal, 
                                      bool preGenerated, int locationIndex) {
  progress->updateStatus("Drawing walls");
  drawBasics(map, shapePal, preGenerated, locationIndex);
  
  progress->updateStatus("Fixing rooms");
  removeColumns(map, shapePal, preGenerated, locationIndex);
   
  progress->updateStatus("Adding containers");
  addContainers(map, shapePal, preGenerated, locationIndex);

  progress->updateStatus("Compressing free space");
  createFreeSpaceMap(map, shapePal, preGenerated, locationIndex);

  progress->updateStatus("Adding party");
  addParty(map, shapePal, preGenerated, locationIndex);

  progress->updateStatus("Locking doors and chests");
  lockDoors(map, shapePal, preGenerated, locationIndex);


  progress->updateStatus("Adding gates");
  if(!preGenerated) {
    addStairs(map, shapePal, preGenerated, locationIndex);
  }

  progress->updateStatus("Adding pre-generated shapes");
  if(preGenerated) {
    addPregeneratedShapes(map, shapePal, preGenerated, locationIndex);
  }

  progress->updateStatus("Adding items and mission objectives");
  if(!preGenerated) {
    addItems(map, shapePal, preGenerated, locationIndex);
    addMissionObjectives(map, shapePal, preGenerated, locationIndex);
  }

  progress->updateStatus("Adding monsters");
  addMonsters(map, shapePal, preGenerated, locationIndex);

  progress->updateStatus("Adding furniture");
  addFurniture(map, shapePal, preGenerated, locationIndex);

  // add a teleporters
  progress->updateStatus("Adding teleporters");
  if(!preGenerated) {
    addTeleporters(map, shapePal, preGenerated, locationIndex);
  }

  progress->updateStatus("Cleaning up");
  deleteFreeSpaceMap(map, shapePal, preGenerated, locationIndex);
}

void DungeonGenerator::drawDoor(Map *map, ShapePalette *shapePal, 
								Sint16 mapx, Sint16 mapy, int doorType) {
  switch(doorType) {
  case E_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->findShapeByName("EW_DOOR"), 
				   mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2)) {
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
					   wallHeight - 2, shapePal->findShapeByName("EW_DOOR_TOP"));
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset +  2, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitOffset * 2 +  2, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx + unitSide - unitOffset + 1, mapy + unitSide - unitOffset - 2, 
					   0, shapePal->findShapeByName("EW_DOOR"));
    if(doorCount < MAX_DOOR_COUNT) {
      door[doorCount][0] = mapx + unitSide - unitOffset + 1;
      door[doorCount][1] = mapy + unitSide - unitOffset - 2;
      doorCount++;
    }
	  map->setPosition(mapx + unitSide - unitOffset, mapy + unitSide - unitOffset, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));

	}
	break;
  case W_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->findShapeByName("EW_DOOR"), 
				   mapx + 1, mapy + unitSide - unitOffset - 2)) {
	  map->setPosition(mapx, mapy + unitSide - unitOffset, 
					   wallHeight - 2, shapePal->findShapeByName("EW_DOOR_TOP"));
	  map->setPosition(mapx, mapy + unitOffset +  2, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx, mapy + unitOffset * 2 +  2, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx + 1, mapy + unitSide - unitOffset - 2, 
					   0, shapePal->findShapeByName("EW_DOOR"));
    if(doorCount < MAX_DOOR_COUNT) {
      door[doorCount][0] = mapx + 1;
      door[doorCount][1] = mapy + unitSide - unitOffset - 2;
      doorCount++;
    }
	  map->setPosition(mapx, mapy + unitSide - unitOffset, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	}
	break;
  case N_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->findShapeByName("NS_DOOR"), 
				   mapx + unitOffset * 2, mapy + unitOffset - 1)) {
	  map->setPosition(mapx + unitOffset, mapy + unitOffset, 
					   wallHeight - 2, shapePal->findShapeByName("NS_DOOR_TOP"));
	  map->setPosition(mapx + unitOffset, mapy + unitOffset, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx + unitOffset * 2, mapy + unitOffset - 1, 
					   0, shapePal->findShapeByName("NS_DOOR"));
    if(doorCount < MAX_DOOR_COUNT) {
      door[doorCount][0] = mapx + unitOffset * 2;
      door[doorCount][1] = mapy + unitOffset - 1;
      doorCount++;
    }
	  map->setPosition(mapx + unitSide - unitOffset * 2, mapy + unitOffset, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx + unitSide - unitOffset * 3, mapy + unitOffset, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));

	}
	break;
  case S_DOOR:
	if(!coversDoor(map, shapePal, 
				   shapePal->findShapeByName("NS_DOOR"), 
				   mapx + unitOffset * 2, mapy + unitSide - 1)) {
	  map->setPosition(mapx + unitOffset, mapy + unitSide, 
					   wallHeight - 2, shapePal->findShapeByName("NS_DOOR_TOP"));
	  map->setPosition(mapx + unitOffset, mapy + unitSide, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx + unitOffset * 2, mapy + unitSide - 1, 
					   0, shapePal->findShapeByName("NS_DOOR"));
    if(doorCount < MAX_DOOR_COUNT) {
      door[doorCount][0] = mapx + unitOffset * 2;
      door[doorCount][1] = mapy + unitSide - 1;
      doorCount++;
    }
	  map->setPosition(mapx + unitSide - unitOffset * 2, mapy + unitSide, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	  map->setPosition(mapx + unitSide - unitOffset * 3, mapy + unitSide, 
					   0, shapePal->findShapeByName("DOOR_SIDE"));
	}
	break;
  }
}

// FIXME: low likelyhood of infinite loop
void DungeonGenerator::getRandomLocation(Map *map, Shape *shape, 
                                         int *xpos, int *ypos, 
                                         bool accessible, int fromX, int fromY) {

  if(accessible) {
    map->configureAccessMap(fromX, fromY);
  }

  int maxCount = 500; // max # of tries to find accessible location
  int count = 0;
  int x, y;
  while(1) {
    // get a random location
    int n = (int)((float)ffCount * rand()/RAND_MAX);
    x = ff[n * 2];
    y = ff[n * 2 + 1];

    // can it fit?
    bool fits = map->shapeFits(shape, x, y, 0);
    // doesn't fit? try again (could be inf. loop)
    if(fits && 
       !coversDoor(map, scourge->getShapePalette(), shape, x, y)) {

      // check if location is accessible
      if(accessible) {
        if(!map->isPositionAccessible(x, y)) {
          count++;
          if(count >= maxCount) {
            // we failed.
            *xpos = *ypos = MAP_WIDTH;
            return;
          }
          continue;
        }
      }



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
				Shape *shape = scourge->getShapePalette()->getShape(rpgItem->getShapeIndex());
				bool fits = getLocationInRoom(scourge->getMap(), i, shape, &x, &y);
				if(fits && !coversDoor(scourge->getMap(), scourge->getShapePalette(), shape, x, y)) {
					Item *item = scourge->getSession()->newItem(rpgItem);
					addItem(scourge->getMap(), NULL, item, NULL, x, y);
					break;
				}
			}
		}
	}
}

bool DungeonGenerator::addShapeInARoom(int shapeIndex) {
	int x, y;
	for(int tt = 0; tt < 5; tt++) { // 5 room tries
		int i = (int)(roomCount * rand() / RAND_MAX);	
		for(int t = 0; t < 5; t++) { // 5 tries
			Shape *shape = scourge->getShapePalette()->getShape(shapeIndex);
			bool fits = getLocationInRoom(scourge->getMap(), i, shape, &x, &y);
			if(fits && !coversDoor(scourge->getMap(), scourge->getShapePalette(), shape, x, y)) {
				addItem(scourge->getMap(), NULL, NULL, shape, x, y);
				return true;
			}
		}
	}
	return false;
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

// move this to Map class
bool DungeonGenerator::coversDoor(Map *map, ShapePalette *shapePal, 
                                  Shape *shape, int x, int y) {
  for(int ty = y - shape->getDepth() - 3; ty < y + 3; ty++) {
    for(int tx = x - 3; tx < x + shape->getWidth() + 3; tx++) {
      if(map->isDoor(tx, ty)) return true;
    }
  }
  return false;
}

bool DungeonGenerator::isAccessible(Map *map, int x, int y, int fromX, int fromY, int stepsTaken, int dir) {
  //cerr << "&&& isAccessible: x=" << x << " y=" << y << " fromX=" << fromX << " fromY=" << fromY << " dir=" << dir << endl;
  if(x == fromX && y == fromY) {
    cerr << "&&& isAccessible is true in " << stepsTaken << " steps." << endl;
    return true;
  }
  if(stepsTaken > MAX_STEPS) {
    //cerr << "&&& isAccessible is false after " << stepsTaken << " steps." << endl;
    return false;
  }
  int ox = x;
  int oy = y;
  switch(dir) {
  case DIR_N: y--; break;
  case DIR_E: x++; break;
  case DIR_S: y++; break;  
  case DIR_W: x--; break;
  }
  bool failed = false;
  if(!(x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_DEPTH)) {
    Location *pos = map->getLocation(x, y, 0);
    
    //if(pos) cerr << "\tpos: shape=" << pos->shape->getName() <<
    //" item=" << (pos->item ? pos->item->getRpgItem()->getName() : "null") <<
      //" creature=" << (pos->creature ? pos->creature->getName() : "null") << endl;

    // if it's not true that it's: empty space or has a creature or an item (movable things)
    // then change dir.
    if(!(!pos || pos->creature || pos->item)) {
      failed = true;
    }
  } else {
    failed = true;
  }
  if(failed) {
    dir++;
    if(dir >= DIR_COUNT) dir = DIR_N;
    x = ox;
    y = oy;
  }
  return isAccessible(map, x, y, fromX, fromY, stepsTaken + 1, dir);
}

void DungeonGenerator::addItem(Map *map, Creature *creature, Item *item, Shape *shape, int x, int y, int z) {
  if(creature) map->setCreature(x, y, z, creature);
  else if(item) map->setItem(x, y, z, item);
  else map->setPosition(x, y, z, shape);
  // remember the containers
  if(item && item->getRpgItem()->getType() == RpgItem::CONTAINER) {
    containers.push_back(item);
  }  
}

