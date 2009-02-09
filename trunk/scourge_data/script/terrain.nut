// =============================================================
// This squirrel file contains the code called during terrain generation.
// Functions here are not story-related and can be used anytime for map building
//

MAP_UNIT <- 16;
MAP_OFFSET <- 80;

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

function addRug( x, y, w, h ) {
	rx <- ( x - MAP_OFFSET ) / MAP_UNIT + ( rand() * w.tofloat() / RAND_MAX ).tointeger();
	ry <- ( y - MAP_OFFSET ) / MAP_UNIT - 1 - ( rand() * h.tofloat() / RAND_MAX ).tointeger();
	scourgeGame.getMission().setRug( rx, ry );
}

function addItemOnFloorAtWall( x, y, w, h, shape, item_name, shape_width, shape_height, ns, ew, is_item, is_container ) {
	obj_pos <- [];
	// isFree also tests that shapes don't cover doors
	if( ew ) {
		for( rx <- x + 2; rx < x + MAP_UNIT * w - 2; rx++ ) {
			if( scourgeGame.getMission().isFree( rx, y - MAP_UNIT * h + 2 + shape_height, 0, shape ) ) {
				obj_pos.append( [ rx, y - MAP_UNIT * h + 2 + shape_height ] );
			}
			if( scourgeGame.getMission().isFree( rx, y - 2, 0, shape ) ) {
				obj_pos.append( [ rx, y - 2 ] );
			}
		}
	}
	if( ns ) {
		for( ry <- y - 2; ry >= y - MAP_UNIT * h + 2 + shape_height; ry-- ) {
			if( scourgeGame.getMission().isFree( x + 2, ry, 0, shape ) ) {
				obj_pos.append( [ x + 2, ry ] );
			}
			if( scourgeGame.getMission().isFree( x + MAP_UNIT * w - 2 - shape_width, ry, 0, shape ) ) {
				obj_pos.append( [ x + MAP_UNIT * w - 2 - shape_width, ry ] );
			}
		}
	}
	if( obj_pos.len() > 0 ) {
		p <- ( rand() * obj_pos.len().tofloat() / RAND_MAX ).tointeger();
		if( is_item ) {
			scourgeGame.getMission().addItem( item_name, obj_pos[p][0], obj_pos[p][1], 0, is_container );
		} else {
			scourgeGame.getMission().setMapPosition( obj_pos[p][0], obj_pos[p][1], 0, shape );
		}
	}	
}

function addItemOnFloor( x, y, w, h, shape, item_name, shape_width, shape_height, is_item, is_container ) {
	obj_pos <- [];
	// isFree also tests that shapes don't cover doors
	for( rx <- x + 2; rx < x + MAP_UNIT * w - 2; rx++ ) {
		for( ry <- y - 2; ry >= y - MAP_UNIT * h + 2 + shape_height; ry-- ) {
			if( scourgeGame.getMission().isFree( rx, ry, 0, shape ) ) {
				obj_pos.append( [ rx, ry ] );
			}
		}
	}
	if( obj_pos.len() > 0 ) {
		p <- ( rand() * obj_pos.len().tofloat() / RAND_MAX ).tointeger();
		if( is_item ) {
			scourgeGame.getMission().addItem( item_name, obj_pos[p][0], obj_pos[p][1], 0, is_container );
		} else {
			scourgeGame.getMission().setMapPosition( obj_pos[p][0], obj_pos[p][1], 0, shape );
		}
	}	
}

