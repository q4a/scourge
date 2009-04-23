/***************************************************************************
            mapeditor.cpp  -  The (not yet supported) map editor
                             -------------------
    begin                : Tue Jun 18 2005
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

#include "common/constants.h"
#include "mapeditor.h"
#include "render/renderlib.h"
#include "rpg/rpglib.h"
#include "mapwidget.h"
#include "sdlhandler.h"
#include "scourge.h"
#include "partyeditor.h"
#include "item.h"
#include "creature.h"
#include "shapepalette.h"
#include "outdoorgenerator.h"
#include "dungeongenerator.h"
#include "landgenerator.h"

using namespace std;

#define OUTDOOR_TEXTURE_WIDTH 4
#define OUTDOOR_TEXTURE_DEPTH 4

char *floorTypeName[2][80] = {
	{ "FLOOR_TILE", N_( "Tile: Passage" ) },
	{ "ROOM_FLOOR_TILE", N_( "Tile: Room" ) }
};

/**
  *@author Gabor Torok
  */


/*
  TODO:
  - ability to "name" item, shape, creature. Then text file can reference name.
  - need to call ShapePalette::decrementSkinRefCount() when closing map?
*/

bool contains( vector<Shape*> *seen, Shape *shape ) {
	for ( int i = 0; i < static_cast<int>( seen->size() ); i++ ) {
		if ( ( *seen )[i] == shape ) return true;
	}
	return false;
}

MapEditor::MapEditor( Scourge *scourge ) {
	this->scourge = scourge;

	// default map settings
	level = 1;
	depth = 0;

	outdoorTextureAngle = 0;
	outdoorTextureHorizFlip = outdoorTextureVertFlip = false;

	mapSettings = new EditorMapSettings();

	scourge->getMap()->setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ] );

	createNewMapDialog();

	scourge->getShapePalette()->loadAllShapes();

	int w = 200;
	mainWin = new Window( scourge->getSDLHandler(),
	                      scourge->getScreenWidth() - w, 0,
	                      w, scourge->getScreenHeight(),
	                      _( "Map Editor" ), false, Window::BASIC_WINDOW,
	                      GuiTheme::DEFAULT_THEME );
	mainWin->setVisible( false );
	mainWin->setLocked( true );

	int startx = 8;
	w -= 2;

	int yy = 8;
	int ystep = 24;

	mainWin->createLabel( 5, yy + 10, _( "Name:" ) );
	nameText = mainWin->createTextField( 60, yy, 10 );
	yy += ystep;

	loadButton = mainWin->createButton( startx, yy, ( w - 10 ) / 2, yy + 20, _( "Load" ) );
	saveButton = mainWin->createButton( ( w - 10 ) / 2 + 5, yy, w - 5, yy + 20, _( "Save" ) );
	yy += ystep;

	wallButton = mainWin->createButton( startx, yy, ( w - 10 ) / 2, yy + 20, _( "Wall" ), true );
	toggleButtonList.push_back( wallButton );
	wallButton->setSelected( true );
	doorButton = mainWin->createButton( ( w - 10 ) / 2 + 5, yy, w - 10, yy + 20, _( "Door" ), true );
	toggleButtonList.push_back( doorButton );
	yy += ystep;



	newButton = mainWin->createButton( startx, yy, ( w - 10 ) / 2, yy + 20, _( "New Map" ) );
	floorType = mainWin->createButton( ( w - 10 ) / 2 + 5, yy, w - 10, yy + 20, _( floorTypeName[ 1 ][ 1 ] ), true );
	yy += ystep;

	startPosButton = mainWin->createButton( startx, yy, ( w - 10 ) / 2, yy + 20, _( "Start Pos" ), true );
	toggleButtonList.push_back( startPosButton );
	yy += ystep;

	rugButton = mainWin->createButton( startx, yy, ( w - 10 ) / 2, yy + 20, _( "Rug" ), true );
	toggleButtonList.push_back( rugButton );
	secretButton = mainWin->createButton( ( w - 10 ) / 2 + 5, yy, w - 10, yy + 20, _( "Secret" ), true );
	toggleButtonList.push_back( secretButton );
	yy += ystep;
	trapButton = mainWin->createButton( startx, yy, ( w - 10 ) / 2, yy + 20, _( "Trap" ), true );
	toggleButtonList.push_back( trapButton );
	roofButton = mainWin->createButton( ( w - 10 ) / 2 + 5, yy, w - 10, yy + 20, _( "Show Roofs" ), true );
	roofButton->setSelected( true );
	//toggleButtonList.push_back( roofButton );
	yy += ystep;

	lowerButton = mainWin->createButton( startx, yy, ( w - 10 ) / 2, yy + 20, _( "Lower" ), true );
	toggleButtonList.push_back( lowerButton );
	lowerButton->setSelected( false );
	lowerButton->setEnabled( false );
	raiseButton = mainWin->createButton( ( w - 10 ) / 2 + 5, yy, w - 10, yy + 20, _( "Raise" ), true );
	toggleButtonList.push_back( raiseButton );
	raiseButton->setSelected( false );
	raiseButton->setEnabled( false );
	yy += ystep;


	// Lists
	vector<Shape*> seen;

	// items
	itemButton = mainWin->createButton( startx, yy, w - 10, yy + 20, _( "Item" ), true );
	toggleButtonList.push_back( itemButton );
	yy += ystep;
	itemList = new ScrollingList( startx, yy, w - 16, 75,
	                              scourge->getShapePalette()->getHighlightTexture() );
	mainWin->addWidget( itemList );
	yy += 79;


	furnitureButton = mainWin->createButton( startx, yy, w - 10, yy + 20, _( "Furniture" ), true );
	toggleButtonList.push_back( furnitureButton );
	yy += ystep;
	furnitureList = new ScrollingList( startx, yy, w - 16, 75,
	    scourge->getShapePalette()->getHighlightTexture() );
	mainWin->addWidget( furnitureList );
	yy += 79;

	// separate items from furniture
	map<string, const RpgItem *> *itemMap = RpgItem::getItemMap();
	vector<RpgItem*> itemVector;
	vector<RpgItem*> furnitureVector;
	for ( map<string, const RpgItem*>::iterator i = itemMap->begin();
	        i != itemMap->end(); ++i ) {
		string name = i->first;
		RpgItem *item = ( RpgItem* )( i->second );
		Shape *shape = scourge->getShapePalette()->getShape( item->getShapeIndex() );
		seen.push_back( shape );
		if ( item->isContainer() || item->isOther() ) {
			furnitureVector.push_back( item );
		} else {
			itemVector.push_back( item );
		}
	}

	itemNames = new string[ itemVector.size() ];
	for ( unsigned int i = 0; i < itemVector.size(); i++ ) {
		itemNames[ i ] = itemVector[ i ]->getDisplayName();
	}
	itemList->setLines( itemVector.size(), itemNames );

	furnitureNames = new string[ furnitureVector.size() ];
	for ( unsigned int i = 0; i < furnitureVector.size(); i++ ) {
		furnitureNames[ i ] = furnitureVector[ i ]->getDisplayName();
	}
	furnitureList->setLines( furnitureVector.size(), furnitureNames );

	// creatures
	creatureButton = mainWin->createButton( startx, yy, w - 10, yy + 20, _( "Creature" ), true );
	toggleButtonList.push_back( creatureButton );
	yy += ystep;
	creatureList = new ScrollingList( startx, yy, w - 16, 75,
	    scourge->getShapePalette()->getHighlightTexture() );
	mainWin->addWidget( creatureList );
	yy += 79;
	map<string, Monster*> *creatureMap = &( Monster::monstersByName );
	creatureNames = new string[ creatureMap->size() ];
	int count = 0;
	for ( map<string, Monster*>::iterator i = creatureMap->begin()
	        ; i != creatureMap->end(); ++i ) {
		string name = i->first;
		Monster *monster = i->second;

		// find its place by level
		int index = count;
		for ( int t = 0; t < count; t++ ) {
			Monster *m = creatures[ creatureNames[ t ] ];
			if ( m->getLevel() > monster->getLevel() ) {
				index = t;
				break;
			}
		}

		// make room
		for ( int t = count - 1; t >= index; t-- ) {
			creatureNames[ t + 1 ] = creatureNames[ t ];
		}
		count++;

		// add new creature
		creatureNames[ index ] = monster->isNpc() ? "NPC " : "";
		char levelStr[ 20 ];
		snprintf( levelStr, 20, "%d - ", monster->getLevel() );
		creatureNames[ index ] += levelStr + name;
		creatures[ creatureNames[ index ] ] = monster;
	}
	creatureList->setLines( creatureMap->size(), creatureNames );

	// shapes
	shapeButton = mainWin->createButton( startx, yy, w - 10, yy + 20, _( "Shape" ), true );
	toggleButtonList.push_back( shapeButton );
	yy += ystep;
	shapeList = new ScrollingList( startx, yy, w - 16, 75,
	    scourge->getShapePalette()->getHighlightTexture() );
	mainWin->addWidget( shapeList );
	yy += 79;

	map< string, GLShape* > *shapeMap = scourge->getShapePalette()->getShapeMap();
	shapeNames = new std::string[shapeMap->size()];
	count = 0;
	for ( map<string, GLShape*>::iterator i = shapeMap->begin(); i != shapeMap->end(); ++i ) {
		GLShape *shape = i->second;
		if ( !contains( &seen, shape ) && shape->isShownInMapEditor() ) {
			shapeNames[ count ] = i->first;
			count++;
		}
	}
	shapeList->setLines( count, shapeNames );

	// outdoor textures
	outdoorTexturesButton = mainWin->createButton( startx, yy, w - 10, yy + 20, _( "Outdoor Textures" ), true );
	outdoorTexturesButton->setEnabled( false );
	toggleButtonList.push_back( outdoorTexturesButton );
	yy += ystep;
	outdoorTexturesList = new ScrollingList( startx, yy, w - 16, 75, scourge->getShapePalette()->getHighlightTexture() );
	mainWin->addWidget( outdoorTexturesList );
	yy += 79;

	count = 0;
	outdoorTextureNames = new string[ WallTheme::OUTDOOR_THEME_REF_COUNT ];
	for ( int i = 0; i < WallTheme::OUTDOOR_THEME_REF_COUNT; i++ ) {
		outdoorTextureNames[ count++ ] = WallTheme::outdoorThemeRefName[ i ];
	}
	outdoorTexturesList->setLines( count, outdoorTextureNames );

	miniMap = new MiniMap( scourge );
}

