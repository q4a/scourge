/***************************************************************************
                          map.h  -  description
                             -------------------
    begin                : Thu Jan 29 2004
    copyright            : (C) 2004 by Daroth-U
    email                : daroth-u@ifrance.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "minimap.h"

/*  This is the first version of the minimap : 
How to enhance it ?
- very slow draw function : a further version will use a texture (much faster)
and a texture mask (certainly possible with openGL) to hide the part of the map 
not discovered by the player yet.
- manage differents display mode of the minimap -> only walls and doors, add creatures...
- handle mouse click on the minimap
-nicer integration in the gui...
    
 */


MiniMap :: MiniMap(Scourge *scourge){
    this->scourge = scourge;
    zoomFactor = 1.0f; // default we see the entire minimap
    
    if(DEBUG_MINIMAP) fprintf(stderr, "Minimap constructor\n");
      
    if(DEBUG_MINIMAP) fprintf(stderr, "mini map =( %d x %d )\n", MINI_MAP_WIDTH, MINI_MAP_DEPTH);
    for (int x = 0 ; x < MINI_MAP_WIDTH ; x++){
        for(int y = 0; y < MINI_MAP_DEPTH ; y++){
            pos[x][y].r = 0.0;
            pos[x][y].g = 0.0;
            pos[x][y].b = 0.0;
            pos[x][y].visible = true; // true for now, to debug
        }
    } 
    
}


MiniMap :: ~MiniMap(){   
}

void MiniMap :: draw(int xCoord, int yCoord){
  int xPartyPos, yPartyPos;        
  
  glDisable(GL_DEPTH_TEST);
  glDisable( GL_TEXTURE_2D );        
  glPushMatrix();   
  glLoadIdentity();            
  glTranslatef(xCoord, yCoord, 100);   
   
  glPointSize(4.0f);
  glBegin(GL_POINTS);  
  glColor3f(1.0f, 1.0f, 0.0f);   	   	  	   	
  glVertex2d(0.0f, 0.f);   	       	    
  glEnd ();
  glPointSize(1.0f);   
      
  glScalef(zoomFactor, zoomFactor, 1.0f);

  // Draw the map 
  glBegin(GL_QUADS);
  //glBegin(GL_POINTS);
  
  for (int x = 0 ; x < MINI_MAP_WIDTH; x++){
	for(int y = 0; y < MINI_MAP_DEPTH; y++){
	  if((pos[x][y].visible == true) && (pos[x][y].r != 0.0f)){
		glColor3f(pos[x][y].r, pos[x][y].g, pos[x][y].b);
        glVertex2d (x - 1.0f , y - 1.0f);
    	glVertex2d (x - 1.0f , y);
        glVertex2d (x , y);
		glVertex2d (x , y - 1.0f);		
	  }
	}       
  }  
  glEnd(); 
    	

  // Draw the position of the player  	
  xPartyPos = (int) scourge->getPlayer()->getX();
  yPartyPos = (int) scourge->getPlayer()->getY();   	
  toMiniMapCoord(xPartyPos, yPartyPos);
  glPointSize(4.0f);
  glBegin(GL_POINTS);  
  glColor3f(1.0f, 0.0f, 0.0f);   	   	  	   	
  glVertex2d(xPartyPos, yPartyPos);   	       	    
  glEnd ();
  
  glPointSize(1.0f);   
  glPopMatrix();
  
  glEnable(GL_DEPTH_TEST);
  glEnable (GL_TEXTURE_2D);
}

void MiniMap :: zoomIn(GLfloat zoom){
    this-> zoomFactor += 0.2f;
}

void MiniMap :: zoomOut(GLfloat zoom){
    this-> zoomFactor -= 0.2f;
}

void MiniMap :: toMiniMapCoord(int &x, int &y){
    x = x / MINI_MAP_X_SCALE;        
    y = y / MINI_MAP_Y_SCALE;
}


