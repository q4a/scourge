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

/*  
How to enhance it ? (or what will be done soon)

- manage differents display mode of the minimap -> only walls and doors, add creatures...
- handle mouse click on the minimap
- dynamically free memory unused for pos[..][..]
- to the very beginning, the minimap flicker on the top-left corner,
this is because it is drawn a first time here. Solution : draw the first
at the same place than asked by scourge.
- the party is now a black pixel ???
- the minimap sometimes disappear -> due to the center algorithm I think.
- the focus on the minimap sometimes violently teleport itself -> same explanation.
- the zoom sometimes zoom on something else than the party -> same explanation.
- activate the "fog" 
- put color in the shape to avoid tests during construction of the minimap


 */


MiniMap :: MiniMap(Scourge *scourge){
    this->scourge = scourge;
    zoomFactor = 1.2f; // default we see the entire minimap
    effectiveWidth = effectiveHeight = 0;
    maxX = maxY = -1;
    midX = midY = -1.0f;
    //errorMargin = 1.2f;
    errorMargin = 1.0f;
    screenHeight = screenHeight = scourge->getSDLHandler()->getScreen()->h; ;
    showMiniMap = true;
    
    
    if(DEBUG_MINIMAP) fprintf(stderr, "Minimap constructor\n");
      
    if(DEBUG_MINIMAP) fprintf(stderr, "mini map =( %d x %d )\n", MINI_MAP_WIDTH, MINI_MAP_DEPTH);
    for (int x = 0 ; x < MINI_MAP_WIDTH ; x++){
        for(int y = 0; y < MINI_MAP_DEPTH ; y++){
            pos[x][y].r = 0.0;
            pos[x][y].g = 0.0;
            pos[x][y].b = 0.0;
            pos[x][y].visible = true;            
        }
    }   
     
    textureSize = 0;
    textureInMemory = NULL;
    mustBuildTexture = true;
}


MiniMap :: ~MiniMap(){
    if(textureInMemory != NULL){
        free(textureInMemory);
        textureInMemory = NULL;
    }          
}


void MiniMap :: computeDrawValues(){
    int tempMax;
    int res;
    
    
    effectiveWidth = int(maxX *errorMargin);
    effectiveHeight = int(maxY*errorMargin);    
    midX = effectiveWidth/(2*errorMargin);
    midY = effectiveHeight/(2*errorMargin);
    
    if(DEBUG_MINIMAP)
    {         
         fprintf(stderr, "effWidth : %d, effHeight : %d, effW : %d, effH : %d errorMargin : %f\n",
        effectiveWidth, effectiveHeight, int(effectiveWidth/errorMargin), int(effectiveHeight/errorMargin), errorMargin); 
    }
    
    // Compute size of the texture that will hold the minimap
    // Must be a power of 2 and a square.
    if (effectiveHeight > effectiveWidth){
        tempMax = effectiveHeight;
    }
    else{
        tempMax = effectiveWidth;
    }
       
    int b;   
    b = int(ceil(log(double(tempMax))/log(2.0)));         
    textureSize = int(pow(2.0, b));           
    if(DEBUG_MINIMAP){
        fprintf(stderr, " log(double(tempMax)) : %f, log(2.0) : %f, b = %d\n", log(double(tempMax)), log(2.0), b);
        fprintf(stderr, "tempMax = %d, minimap textureSize : %d\n", tempMax, textureSize);
    }
    
    
    
}

// Create and fill the texture for the minimap
void MiniMap :: buildTexture(int xCoord, int yCoord){             
    
    // Create texture and copy minimap date from backbuffer on it    
    textureInMemory = (unsigned char *) malloc(textureSize * textureSize * 4);    
    glGenTextures(1, texture);    
    glBindTexture(GL_TEXTURE_2D, texture[0]); 
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);        
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);                                          // filtre appliqué a la texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);  
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT); 
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, textureInMemory
    );                       
             
    // Draw minimap a first time to screen with quads 
    glPushMatrix();   
    glLoadIdentity(); 
    /*glPushMatrix();   
    glTranslatef(xCoord, yCoord, 100);  */ 
    glBegin(GL_QUADS);      
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
    //glPopMatrix();
 
    // Copy to a texture
    //glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glCopyTexSubImage2D(
				GL_TEXTURE_2D,
				0,      // MIPMAP level
				0,      // x texture offset
				0,      // y texture offset
				0,      // x window coordinates
				600 - textureSize,      // y window coordinates
				128,    // width
				128     // height
			); 
    if(DEBUG_MINIMAP){  
        fprintf(stderr, "Error during minimap texture building : %s\n", Util::getOpenGLError());          
    }
    glPopMatrix();
  
}