void MapEditor::createNewMapDialog() {
	// new map ui
	int nw = 450;
	int nh = 430;
	newMapWin = new Window( scourge->getSDLHandler(),
	                        40, 40, nw, nh,
	                        _( "Map Properties" ),
	                        false,
	                        Window::BASIC_WINDOW,
	                        GuiTheme::DEFAULT_THEME );
	newMapWin->setVisible( false );
	newMapWin->setModal( true );
	int startx = 8;
	enum { TMP_SIZE = 300 };
	char tmp[ TMP_SIZE ];
	snprintf( tmp, TMP_SIZE, "%s (0-50):", _( "Map level" ) );
	newMapWin->createLabel( startx, 20, tmp );
	levelText = newMapWin->createTextField( 150, 10, 20 );
	snprintf( tmp, TMP_SIZE, "%s (0-10):", _( "Map depth" ) );
	newMapWin->createLabel( startx, 40, tmp );
	depthText = newMapWin->createTextField( 150, 30, 20 );
	newMapWin->createLabel( startx, 60, _( "Map theme:" ) );
	nw -= 12;


	themeList = new ScrollingList( 150, 50, 220, 60,
	    scourge->getShapePalette()->getHighlightTexture() );
	newMapWin->addWidget( themeList );
	themeNames = new string[ scourge->getShapePalette()->getAllThemeCount() ];

	for ( int i = 0; i < scourge->getShapePalette()->getAllThemeCount(); i++ ) {
		themeNames[ i ] = scourge->getShapePalette()->getAllThemeName( i );
		if ( scourge->getShapePalette()->isThemeSpecial( i ) )
			themeNames[i] += "(S)";
	}
	themeList->setLines( scourge->getShapePalette()->getAllThemeCount(), themeNames );

	newMapWin->createLabel( startx, 135, _( "Map Type:" ) );
	outdoorMapButton = newMapWin->createButton( startx + 100, 120, startx + 200, 140, "Outdoors", true );
	dungeonMapButton = newMapWin->createButton( startx + 210, 120, startx + 310, 140, "Dungeon", true );
	landMapButton = newMapWin->createButton( startx + 320, 120, startx + 420, 140, "Land", true );

	newMapWin->createLabel( startx, 160, _( "Select map location: (click on map, drag to move)" ) );
	mapWidget = new MapWidget( scourge, newMapWin, startx, 170, nw - startx, 335 );
	newMapWin->addWidget( mapWidget->getCanvas() );

	int bw = nw / 6;
	okButton = newMapWin->createButton( nw - bw * 3 - 10, 375, nw - bw * 2 - 5, 395, _( "New Map" ) );
	applyButton = newMapWin->createButton( nw - bw * 2 + 5, 375, nw - bw - 5, 395, _( "Apply" ) );
	cancelButton = newMapWin->createButton( nw - bw, 375, nw - 5, 395, _( "Cancel" ) );
}

MapEditor::~MapEditor() {
//  map< string, GLShape* > *shapeMap = scourge->getShapePalette()->getShapeMap();
	delete[] shapeNames;
	delete[] itemNames;
	delete mainWin;
	delete miniMap;
}

void MapEditor::drawView() {

	scourge->getMap()->draw();
	
	miniMap->drawMap();

	glDisable( GL_CULL_FACE );
	glDisable( GL_SCISSOR_TEST );

	glDisable( GL_CULL_FACE );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_TEXTURE_2D );
	glPushMatrix();
	glColor3f( 1, 0, 0 );

	glLoadIdentity();

	if ( scourge->getMap()->getCursorFlatMapX() < MAP_WIDTH &&
	        scourge->getMap()->getCursorFlatMapY() < MAP_DEPTH ) {
		Location *pos = scourge->getMap()->getLocation( scourge->getMap()->getCursorFlatMapX(),
		                scourge->getMap()->getCursorFlatMapY(),
		                0 );
		scourge->getSDLHandler()->texPrint( 50, 120, "F:%d,%d,%d C:%d,%d %s=%s %s=%s %s=%s",
		    scourge->getMap()->getCursorFlatMapX(),
		    scourge->getMap()->getCursorFlatMapY(),
		    scourge->getMap()->cursorZ,
		    scourge->getMap()->getCursorChunkX(),
		    scourge->getMap()->getCursorChunkY(),
		    _( "Shape" ),
		    ( pos && pos->shape ? pos->shape->getName() : "NULL" ),
		    _( "Item" ),
		    ( pos && pos->item ? ( ( Item* )( pos->item ) )->getRpgItem()->getDisplayName() : "NULL" ),
		    _( "Creature" ),
		    ( pos && pos->creature ? pos->creature->getName() : "NULL" ) );
	}
	if ( outdoorTexturesButton->isSelected() ) {
		glColor3f( 1, 1, 0 );
		scourge->getSDLHandler()->texPrint( 50, 130, "Keys: z,x - rotate, q,w - mirror" );
		glColor3f( 1, 0, 0 );
	}
	glTranslatef( 50, 50, 0 );
	glRotatef( scourge->getMap()->getZRot(), 0, 0, 1 );

	int n = 30;
	glBegin( GL_LINES );
	glVertex2i( 0, 0 );
	glVertex2i( 0, n );

	glVertex2i( 0, 0 );
	glVertex2i( -n / 2, n / 2 );

	glVertex2i( 0, 0 );
	glVertex2i( n / 2, n / 2 );
	glEnd();

	glPopMatrix();


}