void MiniMap :: colorMiniMapPoint(int x, int y, Shape *shape){
    toMiniMapCoord(x, y);
    if(DEBUG_MINIMAP) fprintf(stderr, "setMiniMapPoint2 apres : %d, %d : ", x, y);

    if ((shape == scourge->getShapePalette()->getShape(Constants::EW_WALL_INDEX)) ||
        (shape == scourge->getShapePalette()->getShape(Constants::EW_WALL_EXTRA_INDEX)) ||
        (shape == scourge->getShapePalette()->getShape(Constants::EW_WALL_TWO_EXTRAS_INDEX))||
        (shape == scourge->getShapePalette()->getShape(Constants::NS_WALL_INDEX))||
        (shape == scourge->getShapePalette()->getShape(Constants::NS_WALL_EXTRA_INDEX))||
        (shape == scourge->getShapePalette()->getShape(Constants::NS_WALL_TWO_EXTRAS_INDEX)))
    {        
         if(DEBUG_MINIMAP) fprintf(stderr, "wall\n");
         pos[x][y].r = 0.5f; //gray
         pos[x][y].g = 0.5f;
         pos[x][y].b = 0.5f;
     }
     else if
     ((shape == scourge->getShapePalette()->getShape(Constants::CORNER_INDEX)))     
     {
     	 if(DEBUG_MINIMAP) fprintf(stderr, "corner\n");
	     pos[x][y].r = 0.3f; // dark gray
         pos[x][y].g = 0.3f;
         pos[x][y].b = 0.3f;
     }      
     else if
     ((shape == scourge->getShapePalette()->getShape(Constants::DOOR_SIDE_INDEX)) ||
     (shape == scourge->getShapePalette()->getShape(Constants::EW_DOOR_INDEX)) ||
     (shape == scourge->getShapePalette()->getShape(Constants::EW_DOOR_TOP_INDEX))||
     (shape == scourge->getShapePalette()->getShape(Constants::NS_DOOR_INDEX))||
     (shape == scourge->getShapePalette()->getShape(Constants::NS_DOOR_TOP_INDEX)))
     {    
          if(DEBUG_MINIMAP) fprintf(stderr, "door\n");
          pos[x][y].r = 0.8f;
          pos[x][y].g = 0.8f;
          pos[x][y].b = 0.8f;          
     }
     else if
     ((shape == scourge->getShapePalette()->getShape(Constants::FLOOR_TILE_INDEX))) 
	 {
          if(DEBUG_MINIMAP) fprintf(stderr, "floor\n");
          pos[x][y].r = 0.3f; //marron
          pos[x][y].g = 0.17f;
          pos[x][y].b = 0.05f;
	 } else if ((shape == scourge->getShapePalette()->getShape(Constants::ROOM_FLOOR_TILE_INDEX)))
     {
          if(DEBUG_MINIMAP) fprintf(stderr, "room\n");
          pos[x][y].r = 0.7f; //marron
          pos[x][y].g = 0.5f;
          pos[x][y].b = 0.1f;
     }
     else if
     ((shape == scourge->getShapePalette()->getShape(Constants::LAMP_NORTH_INDEX)) ||
     (shape == scourge->getShapePalette()->getShape(Constants::LAMP_SOUTH_INDEX))||
     (shape == scourge->getShapePalette()->getShape(Constants::LAMP_WEST_INDEX))||
     (shape == scourge->getShapePalette()->getShape(Constants::LAMP_EAST_INDEX))||
     (shape == scourge->getShapePalette()->getShape(Constants::LAMP_BASE_INDEX)))
     {
        if(DEBUG_MINIMAP) fprintf(stderr, "lamp\n");
            pos[x][y].r = 0.8f; //yellow
            pos[x][y].g = 0.8f;
            pos[x][y].b = 0.1f;
     }
     else
     {
        if(DEBUG_MINIMAP) fprintf(stderr, "unknown shape\n");     
     }
     
}  

void MiniMap :: eraseMiniMapPoint(int x, int y){
    toMiniMapCoord(x, y);
    pos[x][y].r = pos[x][y].g = pos[x][y].b = 0.0f;   

}
