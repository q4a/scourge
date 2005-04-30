#ifndef FREE_NEHE_H
#define FREE_NEHE_H

//FreeType Headers
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

//OpenGL Headers 
#include "../constants.h"

//Some STL headers
#include <vector>
#include <string>
#include <iostream>

using namespace std;

//This holds all of the information related to any
//freetype font that we want to create.  
struct freetype_font_data {
  int width[128];
	float h;			///< Holds the height of the font.
	GLuint * textures;	///< Holds the texture id's 
	GLuint list_base;	///< Holds the first display list id

	//The init function will create a font of
	//of the height h from the file fname.
	void init(const char * fname, unsigned int h);

	//Free all the resources assosiated with the font.
	void clean();
};

//The flagship function of the library - this thing will print
//out text at window coordinates x,y, using the font ft_font.
//The current modelview matrix will also be applied to the text. 
void freetype_print(const freetype_font_data &ft_font, float x, float y, const char *fmt, ...) ;
void freetype_print_simple(const freetype_font_data &ft_font, float x, float y, const char *str) ;
int getTextLength(const freetype_font_data &ft_font, const char *fmt, ...);
int getTextLengthSimple( const freetype_font_data &ft_font, char *text );
inline int getCharWidth( const freetype_font_data &ft_font, char c ) {
  return ft_font.width[ (unsigned char)c ];
}


#endif