void MapEditor::drawAfter() {
}

bool MapEditor::handleEvent( SDL_Event *event ) {
	scourge->getMap()->cursorWidth = scourge->getMap()->cursorDepth = 1;
	scourge->getMap()->cursorHeight = MAP_WALL_HEIGHT;
	if ( outdoorTexturesButton->isSelected() ) {
		int ref = outdoorTexturesList->getSelectedLine();
		scourge->getMap()->cursorWidth = scourge->getShapePalette()->getCurrentTheme()->getOutdoorTextureWidth( ref );
		scourge->getMap()->cursorDepth = scourge->getShapePalette()->getCurrentTheme()->getOutdoorTextureHeight( ref );
		scourge->getMap()->cursorHeight = 1;
	} else {
		GLShape *shape;
		if ( getShape( &shape ) ) {
			scourge->getMap()->cursorWidth = shape->getWidth();
			scourge->getMap()->cursorDepth = shape->getDepth();
			scourge->getMap()->cursorHeight = shape->getHeight();
		}
	}

	// find the highest point under the cursor
	int maxz = 0;
	for ( int gx = 0; gx < scourge->getMap()->cursorWidth; gx++ ) {
		for ( int gy = 0; gy < scourge->getMap()->cursorDepth; gy++ ) {
			for ( int i = MAP_VIEW_HEIGHT - 1; i >= 0; i-- ) {
				Location *pos = scourge->getMap()->getLocation( scourge->getMap()->getCursorFlatMapX() + gx,
				                scourge->getMap()->getCursorFlatMapY() - gy,
				                i );
				if ( pos && pos->shape && !( ( GLShape* )pos->shape )->hasVirtualShapes() && !pos->shape->isRoof() && maxz < i + 1 ) {
					maxz = i + 1;
				}
			}
		}
	}
	//scourge->getMap()->cursorZ = ( maxz >= MAP_VIEW_HEIGHT - 1 ? 0 : maxz + 1 );
	//int editorZ = scourge->getMap()->cursorZ;
	//if( !maxz ) editorZ = 0;

	int editorZ = scourge->getMap()->cursorZ = maxz;


	scourge->getMap()->handleEvent( event );

	switch ( event->type ) {
		//case SDL_MOUSEMOTION:
		/*
		if( event->motion.state == SDL_BUTTON_LEFT &&
		    newMapWin->isVisible() ) {
		  mapWidget->setPosition( scourge->getMouseX() - newMapWin->getX() - mapWidget->getCanvas()->getX(),
		                          scourge->getMouseY() - newMapWin->getY() - mapWidget->getCanvas()->getY() - Window::TOP_HEIGHT );
		}
		*/
	case SDL_MOUSEBUTTONUP:
		processMouseMotion( event->button.button, editorZ );
		break;
	case SDL_KEYUP:
		if ( event->key.keysym.sym == SDLK_ESCAPE ) {
			hide();
			return true;
		} else if ( event->key.keysym.sym == SDLK_KP_PLUS ||
		            event->key.keysym.sym == SDLK_KP_MINUS ) {
			miniMap->setShowMiniMap( miniMap->isMiniMapShown() ? false : true );
			return false;
		} else if ( event->key.keysym.sym == SDLK_x ) {
			outdoorTextureAngle += 90;
			if ( outdoorTextureAngle >= 360 ) outdoorTextureAngle -= 360;
		} else if ( event->key.keysym.sym == SDLK_z ) {
			outdoorTextureAngle -= 90;
			if ( outdoorTextureAngle < 360 ) outdoorTextureAngle += 360;
		} else if ( event->key.keysym.sym == SDLK_q ) {
			outdoorTextureHorizFlip = !( outdoorTextureHorizFlip );
		} else if ( event->key.keysym.sym == SDLK_w ) {
			outdoorTextureVertFlip = !( outdoorTextureVertFlip );
		} else if ( event->key.keysym.sym == SDLK_s ) {
			scourge->getSquirrelConsole()->setVisible( scourge->getSquirrelConsole()->isVisible() ? false : true );
		} else if ( event->key.keysym.sym == SDLK_g ) {
			scourge->getMap()->setGridEnabled( scourge->getMap()->isGridEnabled() ? false : true );
		}
		break;
	default: break;
	}
	return false;
}

bool MapEditor::handleEvent( Widget *widget, SDL_Event *event ) {

	int found = -1;
	for ( int i = 0; i < static_cast<int>( toggleButtonList.size() ); i++ ) {
		if ( toggleButtonList[ i ] == widget ) {
			found = i;
			break;
		}
	}
	if ( found > -1 ) {
		for ( int i = 0; i < static_cast<int>( toggleButtonList.size() ); i++ ) {
			toggleButtonList[ i ]->setSelected( i == found );
		}
	}

	string result;
	if( widget == outdoorMapButton && outdoorMapButton->isSelected() ) {
		dungeonMapButton->setSelected( !outdoorMapButton->isSelected() );
		landMapButton->setSelected( !outdoorMapButton->isSelected() );
	} else if( widget == dungeonMapButton && dungeonMapButton->isSelected() ) {
		outdoorMapButton->setSelected( !dungeonMapButton->isSelected() );
		landMapButton->setSelected( !dungeonMapButton->isSelected() ); 
	} else if( widget == landMapButton && landMapButton->isSelected() ) {
		outdoorMapButton->setSelected( !landMapButton->isSelected() );
		dungeonMapButton->setSelected( !landMapButton->isSelected() );		
	} else if ( widget == roofButton ) {
		scourge->getMap()->setRoofShowing( roofButton->isSelected() );
	} else if ( widget == saveButton ) {
		string tmp( nameText->getText() );
		scourge->getMap()->saveMap( tmp, result );
		scourge->showMessageDialog( result.c_str() );
//    scourge->getParty()->toggleRound( false );
	} else if ( widget == loadButton ) {
		string tmp( nameText->getText() );
		scourge->getMap()->loadMap( tmp, result );
		if ( scourge->getMap()->isHeightMapEnabled() ) {
			lowerButton->setEnabled( true );
			raiseButton->setEnabled( true );
			outdoorTexturesButton->setEnabled( true );
		}
		scourge->showMessageDialog( result.c_str() );
//    scourge->getParty()->toggleRound( false );
	} else if ( widget == newButton ) {
		newMapWin->setVisible( true );
		stringstream tmp;
		tmp << this->level;
		levelText->setText( tmp.str().c_str() );
		tmp.clear();
		tmp << this->depth;
		depthText->setText( tmp.str().c_str() );
		// FIXME: select theme!
		cerr << "FIXME: select theme!" << endl;
		mapWidget->setSelection( scourge->getMap()->mapGridX, scourge->getMap()->mapGridY );
	} else if ( widget == okButton || widget == applyButton ) {
		newMapWin->setVisible( false );

		this->level = atoi( levelText->getText() );
		this->depth = atoi( depthText->getText() );
		if ( widget == okButton ) {
			scourge->getMap()->reset();
			int line = themeList->getSelectedLine();
			if ( line > -1 ) {
				string str = themeNames[ line ];
				int pos = str.rfind( "(S)" );
				if ( pos == str.size() - 3 ) str.erase( pos );
				scourge->getShapePalette()->loadTheme( str.c_str() );
			}

			if ( outdoorMapButton->isSelected() ) {
				scourge->getMap()->setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::OUTDOOR_HELPER ] );
				OutdoorGenerator *og = new OutdoorGenerator( scourge, level, depth, 1, false, false, NULL );
				og->toMap( scourge->getMap(), scourge->getShapePalette(), false, false );
				delete og;
				raiseButton->setEnabled( true );
				lowerButton->setEnabled( true );
				outdoorTexturesButton->setEnabled( true );
			} else if ( landMapButton->isSelected() ) {
				scourge->getMap()->setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::OUTDOOR_HELPER ] );
				LandGenerator *og = new LandGenerator( scourge, level, depth, 1, false, false, NULL );
				// todo: this should not be hard-coded (could come from map in dialog)
				og->setRegion( 6, 4 * 8 + 7 );
				og->setMapPosition( 75, 75 ); // the fourth quarter
				og->toMap( scourge->getMap(), scourge->getShapePalette(), false, false );
				delete og;
				raiseButton->setEnabled( true );
				lowerButton->setEnabled( true );
				outdoorTexturesButton->setEnabled( true );				
			} else if ( dungeonMapButton->isSelected() ) {
				scourge->getMap()->setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ] );
				DungeonGenerator *og = new DungeonGenerator( scourge, level, depth, 1, false, false, NULL );
				og->toMap( scourge->getMap(), scourge->getShapePalette(), false, false );
				delete og;
			} else {
				scourge->getMap()->setMapRenderHelper( MapRenderHelper::helpers[ MapRenderHelper::ROOM_HELPER ] );
				lowerButton->setEnabled( false );
				raiseButton->setEnabled( false );
				outdoorTexturesButton->setEnabled( false );
			}
		}
		mapWidget->getSelection( &( scourge->getMap()->mapGridX ), &( scourge->getMap()->mapGridY ) );

	} else if ( widget == cancelButton ) {
		newMapWin->setVisible( false );
	} else if ( widget == floorType ) {
		floorType->setLabel( _( floorTypeName[ floorType->isSelected() ? 0 : 1 ][ 1 ] ) );
	} else if ( widget == mapWidget->getCanvas() ) {
//    mapWidget->setPosition( scourge->getMouseX() - newMapWin->getX() - mapWidget->getCanvas()->getX(),
//                            scourge->getMouseY() - newMapWin->getY() - mapWidget->getCanvas()->getY() - Window::TOP_HEIGHT );
	} else if ( widget == scourge->getSquirrelRun() ||
	            widget == scourge->getSquirrelText() ) {
		scourge->runSquirrelConsole();
	} else if ( widget == scourge->getSquirrelClear() ) {
		scourge->clearSquirrelConsole();
	} else if ( widget == scourge->getSquirrelReload() ) {
		scourge->runSquirrelConsole( "scourgeGame.reloadNuts();" );
	}
	return false;
}

