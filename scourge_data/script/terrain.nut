// =============================================================
// This squirrel file contains the code called during terrain generation.
// Functions here are not story-related and can be used anytime for map building
//

/**
 * Add some objects to a village road. This method is called by the outdoor terrain generation code.
 * 
 * The following global variables will exist:
 * villageX, villageY - upper left of area where village is
 * villageWidth, villageHeight - width and height of village area
 * villageRoadX, villageRoadY - x and y of two village roads (running entire width and height of village)
 */
function villageRoads() {
//	print( "villageX=" + villageX + " villageY=" + villageY + "\n" );
//	print( "villageWidth=" + villageWidth + " villageHeight=" + villageWidth + "\n" );
//	print( "villageRoadX=" + villageRoadX + " villageRoadY=" + villageRoadY + "\n" );
	
	containers <- [ ["Barrel", "BARREL"], ["Crate", "CRATE"] ];
	
	i <- 0;
	c <- 0;
	last <- 0;
	// update the vertical road: add lamps and containers
	for( i = villageY; i < villageY + villageHeight; i+=4 ) {
		if( i < villageRoadY || i > villageRoadY + 16 ) {
			if( i - last > 6 && 0 == ( rand() * 5.0 / RAND_MAX ).tointeger() ) {
				if( scourgeGame.getMission().isFree( villageRoadX, i, 0, "STREETLIGHT" ) ) {
					scourgeGame.getMission().setMapPosition( villageRoadX, i, 0, "STREETLIGHT" );
					last = i;
				}
			} else if( 0 == ( rand() * 8.0 / RAND_MAX ).tointeger() ) {
				if( scourgeGame.getMission().isFree( villageRoadX, i, 0, "PEDESTAL" ) ) {
					scourgeGame.getMission().setMapPosition( villageRoadX, i, 0, "PEDESTAL" );
				}
			} else if( 0 == ( rand() * 5.0 / RAND_MAX ).tointeger() ) {
				c = ( rand() * containers.len().tofloat() / RAND_MAX ).tointeger();
				ix <- ( 0 == ( rand() * 2.0 / RAND_MAX ).tointeger() ? villageRoadX + 4 : villageRoadX + 9 );
				if( scourgeGame.getMission().isFree( ix, i, 0, containers[c][1] ) ) {
					scourgeGame.getMission().addItem( containers[c][0], ix, i, 0, true );
				}
			}
		}
	}
	
	// update the horizontal road: add benches and containers
	last = 0;
	for( i = villageX; i < villageX + villageWidth; i+=4 ) {
		if( i < villageRoadX || i > villageRoadX + 16 ) {
			if( i - last > 16 && 0 == ( rand() * 5.0 / RAND_MAX ).tointeger() ) {
				iy <- ( 0 == ( rand() * 2.0 / RAND_MAX ).tointeger() ? villageRoadY + 7 : villageRoadY + 13 );
				if( scourgeGame.getMission().isFree( i, iy, 0, "BENCH" ) ) {
					scourgeGame.getMission().setMapPosition( i, iy, 0, "BENCH" );
					last = i;
				}
			} else if( 0 == ( rand() * 8.0 / RAND_MAX ).tointeger() ) {
				iy <- ( 0 == ( rand() * 2.0 / RAND_MAX ).tointeger() ? villageRoadY + 8 : villageRoadY + 13 );
				if( scourgeGame.getMission().isFree( i, iy, 0, "PEDESTAL" ) ) {
					scourgeGame.getMission().setMapPosition( i, iy, 0, "PEDESTAL" );
				}				
			} else if( 0 == ( rand() * 5.0 / RAND_MAX ).tointeger() ) {
				c = ( rand() * containers.len().tofloat() / RAND_MAX ).tointeger()
				iy <- ( 0 == ( rand() * 2.0 / RAND_MAX ).tointeger() ? villageRoadY + 8 : villageRoadY + 13 );
				if( scourgeGame.getMission().isFree( i, iy, 0, containers[c][1] ) ) {
					scourgeGame.getMission().addItem( containers[c][0], i, iy, 0, true );
				}
			} else if( 0 == ( rand() * 8.0 / RAND_MAX ).tointeger() ) {
				iy <- ( 0 == ( rand() * 2.0 / RAND_MAX ).tointeger() ? villageRoadY + 8 : villageRoadY + 13 );
				if( scourgeGame.getMission().isFree( i, iy, 0, "ANVIL" ) ) {
					scourgeGame.getMission().setMapPosition( i, iy, 0, "ANVIL" );
				}
			}		
		}
	}
}