void MiniMap :: draw(int xCoord, int yCoord){

    int xPartyPos, yPartyPos;     
    float distX, distY; 
  
    if (!showMiniMap) return;    
  
    if (mustBuildTexture){
    // Create minimap texture
        mustBuildTexture = false;
        buildTexture(xCoord, yCoord);    
    } 
                
    // Compute the postition of the player in the minimap
    xPartyPos = (int) scourge->getPlayer()->getX();
    yPartyPos = (int) scourge->getPlayer()->getY();   	
    toMiniMapCoord(xPartyPos, yPartyPos);   

    //updateFog(xPartyPos, yPartyPos);  
    glPushMatrix();   
    glLoadIdentity(); 
  
    // glScissor(x, y, width, height). (x, y) is the lower-left pixel. And y axis
    // is reversed.   
    glScissor(xCoord, screenHeight - (yCoord + effectiveHeight), effectiveWidth + 5, effectiveHeight + 5); 
    glEnable(GL_SCISSOR_TEST); 
                 
    // Set origin to top-left pixel of minimap
    glTranslatef(xCoord, yCoord, 100);                                                                                                                          
  
    if (zoomFactor > errorMargin){    
        // minimap is too big : center it on player location ..       
        distX = (xPartyPos*zoomFactor-midX);
        if (distX < 0.0f) distX *=-1.0f;
        distY = (yPartyPos*zoomFactor-midY);
        if (distY < 0.0f) distY *=-1.0f;
               
        if(xPartyPos > midX){
            distX *= -1.0f;
        }
        if(yPartyPos > midY){
            distY *= -1.0f;
        }
        
        // smooth center
        float slowDown;
        if( zoomFactor > 2.2)
            slowDown = 1.0f;
        else
            slowDown = ((zoomFactor - 0.4)/2.0f)+0.2f;           
        glTranslatef(distX * slowDown, distY * slowDown, 0);
        
        if(DEBUG_MINIMAP) {
            fprintf(stderr, "zoom : %f    translating : %f, %f   mid (%f, %f),   partyPos (%d, %d)\n", zoomFactor, distX, distY, midX, midY, xPartyPos, yPartyPos); 
        }       
     }
  
    /*else
      glTranslatef(-5.0f, -5.0f, 0.0f);*/
    glScalef(zoomFactor, zoomFactor, 1.0f); 
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBegin(GL_QUADS); 
    glColor3f(1.0f, 1.0f, 1.0f);
    glTexCoord2i(0, 0);
    glVertex2d(0, textureSize); 
    glTexCoord2i(1, 0);    
    glVertex2d(textureSize, textureSize); 
    glTexCoord2i(1, 1);    
    glVertex2d(textureSize, 0); 
    glTexCoord2i(0, 1);    
    glVertex2d(0, 0);     
    glEnd();       
                
           
    // Draw the position of the player  	
    glPointSize(4.0f * zoomFactor);
    glBegin(GL_POINTS);  
    glColor3f(1.0f, 0.0f, 0.0f);   	   	  	   	
    glVertex2d(xPartyPos, yPartyPos);   	       	    
    glEnd (); 
    glPointSize(1.0f);   
  
    glPopMatrix();   
    glDisable(GL_SCISSOR_TEST);        
  
}

void MiniMap :: updateFog(int a, int b){   
    for (int i = -1*MINI_MAP_PARTY_VIEW; i < MINI_MAP_PARTY_VIEW; i++){
        for (int j = -1*MINI_MAP_PARTY_VIEW; j < MINI_MAP_PARTY_VIEW; j++){                       
            if(checkInside(a + i, b + j)){
                pos[a + i][b + j].visible = true;
            }               
        }
    }
}

bool MiniMap::checkInside(int a, int b){
    if((a < 0) || (a > MINI_MAP_WIDTH)||(b < 0)||(b > MINI_MAP_DEPTH)){
        return false;       
    }   
    else{
        return true;
    }       
}


void MiniMap :: zoomIn(){
    this-> zoomFactor += 0.1f;
}

void MiniMap :: zoomOut(){
    this-> zoomFactor -= 0.1f;
}


void MiniMap :: toMiniMapCoord(int &x, int &y){
    x = x / MINI_MAP_X_SCALE;        
    y = y / MINI_MAP_Y_SCALE;
}


void MiniMap :: colorMiniMapPoint(int x, int y, Shape *shape){
    toMiniMapCoord(x, y);
    
    // Update maximums
    if (x > maxX){
        maxX = x;   
    }
    if (y > maxY){
        maxY = y;       
    }
    
    if(DEBUG_MINIMAP) fprintf(stderr, "colorMiniMapPoint : %d, %d : ", x, y);

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
          pos[x][y].r = 0.3f; //braun
          pos[x][y].g = 0.17f;
          pos[x][y].b = 0.05f;
	 } else if ((shape == scourge->getShapePalette()->getShape(Constants::ROOM_FLOOR_TILE_INDEX)))
     {
          if(DEBUG_MINIMAP) fprintf(stderr, "room\n");
          pos[x][y].r = 0.7f; //braun
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
        if(DEBUG_MINIMAP) fprintf(stderr, "Unknown shape\n");     
     }
     
}  

void MiniMap :: eraseMiniMapPoint(int x, int y){
    toMiniMapCoord(x, y);
    pos[x][y].r = pos[x][y].g = pos[x][y].b = 0.0f;   

}
