/***************************************************************************
                          util.cpp  -  description
                             -------------------
    begin                : Sun Jun 22 2003
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

#include "util.h"

Util::Util(){
}
Util::~Util(){
}

void Util::rotate(Sint16 x, Sint16 y, Sint16 *px, Sint16 *py, float angle) {
    // convert to radians
    angle = degreesToRadians(angle);
    // rotate
    float oldx = (float)(x);
    float oldy = (float)(y);
    *px = (Sint16)round((oldx * cos(angle)) - (oldy * sin(angle)));
    *py = (Sint16)round((oldx * sin(angle)) + (oldy * cos(angle)));
}

// A Sample by D.S.Reynolds
//
// How It Works:
//
// The basic formula for the A* algorithm is f = g + h.  g represents the cost of
// distance travelled (or gone).  h represents the estimate (or heuristic) of distance
// from current node to the destination.  Some sort of contianers are needed.  Some
// chose stacks, I chose STL vectors and a heap.  I believe the most critical part
// of the formula is the calculation for the heuristic.  Also, Breadth-First and Depth-First
// algorithms are simplified subsets of the A*.
//
//  1.  Create OPEN, CLOSED and PATH containers.
//  2.  Start with the OPEN container containing only the starting node.
//      Set that nodes g value to 0, it's h value to the euclidian difference
//      ((dx - sx)*(dx - sx)) + ((dy - sy)*(dy - sy)), calc the f value and
//      set the parent node to NULL.
//  3.  Until the goal node is found, repeat the following:
//        If no nodes found exit with failure.
//        Select Node on OPEN with the lowest f value.  This is the BestNode.
//        Push BestNode to Closed (This is the only place CLOSED is populated).
//        If BestNode is the destination, exit.  Otherwise, determine the
//        child nodes (adjacents) of BestNode.
//           For each child node do the following:
//           Set each child nodes Parent to BestNode (We'll use this later to
//             get the path.)
//           Calc the cost of g:  Child.g = BestNode.g + 1  (Use other than 1
//             for various terrain)
//           See if child node is already on OPEN.  If so, determine which has
//             the lower g value and use it's other corresponding values.
//           See if child node is already on CLOSED.  If so, determine which
//             has the lower g value and use it's other corresponding values.
//           If the child node was NOT on OPEN or CLOSED, add it to OPEN.
//
///////////////////////////////////////////////////////////
void Util::findPath(Sint16 sx, Sint16 sy, Sint16 sz,
                    Sint16 dx, Sint16 dy, Sint16 dz,
                    vector<Location> *pVector,
                    Map *map,
                    Shape *shape) {
  vector<CPathNode> OPEN;                 // STL Vectors chosen because of rapid
  vector<CPathNode> CLOSED;               // insertions/deletions at back,
  vector<CPathNode> PATH;                 // and Direct access to any element.
  CPathNode Node, BestNode;               // Temporary Node and BestNode
  bool bNodeFound = false;                // Flag if node is found in container


  Node.x = sx;                            // Create the start node
  Node.y = sy;
  Node.gone = 0;
  Node.heuristic = ((dx - sx)*(dx - sx)) + ((dy - sy)*(dy - sy));
  Node.f = Node.gone + Node.heuristic;    // The classic formula f = g + h
  Node.px = 0;                            // No parent for start location
  Node.py = 0;                            // No parent for start location
  OPEN.push_back(Node);                   // Populate the OPEN container with the first location
  make_heap( OPEN.begin(), OPEN.end() );  // Create a heap from OPEN for sorting


  while (!OPEN.empty()) {
    sort_heap(OPEN.begin(), OPEN.end());// Ascending sort based on overloaded operators below
    BestNode = OPEN.front();            // Set the Node with lowest f value to BESTNODE
    pop_heap(OPEN.begin(), OPEN.end()); // Pop off the heap.  Actually this moves the
                                        // far left value to the far right.  The node
                                        // is not actually removed since the pop is to
                                        // the heap and not the container.
    OPEN.pop_back();                    // Remove node from right (the value we pop_heap'd)
    CLOSED.push_back(BestNode);         // Push the BestNode onto CLOSED

    // If at destination, break and create path below
    if ((BestNode.x == dx) && (BestNode.y == dy)) {
      //bPathFound = true; // arrived at destination...
      break;
    }

    // Set limit to break if looking too long
    if ( CLOSED.size() > MAX_CLOSED_NODES )
      break;

    // Check adjacent locations (This is done in a clockwise order to lessen jaggies)
    for (int i=1; i<9; i++) {
      switch(i) {
      case 1:
        Node.x = BestNode.x;
        Node.y = BestNode.y - 1;
        break;
      case 2:
        Node.x = BestNode.x + 1;
        Node.y = BestNode.y - 1;
        break;
      case 3:
        Node.x = BestNode.x + 1;
        Node.y = BestNode.y;
        break;
      case 4:
        Node.x = BestNode.x + 1;
        Node.y = BestNode.y + 1;
        break;
      case 5:
        Node.x = BestNode.x;
        Node.y = BestNode.y + 1;
        break;
      case 6:
        Node.x = BestNode.x - 1;
        Node.y = BestNode.y + 1;
        break;
      case 7:
        Node.x = BestNode.x - 1;
        Node.y = BestNode.y;
        break;
      case 8:
        Node.x = BestNode.x - 1;
        Node.y = BestNode.y - 1;
        break;
      }

      if ((Node.x >= 0) && (Node.x < MAP_WIDTH) &&
          (Node.y >= 0) && (Node.y < MAP_DEPTH)) {
        // Determine cost of distance travelled
        if(map->isBlocked(Node.x, Node.y, sz,
                          sx, sy, sz, shape)) // If location is obstruction
          Node.gone = 1000;
        else
          Node.gone = BestNode.gone + 1;

        // Determine the Heuristic.  Probably the most crucial aspect
        // Heuristic by Simple Euclidian method
        // Node.heuristic = ((dx - Node.x)*(dx - Node.x)) + ((dy - Node.y)*(dy - Node.y));
        // Heuristic by my own Orthogonal/Diagonal + Euclidian modifier
        int DX = abs(dx - Node.x);
        int DY = abs(dy - Node.y);
        int Orthogonal = abs(DX - DY);
        int Diagonal = abs(((DX + DY) - Orthogonal)/2);
        Node.heuristic = Diagonal + Orthogonal + DX + DY;

        // The A* formula
        Node.f = Node.gone + Node.heuristic;
        Node.px = BestNode.x; // Point parent to last BestNode (pushed onto CLOSED)
        Node.py = BestNode.y; // Point parent to last BestNode (pushed onto CLOSED)


        bNodeFound = false;

        // Check to see if already on OPEN
        for (int t=0; t<(int)OPEN.size(); t++) {
          if ((Node.x == OPEN.at(t).x) &&
              (Node.y == OPEN.at(t).y)) {   // If already on OPEN
            if (Node.gone < OPEN.at(t).gone) {
              OPEN.at(t).gone = Node.gone;
              OPEN.at(t).f = Node.gone + OPEN.at(t).heuristic;
              OPEN.at(t).px = Node.px;
              OPEN.at(t).py = Node.py;
            }
            bNodeFound = true;
            break;
          }
        }
        if (!bNodeFound ) { // If Node NOT found on OPEN
          // Check to see if already on CLOSED
          for (int t=0; t<(int)CLOSED.size(); t++) {
            if ((Node.x == CLOSED.at(t).x) &&
                (Node.y == CLOSED.at(t).y)) {   // If on CLOSED, Which has lower gone?
              if (Node.gone < CLOSED.at(t).gone) {
                CLOSED.at(t).gone = Node.gone;
                CLOSED.at(t).f = Node.gone + CLOSED.at(t).heuristic;
                CLOSED.at(t).px = Node.px;
                CLOSED.at(t).py = Node.py;
              }
              bNodeFound = true;
              break;
            }
          }
        }
        if (!bNodeFound ) { // If Node NOT found on OPEN or CLOSED
          OPEN.push_back(Node);                  // Push NewNode onto OPEN
          push_heap( OPEN.begin(), OPEN.end() ); // Push NewNode onto heap
          make_heap( OPEN.begin(), OPEN.end() ); // Re-Assert heap, or will be short by one

                   /*
                   // Display OPEN and CLOSED containers (For Debugging)
                   int i;
                   cout << "OPEN:   ";
                   for (i=0; i<OPEN.size(); i++)
                   {
                       cout << OPEN.at(i).x << "," << OPEN.at(i).y << ",";
                       cout << OPEN.at(i).gone << "," << OPEN.at(i).heuristic << "  ";
                   }
                   cout << endl;
                   cout << "CLOSED:   ";
                   for (i=0; i<CLOSED.size(); i++)
                   {
                       cout << CLOSED.at(i).x << "," << CLOSED.at(i).y << ",";
                       cout << CLOSED.at(i).gone << "," << CLOSED.at(i).heuristic << "  ";
                   }
                   cout << endl << endl;
                   int ch = _getch();
                   //*/
        }
      }
    }
  }

  if (CLOSED.size() > 0) {
    // Create the path from elements of the CLOSED container
    PATH.clear();
    PATH.push_back(CLOSED.back());
    CLOSED.pop_back();

    while (!CLOSED.empty()) {
      if ((CLOSED.back().x == PATH.back().px) &&
          (CLOSED.back().y == PATH.back().py))
        PATH.push_back(CLOSED.back());

      CLOSED.pop_back();
    }

    // Populate the vector that was passed in by reference
    Location Fix;
    pVector->clear();
    for (int i=(PATH.size()-1); i>=0; i--) {
      //for (container::iterator i=PATH.begin(); i!= PATH.end(); ++i)
      Fix.x = PATH.at(i).x;
      Fix.y = PATH.at(i).y;
      Fix.z = 0;
      pVector->push_back(Fix);
    }
  }
}