function bedroom( x, y, w, h ) {
	addRug( x, y, w, h );
	addRug( x, y, w, h );
	addItemOnFloorAtWall( x, y, w, h, "BED", "Bed", 5, 7, true, true, true, false );
	for( i <- 0; i < 2; i++ ) {
		addItemOnFloorAtWall( x, y, w, h, "BOOKSHELF", "Bookshelf", 2, 5, true, false, true, true );
		addItemOnFloorAtWall( x, y, w, h, "CHEST", "Chest", 2, 3, true, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "VASE", "Vase", 2, 2, true, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "PLANT", "Large potted plant", 2, 2, true, true, true, false );		
		addItemOnFloorAtWall( x, y, w, h, "BOOKSHELF2", "Bookshelf2", 5, 2, false, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "CHEST2", "Chest2", 3, 2, false, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "COMMODE", "Commode", 5, 3, false, true, true, true );
	}
	if( w >= 2 && h >= 2 ) {
		addItemOnFloor( x, y, w, h, "TABLE", "Table", 5, 5, true, false );
		for( i <- 0; i < 3; i++ ) {
			addItemOnFloor( x, y, w, h, "CHAIR", "Chair", 2, 2, true, false );
		}
	}
}

function storage( x, y, w, h ) {
	addRug( x, y, w, h );
	for( i <- 0; i < 2; i++ ) {
		addItemOnFloorAtWall( x, y, w, h, "CRATE", "Crate", 3, 3, true, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "BARREL", "Barrel", 3, 3, true, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "VASE", "Vase", 2, 2, true, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "CHEST", "Chest", 2, 3, true, false, true, true );
		addItemOnFloorAtWall( x, y, w, h, "CHEST2", "Chest2", 3, 2, false, true, true, true );
		addItemOnFloorAtWall( x, y, w, h, "COMMODE", "Commode", 5, 3, false, true, true, true );
	}
}

room_functions <- [ bedroom, storage ];
function furnishArea( x, y, w, h ) {
	n <- ( rand() * room_functions.len().tofloat() / RAND_MAX ).tointeger();
	room_functions[ n ].call( this, x, y, w, h );
}

