/*
	A quick and simple opengl font library that uses GNU freetype2, written
	and distributed as part of a tutorial for nehe.gamedev.net.
	Sven Olsen, 2003
*/


//Include our header file.
#include "FreeType.h"

using namespace std;

///This function gets the first power of 2 >= the
///int that we pass it.
inline int next_p2 ( int a )
{
	int rval=1;
	while(rval<a) rval<<=1;
	return rval;
}

///Create a display list coresponding to the give character.
void make_dlist ( FT_Face face, char ch, GLuint list_base, GLuint * tex_base, int *charWidth ) {

	//The first thing we do is get FreeType to render our character
	//into a bitmap.  This actually requires a couple of FreeType commands:

	//Load the Glyph for our character.
  if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT )) {
	cerr << "FT_Load_Glyph failed" << endl;
	exit(0);
  }

	//Move the face's glyph into a Glyph object.
    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph )) {
	  cerr << "FT_Get_Glyph failed" << endl;
	  exit(0);
	}

	//Convert the glyph to a bitmap.
	FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

	//This reference will make accessing the bitmap easier
	FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	//Use our helper function to get the widths of
	//the bitmap data that we will need in order to create
	//our texture.
	int width = next_p2( bitmap.width );
	int height = next_p2( bitmap.rows );

	//Allocate memory for the texture data.
	GLubyte* expanded_data = new GLubyte[ 2 * width * height];

	//Here we fill in the data for the expanded bitmap.
	//Notice that we are using two channel bitmap (one for
	//luminocity and one for alpha), but we assign
	//both luminocity and alpha to the value that we
	//find in the FreeType bitmap. 
	//We use the ?: operator so that value which we use
	//will be 0 if we are in the padding zone, and whatever
	//is the the Freetype bitmap otherwise.
	for(int j=0; j <height;j++) {
		for(int i=0; i < width; i++){
			expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] = 
				(i>=bitmap.width || j>=bitmap.rows) ?
				0 : bitmap.buffer[i + bitmap.width*j];
		}
	}


	//Now we just setup some texture paramaters.
    glBindTexture( GL_TEXTURE_2D, tex_base[ch]);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

  char msg[80];
  sprintf(msg, "FreeType::make_dlist char=%d", ch);
  Constants::checkTexture(msg, width, height);


	//Here we actually create the texture itself, notice
	//that we are using GL_LUMINANCE_ALPHA to indicate that
	//we are using 2 channel data.
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
		  0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

	//With the texture created, we don't need to expanded data anymore
    delete [] expanded_data;

	//So now we can create the display list
	glNewList(list_base+ch,GL_COMPILE);

	glBindTexture(GL_TEXTURE_2D,tex_base[ch]);

	//first we need to move over a little so that
	//the character has the right amount of space
	//between it and the one before it.
	glTranslatef(bitmap_glyph->left,0,0);

	//Now we move down a little in the case that the
	//bitmap extends past the bottom of the line 
	//(this is only true for characters like 'g' or 'y'.
	glPushMatrix();
	glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);

	//Now we need to account for the fact that many of
	//our textures are filled with empty padding space.
	//We figure what portion of the texture is used by 
	//the actual character and store that information in 
	//the x and y variables, then when we draw the
	//quad, we will only reference the parts of the texture
	//that we contain the character itself.
	float	x=(float)bitmap.width / (float)width,
			y=(float)bitmap.rows / (float)height;

	//Here we draw the texturemaped quads.
	//The bitmap that we got from FreeType was not 
	//oriented quite like we would like it to be,
	//so we need to link the texture to the quad
	//so that the result will be properly aligned.
	glBegin(GL_QUADS);
	glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
	glTexCoord2d(0,y); glVertex2f(0,0);
	glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
	glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
	glEnd();
	glPopMatrix();
	glTranslatef(face->glyph->advance.x >> 6 ,0,0);


	//increment the raster position as if we were a bitmap font.
	//(only needed if you want to calculate text length)
	//glBitmap(0,0,0,0,face->glyph->advance.x >> 6,0,NULL);
  *charWidth = (face->glyph->advance.x >> 6) + 1;

	//Finnish the display list
	glEndList();
}

