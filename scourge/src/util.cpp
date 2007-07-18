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
#include "render/renderlib.h"
#include "debug.h"

using namespace std;

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
    *px = (Sint16)rint((oldx * cos(angle)) - (oldy * sin(angle)));
    *py = (Sint16)rint((oldx * sin(angle)) + (oldy * cos(angle)));
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

void Util::multiply_vector_by_matrix2(const float m[16], float v[4]) {
  float tmp[4];
  
  tmp[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
  tmp[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
  tmp[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
  tmp[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];

  
  v[0] = tmp[0];
  v[1] = tmp[1];
  v[2] = tmp[2];
  v[3] = tmp[3];
}


// Return a string containing the last OpenGL error.
// Useful to debug strange OpenGL behaviors
char * Util :: getOpenGLError(){
    int error;
    error = glGetError();
    
    // All openGl errors possible
    switch(error){
        case GL_NO_ERROR : return "GL_NO_ERROR";break;
        case GL_INVALID_ENUM : return "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE : return "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION : return "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW : return "GL_STACK_OVERFLOW"; break;
        case GL_OUT_OF_MEMORY : return "GL_OUT_OF_MEMORY"; break;
        default : return "Unknown error"; break;
    }
}

// Returns next word from the given position. If there is not a space at the given
// position, the function suppose it is the first letter of the word wanted. 
string Util::getNextWord(const string theInput, int fromPos, int &endWord){
    int firstChar, lastStringChar;
    string sub;
	if(sub.size()) sub.erase(sub.begin(), sub.end());
    
    if (theInput.empty() || fromPos==-1) {return sub;}

    lastStringChar = theInput.find_last_not_of(' ');    
    if(theInput[fromPos] == ' '){
        firstChar = theInput.find_first_not_of(' ', fromPos);
    }
    else{
        firstChar = fromPos;
    }    
    endWord = theInput.find_first_of(' ', firstChar);
   // cout << "line :" << theInput << endl;
   // cout << "\t\tpos = " << fromPos << " firstChar = " << firstChar << " endWord = " << endWord << " lastStringChar =" << lastStringChar << endl; 
    if(endWord == -1){
        if( (lastStringChar >= firstChar)&&(firstChar!=-1)){
            sub = theInput.substr(firstChar, lastStringChar - firstChar + 1);
        }                    
    } 
    else{        
        sub = theInput.substr(firstChar, endWord - firstChar);
    }
    return sub;

}

float Util::getAngle(float fx, float fy, float fw, float fd,
					 float tx, float ty, float tw, float td) {
  // figure out targetCreatureAngle
  float sx = fx + (fw / 2);
  float sy = fy - (fd / 2);
  float ex = tx + (tw / 2);
  float ey = ty - (td / 2);

  float x = ex - sx;
  float y = ey - sy;
  if( x == 0.0f ) x = 0.001f;
  float angle = Constants::toAngle(atan(y / x));

  // read about the arctan problem: 
  // http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
  //  q = 1;
  if(x < 0) { 		// Quadrant 2 & 3
	//	q = ( y >= 0 ? 2 : 3);
	angle += 180;
  } else if(y < 0) { // Quadrant 4
	//	q = 4;
	angle += 360;
  }

  // normalize
  if( angle < 0.0f ) angle = 360.0f + angle;
  if( angle >= 360.0f ) angle -= 360.0f;

  return angle;
}

#define FOV_ANGLE 60

/**
 * Is px,py in the field of vision defined by x,y,angle?
 */
bool Util::isInFOV( float x, float y, float angle, float px, float py ) {
	float angleToP = getAngle( x, y, 1, 1, 
														 px, py, 1, 1 );
	//cerr << "fov: angle=" << angle << " toP=" << angleToP << endl;
	float diff = fabs( diffAngle( angle, angleToP ) );
	bool b = ( diff < FOV_ANGLE );
	//cerr << "\tb=" << b << " diff=" << diff << endl;
	return b;
}

float Util::diffAngle(float a, float b) {
//  a -= (((int)a / 360) * 360);
//  b -= (((int)b / 360) * 360);
  float diff = a - b;
  if( diff > 180.0f ) {
    diff = -(360.0f - diff);
  } else if( diff < -180.0f ) {
    diff = 360 + diff;
  }
  return diff;
}

void Util::drawBar( int x, int y, float barLength, float value, float maxValue,
                    float red, float green, float blue, float gradient, GuiTheme *theme,
                    int layout ) {
  float percent = (maxValue == 0 ? 0 : (value >= maxValue ? 100.0f : value / (maxValue / 100.0f)));
  float length = barLength * (percent / 100.0f);
  if(length < 0) {
    length = percent = 0;
  }

  glPushMatrix();
  glTranslatef( x, y, 0 );

  /*
  if( theme && theme->getInputBackground() ) {
    glColor4f( theme->getInputBackground()->color.r,
               theme->getInputBackground()->color.g,
               theme->getInputBackground()->color.b,
               theme->getInputBackground()->color.a );
  } else {
    glColor3f( 0.8f, 0.5f, 0.2f );
  }
  glBegin( GL_QUADS );
  if( layout == HORIZONTAL_LAYOUT ) {
    glVertex3f( barLength + 1, -4, 0 );
    glVertex3f( -1, -4, 0 );
    glVertex3f( -1, 4, 0 );
    glVertex3f( barLength + 1, 4, 0 );
  } else {
    glVertex3f( -4, barLength + 1, 0 );
    glVertex3f( -4, -1, 0 );
    glVertex3f( 4, -1, 0 );
    glVertex3f( 4, barLength + 1, 0 );
  }
  glEnd();
  */

  glLineWidth(6.0f);

  //  glColor3f( 0.2f, 0.2f, 0.2f );
  if( theme && theme->getWindowBorder() ) {
    glColor4f( theme->getWindowBorder()->color.r,
               theme->getWindowBorder()->color.g,
               theme->getWindowBorder()->color.b,
               theme->getWindowBorder()->color.a );
  } else {
    //glColor3f( 0.8f, 0.5f, 0.2f );
    glColor3f( 0, 0, 0 );
  }
  //glColor3f( 1, 0.75f, 0.45f );
  glBegin( GL_LINES );
  if( layout == HORIZONTAL_LAYOUT ) {
    glVertex3f( 0, 0, 0 );
    glVertex3f( barLength, 0, 0 );
  } else {
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, barLength, 0 );
  }
  glEnd();

  // default args so I don't have to recompile .h file
  if(red == -1) {
    red = 0.5f;
    green = 1.0f;
    blue = 0.5f;
  }
  if(!gradient || percent > 40.0f) {  
    glColor3f( red, green, blue );
  } else if(percent > 25.0f) {
    glColor3f( 1.0f, 1.0f, 0.5f );
  } else {
    glColor3f( 1.0f, 0.5f, 0.5f );
  }
  glBegin( GL_LINES );
  if( layout == HORIZONTAL_LAYOUT ) {
    glVertex3f( 0, 0, 0 );
    glVertex3f( length, 0, 0 );
  } else {
    glVertex3f( 0, barLength - length, 0 );
    glVertex3f( 0, barLength, 0 );
  }
  glEnd();

  glLineWidth(1.0f);
  /*
  if(percent > 0.0f && percent < 100.0f) {
    if( theme && theme->getWindowBorder() ) {
      glColor4f( theme->getWindowBorder()->color.r,
                 theme->getWindowBorder()->color.g,
                 theme->getWindowBorder()->color.b,
                 theme->getWindowBorder()->color.a );
    } else {
      glColor3f( 0.8f, 0.5f, 0.2f );
    }
    glBegin( GL_LINES );
    if( layout == HORIZONTAL_LAYOUT ) {
      glVertex3f( length, -4, 0 );
      glVertex3f( length, 4, 0 );
    } else {
      glVertex3f( -4, length, 0 );
      glVertex3f( 4, length, 0 );
    }
    glEnd();
  }
  */
  glPopMatrix();
}

float Util::getRandomSum( float base, int count, float div ) {
  float sum = 0;
  float third = base / div;
  for( int i = 0; i < ( count < 1 ? 1 : count ); i++ ) {
    sum += ( ( third * rand()/RAND_MAX ) + ( base - third ) );
  }
  return sum;
}

char *Util::toLowerCase( char *s ) {
  char *p = s;
  while( *p ) {
    if( *p >= 'A' && *p <= 'Z' ) *p = *p - 'A' + 'a';
    p++;
  }
  return s;
}

// FIXME: take into account, existing |-s in text
char *Util::addLineBreaks( const char *in, char *out, int lineLength ) {
	strcpy( out, "" );
	char tmp[3000];
	strcpy( tmp, in );
	char *token = strtok( tmp, " \r\n\t" );
	int count = 0;
	while( token ) {
		int len = (int)strlen( token );
		for( int i = len - 1; i >= 0; i-- ) {
			if( token[i] == '|' ) {
				count = len - i;
				len = count;
				break;
			}
		}
		if( count + len >= lineLength ) {
			strcat( out, "|" );
			count = 0;
		}
		strcat( out, token );
		strcat( out, " " );
		count += len + 1;
		token = strtok( NULL, " \r\n\t" );
	}
	return out;
}

void Util::getLines( const char *in, vector<string> *out ) {
	char tmp[3000];
	char *q = tmp;
	strncpy( tmp, in, 2999 );
	tmp[2999] = '\0';

	char *p = strchr( q, '|' );
	while( p ) {
		*p = 0;
		string s = q;
		out->push_back( s );
		q = p + 1;
		p = strchr( q, '|' );
	}
	string s = q;
	out->push_back( s );
	/*
	cerr << "count=" << out->size() << endl;
	cerr << in << endl;
	for( unsigned int i = 0; i < out->size(); i++ ) {
		cerr << "\t" << (*out)[i] << endl;		
	}
	*/
}

float Util::getLight( float *normal, float lightAngle ) {
	// Simple light rendering:
	// need the normal as mapped on the xy plane
	// it's degree is the intensity of light it gets
	float x = ( normal[0] == 0 ? 0.01f : normal[0] );
	float y = ( normal[1] == 0 ? 0.01f : normal[1] );
	float z = ( normal[2] == 0 ? 0.01f : normal[2] );

	return ( getLightComp( x, y, lightAngle ) + 
					 getLightComp( z, y, lightAngle ) +
					 getLightComp( z, z, lightAngle ) ) / 3.0f;
}

float Util::getLightComp( float x, float y, float lightAngle ) {
	float rad = atan( y / x );
	float angle = ( 180.0f * rad ) / 3.14159;

	// read about the arctan problem: 
	// http://hyperphysics.phy-astr.gsu.edu/hbase/ttrig.html#c3
	int q = 1;
	if( x < 0 ) {     // Quadrant 2 & 3
		q = ( y >= 0 ? 2 : 3 );
		angle += 180;
	} else if( y < 0 ) { // Quadrant 4
		q = 4;
		angle += 360;
	}

	// assertion
	if( angle < 0 || angle > 360 ) {
#ifdef DEBUG_3DS
		cerr << "Warning: object: " << getName() << " angle=" << angle << " quadrant=" << q << endl;
#endif
	}

	// these are not used
	//if(angle < 0) angle += 360.0f;
	//if(angle > 360) angle -= 360.0f;

	// calculate the angle distance from the light
	float delta = 0;
	if( angle > lightAngle && angle < lightAngle + 180.0f ) {
		delta = angle - lightAngle;
	} else {
		if( angle < lightAngle ) angle += 360.0f;
		delta = ( 360 + lightAngle ) - angle;
	}

	// assertion
	if (delta < 0 || delta > 180.0f) {
#ifdef DEBUG_3DS
		cerr << "WARNING: object: " << getName() << " angle=" << angle << " delta=" << delta << endl;
#endif
	}

	// reverse and convert to value between 0 and 1
	delta = 1 - ( 0.4f * ( delta / 180.0f ) );

	// store the value
	return delta;
}