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
- activate the "fog" 
- put color in the shape to avoid tests during construction of the minimap


 */

#define DEBUG_MINIMAP 0

MiniMap :: MiniMap(Scourge *scourge){
    this->scourge = scourge;
    zoomFactor = 1.2f; // default we see the entire minimap
    effectiveWidth = effectiveHeight = 0;
    maxX = maxY = -1;
    minX = minY = 3000;
    midX = midY = -1.0f;
    screenHeight = screenHeight = scourge->getSDLHandler()->getScreen()->h; ;
    showMiniMap = true;            

    win = new Window( scourge->getSDLHandler(),
                      0, 400, MINIMAP_WINDOW_WIDTH, MINIMAP_WINDOW_HEIGHT, 
                      strdup("Minimap"), 
                      scourge->getShapePalette()->getGuiTexture(), 
                      false );
    canvas = new Canvas( 0, 0, MINIMAP_WINDOW_WIDTH, MINIMAP_WINDOW_HEIGHT - 25, this );
    win->addWidget(canvas);

    if(DEBUG_MINIMAP) fprintf(stderr, "mini map =( %d x %d )\n", MINI_MAP_WIDTH, MINI_MAP_DEPTH);
    for (int x = 0 ; x < MINI_MAP_WIDTH ; x++){
        for(int y = 0; y < MINI_MAP_DEPTH ; y++){
            pos[x][y].r = 0.0;
            pos[x][y].g = 0.0;
            pos[x][y].b = 0.0;
            pos[x][y].visible = true;            
        }
    }   
     
    textureSizeH = textureSizeW = 0;
    textureInMemory = NULL;
    mustBuildTexture = true;
}


MiniMap :: ~MiniMap(){
  reset();
}

void MiniMap::reset() {
  if(textureInMemory != NULL){
    free(textureInMemory);
    textureInMemory = NULL;
    // delete the overlay texture
    glDeleteTextures(1, texture);
  }          

  zoomFactor = 1.2f; // default we see the entire minimap
  effectiveWidth = effectiveHeight = 0;
  maxX = maxY = -1;
  minX = minY = 3000;
  midX = midY = -1.0f;
  screenHeight = screenHeight = scourge->getSDLHandler()->getScreen()->h; ;
  showMiniMap = true;            

  if(DEBUG_MINIMAP) fprintf(stderr, "mini map =( %d x %d )\n", MINI_MAP_WIDTH, MINI_MAP_DEPTH);
  for (int x = 0 ; x < MINI_MAP_WIDTH ; x++){
    for(int y = 0; y < MINI_MAP_DEPTH ; y++){
      pos[x][y].r = 0.0;
      pos[x][y].g = 0.0;
      pos[x][y].b = 0.0;
      pos[x][y].visible = true;            
    }
  }   
  
  textureSizeH = textureSizeW = 0;
  textureInMemory = NULL;
  mustBuildTexture = true;
}

void MiniMap :: computeDrawValues(){        
    effectiveWidth = int(maxX - minX);
    effectiveHeight = int(maxY - minY); 
    midX = (effectiveWidth + minX)/2;
    midY = (effectiveHeight + minY)/2;
    if(minX >= 2) minX -= 2;
    if(minY >= 2) minY -= 2;
    
    if(DEBUG_MINIMAP)
    {         
         fprintf(stderr, "effWidth : %d, effHeight : %d\n", effectiveWidth, effectiveHeight); 
    }
    
    // Compute dimensions of the texture that will hold the minimap
    // Each dimension must be a power of 2       
    int b;   
    b = int(ceil(log(double(effectiveHeight))/log(2.0)));         
    textureSizeH = int(pow(2.0, b));           
    b = int(ceil(log(double(effectiveWidth))/log(2.0)));         
    textureSizeW = int(pow(2.0, b));           
    if(DEBUG_MINIMAP){        
        fprintf(stderr, "textSzW = %d, textSzH : %d\n", textureSizeW, textureSizeH);
    }            
}

// Create and fill the texture for the minimap
void MiniMap :: buildTexture(int xCoord, int yCoord){             

  if(!mustBuildTexture) return;
  mustBuildTexture = false;

  computeDrawValues();

  //glPushAttrib(GL_ENABLE_BIT);
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_DEPTH_TEST );
  glDisable( GL_SCISSOR_TEST );
  glDisable( GL_CULL_FACE );
  glDisable( GL_BLEND );

  // Create texture and copy minimap date from backbuffer on it    
  textureInMemory = (unsigned char *) malloc(textureSizeH * textureSizeW * 4);    

  Constants::checkTexture("MiniMap::buildTexture", 
                          textureSizeW, textureSizeH);

  glGenTextures(1, texture);    
  glBindTexture(GL_TEXTURE_2D, texture[0]); 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);        
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);                                          // filtre appliqué a la texture
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);  
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP ); 
  glTexImage2D(
              GL_TEXTURE_2D, 0, GL_RGBA, textureSizeW, textureSizeH, 0,
              GL_RGBA, GL_UNSIGNED_BYTE, textureInMemory
              );                       

  // Draw minimap a first time to screen with quads     
  glPushMatrix();   
  glLoadIdentity();    
  glTranslatef(xCoord, yCoord, 0);  
  glBegin(GL_QUADS);      