void MapEditor::show() {
	scourge->getMap()->setMapSettings( mapSettings );
	scourge->getMap()->reset();
	scourge->getMap()->center( MAP_WIDTH / 2, MAP_DEPTH / 2, true );
	scourge->getMap()->setViewArea( 0, 0,
	    mainWin->getX(),
	    mainWin->getY() + mainWin->getHeight() );
	scourge->getShapePalette()->loadTheme( "egypt" );
	mainWin->setVisible( true );
}

void MapEditor::hide() {
	mainWin->setVisible( false );
	scourge->getMap()->setMapSettings( scourge->getMapSettings() );
}

bool MapEditor::getShape( GLShape **shape,
                          Item **item,
                          Creature **creature ) {
	*shape = NULL;
	if ( item ) *item = NULL;
	if ( creature ) *creature = NULL;
	if ( creatureButton->isSelected() &&
	        creatureList->getSelectedLine() > -1 ) {
		string s = creatureNames[ creatureList->getSelectedLine() ];
		Monster *monster = creatures[ s ];
		//Monster *monster = Monster::getMonsterByName( creatureNames[ creatureList->getSelectedLine() ] );
		// cache creature shapes
		if ( creatureOutlines.find( monster ) == creatureOutlines.end() ) {
			creatureOutlines[ monster ] = scourge->getSession()->getShapePalette()->
			                              getCreatureShape( monster->getModelName(),
			                                                monster->getSkinName(),
			                                                monster->getScale(),
			                                                monster );
		}
		*shape = creatureOutlines[ monster ];
		if ( creature ) {
			cerr << "new creature" << endl;
			*creature = scourge->getSession()->newCreature( monster, *shape );
		}
		return true;
	} else if ( itemButton->isSelected() &&
	            itemList->getSelectedLine() > -1 ) {
		addNewItem( itemNames[ itemList->getSelectedLine() ].c_str(), shape, item, creature );
		return true;
	} else if ( furnitureButton->isSelected() &&
	            furnitureList->getSelectedLine() > -1 ) {
		addNewItem( furnitureNames[ furnitureList->getSelectedLine() ].c_str(), shape, item, creature );
		return true;
	} else if ( shapeButton->isSelected() &&
	            shapeList->getSelectedLine() > -1 ) {
		*shape = scourge->getShapePalette()->
		         findShapeByName( shapeNames[ shapeList->getSelectedLine() ].c_str() );
		return true;
	} else {
		return false;
	}
}

void MapEditor::addNewItem( char const* name,
                            GLShape **shape,
                            Item **item,
                            Creature **creature ) {
	RpgItem *rpgItem = RpgItem::getItemByName( name );
	*shape = scourge->getShapePalette()->getShape( rpgItem->getShapeIndex() );
	if ( item ) {
		cerr << "new item" << endl;
		*item = scourge->getSession()->newItem( rpgItem, level );
		// fill the container with random items
		if ( rpgItem->isContainer() ) {
			// some items
			int n = Util::dice( 3 );
			for ( int i = 0; i < n; i++ ) {
				RpgItem *containedItem = RpgItem::getRandomItem( depth );
				if ( containedItem )
					( *item )->addContainedItem( scourge->getSession()->
					                             newItem( containedItem, level ),
					                             true );
			}
			// some spells
			if ( 0 == Util::dice( 25 ) ) {
				int n = Util::pickOne( 1, 2 );
				int spellLevel = level / 5;
				for ( int i = 0; i < n; i++ ) {
					Spell *spell = MagicSchool::getRandomSpell( spellLevel );
					if ( spell ) {
						Item *scroll = scourge->getSession()->
						               newItem( RpgItem::getItemByName( "Scroll" ), level, spell );
						( *item )->addContainedItem( scroll, true );
					}
				}
			}

			// print summary
			cerr << "Container contents:" << endl;
			for ( int i = 0; i < ( *item )->getContainedItemCount(); i++ ) {
				cerr << "\t" << ( *item )->getContainedItem( i )->getRpgItem()->getDisplayName() << endl;
			}
		}
	}
}