// draw parts of a house
// NOTE: make sure roof-s are placed on the map, after walls! (so they're drawn together)
function drawHousePart( postfix, roof_postfix, x, y, w, h, angle, furnish ) {
	//print( "postfix=" + postfix + " roof=" + roof_postfix + "\n" );
	i <- 0;
	t <- 0;
	if( w == 1 && h == 3 && angle == 0 ) {
		scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x3" + postfix );
		scourgeGame.getMission().setMapPosition( x + 1, y - 5, 0, "EW_DOOR" );
		scourgeGame.getMission().setMapPosition( x + 14, y - 37, 0, "EW_DOOR" );
		scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - ( 2 * MAP_UNIT ), "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - ( 3 * MAP_UNIT ), "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_1x3" + roof_postfix );
	} else if( w == 2 && h == 2 && angle == 0 ) {
		scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_2x2" + postfix );
		scourgeGame.getMission().setMapPosition( x + 21, y - 1, 0, "NS_DOOR" );
		scourgeGame.getMission().setMapPosition( x + 5, y - 30, 0, "NS_DOOR" );
		scourgeGame.getMission().setMapFloorPosition( x,            y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x,            y - 2 * MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - 2 * MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_2x2" + roof_postfix );
	} else if( w == 2 && h == 1 ) {
		switch( angle ) {	
		case 0: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_2x1" + postfix );
			scourgeGame.getMission().setMapPosition( x + 5, y - 14, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_2x1" + roof_postfix );
			break;
		case 180: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_2x1_180" + postfix );
			scourgeGame.getMission().setMapPosition( x + 21, y - 1, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x + 2, y + 2, 12, "P_ROOF_2x1_180" + roof_postfix );
			break;
		}		
	} else if( w == 1 && h == 1 ) {
		switch( angle ) {
		case 0: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1" + postfix );
			scourgeGame.getMission().setMapPosition( x + 5, y - 14, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_1x1" + roof_postfix );
			break;
		case 90: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1_90" + postfix );
			scourgeGame.getMission().setMapPosition( x + 1, y - 5, 0, "EW_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y - 2, 12, "P_ROOF_1x1_90" + roof_postfix );
			break;
		case 180: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1_180" + postfix );
			scourgeGame.getMission().setMapPosition( x + 5, y - 1, 0, "NS_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x + 2, y + 2, 12, "P_ROOF_1x1_180" + roof_postfix );
			break;
		case 270: 
			scourgeGame.getMission().setMapPosition( x, y, 0, "P_BASE_1x1_270" + postfix );
			scourgeGame.getMission().setMapPosition( x + 14, y - 5, 0, "EW_DOOR" );
			scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "FLOOR_TILE" );
			scourgeGame.getMission().setMapPosition( x - 2, y + 2, 12, "P_ROOF_1x1_270" + roof_postfix );
			break;
		}
	} else {
		print( "Unknown shape: w=" + w.tostring() + " h=" + h.tostring() + " angle=" + angle.tostring() );
	}
	if( furnish ) furnishArea( x, y, w, h );
}

HOUSE_POSTFIX <- [ "", "_WOOD" ];
function getHousePostfix() {
	house_postfix <- HOUSE_POSTFIX[ ( rand() * HOUSE_POSTFIX.len().tofloat() / RAND_MAX ).tointeger() ];
	//print( "Using house type: " + house_postfix + "\n" );
	return house_postfix;
}

ROOF_POSTFIX <- [ "", "_SLATE", "_RED" ];
function getRoofPostfix() {
	roof_postfix <- ROOF_POSTFIX[ ( rand() * ROOF_POSTFIX.len().tofloat() / RAND_MAX ).tointeger() ];
	//print( "Using roof type: " + roof_postfix + "\n" );
	return roof_postfix;
}

function drawHouseZ( x, y, furnish ) {
	postfix <- getHousePostfix();
	roof_postfix <- getRoofPostfix();
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x, y + MAP_UNIT * 2, 1, 1, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y + MAP_UNIT * 2, 1, 3, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT * 2, y, 1, 1, 180, furnish );
	scourgeGame.getMission().endHouse();
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT * 2, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 9, y + MAP_UNIT * 2 - 10, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 2, y - 4, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 5, y + 4, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y + 6, 0, "FENCE_90" );
	scourgeGame.getMission().setMapPosition( x + 2, y + MAP_UNIT, 0, "FENCE_90" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2, y + MAP_UNIT * 2 - 2, 0, "FENCE" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 6, y + MAP_UNIT * 2 - 2, 0, "FENCE" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT - 6, y - MAP_UNIT + 2, 0, "FENCE" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT - 12, y - MAP_UNIT + 2, 0, "FENCE" );
}

function drawHouseL( x, y, furnish ) {
	postfix <- getHousePostfix();
	roof_postfix <- getRoofPostfix();
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x, y + 2 * MAP_UNIT, 1, 3, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + 1 * MAP_UNIT, y, 2, 1, 180, furnish );
	scourgeGame.getMission().endHouse();
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 5, y + MAP_UNIT - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 8, y + MAP_UNIT * 2 - 8, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 19, y + MAP_UNIT * 2 - 12, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 30, y + MAP_UNIT * 2 - 8, 0, getTree() );
	for( fx <- 0; fx < 5; fx++ ) {
		scourgeGame.getMission().setMapPosition( x + MAP_UNIT + ( fx * 6 ), y + MAP_UNIT * 2 - 2, 0, "FENCE" );
	}
	for( fy <- 0; fy < 1; fy++ ) {
		scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y + ( ( fy + 1 ) * 6 ), 0, "FENCE_90" );
	}
}

function drawHouseL2( x, y, furnish ) {
	postfix <- getHousePostfix();
	roof_postfix <- getRoofPostfix();
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x + 2 * MAP_UNIT, y + 2 * MAP_UNIT, 1, 3, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x, y + 2 * MAP_UNIT, 2, 1, 0, furnish );
	scourgeGame.getMission().endHouse();
	scourgeGame.getMission().setMapPosition( x, y - 4, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 11, y, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 22, y - 4, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 23, y + 11, 0, getTree() );
	for( fx <- 0; fx < 5; fx++ ) {
		scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 - ( ( fx + 1 ) * 6 ), y - MAP_UNIT + 2, 0, "FENCE" );
	}
	for( fy <- 0; fy < 1; fy++ ) {
		scourgeGame.getMission().setMapPosition( x + 2, y + MAP_UNIT, 0, "FENCE_90" );
	}
}