//  glColor4f( 1, 1, 1, 1 );
//  glVertex2d( 10, 10 );
//  glVertex2d( 200, 10 );
//  glVertex2d( 200, 200 );
//  glVertex2d( 10, 200 );

  for(int x = minX ; x < MINI_MAP_WIDTH; x++){
    for(int y = minY; y < MINI_MAP_DEPTH; y++){
      if((pos[x][y].visible == true) && (pos[x][y].r != 0.0f)){
        glColor4f(pos[x][y].r, pos[x][y].g, pos[x][y].b, 1);
      } else {
        glColor4f(0, 0, 0, 0);
      }
      glVertex2d (x - 1.0f , y - 1.0f);
      glVertex2d (x - 1.0f , y);
      glVertex2d (x , y);
      glVertex2d (x , y - 1.0f);                
    }       
  }    
  glEnd();      

  cerr << "building minimap texture: minX=" << minX << " minY=" << minY << 
        " xCoord=" << xCoord << " yCoord=" << yCoord << " textureSizeW=" << textureSizeW <<
        " textureSizeH=" << textureSizeH << " screenHeight=" << screenHeight << endl;

  // Copy to a texture
  glLoadIdentity();
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glCopyTexSubImage2D(
                     GL_TEXTURE_2D,
                     0,      // MIPMAP level
                     0,      // x texture offset
                     0,      // y texture offset
                     xCoord + minX,                                  // x window coordinates
                     screenHeight - (yCoord +textureSizeH+ minY) ,   // y window coordinates
                     textureSizeW,    // width
                     textureSizeH     // height
                     ); 
  if(DEBUG_MINIMAP){  
    fprintf(stderr, "OpenGl result for minimap texture building : %s\n", Util::getOpenGLError());          
  }
  glPopMatrix();
  //  glPopAttrib();

  glEnable( GL_TEXTURE_2D );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_CULL_FACE );

}