/**
 * Return a random npc type (name, really) to use for a village population.
 * This method is called repeatedly by the outdoor terrain generator code.
 */
function getVillageNpcType() {
	// in the future, there should be human towns, dwarven towns, etc.
	npcs <- [ "Male Vagrant", "Male Traveling Wizard", "Male Knight", "Male Rogue", 
	          "Female Hobo", "Female Bard", "Female Ranger", "Female Priestess" ];
	c = ( rand() * npcs.len().tofloat() / RAND_MAX ).tointeger();
	return npcs[ c ];
}

/**
 * Add some random shapes throughout the town. Currently it's trees only but it could
 * contain other random shapes.
 */
function villageShapes() {
	shapes <- [ "tree01", "tree02", "tree03", "tree07", "tree12", "tree13", "tree14", "tree15", "tree17", "tree20", "tree21" ];
	i <- 0;
	c <- 0;
	x <- 0;
	y <- 0;
	for( i = 0; i < 20; i++ ) {
		x = villageX + ( rand() * villageWidth.tofloat() / RAND_MAX ).tointeger();
		y = villageY + ( rand() * villageHeight.tofloat() / RAND_MAX ).tointeger();
		c = ( rand() * shapes.len().tofloat() / RAND_MAX ).tointeger();
		if( scourgeGame.getMission().isFreeOutdoors( x, y, 0, shapes[c] ) ) {
			scourgeGame.getMission().setMapPosition( x, y, 0, shapes[c] );
		}
	}
}

MAP_UNIT <- 16
function drawHouse( x, y, w, h ) {
	if( w >= 3 * MAP_UNIT ) {
		drawHouse_3x2( x, y, w, h );
	} else if( h >= 3 * MAP_UNIT ) {
		drawHouse_2x3( x, y, w, h );
	} else if( w == 2 * MAP_UNIT ) {
		drawHouse_2x2( x, y, w, h );
	}
}

function drawHouse_2x3( x, y, w, h ) {
	xx <- x + 5;
	yy <- y + h - 4;
	if( scourgeGame.getMission().isFree( xx, yy, 0, "HOUSE_3_BASE" ) ) {
		//scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, "HOUSE_1_BASE" );
		scourgeGame.getMission().setMapPosition( xx, yy, 0, "HOUSE_3_BASE_1" );
		scourgeGame.getMission().setMapPosition( xx + 19, yy, 0, "HOUSE_3_BASE_2" );
		scourgeGame.getMission().setMapPosition( xx + 3, yy - 29, 0, "HOUSE_3_BASE_3" );
		scourgeGame.getMission().setMapPosition( xx + 11, yy, 0, "HOUSE_3_BASE_4" );
		
		// the door
		scourgeGame.getMission().setMapPosition( xx + 4, yy - 1, 0, "NS_DOOR" );
		
		// the top
		scourgeGame.getMission().setMapPosition( xx - 5, yy + 1, 12, "HOUSE_3_TOP" );
		
		// add some objects
		scourgeGame.getMission().setMapPosition( xx + 14, yy - 22, 0, "BED" );
		scourgeGame.getMission().setMapPosition( xx + 4, yy - 20, 0, "TABLE" );
		scourgeGame.getMission().setMapPosition( xx + 6, yy - 26 0, "CHAIR" );
		scourgeGame.getMission().setMapPosition( xx + 7, yy - 17, 0, "CHAIR" );
		scourgeGame.getMission().setMapPosition( xx + 16, yy - 3, 0, "STOVE" );
		
		// tell the terrain generator where to populate the room with containers
		scourgeGame.addRoom( xx + 1, yy - 31, 20, 30 );
	}
}