///////////////////////////////////////////////////////////
// Needed because the template doesn't know what a PathNode is
bool operator<(const CPathNode &a, const CPathNode &b) {
  return a.f < b.f;
}


///////////////////////////////////////////////////////////
// Needed because the template doesn't know what a PathNode is
bool operator>(const CPathNode &a, const CPathNode &b) {
  return a.f > b.f;
}

float Util::dot_product(float v1[3], float v2[3]) {
  return (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]);
}

void Util::normalize(float v[3]) {
  float f = 1.0f / sqrt(dot_product(v, v));
  
  v[0] *= f;
  v[1] *= f;
  v[2] *= f;
}

void Util::cross_product(const float *v1, const float *v2, float *out) {
  out[0] = v1[1] * v2[2] - v1[2] * v2[1];
  out[1] = v1[2] * v2[0] - v1[0] * v2[2];
  out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void Util::multiply_vector_by_matrix(const float m[9], float v[3]) {
  float tmp[3];
  
  tmp[0] = v[0] * m[0] + v[1] * m[3] + v[2] * m[6];
  tmp[1] = v[0] * m[1] + v[1] * m[4] + v[2] * m[7];
  tmp[2] = v[0] * m[2] + v[1] * m[5] + v[2] * m[8];
  
  v[0] = tmp[0];
  v[1] = tmp[1];
  v[2] = tmp[2];
}