function drawHouseSquare( x, y, furnish ) {
	postfix <- getHousePostfix();
	roof_postfix <- getRoofPostfix();
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y + 2 * MAP_UNIT, 2, 2, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y, 1, 1, 90, furnish );
	drawHousePart( postfix, roof_postfix,  x, y, 1, 1, 0, furnish );
	scourgeGame.getMission().endHouse();
	scourgeGame.getMission().setMapPosition( x, y + MAP_UNIT - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 7, y + MAP_UNIT * 2, 0, getTree() );	
}

function drawHouseSquare2( x, y, furnish ) {
	postfix <- getHousePostfix();
	roof_postfix <- getRoofPostfix();
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x, y + 1 * MAP_UNIT, 2, 2, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y + 2 * MAP_UNIT, 1, 1, 270, furnish );
	drawHousePart( postfix, roof_postfix,  x + 2 * MAP_UNIT, y + 2 * MAP_UNIT, 1, 1, 180, furnish );
	scourgeGame.getMission().endHouse();
	scourgeGame.getMission().setMapPosition( x, y + MAP_UNIT * 2 - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT - 7, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 5, y - 10, 0, getTree() );
}

house_functions <- [ drawHouseZ, drawHouseL, drawHouseL2, drawHouseSquare, drawHouseSquare2 ];
function drawRandomHouse( x, y ) {
	n <- ( rand() * house_functions.len().tofloat() / RAND_MAX ).tointeger();
	house_functions[ n ].call( this, x, y, true );
}

armor <- [ "Horned helmet", "Leather Work Gloves", "Adventuring Hat", "Wooden Shield" ];
function getRandomArmorItem() {
	c <- ( rand() * armor.len().tofloat() / RAND_MAX ).tointeger();
	return armor[c];
}

weapon <- [ "Kitchen Knife", "Dagger", "Ornamental officer sword", "Battleaxe", "Smallbow", "Rounded mace" ];
function getRandomWeaponItem() {
	c <- ( rand() * weapon.len().tofloat() / RAND_MAX ).tointeger();
	return weapon[c];
}

inn <- [ "Beer barrel", "Wine barrel", "Fine wine bottle", "Wine bottle", "Milk", "Apple", "Bread", "Mutton meat", "Chalice" ];
function getRandomInnItem() {
	c <- ( rand() * inn.len().tofloat() / RAND_MAX ).tointeger();
	return inn[c];
}

function createTable( x, y ) {
	scourgeGame.getMission().addItem( "Table", x, y, 0, false );
	xx <- mixHouseCoordinates( [ 0, 1, 2 ] );
	yy <- mixHouseCoordinates( [ 0, 1, 2 ] );
	n <- ( rand() * 2.0 / RAND_MAX ).tointeger() + 1;
	for( tt <- 0; tt < n; tt++ ) {
		scourgeGame.getMission().addItem( getRandomInnItem(), x + 1 + xx[tt], y - 1 - yy[tt], 3, true );
	}
}

function drawArmorShop( x, y ) {
	print("Making armor shop!\n");
	drawHouseZ( x, y, false );
	scourgeGame.getMission().setMapPosition( x + 3, y + MAP_UNIT, 7, "SIGN_ARMOR" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 12, y + 4, 7, "SIGN_ARMOR_180" );
	
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 6, y + MAP_UNIT, 0, "BAR_90" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 6, y + MAP_UNIT - 6, 0, "BAR_90" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT + 6, y + MAP_UNIT - 12, 0, "BAR_90" );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT + 2, y + MAP_UNIT + 2, 0, true );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT + 5, y + MAP_UNIT + 3, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + MAP_UNIT + 2, y, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + MAP_UNIT + 4, y - 1, 0, true );
	
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 6, y + MAP_UNIT - 2, 4, false );
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 6, y + MAP_UNIT - 8, 4, false );
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 6, y + MAP_UNIT - 14, 4, false );
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 2, y + 4, 0, true );
	
	// add the shop-keeper
	scourgeGame.getMission().addCreature( x + MAP_UNIT + 3, y + 10, 0, getVillageNpcType() );
	// now make this person a trader of armor	
}