function drawHouse_3x2( x, y, w, h ) {
	if( scourgeGame.getMission().isFree( x + 2, y + h - 4, 0, "HOUSE_1_BASE" ) ) {
		//scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, "HOUSE_1_BASE" );
		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, "HOUSE_1_BASE_1" );
		scourgeGame.getMission().setMapPosition( x + 2 + 6, y + h - 4, 0, "HOUSE_1_BASE_2" );
		scourgeGame.getMission().setMapPosition( x + 2 + 36, y + h - 4, 0, "HOUSE_1_BASE_3" );
		scourgeGame.getMission().setMapPosition( x + 2 + 16, y + h - 4 - 19, 0, "HOUSE_1_BASE_4" );
		scourgeGame.getMission().setMapPosition( x + 2 + 6, y + h - 4 - 19, 0, "HOUSE_1_BASE_5" );
		
		// the door
		scourgeGame.getMission().setMapPosition( x + 2 + 10, y + h - 4 - 22, 0, "NS_DOOR" );
		
		// the top
		scourgeGame.getMission().setMapPosition( x + 2 - 3, y + h - 4, 10, "HOUSE_1_TOP" );
		
		// add some objects
		scourgeGame.getMission().setMapPosition( x + 2 + 31, y + h - 10, 0, "BED" );
		scourgeGame.getMission().setMapPosition( x + 2 + 12, y + h - 14, 0, "TABLE" );
		scourgeGame.getMission().setMapPosition( x + 2 + 14, y + h - 12, 0, "CHAIR" );
		scourgeGame.getMission().setMapPosition( x + 2 + 10, y + h - 16, 0, "CHAIR" );
		scourgeGame.getMission().setMapPosition( x + 2 + 27, y + h - 10, 0, "STOVE" );
		
		// tell the terrain generator where to populate the room with containers
		scourgeGame.addRoom( x + 6, y + h - 4 - 21, 34, 17 );
	}
}

function drawHouse_2x2( x, y, w, h ) {
	if( scourgeGame.getMission().isFree( x + 2, y + h - 4, 0, "HOUSE_2_BASE" ) ) {
		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4 + 2, 0, "HOUSE_2_BASE_1" );
		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4 - 16, 0, "HOUSE_2_BASE_2" );
		scourgeGame.getMission().setMapPosition( x + 2 + 2 + 16, y + h - 4 + 2, 0, "HOUSE_2_BASE_3" );
		scourgeGame.getMission().setMapPosition( x + 2 + 2, y + h - 4 - 18, 0, "HOUSE_2_BASE_4" );
		scourgeGame.getMission().setMapPosition( x + 2 + 2, y + h - 4 + 2, 0, "HOUSE_2_BASE_4" );
		
		// the door
		scourgeGame.getMission().setMapPosition( x + 2 + 1, y + h - 4 - 10, 0, "EW_DOOR" );
		
		// the top
		scourgeGame.getMission().setMapPosition( x + 2 - 2, y + h - 4 + 4, 12, "HOUSE_2_TOP" );
		
		// add some objects
		scourgeGame.getMission().setMapPosition( x + 2 + 2, y + h - 4, 0, "BED" );
		scourgeGame.getMission().setMapPosition( x + 2 + 8, y + h - 4 - 7, 0, "TABLE" );
		scourgeGame.getMission().setMapPosition( x + 2 + 10, y + h - 4 - 4, 0, "CHAIR" );
		//scourgeGame.getMission().setMapPosition( x + 2 + 5, y + h - 4 - 8, 0, "CHAIR" );
		scourgeGame.getMission().setMapPosition( x + 2 + 16, y + h - 4, 0, "STOVE" );
		
		//scourgeGame.getMission().addRug( x + 2 + 8, y + h - 4 - 8 );
		
		// tell the terrain generator where to populate the room with containers
		scourgeGame.addRoom( x + 2, y + h - 4 - 20, 20, 22 );
	}
}