void MapEditor::processMouseMotion( Uint8 button, int editorZ ) {
	if ( scourge->getSDLHandler()->mouseX < mainWin->getX() &&
	        ( button == SDL_BUTTON_LEFT ||
	          button == SDL_BUTTON_RIGHT &&
	          ( SDL_GetModState() & KMOD_SHIFT ) ) ) {

		// draw the correct walls in this chunk
		int xx = scourge->getMap()->getCursorFlatMapX();
		int yy = scourge->getMap()->getCursorFlatMapY() - 1;

		if ( startPosButton->isSelected() ) {
			scourge->getMap()->startx = xx;
			scourge->getMap()->starty = yy;
			return;
		}

		int mapx = ( ( xx - MAP_OFFSET )  / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;
		int mapy = ( ( yy - MAP_OFFSET )  / MAP_UNIT ) * MAP_UNIT + MAP_OFFSET;

		//int mapx = scourge->getMap()->getCursorChunkX() * MAP_UNIT + MAP_OFFSET;
		//int mapy = scourge->getMap()->getCursorChunkY() * MAP_UNIT + MAP_OFFSET;

		GLShape *shape;
		if ( button == SDL_BUTTON_LEFT ) {
			if ( outdoorTexturesButton->isSelected() ) {
				if ( outdoorTexturesList->getSelectedLine() > -1 ) {
					//string s = outdoorTextureNames[ outdoorTexturesList->getSelectedLine() ];
					//NamedOutdoorTexture *ot = scourge->getShapePalette()->getOutdoorNamedTexture( s );
					int ref = outdoorTexturesList->getSelectedLine();
					int faceCount = scourge->getShapePalette()->getCurrentTheme()->getOutdoorFaceCount( ref );
					if ( faceCount == 0 ) {
						cerr << "Map Editor Error: no textures for outdoor theme! ref=" << WallTheme::outdoorThemeRefName[ref] << endl;
						return;
					}
//      int w = scourge->getShapePalette()->getCurrentTheme()->getOutdoorTextureWidth( ref );
//      int h = scourge->getShapePalette()->getCurrentTheme()->getOutdoorTextureHeight( ref );
					scourge->getMap()->setOutdoorTexture( xx, yy + 1,
					    0, 0,
					    ref,
					    outdoorTextureAngle,
					    outdoorTextureHorizFlip,
					    outdoorTextureVertFlip,
					    ROAD_LAYER );
				}
				return;
			} else {
				Item *item;
				Creature *creature;
				if ( getShape( &shape, &item, &creature ) ) {
					if ( item ) {
						scourge->getMap()->setItem( xx, yy + 1, editorZ, item );
					} else if ( creature ) {
						scourge->getMap()->setCreature( xx, yy + 1, editorZ, creature );
						creature->moveTo( xx, yy + 1, editorZ );
					} else if ( shape ) {
						if ( shape->getIgnoreHeightMap() && !shape->isRoof() ) {
							for ( int sx = mapx; sx < mapx + shape->getWidth(); sx++ ) {
								for ( int sy = mapy - shape->getDepth(); sy <= mapy; sy++ ) {
									scourge->getMap()->flattenChunk( sx, sy );
								}
							}
						}
						scourge->getMap()->setPosition( xx, yy + 1, editorZ, shape );
					}
					return;
				}
			}
		} else if ( button == SDL_BUTTON_RIGHT ) {
			if ( outdoorTexturesButton->isSelected() ) {
				int ref = outdoorTexturesList->getSelectedLine();
				scourge->getMap()->removeOutdoorTexture( xx, yy + 1, scourge->getShapePalette()->getCurrentTheme()->getOutdoorTextureWidth( ref ), scourge->getShapePalette()->getCurrentTheme()->getOutdoorTextureHeight( ref ), ROAD_LAYER );
			} else if ( getShape( &shape ) ) {
				for ( int sx = 0; sx < shape->getWidth(); sx++ ) {
					for ( int sy = 0; sy < shape->getDepth(); sy++ ) {
						for ( int sz = MAP_VIEW_HEIGHT - 1; sz >= 0; sz-- ) {
							scourge->getMap()->removeLocation( xx + sx, yy - sy + 1, sz );
						}
					}
				}
				return;
			}
		}

		int innerX = xx - mapx;
		int innerY = yy - mapy;

		//    cerr << "pos: " << mapx << "," << mapy <<
		//      " map:" << scourge->getMap()->getCursorFlatMapX() << "," <<
		//      scourge->getMap()->getCursorFlatMapX() << endl;


		// find the region in the chunk
		int mx = -1;
		int my = -1;
		int dir = -1;
		if ( innerX < MAP_UNIT_OFFSET ) {
			mx = mapx;
			my = mapy;
			dir = Constants::WEST;
		} else if ( innerY < MAP_UNIT_OFFSET ) {
			mx = mapx;
			my = mapy;
			dir = Constants::NORTH;
		} else if ( innerX >= MAP_UNIT - MAP_UNIT_OFFSET ) {
			mx = mapx + MAP_UNIT - MAP_UNIT_OFFSET;
			my = mapy;
			dir = Constants::EAST;
		} else if ( innerY >= MAP_UNIT - MAP_UNIT_OFFSET ) {
			mx = mapx;
			my = mapy + MAP_UNIT - MAP_UNIT_OFFSET;
			dir = Constants::SOUTH;
		}

		if ( dir != -1 ) {
			if ( secretButton->isSelected() ) {
				if ( button == SDL_BUTTON_RIGHT ) {
					removeSecret( xx, yy + 1 );
				} else {
					addSecret( xx, yy + 1 );
				}
			} else if ( button == SDL_BUTTON_RIGHT ) {
				removeWall( mx, my, dir );
			} else if ( wallButton->isSelected() ) {
				addWall( mx, my, dir );
			} else if ( doorButton->isSelected() ) {
				addDoor( mx, my, dir );
			}

			// blend the corners
			for ( int x = -1; x <= 1; x++ ) {
				for ( int y = -1; y <= 1; y++ ) {
					blendCorners( mapx + ( x * MAP_UNIT ),
					              mapy + ( y * MAP_UNIT ) );
				}
			}
		} else {
			int chunkX = ( mapx - MAP_OFFSET ) / MAP_UNIT;
			int chunkY = ( mapy - MAP_OFFSET ) / MAP_UNIT;
			if ( rugButton->isSelected() ) {
				if ( button == SDL_BUTTON_RIGHT ) {
					scourge->getMap()->removeRugPosition( chunkX, chunkY );
				} else {
					addRug( chunkX, chunkY );
				}
			} else if ( trapButton->isSelected() ) {
				if ( button == SDL_BUTTON_RIGHT ) {
					removeTrap( xx, yy );
				} else {
					addTrap( xx, yy );
				}
			} else {
				if ( button == SDL_BUTTON_RIGHT ) {
					removeFloor( mapx, mapy );
				} else if ( lowerButton->isSelected() ) {
					lowerGroundHeight( xx, yy );
				} else if ( raiseButton->isSelected() ) {
					raiseGroundHeight( xx, yy );
				} else {
					addFloor( mapx, mapy );
				}
			}
		}
	}
}

void MapEditor::addRug( Sint16 mapx, Sint16 mapy ) {
	// pick an orientation
	bool isHorizontal = ( Util::dice( 2 ) == 0 ? true : false );

	// save it
	Rug rug;
	rug.isHorizontal = isHorizontal;
	rug.texture = scourge->getSession()->getShapePalette()->getRandomRug();
	rug.angle = Util::roll( -15.0f, 15.0f );

	scourge->getMap()->setRugPosition( mapx, mapy, &rug );
}

void MapEditor::addSecret( Sint16 mapx, Sint16 mapy ) {
	Location *pos = scourge->getMap()->getLocation( mapx, mapy, 0 );
	if ( pos ) {
		scourge->getMap()->addSecretDoor( pos->x, pos->y );
	}
}

void MapEditor::removeSecret( Sint16 mapx, Sint16 mapy ) {
	Location *pos = scourge->getMap()->getLocation( mapx, mapy, 0 );
	if ( pos ) {
		scourge->getMap()->removeSecretDoor( pos->x, pos->y );
	}
}

void MapEditor::addTrap( Sint16 mapx, Sint16 mapy ) {
	int w = Util::pickOne( 4, 9 );
	int h = Util::pickOne( 4, 9 );
	scourge->getMap()->addTrap( mapx, mapy, w, h );
}

void MapEditor::removeTrap( Sint16 mapx, Sint16 mapy ) {
	int trapIndex = scourge->getMap()->getTrapAtLoc( mapx, mapy );
	if ( trapIndex > -1 ) scourge->getMap()->removeTrap( trapIndex );
}

void MapEditor::addWall( Sint16 mapx, Sint16 mapy, int dir ) {
	scourge->getMap()->flattenChunk( mapx, mapy );
	switch ( dir ) {
	case Constants::NORTH: addNSWall( mapx, mapy, 1 ); break;
	case Constants::SOUTH: addNSWall( mapx, mapy, -1 ); break;
	case Constants::WEST: addEWWall( mapx, mapy, 1 ); break;
	case Constants::EAST: addEWWall( mapx, mapy, -1 ); break;
	default: cerr << "*** addWall, Unknown dir=" << dir << endl;
	}
}

void MapEditor::addDoor( Sint16 mapx, Sint16 mapy, int dir ) {
	scourge->getMap()->flattenChunk( mapx, mapy );
	switch ( dir ) {
	case Constants::NORTH: addNSDoor( mapx, mapy, 1 ); break;
	case Constants::SOUTH: addNSDoor( mapx, mapy, -1 ); break;
	case Constants::WEST: addEWDoor( mapx, mapy, 1 ); break;
	case Constants::EAST: addEWDoor( mapx, mapy, -1 ); break;
	default: cerr << "*** addDoor, Unknown dir=" << dir << endl;
	}
}

void MapEditor::removeWall( Sint16 mapx, Sint16 mapy, int dir ) {
	switch ( dir ) {
	case Constants::NORTH: removeNSWall( mapx, mapy, 1 ); break;
	case Constants::SOUTH: removeNSWall( mapx, mapy, -1 ); break;
	case Constants::WEST: removeEWWall( mapx, mapy, 1 ); break;
	case Constants::EAST: removeEWWall( mapx, mapy, -1 ); break;
	default: cerr << "*** removeWall, Unknown dir=" << dir << endl;
	}
}

void MapEditor::addEWDoor( Sint16 mapx, Sint16 mapy, int dir ) {
	ShapePalette *shapePal = scourge->getShapePalette();
	if ( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT / 2, 0 ) ) {
		scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET,
		    MAP_WALL_HEIGHT - 2, shapePal->findShapeByName( "EW_DOOR_TOP" ) );
		scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET +  2,
		    0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET * 2 +  2,
		    0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		scourge->getMap()->setPosition( mapx + 1, mapy + MAP_UNIT - MAP_UNIT_OFFSET - 2,
		    0, shapePal->findShapeByName( "EW_DOOR" ) );
		scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET,
		    0, shapePal->findShapeByName( "DOOR_SIDE" ) );

		// corners
		if ( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {
			scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
			    shapePal->findShapeByName( "CORNER" ) );
		}
		if ( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT, 0 ) ) {
			scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT, 0,
			    shapePal->findShapeByName( "CORNER" ) );
		}
	}
}