function drawWeaponShop( x, y ) {
	print("Making weapon shop!\n");
	drawHouseL( x, y, false );
	scourgeGame.getMission().setMapPosition( x - 4, y + MAP_UNIT * 2 - 3, 7, "SIGN_WEAPON_90" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 12, y + 4, 7, "SIGN_WEAPON_180" );
	
	scourgeGame.getMission().setMapPosition( x + 6, y + MAP_UNIT, 0, "BAR_90" );
	scourgeGame.getMission().setMapPosition( x + 6, y + MAP_UNIT - 6, 0, "BAR_90" );
	scourgeGame.getMission().setMapPosition( x + 6, y + MAP_UNIT - 12, 0, "BAR_90" );
	scourgeGame.getMission().addItem( "Barrel", x + 2, y + MAP_UNIT + 2, 0, true );
	scourgeGame.getMission().addItem( "Barrel", x + 5, y + MAP_UNIT + 3, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + 2, y, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + 4, y - 1, 0, true );
	
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 6, y + MAP_UNIT - 2, 4, false );
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 6, y + MAP_UNIT - 8, 4, false );
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 6, y + MAP_UNIT - 14, 4, false );
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 2, y + 4, 0, true );
	
	// add the shop-keeper
	scourgeGame.getMission().addCreature( x + 3, y + 10, 0, getVillageNpcType() );
	// now make this person a trader of weapons
	
}

function drawInn( x, y ) {
	print("Making inn!\n");
	drawHouseSquare( x, y, false );
	scourgeGame.getMission().setMapPosition( x + 2, y - MAP_UNIT, 7, "SIGN_INN" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 12, y + MAP_UNIT * 2 + 4, 7, "SIGN_INN_180" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 8, y + MAP_UNIT, 0, "BAR_270" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 8, y + MAP_UNIT + 6, 0, "BAR_270" );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT * 2 + 10, y + MAP_UNIT - 6, 0, true );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT * 2 + 11, y + MAP_UNIT - 9, 0, true );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT * 2 + 10, y + MAP_UNIT + 8, 0, true );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT * 2 + 11, y + MAP_UNIT + 11, 0, true );
	
	scourgeGame.getMission().addItem( getRandomInnItem(), x + MAP_UNIT * 2 + 8, y + MAP_UNIT + 4, 4, false );
	scourgeGame.getMission().addItem( getRandomInnItem(), x + MAP_UNIT * 2 + 9, y + MAP_UNIT + 6, 4, false );
	scourgeGame.getMission().addItem( getRandomInnItem(), x + MAP_UNIT * 2 + 8, y + MAP_UNIT - 3, 4, false );
	
	createTable( x + MAP_UNIT + 6, y + MAP_UNIT - 3 );
	//scourgeGame.getMission().addItem( "Table", x + MAP_UNIT + 6, y + MAP_UNIT - 3, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT + 4, y + MAP_UNIT - 4, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT + 8, y + MAP_UNIT - 1, 0, false );
	
	createTable( x + MAP_UNIT + 6, y + MAP_UNIT * 2 - 8 );
	//scourgeGame.getMission().addItem( "Table", x + MAP_UNIT + 6, y + MAP_UNIT * 2 - 8, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT + 9, y + MAP_UNIT * 2 - 13, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT + 4, y + MAP_UNIT * 2 - 9, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT + 7, y + MAP_UNIT * 2 - 6, 0, false );
	
	createTable( x + MAP_UNIT + 15, y + MAP_UNIT * 2 - 9 );
	//scourgeGame.getMission().addItem( "Table", x + MAP_UNIT + 15, y + MAP_UNIT * 2 - 9, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT * 2, y + MAP_UNIT * 2 - 7, 0, false );	
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT * 2 + 2, y + MAP_UNIT * 2 - 14, 0, false );
	
	createTable( x + MAP_UNIT * 2, y + MAP_UNIT - 4 );
	//scourgeGame.getMission().addItem( "Table", x + MAP_UNIT * 2, y + MAP_UNIT - 4, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT * 2 + 3, y + MAP_UNIT - 2, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT * 2 + 5, y + MAP_UNIT - 7, 0, false );
	scourgeGame.getMission().addItem( "Chair", x + MAP_UNIT * 2 + 1, y + MAP_UNIT - 9, 0, false );
	
	// add the shop-keeper
	scourgeGame.getMission().addCreature( x + MAP_UNIT * 2 + 10, y + MAP_UNIT + 3, 0, getVillageNpcType() );
	// now make this person an inn-keeper
}

