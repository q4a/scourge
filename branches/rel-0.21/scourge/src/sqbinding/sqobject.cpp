/***************************************************************************
                    sqobject.cpp  -  Squirrel object class
                             -------------------
    begin                : Sat Nov 6 2005
    copyright            : (C) 2005 by Gabor Torok
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
#include "../common/constants.h"
#include "sqobject.h"

using namespace std;

SqObject::SqObject() {
}

SqObject::~SqObject() {
}

void SqObject::documentSOM( char *path, set<string> *names ) {
	char filename[3000];
	snprintf( filename, 3000, "%s%s%s.html",
	          path,
	          ( path[ strlen( path ) - 1 ] == '/' || path[ strlen( path ) - 1 ] == '\\' ? "" : "/" ),
	          this->getClassName() );
	FILE *fp = fopen( filename, "w" );
	if ( !fp ) {
		cerr << "*** Error: can't open " << filename << endl;
		return;
	}

	// header
	fprintf( fp, "<!-- Do not edit this file. Generated by SqBinding::documentSOM(). -->\n\n" );
	fprintf( fp, "<html><head><link rel=StyleSheet href=\"som.css\" type=\"text/css\"></head><body>\n" );

	// class info
	SquirrelClassDecl *decl = this->getClassDeclaration();
	fprintf( fp, "<div class=section>%s</div>\n", this->getClassName() );
	fprintf( fp, "%s<p>\n\n", decl->description );

	// member info
	const ScriptClassMemberDecl *member;
	int n = 0;
	char buffer[ 255 ], buffer2[ 255 ];
	while ( decl->members[ n ].name ) {
		member = &( decl->members[ n++ ] );

		// skip the meta-methods
		if ( !strcmp( member->name, "_typeof" ) ||
		        !strcmp( member->name, "constructor" ) ||
		        !strcmp( member->name, "_nexti" ) )
			continue;

		fprintf( fp, "<div class=code>%s %s(%s)</div>\n%s<p>\n",
		         describeReturnType( member->returnType, names, buffer2, 255 ),
		         member->name,
		         describeTypeMask( member->typemask, buffer ),
		         member->description );
	}

	// footer
	fprintf( fp, "</body></html>\n" );

	fclose( fp );
}

char *SqObject::describeTypeMask( const SQChar *typemask, char *buffer ) {
	strcpy( buffer, "" );
	if ( typemask ) {
		char tmp[ 20 ], tmp2[ 80 ];
		// skip the first 'x', it's an internal reference:
		int start = ( ( char )( typemask[0] ) == 'x' ? 1 : 0 );
		for ( int i = start; i < static_cast<int>( strlen( ( char const* )typemask ) ); i++ ) {
			if ( strlen( buffer ) ) strcat( buffer, ", " );
			switch ( ( char )typemask[i] ) {
			case 'n' : strcpy( tmp, "int" ); break;
			case 'b' : strcpy( tmp, "bool" ); break;
			case 'f' : strcpy( tmp, "float" ); break;
			case 's' : strcpy( tmp, "string" ); break;
			default : strcpy( tmp, "unknown_type" );
			}
			snprintf( tmp2, 80, "%s p%d", tmp, ( ( i - start ) + 1 ) );
			strcat( buffer, tmp2 );
		}
	}
	return buffer;

}

char *SqObject::describeReturnType( const char *returnType,
    set<string> *names,
    char *buffer, size_t bufferSize ) {
	string s( returnType );
	if ( names->find( s ) == names->end() ) {
		strcpy( buffer, returnType );
	} else {
		snprintf( buffer, bufferSize, "<a href=\"%s.html\">%s</a>",
		          returnType,
		          returnType );
	}
	return buffer;
}