void MapEditor::addNSDoor( Sint16 mapx, Sint16 mapy, int dir ) {
	ShapePalette *shapePal = scourge->getShapePalette();
	if ( !scourge->getMap()->getLocation( mapx + MAP_UNIT / 2,
	        mapy + MAP_UNIT_OFFSET,
	        0 ) ) {
		scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET,
		    MAP_WALL_HEIGHT - 2, shapePal->findShapeByName( "NS_DOOR_TOP" ) );
		scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET,
		    0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET * 2, mapy + MAP_UNIT_OFFSET - 1,
		    0, shapePal->findShapeByName( "NS_DOOR" ) );
		scourge->getMap()->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET * 2, mapy + MAP_UNIT_OFFSET,
		    0, shapePal->findShapeByName( "DOOR_SIDE" ) );
		scourge->getMap()->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET * 3, mapy + MAP_UNIT_OFFSET,
		    0, shapePal->findShapeByName( "DOOR_SIDE" ) );

		// corners
		if ( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {
			scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
			    shapePal->findShapeByName( "CORNER" ) );
		}
		if ( !scourge->getMap()->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 ) ) {
			scourge->getMap()->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
			    shapePal->findShapeByName( "CORNER" ) );
		}
	}
}

void MapEditor::addEWWall( Sint16 mapx, Sint16 mapy, int dir ) {
	// short wall
	bool north = false;
	bool south = false;
	if ( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT / 2, 0 ) ) {

		// see if EW_WALL to the north and south
		north = ( mapy - 1 >= 0 &&
		          scourge->getMap()->getLocation( mapx, mapy - 1, 0 ) &&
		          !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + 1, 0 ) );
		south = ( mapy + MAP_UNIT + 1 < MAP_DEPTH &&
		          scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT + 1, 0 ) &&
		          !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + MAP_UNIT - 1, 0 ) );

		// corner
		if ( south ) {
			scourge->getMap()->
			removeLocation( mapx, mapy + MAP_UNIT, 0 );
		} else if ( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT, 0 ) ) {
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "CORNER" ) );
		}

		// corner
		if ( north ) {
			scourge->getMap()->
			removeLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 );
		} else if ( !scourge->getMap()->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "CORNER" ) );
		}

		if ( north && south ) {
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
		} else if ( south ) {
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "EW_WALL_EXTRA" ) );
		} else if ( north ) {
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "EW_WALL_EXTRA" ) );
		} else {
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT - MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "EW_WALL" ) );
		}

		// add a light
		if ( Util::dice( 100 ) <= 25 ) {
			if ( dir == 1 ) {
				scourge->getMap()->
				setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT - 4,
				             6, scourge->getShapePalette()->findShapeByName( "LAMP_WEST" ) );
				scourge->getMap()->
				setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT - 4,
				             4, scourge->getShapePalette()->findShapeByName( "LAMP_BASE" ) );
			} else {
				scourge->getMap()->
				setPosition( mapx - 1, mapy + MAP_UNIT - 4,
				             6, scourge->getShapePalette()->findShapeByName( "LAMP_EAST" ) );
				scourge->getMap()->
				setPosition( mapx - 1, mapy + MAP_UNIT - 4,
				             4, scourge->getShapePalette()->findShapeByName( "LAMP_BASE" ) );
			}
		}


		// change north chunk
		//cerr << "Looking north of EW_WALL map=" << mapx << "," << mapy << endl;
		if ( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL" ) &&
		        isShape( mapx, mapy, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy - 1, 0 ) ) {
			//cerr << "Success!" << endl;
			scourge->getMap()->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->removeLocation( mapx, mapy, 0 );
			scourge->getMap()->
			setPosition( mapx, mapy, 0, scourge->getShapePalette()->
			             findShapeByName( "EW_WALL_EXTRA" ) );
		}

		//cerr << "Looking north of EW_WALL_EXTRA map=" << mapx << "," << mapy << endl;
		if ( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL_EXTRA" ) &&
		        isShape( mapx, mapy, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy - 1, 0 ) ) {
			//cerr << "Success!" << endl;
			scourge->getMap()->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->removeLocation( mapx, mapy, 0 );
			scourge->getMap()->
			setPosition( mapx, mapy, 0, scourge->getShapePalette()->
			             findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
		}

		// change the south chunk
		//cerr << "Looking south of EW_WALL map=" << mapx << "," << mapy << endl;
		if ( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0, "EW_WALL" ) &&
		        isShape( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + MAP_UNIT + 1, 0 ) ) {
			//cerr << "Success!" << endl;
			scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "EW_WALL_EXTRA" ) );
		}
		//cerr << "Looking south of EW_WALL_EXTRA map=" << mapx << "," << mapy << endl;
		if ( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT, 0, "EW_WALL_EXTRA" ) &&
		        isShape( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx + dir * ( MAP_UNIT / 2 ), mapy + MAP_UNIT + 1, 0 ) ) {
			//cerr << "Success!" << endl;
			scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT, 0 );
			scourge->getMap()->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
		}
		//cerr << "----------------------------------------------------" << endl;
	}
}