function drawFence( x, y ) {
	scourgeGame.getMission().setMapPosition( x, y - MAP_UNIT + 2, 0, "FENCE_CORNER" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y - MAP_UNIT + 2, 0, "FENCE_CORNER_90" );
	scourgeGame.getMission().setMapPosition( x, y + MAP_UNIT * 2, 0, "FENCE_CORNER_270" );
	scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT * 2, 0, "FENCE_CORNER_180" );
	for( ix <- 0; ix < 3; ix++ ) {
		scourgeGame.getMission().setMapPosition( x + 2 + ix * 6, y - MAP_UNIT + 1, 0, "FENCE" );
		scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 8 - ( ix * 6 ), y - MAP_UNIT + 1, 0, "FENCE" );
		scourgeGame.getMission().setMapPosition( x + 2 + ix * 6, y + MAP_UNIT * 2, 0, "FENCE" );		
		scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 8 - ( ix * 6 ), y + MAP_UNIT * 2, 0, "FENCE" );
		scourgeGame.getMission().setMapPosition( x, y - MAP_UNIT + 8 + ix * 6, 0, "FENCE_90" );
		scourgeGame.getMission().setMapPosition( x, y + MAP_UNIT * 2 - 2 - ix * 6, 0, "FENCE_90" );
		scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 15, y - MAP_UNIT + 8 + ix * 6, 0, "FENCE_90" );
		scourgeGame.getMission().setMapPosition( x + MAP_UNIT * 2 + 15, y + MAP_UNIT * 2 - 2 - ix * 6, 0, "FENCE_90" );
	}
}

function drawMarket( x, y ) {
	print("Making market!\n");
	drawFence( x, y );
}

function drawGarden( x, y ) {
	print("Making garden!\n");
	drawFence( x, y );
	scourgeGame.getMission().addItem( "Bench", x + 4, y - MAP_UNIT + 3, 0, false );
	scourgeGame.getMission().addItem( "Bench", x + MAP_UNIT * 2, y - MAP_UNIT + 3, 0, false );
	scourgeGame.getMission().addItem( "Bench", x + 4, y + MAP_UNIT * 2 - 1, 0, false );
		scourgeGame.getMission().addItem( "Bench", x + MAP_UNIT * 2, y + MAP_UNIT * 2 - 1, 0, false );
	scourgeGame.getMission().setMapPosition( x + 10, y - MAP_UNIT + 14, 0, getTree() );			
	scourgeGame.getMission().setMapPosition( x + 36, y - MAP_UNIT + 14, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 10, y - MAP_UNIT + 38, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 36, y - MAP_UNIT + 38, 0, getTree() );
	scourgeGame.getMission().setMapPosition( x + 23, y - MAP_UNIT + 20, 0, "RUINS" );
	scourgeGame.getMission().setMapPosition( x + 22, y - MAP_UNIT + 28, 0, "POOL" );
	scourgeGame.getMission().setMapPosition( x + 23, y - MAP_UNIT + 34, 0, "RUINS2" );
}