void MiniMap::drawWidget(Widget *w) {
  //	glDisable(GL_CULL_FACE);
  int xPartyPos, yPartyPos;     
  float distX, distY; 
  int k;
  int xCoord=0;
  int yCoord=0;

  if(!showMiniMap) return;

  // Compute the postition of the player in the minimap
  xPartyPos = (int) scourge->getParty()->getPlayer()->getX();
  yPartyPos = (int) scourge->getParty()->getPlayer()->getY();     
  toMiniMapCoord(xPartyPos, yPartyPos);
  xPartyPos -= minX;   
  yPartyPos -= minY; 

  //updateFog(xPartyPos, yPartyPos);  
  //glPushAttrib(GL_ENABLE_BIT);

  win->scissorToWindow();

  glPushMatrix();   
  glLoadIdentity();
  glTranslatef(win->getX(), win->getY(), 0);

  // Set origin to top-left pixel of minimap
  glTranslatef(xCoord, yCoord, 100);                                                                                                                          

  // Translate minimap so that it is centered on party position if needed
  if(zoomFactor > 1.0){                 
    distX = (xPartyPos*zoomFactor-midX);
    if(distX < 0.0f) distX *=-1.0f;
    distY = (yPartyPos*zoomFactor-midY);
    if(distY < 0.0f) distY *=-1.0f;

    if(xPartyPos * zoomFactor > midX){
      distX *= -1.0f;
    }
    if(yPartyPos * zoomFactor > midY){
      distY *= -1.0f;
    }

    // smooth centering
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


  glScalef(zoomFactor, zoomFactor, 1.0f); 

  // Draw the minimap using a texture
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc( GL_NOTEQUAL, 0 );
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glBegin(GL_QUADS); 
  glColor3f(1.0f, 1.0f, 1.0f);
  glTexCoord2i(0, 0);
  glVertex2d(0, textureSizeH); 
  glTexCoord2i(1, 0);    
  glVertex2d(textureSizeW, textureSizeH); 
  glTexCoord2i(1, 1);    
  glVertex2d(textureSizeW, 0); 
  glTexCoord2i(0, 1);    
  glVertex2d(0, 0);     
  glEnd();       
  glDisable(GL_TEXTURE_2D);                           

  // Draw the position of the active player and its orientation
  // (Must draw counter-clock wise)
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 1.0f, 1.0f);        
  k = scourge->getParty()->getPlayer()->getFacingDirection();    
  if(k == Constants::MOVE_UP){
    // North
    glVertex2d(xPartyPos - 4, yPartyPos + 5); 
    glVertex2d(xPartyPos + 4, yPartyPos + 5); 
    glVertex2d(xPartyPos, yPartyPos - 5); 
  } else if(k == Constants::MOVE_RIGHT){
    // East
    glVertex2d(xPartyPos + 5, yPartyPos); 
    glVertex2d(xPartyPos - 5, yPartyPos - 4); 
    glVertex2d(xPartyPos - 5, yPartyPos + 4);         
  } else if(k == Constants::MOVE_LEFT){
    // West
    glVertex2d(xPartyPos - 5, yPartyPos); 
    glVertex2d(xPartyPos + 5, yPartyPos + 4); 
    glVertex2d(xPartyPos + 5, yPartyPos - 4);         
  } else if(k == Constants::MOVE_DOWN){
    // South
    glVertex2d(xPartyPos + 4, yPartyPos - 5); 
    glVertex2d(xPartyPos - 4, yPartyPos - 5); 
    glVertex2d(xPartyPos, yPartyPos + 5); 
  }
  glEnd();       


  glPopMatrix();   
  //  glPopAttrib();

  glEnable( GL_TEXTURE_2D );
  glDisable(GL_ALPHA_TEST);
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


void MiniMap::colorMiniMapPoint(int x, int y, Shape *shape, Location *location){
  int mapx = x;
  int mapy = y;
  toMiniMapCoord(x, y);  

  // Update extremums
  if (x > maxX) {
    maxX = x;   
  } else {
    if (x < minX) {
      minX = x;
    }
  }
  if (y > maxY) {
    maxY = y;       
  } else {
    if (y < minY) {
      minY = y;
    }
  }

  if ((shape == scourge->getShapePalette()->findShapeByName("EW_WALL")) ||
      (shape == scourge->getShapePalette()->findShapeByName("EW_WALL_EXTRA")) ||
      (shape == scourge->getShapePalette()->findShapeByName("EW_WALL_TWO_EXTRAS"))||
      (shape == scourge->getShapePalette()->findShapeByName("NS_WALL"))||
      (shape == scourge->getShapePalette()->findShapeByName("NS_WALL_EXTRA"))||
      (shape == scourge->getShapePalette()->findShapeByName("NS_WALL_TWO_EXTRAS"))) {
    if (DEBUG_MINIMAP) fprintf(stderr, "wall\n");
    pos[x][y].r = 0.5f; //gray
    pos[x][y].g = 0.5f;
    pos[x][y].b = 0.5f;
  } else if((shape == scourge->getShapePalette()->findShapeByName("CORNER"))) {
    if (DEBUG_MINIMAP) fprintf(stderr, "corner\n");
    pos[x][y].r = 0.3f; // dark gray
    pos[x][y].g = 0.3f;
    pos[x][y].b = 0.3f;
  } else if((shape == scourge->getShapePalette()->findShapeByName("DOOR_SIDE")) ||
            (shape == scourge->getShapePalette()->findShapeByName("EW_DOOR")) ||
            (shape == scourge->getShapePalette()->findShapeByName("EW_DOOR_TOP"))||
            (shape == scourge->getShapePalette()->findShapeByName("NS_DOOR"))||
            (shape == scourge->getShapePalette()->findShapeByName("NS_DOOR_TOP"))) {
    if (DEBUG_MINIMAP) fprintf(stderr, "door\n");
    Location *p = scourge->getMap()->getLocation(mapx, mapy, 0);
    if (p && scourge->getMap()->isLocked(p->x, p->y, p->z)) {
      pos[x][y].r = 0.8f;
      pos[x][y].g = 0.1f;
      pos[x][y].b = 0.1f;
    } else {
      pos[x][y].r = 0.8f;
      pos[x][y].g = 0.8f;
      pos[x][y].b = 0.8f;
    }
  } else if((shape == scourge->getShapePalette()->findShapeByName("FLOOR_TILE"))) {
    if (DEBUG_MINIMAP) fprintf(stderr, "floor\n");
    pos[x][y].r = 0.3f; //braun
    pos[x][y].g = 0.17f;
    pos[x][y].b = 0.05f;
  } else if ((shape == scourge->getShapePalette()->findShapeByName("ROOM_FLOOR_TILE"))) {
    if (DEBUG_MINIMAP) fprintf(stderr, "room\n");
    pos[x][y].r = 0.7f; //braun
    pos[x][y].g = 0.5f;
    pos[x][y].b = 0.1f;
  } else if((shape == scourge->getShapePalette()->findShapeByName("LAMP_NORTH")) ||
            (shape == scourge->getShapePalette()->findShapeByName("LAMP_SOUTH"))||
            (shape == scourge->getShapePalette()->findShapeByName("LAMP_WEST"))||
            (shape == scourge->getShapePalette()->findShapeByName("LAMP_EAST"))||
            (shape == scourge->getShapePalette()->findShapeByName("LAMP_BASE"))) {
    if (DEBUG_MINIMAP) fprintf(stderr, "lamp\n");
    pos[x][y].r = 0.8f; //yellow
    pos[x][y].g = 0.8f;
    pos[x][y].b = 0.1f;
  } else {
    if (DEBUG_MINIMAP) fprintf(stderr, "Unknown shape\n");
  }

  mustBuildTexture = true;
}  

void MiniMap :: eraseMiniMapPoint(int x, int y){
  toMiniMapCoord(x, y);
  pos[x][y].r = pos[x][y].g = pos[x][y].b = 0.0f;   
  mustBuildTexture = true;
}