void MapEditor::addNSWall( Sint16 mapx, Sint16 mapy, int dir ) {

	bool east = false;
	bool west = false;

	// short wall
	if ( !scourge->getMap()->getLocation( mapx + MAP_UNIT / 2,
	        mapy + MAP_UNIT_OFFSET,
	        0 ) ) {

		// see if NS_WALL to the east and west
		east = ( mapx + MAP_UNIT < MAP_WIDTH &&
		         scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 ) &&
		         !scourge->getMap()->getLocation( mapx + MAP_UNIT - 1, mapy + dir * ( MAP_UNIT / 2 ), 0 ) );
		west = ( mapx - 1 >= 0 &&
		         scourge->getMap()->getLocation( mapx - 1, mapy + MAP_UNIT_OFFSET, 0 ) &&
		         !scourge->getMap()->getLocation( mapx, mapy + dir * ( MAP_UNIT / 2 ), 0 ) );

		// corner
		if ( west ) {
			scourge->getMap()->
			removeLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 );
		} else if ( !scourge->getMap()->getLocation( mapx,
		            mapy + MAP_UNIT_OFFSET,
		            0 ) ) {
			scourge->getMap()->
			setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "CORNER" ) );
		}

		// corner
		if ( east ) {
			scourge->getMap()->
			removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
		} else if ( !scourge->getMap()->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET,
		            mapy + MAP_UNIT_OFFSET,
		            0 ) ) {
			scourge->getMap()->
			setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "CORNER" ) );
		}


		if ( east && west ) {
			scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
			    scourge->getShapePalette()->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
		} else if ( west ) {
			scourge->getMap()->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
			    scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA" ) );
		} else if ( east ) {
			scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
			    scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA" ) );
		} else {
			scourge->getMap()->setPosition( mapx + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
			    scourge->getShapePalette()->findShapeByName( "NS_WALL" ) );
		}

		// Add a light
		if ( Util::dice( 100 ) <= 25 ) {
			if ( dir == 1 ) {
				scourge->getMap()->
				setPosition( mapx + 4, mapy + MAP_UNIT_OFFSET + 1, 6,
				             scourge->getShapePalette()->findShapeByName( "LAMP_NORTH" ) );
				scourge->getMap()->
				setPosition( mapx + 4, mapy + MAP_UNIT_OFFSET + 1, 4,
				             scourge->getShapePalette()->findShapeByName( "LAMP_BASE" ) );
			} else {
				/*
				See gltorch.cpp on why there aren't southern torches... <sigh>
				cerr << "adding South light" << endl;
				scourge->getMap()->
				  setPosition(mapx + 4, mapy, 6,
				              scourge->getShapePalette()->findShapeByName("LAMP_SOUTH", true));
				scourge->getMap()->
				  setPosition(mapx + 4, mapy, 4,
				              scourge->getShapePalette()->findShapeByName("LAMP_BASE", true));

				*/
			}
		}

		// change west chunk
		if ( isShape( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) &&
		        isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx - MAP_UNIT_OFFSET, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
			scourge->getMap()->removeLocation( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->removeLocation( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->
			setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->findShapeByName( "NS_WALL_EXTRA" ) );
		}
		if ( isShape( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) &&
		        isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx - MAP_UNIT_OFFSET, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
			scourge->getMap()->removeLocation( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->removeLocation( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->
			setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
		}

		// change the east chunk
		if ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) &&
		        isShape( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
			scourge->getMap()->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->removeLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->
			setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "NS_WALL_EXTRA" ) );
		}
		if ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) &&
		        isShape( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
		        !scourge->getMap()->getLocation( mapx + MAP_UNIT, mapy + dir * ( MAP_UNIT / 2 ), 0 ) ) {
			scourge->getMap()->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->removeLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
			scourge->getMap()->
			setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
			             scourge->getShapePalette()->
			             findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
		}
	}
}