function drawVillage( x, y, village_width, village_height ) {
	scourgeGame.getMission().clearHouses();
	
	vx <- 0;
	vy <- 0;
	xp <- 0;
	yp <- 0;
	
	pos <- mixHouseCoordinates( [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 ] );
	for( h <- 0; h < 16; h++ ) {
		vx = pos[h] % 4;
		vy = pos[h] / 4;
		
		xp = x + vx * 4 * MAP_UNIT;
		yp = y + (( vy * 4 ) + 1 ) * MAP_UNIT;
		
		if( h == 0 ) {
			print( "amor shop at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawArmorShop( xp, yp )
		} else if( h == 1 ) {
			print( "weapon shop at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawWeaponShop( xp, yp )
		} else if( h == 2 ) {
			print( "inn at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawInn( xp, yp )
		} else if( h == 3 ) {
			print( "market at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawMarket( xp, yp );
		} else if( h == 4 ) {
			print( "garden at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawGarden( xp, yp );			
		} else {
			drawRandomHouse( xp, yp );
		}
	}	
}

function mixHouseCoordinates( pos ) {
  tmp <- 0;
  p <- 0;
  for( im <- 0; im < pos.len(); im++ ) {
  	p = ( rand() * pos.len().tofloat() / RAND_MAX ).tointeger();
  	tmp = pos[im];
  	pos[im] = pos[p];
  	pos[p] = tmp;
  }
  
  return pos;
}

/** 
 * Called whenever a shape is added to the map.
 */
function shapeAdded( shape_name, x, y, z ) {
	if( startsWith( shape_name, "P_ROOF_2x2" ) ) {
		scourgeGame.getMission().setMapEffect( x + 8, y - 14, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( startsWith( shape_name, "P_ROOF_2x1_180" ) ) {
		scourgeGame.getMission().setMapEffect( x + 17, y - 15, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( startsWith( shape_name, "P_ROOF_2x1" ) ) {
		scourgeGame.getMission().setMapEffect( x + 11, y - 7, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( startsWith( shape_name, "P_ROOF_1x3" ) ) {
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
 * 
 * Note: in the if-s below, order matters, because of the startsWith test.
 */
function addVirtualShapes( shape_name ) {
	scourgeGame.clearVirtualShapes( shape_name );
	if( startsWith( shape_name, "P_BASE_2x2" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -30, 0, 5, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 11, -30, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 27, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 28, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 28, 12, false );
	} else if( startsWith( shape_name, "P_BASE_1x1_90" ) ) {		
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 16, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 0, -11, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 2, 5, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, 0, 0, 2, 14, 12, false );
	} else if( startsWith( shape_name, "P_BASE_1x1_180" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 11, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 16, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 14, -2, 0, 2, 12, 12, false );		
	} else if( startsWith( shape_name, "P_BASE_1x1_270" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 16, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -11, 0, 2, 5, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -2, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 14, 12, true );
	} else if( startsWith( shape_name, "P_BASE_1x1" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 5, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 11, -14, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 16, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 12, 12, false );		
	} else if( startsWith( shape_name, "P_BASE_1x3" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -46, 0, 16, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 16, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -11, 0, 2, 35, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -2, 0, 2, 35, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -43, 0, 2, 3, 12, false );
	} else if( startsWith( shape_name, "P_BASE_2x1_180" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 27, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 32, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 12, 12, false );
	} else if( startsWith( shape_name, "P_BASE_2x1" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 5, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 11, -14, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 32, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 12, 12, false );
	}
}

/**
 * Return true if a.startsWith(b)
 */
function startsWith( a, b ) {
	if( a == null || b == null ) return false;
	if( a.len() < b.len() ) return false;
	return ( a.slice( 0, b.len() ) == b );
}
