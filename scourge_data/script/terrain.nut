// =============================================================
// This squirrel file contains the code called during terrain generation.
// Functions here are not story-related and can be used anytime for map building
//

MAP_UNIT <- 16;

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

trees <- [ "tree01", "tree02", "tree03", "tree07", "tree12", "tree13", "tree14", "tree15", "tree17", "tree20", "tree21" ];
function getTree() {
	c <- ( rand() * trees.len().tofloat() / RAND_MAX ).tointeger();
	return trees[c];
}

/**
 * Add some random shapes throughout the town. Currently it's trees only but it could
 * contain other random shapes.
 */
function villageShapes() {
	i <- 0;
	x <- 0;
	y <- 0;
	for( i = 0; i < 20; i++ ) {
		x = villageX + ( rand() * villageWidth.tofloat() / RAND_MAX ).tointeger();
		y = villageY + ( rand() * villageHeight.tofloat() / RAND_MAX ).tointeger();
		tree <- getTree();
		if( scourgeGame.getMission().isFreeOutdoors( x, y, 0, tree ) ) {
			scourgeGame.getMission().setMapPosition( x, y, 0, tree );
		}
	}
}

function drawHousePart( x, y, w, h, angle ) {
	i <- 0;
	t <- 0;
	if( w == 1 && h == 3 && angle == 0 ) {
		scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x3" );
		scourgeGame.getMission().setMapPosition( x + 1, y - 5, 0, "EW_DOOR" );
		scourgeGame.getMission().setMapPosition( x + 14, y - 37, 0, "EW_DOOR" );
		scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - ( 2 * MAP_UNIT ), "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - ( 3 * MAP_UNIT ), "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_1x3" );
	} else if( w == 2 && h == 2 && angle == 0 ) {
		scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_2x2" );
		scourgeGame.getMission().setMapPosition( x + 21, y - 1, 0, "NS_DOOR" );
		scourgeGame.getMission().setMapPosition( x + 5, y - 30, 0, "NS_DOOR" );
		scourgeGame.getMission().setMapFloorPosition( x,            y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x,            y - 2 * MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - 2 * MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_2x2" );
	} else if( w == 2 && h == 1 ) {
		switch( angle ) {	
		case 0: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_2x1" );
			scourgeGame.getMission().setMapPosition( x + 5, y - 14, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_2x1" );
			break;
		case 180: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_2x1_180" );
			scourgeGame.getMission().setMapPosition( x + 21, y - 1, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x + 2, y + 2, 12, "P_ROOF_2x1_180" );
			break;
		}		
	} else if( w == 1 && h == 1 ) {
		switch( angle ) {
		case 0: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1" );
			scourgeGame.getMission().setMapPosition( x + 5, y - 14, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_1x1" );
			break;
		case 90: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1_90" );
			scourgeGame.getMission().setMapPosition( x + 1, y - 5, 0, "EW_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y - 2, 12, "P_ROOF_1x1_90" );
			break;
		case 180: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1_180" );
			scourgeGame.getMission().setMapPosition( x + 5, y - 1, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x + 2, y + 2, 12, "P_ROOF_1x1_180" );
			break;
		case 270: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1_270" );
			scourgeGame.getMission().setMapPosition( x + 14, y - 5, 0, "EW_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_1x1_270" );
			break;
		}
	} else {
		print( "Unknown shape: w=" + w.tostring() + " h=" + h.tostring() + " angle=" + angle.tostring() );
	}
}

function drawHouseZ( x, y ) {
	drawHousePart( x, y + MAP_UNIT * 2, 1, 1, 0 );
	drawHousePart( x + MAP_UNIT, y + MAP_UNIT * 2, 1, 3, 0 );
	drawHousePart( x + MAP_UNIT * 2, y, 1, 1, 180 );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT * 2, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 9, y + MAP_UNIT * 2 - 10, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 2, y - 10, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 5, y + 4, 0, getTree() );
}

function drawHouseL( x, y ) {
	drawHousePart( x, y + 2 * MAP_UNIT, 1, 3, 0 );
	drawHousePart( x + 1 * MAP_UNIT, y, 2, 1, 180 );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 5, y + MAP_UNIT - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 8, y + MAP_UNIT * 2 - 8, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 19, y + MAP_UNIT * 2 - 12, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 30, y + MAP_UNIT * 2 - 8, 0, getTree() );
}

function drawHouseL2( x, y ) {
	drawHousePart( x + 2 * MAP_UNIT, y + 2 * MAP_UNIT, 1, 3, 0 );
	drawHousePart( x, y + 2 * MAP_UNIT, 2, 1, 0 );
	scourgeGame.getMission().setMapPosition( x, y - 8, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 11, y - 2, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 22, y - 8, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 23, y + 11, 0, getTree() );
}

function drawHouseSquare( x, y ) {
	drawHousePart( x + MAP_UNIT, y + 2 * MAP_UNIT, 2, 2, 0 );
	drawHousePart( x + MAP_UNIT, y, 1, 1, 90 );
	drawHousePart( x, y, 1, 1, 0 );
	scourgeGame.getMission().setMapPosition( x, y + MAP_UNIT - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 7, y + MAP_UNIT * 2, 0, getTree() );	
}

function drawHouseSquare2( x, y ) {
	drawHousePart( x, y + 1 * MAP_UNIT, 2, 2, 0 );
	drawHousePart( x + MAP_UNIT, y + 2 * MAP_UNIT, 1, 1, 270 );
	drawHousePart( x + 2 * MAP_UNIT, y + 2 * MAP_UNIT, 1, 1, 180 );
	scourgeGame.getMission().setMapPosition( x, y + MAP_UNIT * 2 - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 5, y - 10, 0, getTree() );
}

house_functions <- [ drawHouseZ, drawHouseL, drawHouseL2, drawHouseSquare, drawHouseSquare2 ];
function drawHouseNew( x, y, w, h ) {
	n <- ( rand() * house_functions.len().tofloat() / RAND_MAX ).tointeger();
	house_functions[ n ].call( this, x, y );
}

///**
// * This method is called by the OutdoorGenerator when it needs to draw a house.
// */
//function drawHouse( x, y, w, h ) {
//	if( w >= 3 * MAP_UNIT ) {
//		drawHouse_3x2( x, y, w, h );
//	} else if( h >= 3 * MAP_UNIT ) {
//		drawHouse_2x3( x, y, w, h );
//	} else if( w == 2 * MAP_UNIT ) {
//		drawHouse_2x2( x, y, w, h );
//	}
//}
//
//function drawHouse_2x3( x, y, w, h ) {
//	xx <- x + 5;
//	yy <- y + h - 4;
//	if( scourgeGame.getMission().isFree( xx, yy, 0, "HOUSE_3_BASE" ) ) {
//		scourgeGame.getMission().setMapPosition( xx, yy, 0, "HOUSE_3_BASE" );
//		
//		// the door
//		scourgeGame.getMission().setMapPosition( xx + 4, yy - 1, 0, "NS_DOOR" );
//		
//		// the top
//		scourgeGame.getMission().setMapPosition( xx - 5, yy + 1, 12, "HOUSE_3_TOP" );
//		
//		// add some objects
//		scourgeGame.getMission().setMapPosition( xx + 14, yy - 22, 0, "BED" );
//		scourgeGame.getMission().setMapPosition( xx + 4, yy - 20, 0, "TABLE" );
//		scourgeGame.getMission().setMapPosition( xx + 6, yy - 26 0, "CHAIR" );
//		scourgeGame.getMission().setMapPosition( xx + 7, yy - 17, 0, "CHAIR" );
//		scourgeGame.getMission().setMapPosition( xx + 16, yy - 3, 0, "STOVE" );
//		
//		// tell the terrain generator where to populate the room with containers
//		scourgeGame.addRoom( xx + 1, yy - 31, 20, 30 );
//	}
//}
//
//function drawHouse_3x2( x, y, w, h ) {
//	shape_name <- "HOUSE_1_BASE"	
//	if( scourgeGame.getMission().isFree( x + 2, y + h - 4, 0, shape_name ) ) {
//		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, shape_name );
//		
//		// the door
//		scourgeGame.getMission().setMapPosition( x + 2 + 10, y + h - 4 - 22, 0, "NS_DOOR" );
//		
//		// the top
//		scourgeGame.getMission().setMapPosition( x + 2 - 3, y + h - 4, 10, "HOUSE_1_TOP" );
//		
//		// add some objects
//		scourgeGame.getMission().setMapPosition( x + 2 + 31, y + h - 10, 0, "BED" );
//		scourgeGame.getMission().setMapPosition( x + 2 + 12, y + h - 14, 0, "TABLE" );
//		scourgeGame.getMission().setMapPosition( x + 2 + 14, y + h - 12, 0, "CHAIR" );
//		scourgeGame.getMission().setMapPosition( x + 2 + 10, y + h - 16, 0, "CHAIR" );
//		scourgeGame.getMission().setMapPosition( x + 2 + 27, y + h - 10, 0, "STOVE" );
//		
//		// tell the terrain generator where to populate the room with containers
//		scourgeGame.addRoom( x + 6, y + h - 4 - 21, 34, 17 );
//	}
//}
//
//function drawHouse_2x2( x, y, w, h ) {
//	if( scourgeGame.getMission().isFree( x + 2, y + h - 4, 0, "HOUSE_2_BASE" ) ) {
//		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, "HOUSE_2_BASE" );
//		
//		// the door
//		scourgeGame.getMission().setMapPosition( x + 2 + 1, y + h - 4 - 10, 0, "EW_DOOR" );
//		
//		// the top
//		scourgeGame.getMission().setMapPosition( x + 2 - 2, y + h - 4 + 4, 12, "HOUSE_2_TOP" );
//		
//		// add some objects
//		scourgeGame.getMission().setMapPosition( x + 3 + 2, y + h - 4, 0, "BED" );
//		scourgeGame.getMission().setMapPosition( x + 3 + 10, y + h - 4 - 7, 0, "TABLE" );
//		scourgeGame.getMission().setMapPosition( x + 3 + 12, y + h - 4 - 4, 0, "CHAIR" );
//		//scourgeGame.getMission().setMapPosition( x + 2 + 5, y + h - 4 - 8, 0, "CHAIR" );
//		scourgeGame.getMission().setMapPosition( x + 3 + 15, y + h - 4, 0, "STOVE" );
//				
//		//scourgeGame.getMission().addRug( x + 2 + 8, y + h - 4 - 8 );
//		
//		// tell the terrain generator where to populate the room with containers
//		scourgeGame.addRoom( x + 2 + 1, y + h - 4 - 20, 19, 22 );
//	}
//}

/** 
 * Called whenever a shape is added to the map.
 */
function shapeAdded( shape_name, x, y, z ) {
	if( shape_name == "P_ROOF_2x2" ) {
		scourgeGame.getMission().setMapEffect( x + 8, y - 14, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( shape_name == "P_ROOF_2x1_180" ) {
		scourgeGame.getMission().setMapEffect( x + 17, y - 15, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( shape_name == "P_ROOF_2x1" ) {
		scourgeGame.getMission().setMapEffect( x + 11, y - 7, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( shape_name == "P_ROOF_1x3" ) {
		scourgeGame.getMission().setMapEffect( x + 4, y - 14, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
		scourgeGame.getMission().setMapEffect( x + 13, y - 40, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);		
	}
}

/**
 * This method is called when a shape is loaded or when squirrel scripts are reloaded.
 * It is used create extra virtual shapes to draw instead when the real shape is drawn.
 */
function addVirtualShapes( shape_name ) {
	scourgeGame.clearVirtualShapes( shape_name );
	if( shape_name == "P_BASE_2x2" ) {
		scourgeGame.addVirtualShape( shape_name, 0, -30, 0, 5, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 11, -30, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 27, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 28, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 28, 12, false );
	} else if( shape_name == "P_BASE_1x1" ) {
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 5, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 11, -14, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 16, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 12, 12, false );
	} else if( shape_name == "P_BASE_1x1_90" ) {		
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 16, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 0, -11, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 2, 5, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, 0, 0, 2, 14, 12, false );
	} else if( shape_name == "P_BASE_1x1_180" ) {
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 11, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 16, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 14, -2, 0, 2, 12, 12, false );		
	} else if( shape_name == "P_BASE_1x1_270" ) {
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 16, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -11, 0, 2, 5, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -2, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 14, 12, true );
	} else if( shape_name == "P_BASE_1x3" ) {
		scourgeGame.addVirtualShape( shape_name, 0, -46, 0, 16, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 16, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -11, 0, 2, 35, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -2, 0, 2, 35, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -43, 0, 2, 3, 12, false );
	} else if( shape_name == "P_BASE_2x1" ) {
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 5, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 11, -14, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 32, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 12, 12, false );
	} else if( shape_name == "P_BASE_2x1_180" ) {
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 27, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 32, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 12, 12, false );		
	}
	
//	if( shape_name == "HOUSE_1_BASE" ) {
//		scourgeGame.clearVirtualShapes( shape_name );
//		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 6, 25, 10, true );
//		scourgeGame.addVirtualShape( shape_name, 6, 0, 0, 30, 6, 10, false );
//		scourgeGame.addVirtualShape( shape_name, 36, 0, 0, 6, 25, 10, false );
//		scourgeGame.addVirtualShape( shape_name, 16, -19, 0, 20, 6, 10, false );
//		scourgeGame.addVirtualShape( shape_name, 6, -19, 0, 4, 6, 10, false );						
//	} else if( shape_name == "HOUSE_2_BASE" ) {
//		scourgeGame.clearVirtualShapes( shape_name );
//		scourgeGame.addVirtualShape( shape_name, 0, 2, 0, 3, 12, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 0, -16, 0, 3, 4, 12, true );
//		scourgeGame.addVirtualShape( shape_name, 19, 2, 0, 2, 22, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 3, -18, 0, 16, 2, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 3, 2, 0, 16, 2, 12, false );
//	} else if( shape_name == "HOUSE_3_BASE" ) {
//		scourgeGame.clearVirtualShapes( shape_name );
//		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 3, 32, 12, true );
//		scourgeGame.addVirtualShape( shape_name, 19, 0, 0, 3, 32, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 3, -29, 0, 16, 3, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 11, 0, 0, 8, 3, 12, false );
//	} else if( shape_name == "MONASTERY_BASE" ) {
//		scourgeGame.clearVirtualShapes( shape_name );
//		scourgeGame.addVirtualShape( shape_name, 0, -20, 0, 4, 6, 12, true );
//		scourgeGame.addVirtualShape( shape_name, 4, -24, 0, 15, 2, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 25, -24, 0, 7, 2, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 32, -20, 0, 4, 6, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 32, -6, 0, 2, 14, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 32, 0, 0, 4, 6, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 4, 6, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 4, 0, 0, 28, 2, 12, false );
//		scourgeGame.addVirtualShape( shape_name, 2, -6, 0, 2, 8, 12, false );
//	}
}