void MapEditor::addFloor( Sint16 mapx, Sint16 mapy, bool doFlattenChunk ) {
	if ( scourge->getMap()->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
	if ( doFlattenChunk ) scourge->getMap()->flattenChunk( mapx, mapy );
	scourge->getMap()->
	setFloorPosition( mapx, mapy + MAP_UNIT,
	                  scourge->getShapePalette()->
	                  findShapeByName( floorTypeName[ floorType->isSelected() ? 0 : 1 ][ 0 ] ) );
}

void MapEditor::lowerGroundHeight( Sint16 mapx, Sint16 mapy ) {
	scourge->getMap()->setGroundHeight( mapx / OUTDOORS_STEP, mapy / OUTDOORS_STEP, Util::roll( 0, 2 ) );
}

void MapEditor::raiseGroundHeight( Sint16 mapx, Sint16 mapy ) {
	scourge->getMap()->setGroundHeight( mapx / OUTDOORS_STEP, mapy / OUTDOORS_STEP, Util::roll( 14.0f, 20.0f ) );
}

void MapEditor::changePathChunk( Sint16 mapx, Sint16 mapy, bool lower ) {
	//if( !scourge->getMap()->getFloorPosition( mapx, mapy + MAP_UNIT ) ) return;
	int chunkX = ( mapx - MAP_OFFSET ) / MAP_UNIT;
	int chunkY = ( mapy - MAP_OFFSET ) / MAP_UNIT;
	for ( int x = OUTDOORS_STEP; x <= MAP_UNIT - OUTDOORS_STEP; x++ ) {
		for ( int y = OUTDOORS_STEP; y <= MAP_UNIT - OUTDOORS_STEP; y++ ) {
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			scourge->getMap()->setGroundHeight( xx, yy, ( lower ? 0 : Util::roll( 7, 10 ) ) );
		}
	}
	for ( int x = OUTDOORS_STEP; x <= MAP_UNIT - OUTDOORS_STEP; x++ ) {
		if ( scourge->getMap()->getFloorPosition( mapx, mapy ) ) {
			int y = 0;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			scourge->getMap()->setGroundHeight( xx, yy, ( lower ? 0 : Util::roll( 7, 10 ) ) );
		}
		if ( scourge->getMap()->getFloorPosition( mapx, mapy + MAP_UNIT + MAP_UNIT ) ) {
			int y = MAP_UNIT - 1;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			scourge->getMap()->setGroundHeight( xx, yy, ( lower ? 0 : Util::roll( 7, 10 ) ) );
		}
	}
	for ( int y = OUTDOORS_STEP; y <= MAP_UNIT - OUTDOORS_STEP; y++ ) {
		if ( scourge->getMap()->getFloorPosition( mapx - MAP_UNIT, mapy + MAP_UNIT ) ) {
			int x = 0;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			scourge->getMap()->setGroundHeight( xx, yy, ( lower ? 0 : Util::roll( 7, 10 ) ) );
		}
		if ( scourge->getMap()->getFloorPosition( mapx + MAP_UNIT, mapy + MAP_UNIT ) ) {
			int x = MAP_UNIT - 1;
			int xx = ( MAP_OFFSET + ( chunkX * MAP_UNIT ) + x ) / OUTDOORS_STEP;
			int yy = ( MAP_OFFSET + ( chunkY * MAP_UNIT ) + y ) / OUTDOORS_STEP;
			scourge->getMap()->setGroundHeight( xx, yy, ( lower ? 0 : Util::roll( 7, 10 ) ) );
		}
	}
}

void MapEditor::removeFloor( Sint16 mapx, Sint16 mapy ) {
	scourge->getMap()->removeFloorPosition( mapx, mapy + MAP_UNIT );
}

void MapEditor::removeEWWall( Sint16 mapx, Sint16 mapy, int dir ) {
	for ( int y = 1; y <= MAP_UNIT; y++ ) {
		for ( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
			scourge->getMap()->removeLocation( mapx + 1, mapy + y, z );
		}
	}
}

void MapEditor::removeNSWall( Sint16 mapx, Sint16 mapy, int dir ) {
	for ( int x = 0; x < MAP_UNIT; x++ ) {
		for ( int z = 0; z < MAP_VIEW_HEIGHT; z++ ) {
			scourge->getMap()->removeLocation( mapx + x, mapy + MAP_UNIT_OFFSET - 1, z );
		}
	}
}

void MapEditor::blendCorners( Sint16 mapx, Sint16 mapy ) {

	if ( !( mapx >= 0 && mapx < MAP_WIDTH &&
	        mapy >= 0 && mapy < MAP_DEPTH ) )
		return;


	Map *levelMap = scourge->getMap();
	ShapePalette *pal = scourge->getShapePalette();

	// check NW corner
	if ( isShape( mapx - 1, mapy + 1, 0, "CORNER" ) &&
	        isShape( mapx, mapy - 1, 0, "CORNER" ) &&
	        !levelMap->getLocation( mapx, mapy + MAP_UNIT_OFFSET, 0 ) ) {

		bool nsWall = ( isShape( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) ? true : false );
		bool nsWallExtra = ( isShape( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) ? true : false );
		bool ewWall = ( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
		bool ewWallExtra = ( isShape( mapx, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL_EXTRA" ) ? true : false );

		if ( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

			levelMap->setPosition( mapx, mapy + MAP_UNIT_OFFSET, 0,
			                       pal->findShapeByName( "CORNER" ) );
			levelMap->removeLocation( mapx - 1, mapy + 1, 0 );
			levelMap->removeLocation( mapx, mapy - 1, 0 );

			// change west chunk
			if ( nsWall ) {
				levelMap->removeLocation( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
				                       pal->findShapeByName( "NS_WALL_EXTRA" ) );
			} else if ( nsWallExtra ) {
				levelMap->removeLocation( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
				                       pal->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
			}

			// change north chunk
			if ( ewWall ) {
				levelMap->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx, mapy, 0,
				                       pal->findShapeByName( "EW_WALL_EXTRA" ) );
			} else if ( ewWallExtra ) {
				levelMap->removeLocation( mapx, mapy - MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx, mapy, 0,
				                       pal->findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
			}
		}
	}

	// check NE corner
	if ( isShape( mapx + MAP_UNIT, mapy + 1, 0, "CORNER" ) &&
	        isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - 1, 0, "CORNER" ) &&
	        !levelMap->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 ) ) {

		bool nsWall = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL" ) ? true : false );
		bool nsWallExtra = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0, "NS_WALL_EXTRA" ) ? true : false );
		bool ewWall = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
		bool ewWallExtra = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0, "EW_WALL_EXTRA" ) ? true : false );

		if ( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

			levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0,
			                       pal->findShapeByName( "CORNER" ) );
			levelMap->removeLocation( mapx + MAP_UNIT, mapy + 1, 0 );
			levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - 1, 0 );

			// change west chunk
			if ( nsWall ) {
				levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
				                       pal->findShapeByName( "NS_WALL_EXTRA" ) );
			} else if ( nsWallExtra ) {
				levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0,
				                       pal->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
			}

			// change north chunk
			if ( ewWall ) {
				levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy, 0,
				                       pal->findShapeByName( "EW_WALL_EXTRA" ) );
			} else if ( ewWallExtra ) {
				levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy - MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy, 0,
				                       pal->findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
			}
		}
	}

	// check SE corner
	if ( isShape( mapx + MAP_UNIT, mapy + MAP_UNIT, 0, "CORNER" ) &&
	        isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
	        !levelMap->getLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 ) ) {

		bool nsWall = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "NS_WALL" ) ? true : false );
		bool nsWallExtra = ( isShape( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "NS_WALL_EXTRA" ) ? true : false );
		bool ewWall = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
		bool ewWallExtra = ( isShape( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT, 0, "EW_WALL_EXTRA" ) ? true : false );

		if ( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

			levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0,
			                       pal->findShapeByName( "CORNER" ) );
			levelMap->removeLocation( mapx + MAP_UNIT, mapy + MAP_UNIT_OFFSET, 0 );
			levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );

			// change west chunk
			if ( nsWall ) {
				levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
				levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT, 0,
				                       pal->findShapeByName( "NS_WALL_EXTRA" ) );
			} else if ( nsWallExtra ) {
				levelMap->removeLocation( mapx + MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
				levelMap->setPosition( mapx + MAP_UNIT, mapy + MAP_UNIT, 0,
				                       pal->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
			}

			// change north chunk
			if ( ewWall ) {
				levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
				                       pal->findShapeByName( "EW_WALL_EXTRA" ) );
			} else if ( ewWallExtra ) {
				levelMap->removeLocation( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT, 0 );
				levelMap->setPosition( mapx + MAP_UNIT - MAP_UNIT_OFFSET, mapy + MAP_UNIT + MAP_UNIT, 0,
				                       pal->findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
			}
		}
	}

	// check SW corner
	if ( isShape( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "CORNER" ) &&
	        isShape( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0, "CORNER" ) &&
	        !levelMap->getLocation( mapx, mapy + MAP_UNIT, 0 ) ) {

		bool nsWall = ( isShape( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0, "NS_WALL" ) ? true : false );
		bool nsWallExtra = ( isShape( mapx - MAP_UNIT, mapy + MAP_UNIT, 0, "NS_WALL_EXTRA" ) ? true : false );
		bool ewWall = ( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0, "EW_WALL" ) ? true : false );
		bool ewWallExtra = ( isShape( mapx, mapy + MAP_UNIT + MAP_UNIT, 0, "EW_WALL_EXTRA" ) ? true : false );

		if ( ( nsWall || nsWallExtra ) && ( ewWall || ewWallExtra ) ) {

			levelMap->setPosition( mapx, mapy + MAP_UNIT, 0,
			                       pal->findShapeByName( "CORNER" ) );
			levelMap->removeLocation( mapx - MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
			levelMap->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT_OFFSET, 0 );

			// change west chunk
			if ( nsWall ) {
				levelMap->removeLocation( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0 );
				levelMap->setPosition( mapx - MAP_UNIT + MAP_UNIT_OFFSET, mapy + MAP_UNIT, 0,
				                       pal->findShapeByName( "NS_WALL_EXTRA" ) );
			} else if ( nsWallExtra ) {
				levelMap->removeLocation( mapx - MAP_UNIT, mapy + MAP_UNIT, 0 );
				levelMap->setPosition( mapx - MAP_UNIT, mapy + MAP_UNIT, 0,
				                       pal->findShapeByName( "NS_WALL_TWO_EXTRAS" ) );
			}

			// change north chunk
			if ( ewWall ) {
				levelMap->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0 );
				levelMap->setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT - MAP_UNIT_OFFSET, 0,
				                       pal->findShapeByName( "EW_WALL_EXTRA" ) );
			} else if ( ewWallExtra ) {
				levelMap->removeLocation( mapx, mapy + MAP_UNIT + MAP_UNIT, 0 );
				levelMap->setPosition( mapx, mapy + MAP_UNIT + MAP_UNIT, 0,
				                       pal->findShapeByName( "EW_WALL_TWO_EXTRAS" ) );
			}
		}
	}
}

bool MapEditor::isShape( Sint16 mapx, Sint16 mapy, Sint16 mapz, const char *name ) {
//  cerr << "\ttesting map=" << mapx << "," << mapy << " looking for " << name;
	if ( mapx >= 0 && mapx < MAP_WIDTH &&
	        mapy >= 0 && mapy < MAP_DEPTH &&
	        mapz >= 0 && mapz < MAP_VIEW_HEIGHT ) {
		Location *pos = scourge->getMap()->getLocation( mapx, mapy, mapz );
//    cerr << " found=" << ( !pos ? "NULL" : pos->shape->getName() ) << endl;
		return( pos && pos->shape && !strcmp( pos->shape->getName(), name ) );
	} else {
//    cerr << " found nothing." << endl;
		return false;
	}
}