void freetype_font_data::init(const char * fname, unsigned int h) {

#ifdef USE_OGLFT
  face = new OGLFT::TranslucentTexture( fname, h );
  //face = new OGLFT::MonochromeTexture( fname, h );

  // Always check to make sure the face was properly constructed
  if( face == 0 || !face->isValid() ) {
    cerr << "Could not construct face from " << fname << endl;
    return;
  }

  // Set the face color to red
  face->setForegroundColor( 1., 1., 1. );

  // For the raster styles, it is essential that the pixel store
  // unpacking alignment be set to 1
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  // Set the window's background color
  //glClearColor( .75, .75, .75, 1. );
#endif  


//  cerr << "FreeType: loading font: " << fname << " h=" << h << endl;

	//Allocate some memory to store the texture ids.
	textures = new GLuint[128];

	this->h=h;

	//Create and initilize a freetype font library.
	FT_Library library;
	if (FT_Init_FreeType( &library )) {
	  cerr <<"FT_Init_FreeType failed" << endl;
	  exit(0);
	}

	//The object in which Freetype holds information on a given
	//font is called a "face".
	FT_Face face;

	//This is where we load in the font information from the file.
	//Of all the places where the code might die, this is the most likely,
	//as FT_New_Face will die if the font file does not exist or is somehow broken.
	if (FT_New_Face( library, fname, 0, &face )) {
	  cerr << "FT_New_Face failed (there is probably a problem with your font file)" << endl;
	  exit(0);
	}

	//For some twisted reason, Freetype measures font size
	//in terms of 1/64ths of pixels.  Thus, to make a font
	//h pixels high, we need to request a size of h*64.
	//(h << 6 is just a prettier way of writting h*64)
	FT_Set_Char_Size( face, h << 6, h << 6, 96, 96);

	//Here we ask opengl to allocate resources for
	//all the textures and displays lists which we
	//are about to create.  
	list_base=glGenLists(128);
	glGenTextures( 128, textures );

	//This is where we actually create each of the fonts display lists.
	for(unsigned char i=0;i<128;i++)
		make_dlist(face,i,list_base,textures,&(width[i]));

	//We don't need the face information now that the display
	//lists have been created, so we free the assosiated resources.
	FT_Done_Face(face);

	//Ditto for the library.
	FT_Done_FreeType(library);
}

void freetype_font_data::clean() {
	glDeleteLists(list_base,128);
	glDeleteTextures(128,textures);
	delete [] textures;
}

/// A fairly straight forward function that pushes
/// a projection matrix that will make object world 
/// coordinates identical to window coordinates.
inline void pushScreenCoordinateMatrix() {
	glPushAttrib(GL_TRANSFORM_BIT);
	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
	glPopAttrib();
}

/// Pops the projection matrix without changing the current
/// MatrixMode.
inline void pop_projection_matrix() {
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

///Much like Nehe's glPrint function, but modified to work
///with freetype fonts.
void freetype_print(const freetype_font_data &ft_font, float x, float y, const char *fmt, ...)  {
  // We want a coordinate system where things coresponding to window pixels.
  //pushScreenCoordinateMatrix();					
  
  //float h=ft_font.h/.63f;						//We make the height about 1.5* that of
  
  char		text[256];								// Holds Our String
  va_list		ap;										// Pointer To List Of Arguments
  
  if (fmt == NULL)									// If There's No Text
    *text=0;											// Do Nothing
  
  else {
    va_start(ap, fmt);									// Parses The String For Variables
    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
    va_end(ap);											// Results Are Stored In Text
  }

  freetype_print_simple(ft_font, x, y, text);
}           

void freetype_print_simple(const freetype_font_data &ft_font, float x, float y, const char *str)  {

  GLuint font=ft_font.list_base;

  // slow on mac os X
  //	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);	
	glMatrixMode(GL_MODELVIEW);
  bool textureOn = glIsEnabled(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	


#ifdef USE_OGLFT
  glPushMatrix();
  glTranslatef(x, y, 0);
  glRotatef(180, 1, 0, 0);
  ft_font.face->draw( 0, 0, str );
  glPopMatrix();
#else
	glListBase(font);

	//This is where the text display actually happens.
	//For each line of text we reset the modelview matrix
	//so that the line's text will start in the correct position.
	//Notice that we need to reset the matrix, rather than just translating
	//down by h. This is because when each character is
	//draw it modifies the current matrix so that the next character
	//will be drawn immediatly after it.  
  glPushMatrix();
  glTranslatef(x, y, 0);
  glRotatef(180, 1, 0, 0);

	//  The commented out raster position stuff can be useful if you need to
	//  know the length of the text that you are creating.
	//  If you decide to use it make sure to also uncomment the glBitmap command
	//  in make_dlist().
	//	glRasterPos2f(0,0);
  glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);

  glPopMatrix();
#endif
  if(!textureOn) glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}






int getTextLength(const freetype_font_data &ft_font, const char *fmt, ...)  {
  char		text[256];								// Holds Our String
  va_list		ap;										// Pointer To List Of Arguments
      
  if (fmt == NULL)									// If There's No Text
    *text=0;											// Do Nothing
  
  else {
    va_start(ap, fmt);									// Parses The String For Variables
    vsprintf(text, fmt, ap);						// And Converts Symbols To Actual Numbers
    va_end(ap);											// Results Are Stored In Text
  }
  
  return getTextLengthSimple(ft_font, text);
}           


int getTextLengthSimple( const freetype_font_data &ft_font, char *text ) {
#ifdef USE_OGLFT
  return toint( ft_font.face->measure( text ).advance_.dx_ );
#else
  int size = 0;
  for( unsigned char *p = (unsigned char*)text; *p; p++ ) {
    size += ft_font.width[*p];
  }
  return size;
#endif
}

