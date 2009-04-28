// =============================================================
// This squirrel file contains the code called during terrain generation.
// Functions here are not story-related and can be used anytime for map building
//

MAP_UNIT <- 16;

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

carts <- [ "CART", "CART2" ];
function getRandomCart() {
	c <- ( rand() * carts.len().tofloat() / RAND_MAX ).tointeger();
	return carts[c];
}

merchant_types <- [ "ARMOR", "FOOD;DRINK", "SCROLL;POTION;WAND;RING;AMULET;STAFF", "SWORD;AXE;BOW;MACE;POLE" ];
function getRandomMerchantType() {
	c <- ( rand() * merchant_types.len().tofloat() / RAND_MAX ).tointeger();
	return merchant_types[c];
}

magic_schools <- [ "Confrontation", "Ambush Trickery and Deceit", "History and Lore", "Life and Death", "Divine Awareness", "Nature" ];
function getRandomMagicSchool() {
	c <- ( rand() * magic_schools.len().tofloat() / RAND_MAX ).tointeger();
	return magic_schools[c];
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
			setPosition( x, y, 0, tree, false );
		}
	}
}

function addRug( x, y, w, h ) {
	rx <- ( x ) / MAP_UNIT + ( rand() * w.tofloat() / RAND_MAX ).tointeger();
	ry <- ( y ) / MAP_UNIT - 1 - ( rand() * h.tofloat() / RAND_MAX ).tointeger();
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
			setPosition( obj_pos[p][0], obj_pos[p][1], 0, shape, false );
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
			setPosition( obj_pos[p][0], obj_pos[p][1], 0, shape, false );
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
function drawHousePart( postfix_param, roof_postfix_param, x, y, w, h, angle, furnish ) {
	//print( "pos=" + x + "," + y + " size=" + w + "," + h + " postfix=" + postfix_param + " roof=" + roof_postfix_param + "\n" );
	postfix <- postfix_param
	if( roof_postfix_param == "church" ) {
		roof_postfix <- "";
	} else {
		roof_postfix <- roof_postfix_param
	}
	i <- 0;
	t <- 0;
	if( w == 1 && h == 3 && angle == 0 ) {
		setPosition( x, y, 0, "P_BASE_1x3" + postfix, true );
		setPosition( x + 1, y - 5, 0, "EW_DOOR", true );
		setPosition( x + 14, y - 37, 0, "EW_DOOR", true );
		scourgeGame.getMission().setMapFloorPosition( x, y, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - ( 2 * MAP_UNIT ), "ROOM_FLOOR_TILE" );
		setPosition( x - 2, y + 2, 12, "P_ROOF_1x3" + roof_postfix, true );
	} else if( w == 2 && h == 2 && angle == 0 ) {
		setPosition( x, y, 0, "P_BASE_2x2" + postfix, true );
		setPosition( x + 21, y - 1, 0, "NS_DOOR", true );
		setPosition( x + 5, y - 30, 0, "NS_DOOR", true );
		scourgeGame.getMission().setMapFloorPosition( x,            y, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x,            y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
		if( roof_postfix_param == "church" ) {
			setPosition( x - 2, y + 2, 12, "P_CHURCH_2x2", true );
		} else {
			setPosition( x - 2, y + 2, 12, "P_ROOF_2x2" + roof_postfix, true );
		}
	} else if( w == 2 && h == 1 ) {
		switch( angle ) {	
		case 0: 
			setPosition( x, y, 0, "P_BASE_2x1" + postfix, true );
			setPosition( x + 5, y - 14, 0, "NS_DOOR", true );
			scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
			scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y, "FLOOR_TILE" );
			setPosition( x - 2, y + 2, 12, "P_ROOF_2x1" + roof_postfix, true );
			break;
		case 180: 
			setPosition( x, y, 0, "P_BASE_2x1_180" + postfix, true );
			setPosition( x + 21, y - 1, 0, "NS_DOOR", true );
			scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
			scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y, "FLOOR_TILE" );
			setPosition( x + 2, y + 2, 12, "P_ROOF_2x1_180" + roof_postfix, true );
			break;
		}		
	} else if( w == 1 && h == 1 ) {
		switch( angle ) {
		case 0: 
			setPosition( x, y, 0, "P_BASE_1x1" + postfix, true );
			setPosition( x + 5, y - 14, 0, "NS_DOOR", true );
			scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
			setPosition( x - 2, y + 2, 12, "P_ROOF_1x1" + roof_postfix, true );
			break;
		case 90: 
			setPosition( x, y, 0, "P_BASE_1x1_90" + postfix, true );
			setPosition( x + 1, y - 5, 0, "EW_DOOR", true );
			scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
			setPosition( x - 2, y - 2, 12, "P_ROOF_1x1_90" + roof_postfix, true );
			break;
		case 180: 
			setPosition( x, y, 0, "P_BASE_1x1_180" + postfix, true );
			setPosition( x + 5, y - 1, 0, "NS_DOOR", true );
			scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
			setPosition( x + 2, y + 2, 12, "P_ROOF_1x1_180" + roof_postfix, true );
			break;
		case 270: 
			setPosition( x, y, 0, "P_BASE_1x1_270" + postfix, true );
			setPosition( x + 14, y - 5, 0, "EW_DOOR", true );
			scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
			setPosition( x - 2, y + 2, 12, "P_ROOF_1x1_270" + roof_postfix, true );
			break;
		}
	} else {
		print( "Unknown shape: w=" + w.tostring() + " h=" + h.tostring() + " angle=" + angle.tostring() );
	}
	if( furnish ) {
		furnishArea( x, y, w, h );
	}
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

function drawStreetlights( x, y ) {
	setPosition( x - 5, y + 8 - 7, 0, "STREETLIGHT_180", true );
	setPosition( x + MAP_UNIT * 3 + 1, y + MAP_UNIT * 2 - 8 - 7, 0, "STREETLIGHT", true );
	setPosition( x + MAP_UNIT * 2 - 8 + 7, y - MAP_UNIT - 1, 0, "STREETLIGHT_270", true );
	setPosition( x + 8 + 7, y + MAP_UNIT * 2 + 5, 0, "STREETLIGHT_90", true );
}

function drawHouseZ( x, y, furnish, postfix, roof_postfix ) {
	if( postfix == null ) {
		postfix = getHousePostfix();
	}
	if( roof_postfix == null ) {
		roof_postfix = getRoofPostfix();
	}
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x, y + MAP_UNIT * 2, 1, 1, 0, true );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y + MAP_UNIT * 2, 1, 3, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT * 2, y, 1, 1, 180, true );
	scourgeGame.getMission().endHouse();
	setPosition( x + MAP_UNIT - 2, y + MAP_UNIT - 4, 0, "LOGS_90", false );
	setPosition( x + MAP_UNIT * 2, y + MAP_UNIT - 6, 0, "LOGS_90", true );
	drawStreetlights( x, y );
	setPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT * 2, 0, getTree(), false );
	setPosition( x + MAP_UNIT * 2 + 9, y + MAP_UNIT * 2 - 10, 0, getTree(), false );
	setPosition( x + 2, y - 4, 0, getTree(), false );
	setPosition( x + 5, y + 4, 0, getTree(), false );
	setPosition( x + MAP_UNIT * 2 + 14, y + 6, 0, "FENCE_90", false );
	setPosition( x + 2, y + MAP_UNIT, 0, "FENCE_90", false );
	setPosition( x + MAP_UNIT * 2, y + MAP_UNIT * 2 - 2, 0, "FENCE", false );
	setPosition( x + MAP_UNIT * 2 + 6, y + MAP_UNIT * 2 - 2, 0, "FENCE", false );
	setPosition( x + MAP_UNIT - 6, y - MAP_UNIT + 2, 0, "FENCE", false );
	setPosition( x + MAP_UNIT - 12, y - MAP_UNIT + 2, 0, "FENCE", false );
	setPosition( x - 1, y + MAP_UNIT * 2 + 2, 0, "ROAD_SIGN", false );
	setPosition( x + MAP_UNIT * 3 - 7, y - MAP_UNIT - 1, 0, "ROAD_SIGN", false );
}

function drawHouseL( x, y, furnish, postfix, roof_postfix ) {
	if( postfix == null ) {
		postfix = getHousePostfix();
	}
	if( roof_postfix == null ) {
		roof_postfix = getRoofPostfix();
	}
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x, y + 2 * MAP_UNIT, 1, 3, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + 1 * MAP_UNIT, y, 2, 1, 180, true );
	scourgeGame.getMission().endHouse();
	drawStreetlights( x, y );
	setPosition( x + MAP_UNIT + 5, y + MAP_UNIT - 7, 0, getTree(), false );
	setPosition( x + MAP_UNIT + 8, y + MAP_UNIT * 2 - 8, 0, getTree(), false );
	setPosition( x + MAP_UNIT + 19, y + MAP_UNIT * 2 - 12, 0, getTree(), false );
	setPosition( x + MAP_UNIT + 30, y + MAP_UNIT * 2 - 8, 0, getTree(), false );
	for( fx <- 0; fx < 5; fx++ ) {
		setPosition( x + MAP_UNIT + ( fx * 6 ), y + MAP_UNIT * 2 - 2, 0, "FENCE", false );
	}
	for( fy <- 0; fy < 1; fy++ ) {
		setPosition( x + MAP_UNIT * 2 + 14, y + ( ( fy + 1 ) * 6 ), 0, "FENCE_90", false );
	}
	setPosition( x - 1, y + MAP_UNIT * 2 + 2, 0, "ROAD_SIGN", false );
	setPosition( x + MAP_UNIT * 3 - 7, y - MAP_UNIT - 1, 0, "ROAD_SIGN", false );
	setPosition( x + MAP_UNIT, y + MAP_UNIT - 6, 0, "LOGS_90", false );
}

function drawHouseL2( x, y, furnish, postfix, roof_postfix ) {
	if( postfix == null ) {
		postfix = getHousePostfix();
	}
	if( roof_postfix == null ) {
		roof_postfix = getRoofPostfix();
	}
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x + 2 * MAP_UNIT, y + 2 * MAP_UNIT, 1, 3, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x, y + 2 * MAP_UNIT, 2, 1, 0, true );
	scourgeGame.getMission().endHouse();
	setPosition( x + MAP_UNIT, y + MAP_UNIT, 0, "LOGS", true );
	drawStreetlights( x, y );
	setPosition( x, y - 4, 0, getTree(), false );
	setPosition( x + 11, y, 0, getTree(), false );
	setPosition( x + 22, y - 4, 0, getTree(), false );
	setPosition( x + 23, y + 11, 0, getTree(), false );
	for( fx <- 0; fx < 5; fx++ ) {
		setPosition( x + MAP_UNIT * 2 - ( ( fx + 1 ) * 6 ), y - MAP_UNIT + 2, 0, "FENCE", false );
	}
	for( fy <- 0; fy < 1; fy++ ) {
		setPosition( x + 2, y + MAP_UNIT, 0, "FENCE_90", false );
	}
	setPosition( x - 2, y + 2 * MAP_UNIT, 0, "ROAD_SIGN_90", false );
	setPosition( x + 3 * MAP_UNIT + 1, y - 11, 0, "ROAD_SIGN_90", false );
	
}

function drawHouseSquare( x, y, furnish, postfix, roof_postfix ) {
	if( postfix == null ) {
		postfix = getHousePostfix();
	}
	if( roof_postfix == null ) {
		roof_postfix = getRoofPostfix();
	}
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y + 2 * MAP_UNIT, 2, 2, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y, 1, 1, 90, true );
	drawHousePart( postfix, roof_postfix,  x, y, 1, 1, 0, true );
	scourgeGame.getMission().endHouse();
	drawStreetlights( x, y );
	setPosition( x - 2, y + 2 * MAP_UNIT, 0, "ROAD_SIGN_90", false );
	setPosition( x + 3 * MAP_UNIT + 1, y - 11, 0, "ROAD_SIGN_90", false );
	setPosition( x, y + MAP_UNIT - 3, 0, getTree(), false );
	setPosition( x + MAP_UNIT * 2 + 5, y - 7, 0, getTree(), false );
	setPosition( x + 3, y + MAP_UNIT * 2, 0, getTree(), false );	
}

function drawHouseSquare2( x, y, furnish, postfix, roof_postfix ) {
	if( postfix == null ) {
		postfix = getHousePostfix();
	}
	if( roof_postfix == null ) {
		roof_postfix = getRoofPostfix();
	}
	scourgeGame.getMission().startHouse();
	drawHousePart( postfix, roof_postfix,  x, y + 1 * MAP_UNIT, 2, 2, 0, furnish );
	drawHousePart( postfix, roof_postfix,  x + MAP_UNIT, y + 2 * MAP_UNIT, 1, 1, 270, true );
	drawHousePart( postfix, roof_postfix,  x + 2 * MAP_UNIT, y + 2 * MAP_UNIT, 1, 1, 180, true );
	scourgeGame.getMission().endHouse();
	drawStreetlights( x, y );
	setPosition( x - 2, y + 2 * MAP_UNIT, 0, "ROAD_SIGN_90", false );
	setPosition( x + 3 * MAP_UNIT + 1, y - 11, 0, "ROAD_SIGN_90", false );
	setPosition( x, y + MAP_UNIT * 2 - 7, 0, getTree(), false );
	setPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT - 7, 0, getTree(), false );
	setPosition( x + MAP_UNIT * 2 + 5, y - 10, 0, getTree(), false );
}

house_functions <- [ drawHouseZ, drawHouseL, drawHouseL2, drawHouseSquare, drawHouseSquare2 ];
function drawRandomHouse( x, y ) {
	n <- ( rand() * house_functions.len().tofloat() / RAND_MAX ).tointeger();
	house_functions[ n ].call( this, x, y, true, null, null );
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
		scourgeGame.getMission().addItem( getRandomInnItem(), x + 1 + xx[tt], y - 1 - yy[tt], 3, false );
	}
}

function drawArmorShop( x, y ) {
	print("Making armor shop!\n");
	drawHouseZ( x, y, false, null, null );
	setPosition( x + 3, y + MAP_UNIT, 7, "SIGN_ARMOR", true );
	setPosition( x + MAP_UNIT * 2 + 12, y + 4, 7, "SIGN_ARMOR_180", true );
	
	setPosition( x + MAP_UNIT + 6, y + MAP_UNIT, 0, "BAR_90", false );
	setPosition( x + MAP_UNIT + 6, y + MAP_UNIT - 6, 0, "BAR_90", false );
	setPosition( x + MAP_UNIT + 6, y + MAP_UNIT - 12, 0, "BAR_90", false );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT + 2, y + MAP_UNIT + 2, 0, true );
	scourgeGame.getMission().addItem( "Barrel", x + MAP_UNIT + 5, y + MAP_UNIT + 3, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + MAP_UNIT + 2, y, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + MAP_UNIT + 4, y - 1, 0, true );
	
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 6, y + MAP_UNIT - 2, 4, false );
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 6, y + MAP_UNIT - 8, 4, false );
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 6, y + MAP_UNIT - 14, 4, false );
	scourgeGame.getMission().addItem( getRandomArmorItem(), x + MAP_UNIT + 2, y + 4, 0, false );
	
	// add the shop-keeper
	merchant <- scourgeGame.getMission().addCreature( x + MAP_UNIT + 3, y + 10, 0, getVillageNpcType() );
	// now make this person a trader of armor
	if( merchant != null ) {
		merchant.setNpcInfo( _("the Armorer"), "merchant", "ARMOR" );
	}
}

function drawWeaponShop( x, y ) {
	print("Making weapon shop!\n");
	drawHouseL( x, y, false, null, null );
	setPosition( x - 4, y + MAP_UNIT * 2 - 3, 7, "SIGN_WEAPON_90", true );
	setPosition( x + MAP_UNIT * 2 + 12, y + 4, 7, "SIGN_WEAPON_180", true );
	
	setPosition( x + 6, y + MAP_UNIT, 0, "BAR_90", false );
	setPosition( x + 6, y + MAP_UNIT - 6, 0, "BAR_90", false );
	setPosition( x + 6, y + MAP_UNIT - 12, 0, "BAR_90", false );
	scourgeGame.getMission().addItem( "Barrel", x + 2, y + MAP_UNIT + 2, 0, true );
	scourgeGame.getMission().addItem( "Barrel", x + 5, y + MAP_UNIT + 3, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + 2, y, 0, true );
	scourgeGame.getMission().addItem( "Crate", x + 4, y - 1, 0, true );
	
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 6, y + MAP_UNIT - 2, 4, false );
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 6, y + MAP_UNIT - 8, 4, false );
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 6, y + MAP_UNIT - 14, 4, false );
	scourgeGame.getMission().addItem( getRandomWeaponItem(), x + 2, y + 4, 0, false );
	
	// add the shop-keeper
	merchant <- scourgeGame.getMission().addCreature( x + 3, y + 10, 0, getVillageNpcType() );
	// now make this person a trader of weapons
	if( merchant != null ) {
		merchant.setNpcInfo( _("the Blacksmith"), "merchant", "SWORD;AXE;BOW;MACE;POLE" );
	}
	
}

function drawInn( x, y ) {
	print("Making inn!\n");
	drawHouseSquare( x, y, false, null, null );
	setPosition( x + 2, y - MAP_UNIT, 7, "SIGN_INN", true );
	setPosition( x + MAP_UNIT * 2 + 12, y + MAP_UNIT * 2 + 4, 7, "SIGN_INN_180", true );
	setPosition( x + MAP_UNIT * 2 + 8, y + MAP_UNIT, 0, "BAR_270", false );
	setPosition( x + MAP_UNIT * 2 + 8, y + MAP_UNIT + 6, 0, "BAR_270", false );
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
	merchant <- scourgeGame.getMission().addCreature( x + MAP_UNIT * 2 + 10, y + MAP_UNIT + 3, 0, getVillageNpcType() );
	// now make this person an inn-keeper
	if( merchant != null ) {
		merchant.setNpcInfo( _("the Innkeeper"), "merchant", "FOOD;DRINK" );
	}

	for( i <- 0; i < 5; i++ ) {
		scourgeGame.getMission().addCreatureAround( x + MAP_UNIT + 2 + ( rand() * (MAP_UNIT * 2.0 - 4.0) / RAND_MAX ).tointeger(),
		                                            y + 2 + ( rand() * (MAP_UNIT * 2.0 - 4.0) / RAND_MAX ).tointeger(),
		                                            0, getVillageNpcType() );
	}	
}

function drawChurch( x, y ) {
	print("Making church!\n");
	drawHouseSquare( x, y, false, null, "church" );
	for( i <- 1; i < 5; i++ ) {
		scourgeGame.getMission().addItem( "Bench", x + MAP_UNIT + 4, y + MAP_UNIT * 2 - ( i * 5 ), 0, false );
		if( i >= 2 ) {
			scourgeGame.getMission().addItem( "Bench", x + MAP_UNIT * 2 + 2, y + MAP_UNIT * 2 - ( i * 5 ), 0, false );
		}
	}
	setPosition( x + MAP_UNIT * 2 + 9, y + 7, 0, "BRAZIER_BASE", true );
	setPosition( x + MAP_UNIT * 2 + 10, y + 6, 2, "BRAZIER", true );
	setPosition( x + MAP_UNIT + 12, y + 5, 0, "COLUMN", true );
	setPosition( x + MAP_UNIT * 2 - 1, y + 8, 0, "POOL", true );
	setPosition( x + MAP_UNIT * 2 + 4, y + 5, 0, "COLUMN", true );
	addRug( x + MAP_UNIT * 2, y + MAP_UNIT * 2, 1, 1 );
	addRug( x + MAP_UNIT, y + MAP_UNIT, 1, 1 );
	setPosition( x + MAP_UNIT * 2 + 1, y + MAP_UNIT * 2 + 4, 0, "COLUMN", true );
	setPosition( x + MAP_UNIT * 3 - 4, y + MAP_UNIT * 2 + 4, 0, "COLUMN", true );
	setPosition( x + 1, y - MAP_UNIT, 0, "COLUMN", true );
	setPosition( x + MAP_UNIT - 4, y - MAP_UNIT, 0, "COLUMN", true );
	
	// add a healer
	healer <- scourgeGame.getMission().addCreature( x + MAP_UNIT * 2 + 6, y + MAP_UNIT + 8, 0, getVillageNpcType() );
	// now make this person a healer
	if( healer != null ) {
		healer.setNpcInfo( _("the Healer"), "healer", getRandomMagicSchool() );
	}
}

function drawFence( x, y ) {
	setPosition( x, y - MAP_UNIT + 2, 0, "FENCE_CORNER", false );
	setPosition( x + MAP_UNIT * 2 + 14, y - MAP_UNIT + 2, 0, "FENCE_CORNER_90", false );
	setPosition( x, y + MAP_UNIT * 2, 0, "FENCE_CORNER_270", false );
	setPosition( x + MAP_UNIT * 2 + 14, y + MAP_UNIT * 2, 0, "FENCE_CORNER_180", false );
	for( ix <- 0; ix < 3; ix++ ) {
		setPosition( x + 2 + ix * 6, y - MAP_UNIT + 1, 0, "FENCE", false );
		setPosition( x + MAP_UNIT * 2 + 8 - ( ix * 6 ), y - MAP_UNIT + 1, 0, "FENCE", false );
		setPosition( x + 2 + ix * 6, y + MAP_UNIT * 2, 0, "FENCE", false );		
		setPosition( x + MAP_UNIT * 2 + 8 - ( ix * 6 ), y + MAP_UNIT * 2, 0, "FENCE", false );
		setPosition( x, y - MAP_UNIT + 8 + ix * 6, 0, "FENCE_90", false );
		setPosition( x, y + MAP_UNIT * 2 - 2 - ix * 6, 0, "FENCE_90", false );
		setPosition( x + MAP_UNIT * 2 + 15, y - MAP_UNIT + 8 + ix * 6, 0, "FENCE_90", false );
		setPosition( x + MAP_UNIT * 2 + 15, y + MAP_UNIT * 2 - 2 - ix * 6, 0, "FENCE_90", false );
	}
}

function drawMarket( x, y ) {
	print("Making market!\n");
	drawFence( x, y );
	
	mx <- 0;
	my <- 0;
	for( mx = 4 + ( rand() * 2.0 / RAND_MAX ).tointeger(); mx < 3 * MAP_UNIT - 8; mx += 8 + ( rand() * 2.0 / RAND_MAX ).tointeger() ) {
		if( mx < MAP_UNIT + 4 || mx >= MAP_UNIT + 12 ) {
			my = ( rand() * 4.0 / RAND_MAX ).tointeger();
			setPosition( x + mx, y - MAP_UNIT + 8 + my, 0, getRandomCart(), false );
		}
	}
	
	for( mx = 4 + ( rand() * 2.0 / RAND_MAX ).tointeger(); mx < 3 * MAP_UNIT - 8; mx += 8 + ( rand() * 2.0 / RAND_MAX ).tointeger() ) {
		if( mx < MAP_UNIT + 4 || mx >= MAP_UNIT + 12 ) {
			my = ( rand() * 4.0 / RAND_MAX ).tointeger();
			setPosition( x + mx, y + MAP_UNIT * 2 - ( 4 + my ), 0, getRandomCart(), false );
		}
	}
	
	for( my = -MAP_UNIT + 10 + ( rand() * 2.0 / RAND_MAX ).tointeger(); my < 2 * MAP_UNIT - 8; my += 8 + ( rand() * 2.0 / RAND_MAX ).tointeger() ) {
		if( my < 4 || my > 12 ) {
			mx = ( rand() * 4.0 / RAND_MAX ).tointeger();
			setPosition( x + 4 + mx, y + my, 0, getRandomCart() + "_90", false );
		}
	}
	
	for( my = -MAP_UNIT + 10 + ( rand() * 2.0 / RAND_MAX ).tointeger(); my < 2 * MAP_UNIT - 8; my += 8 + ( rand() * 2.0 / RAND_MAX ).tointeger() ) {
		if( my < 4 || my > 12 ) {
			mx = ( rand() * 4.0 / RAND_MAX ).tointeger();
			setPosition( x + MAP_UNIT * 3 - ( 8 + mx ), y + my, 0, getRandomCart() + "_90", false );
		}
	}
	
	setPosition( x + MAP_UNIT - 2, y + 5, 0, getTree(), false );
	setPosition( x + 2 * MAP_UNIT - 6, y + 5, 0, getTree(), false );
	setPosition( x + MAP_UNIT - 2, y + MAP_UNIT + 2, 0, getTree(), false );
	setPosition( x + 2 * MAP_UNIT - 6, y + MAP_UNIT + 2, 0, getTree(), false );
	
	// add merchants
	merchant <- null;
	for( i <- 0; i < 5; i++ ) {
		merchant = scourgeGame.getMission().addCreatureAround( xp + 5 + ( rand() * (MAP_UNIT * 3.0 - 10.0) / RAND_MAX ).tointeger(),
		                                                       yp - MAP_UNIT + 5 + ( rand() * (MAP_UNIT * 3.0 - 10.0) / RAND_MAX ).tointeger(),
		                                                       0, getVillageNpcType() );
		if( merchant != null ) {
			merchant.setNpcInfo( _("the Traveling Merchant"), "merchant", getRandomMerchantType() );
		}
	}
	
	// todo: add boxes, barrels	
}

function drawGarden( x, y ) {
	print("Making garden!\n");
	drawFence( x, y );
	scourgeGame.getMission().addItem( "Bench", x + 4, y - MAP_UNIT + 3, 0, false );
	scourgeGame.getMission().addItem( "Bench", x + MAP_UNIT * 2, y - MAP_UNIT + 3, 0, false );
	scourgeGame.getMission().addItem( "Bench", x + 4, y + MAP_UNIT * 2 - 1, 0, false );
	scourgeGame.getMission().addItem( "Bench", x + MAP_UNIT * 2, y + MAP_UNIT * 2 - 1, 0, false );
	setPosition( x + 10, y - MAP_UNIT + 14, 0, getTree(), false );			
	setPosition( x + 36, y - MAP_UNIT + 14, 0, getTree(), false );
	setPosition( x + 10, y - MAP_UNIT + 38, 0, getTree(), false );
	setPosition( x + 36, y - MAP_UNIT + 38, 0, getTree(), false );
	setPosition( x + 23, y - MAP_UNIT + 20, 0, "RUINS", false );
	setPosition( x + 22, y - MAP_UNIT + 28, 0, "POOL", false );
	setPosition( x + 23, y - MAP_UNIT + 34, 0, "RUINS2", false );
}

// from shapes.h
OUTDOOR_THEME_REF_GRASS <- 0;
OUTDOOR_THEME_REF_STREET <- 1;
OUTDOOR_THEME_REF_STREET_CROSS <- 2;
OUTDOOR_THEME_REF_STREET_END <- 3;
OUTDOOR_THEME_REF_TRAIL <- 4;
OUTDOOR_THEME_REF_TRAIL_TURN <- 5;
OUTDOOR_THEME_REF_TRAIL_END <- 6;
OUTDOOR_THEME_REF_WATER <- 7;
OUTDOOR_THEME_REF_ROCK <- 8;
OUTDOOR_THEME_REF_GRASS_EDGE <- 9;
OUTDOOR_THEME_REF_GRASS_CORNER <- 10;
OUTDOOR_THEME_REF_GRASS_TIP <- 11;
OUTDOOR_THEME_REF_GRASS_NARROW <- 12;
OUTDOOR_THEME_REF_SNOW <- 13;
OUTDOOR_THEME_REF_SNOW_BIG <- 14;
OUTDOOR_THEME_REF_LAKEBED <- 15;
OUTDOOR_THEME_REF_EXTRA <- 16;
OUTDOOR_THEME_REF_STREET_90 <- 17;
OUTDOOR_THEME_REF_STREET_END_90 <- 18;
OUTDOOR_THEME_REF_STREET_END_180 <- 19;
OUTDOOR_THEME_REF_STREET_END_270 <- 20;

// must be the last one
OUTDOOR_THEME_REF_COUNT <- 21;

function drawRoads( x, y, village_width, village_height ) {
	vx <- 0;
	vy <- 0;
	for( yy <- 4; yy <= village_height; yy += 4 ) {
		vy = y + ( yy * MAP_UNIT );
		for ( i <- 0; i < village_width; i++ ) {
			vx = x + ( i * MAP_UNIT );
			if ( i == 0 ) {
				scourgeGame.getMission().addOutdoorTexture( vx, vy, OUTDOOR_THEME_REF_STREET_END, 0, false, false );
			} else if ( i >= village_width - 1 ) {
				scourgeGame.getMission().addOutdoorTexture( vx, vy, OUTDOOR_THEME_REF_STREET_END_180, 0, false, false );
			} else {
				scourgeGame.getMission().addOutdoorTexture( vx, vy, OUTDOOR_THEME_REF_STREET, 0, false, false );
			}
		}
	}
	for( xx <- 3; xx <= village_height - 1; xx += 4 ) {
		vx = x + ( xx * MAP_UNIT );
		for ( i <- 1; i <= village_height; i++ ) {
			vy = y + ( i * MAP_UNIT );
			if ( i == 1 ) {
				scourgeGame.getMission().addOutdoorTexture( vx, vy, OUTDOOR_THEME_REF_STREET_END_270, 0, false, false );
			} else if ( i % 4 == 0 ) {
				scourgeGame.getMission().addOutdoorTexture( vx, vy, OUTDOOR_THEME_REF_STREET_CROSS, 0, false, false );
			} else if ( i >= village_height  ) {
				scourgeGame.getMission().addOutdoorTexture( vx, vy, OUTDOOR_THEME_REF_STREET_END_90, 0, false, false );
			} else {
				scourgeGame.getMission().addOutdoorTexture( vx, vy, OUTDOOR_THEME_REF_STREET_90, 0, false, false );
			}
		}
	}		
}

function drawVillage( x, y, village_width, village_height ) {
	
	drawRoads( x, y, village_width, village_height );
	
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
		} else if( h == 5 ) {
			print( "church at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawChurch( xp, yp );			
		} else {
			drawRandomHouse( xp, yp );
		}
		
		// add some peeps
		for( i <- 0; i < 5; i++ ) {
			scourgeGame.getMission().addCreatureAround( xp + 5 + ( rand() * (MAP_UNIT * 3.0 - 10.0) / RAND_MAX ).tointeger(),
			                                            yp - MAP_UNIT + 5 + ( rand() * (MAP_UNIT * 3.0 - 10.0) / RAND_MAX ).tointeger(),
			                                            0, getVillageNpcType() );
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

// 2x3 base
function drawHouseHut( x, y ) {
	w <- 2 * MAP_UNIT;
	h <- 3 * MAP_UNIT;
	xx <- x + 5;
	yy <- y + h - 4;
	if( scourgeGame.getMission().isFree( xx, yy, 0, "HOUSE_3_BASE" ) ) {
		scourgeGame.getMission().setMapPosition( xx, yy, 0, "HOUSE_3_BASE" );
		
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

// 3x2 base
function drawHouseLogs( x, y ) {
	w <- 3 * MAP_UNIT;
	h <- 2 * MAP_UNIT;
	shape_name <- "HOUSE_1_BASE"	
	if( scourgeGame.getMission().isFree( x + 2, y + h - 4, 0, shape_name ) ) {
		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, shape_name );
		
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

// 2x2 base
function drawHouseTower( x, y ) {
	w <- 2 * MAP_UNIT;
	h <- 2 * MAP_UNIT;
	if( scourgeGame.getMission().isFree( x + 2, y + h - 4, 0, "HOUSE_2_BASE" ) ) {
		scourgeGame.getMission().startHouse();
		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, "HOUSE_2_BASE" );
		
		// the door
		scourgeGame.getMission().setMapPosition( x + 2 + 1, y + h - 4 - 10, 0, "EW_DOOR" );
		
		// the top
		scourgeGame.getMission().setMapPosition( x + 2 - 2, y + h - 4 + 4, 12, "HOUSE_2_TOP" );
		scourgeGame.getMission().endHouse();
		
		// add some objects
		scourgeGame.getMission().setMapPosition( x + 3 + 2, y + h - 4, 0, "BED" );
		scourgeGame.getMission().setMapPosition( x + 3 + 10, y + h - 4 - 7, 0, "TABLE" );
		scourgeGame.getMission().setMapPosition( x + 3 + 12, y + h - 4 - 4, 0, "CHAIR" );
		//scourgeGame.getMission().setMapPosition( x + 2 + 5, y + h - 4 - 8, 0, "CHAIR" );
		scourgeGame.getMission().setMapPosition( x + 3 + 15, y + h - 4, 0, "STOVE" );
				
		//scourgeGame.getMission().addRug( x + 2 + 8, y + h - 4 - 8 );
		
		// tell the terrain generator where to populate the room with containers
		scourgeGame.addRoom( x + 2 + 1, y + h - 4 - 20, 19, 22 );
	}
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
	} else if( shape_name == "HOUSE_1_TOP" ) {
		scourgeGame.getMission().setMapEffect( x + 9, y - 7, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 15, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( shape_name == "HOUSE_2_BASE" ) {
		scourgeGame.getMission().setMapEffect( x - 1, y - 17, z + 9, // map location 
		                                       	"EFFECT_FIRE",  												// effect 
				                                       1, 1, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0.3, -0.7, 0, 														// offset
				                                       0.5, 0.3, 0.1 														// color
																							);		
		scourgeGame.getMission().setMapEffect( x - 1, y - 7, z + 9, // map location 
		                                       "EFFECT_FIRE",  												// effect 
				                                       1, 1, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0.3, -0.7, 0, 														// offset
				                                       0.5, 0.3, 0.1 														// color
																							);
	} else if( shape_name == "HOUSE_2_TOP" ) {
		scourgeGame.getMission().setMapEffect( x + 16, y + - 7, z - 2, // map location 
				                                       "EFFECT_SMOKE",  												// effect 
				                                       4, 4, 																	// base size
				                                       0,																			// delay
				                                       true,																	// forever 
				                                       0, 0, 18, 														// offset
				                                       0.3, 0.1, 0.3 														// color
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
	} else if( shape_name == "HOUSE_1_BASE" ) {
		scourgeGame.clearVirtualShapes( shape_name );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 6, 25, 10, true );
		scourgeGame.addVirtualShape( shape_name, 6, 0, 0, 30, 6, 10, false );
		scourgeGame.addVirtualShape( shape_name, 36, 0, 0, 6, 25, 10, false );
		scourgeGame.addVirtualShape( shape_name, 16, -19, 0, 20, 6, 10, false );
		scourgeGame.addVirtualShape( shape_name, 6, -19, 0, 4, 6, 10, false );						
	} else if( shape_name == "HOUSE_2_BASE" ) {
		scourgeGame.clearVirtualShapes( shape_name );
		scourgeGame.addVirtualShape( shape_name, 0, 2, 0, 3, 12, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -16, 0, 3, 4, 12, true );
		scourgeGame.addVirtualShape( shape_name, 19, 2, 0, 2, 22, 12, false );
		scourgeGame.addVirtualShape( shape_name, 3, -18, 0, 16, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 3, 2, 0, 16, 2, 12, false );
	} else if( shape_name == "HOUSE_3_BASE" ) {
		scourgeGame.clearVirtualShapes( shape_name );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 3, 32, 12, true );
		scourgeGame.addVirtualShape( shape_name, 19, 0, 0, 3, 32, 12, false );
		scourgeGame.addVirtualShape( shape_name, 3, -29, 0, 16, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 11, 0, 0, 8, 3, 12, false );
	} else if( shape_name == "MONASTERY_BASE" ) {
		scourgeGame.clearVirtualShapes( shape_name );
		scourgeGame.addVirtualShape( shape_name, 0, -20, 0, 4, 6, 12, true );
		scourgeGame.addVirtualShape( shape_name, 4, -24, 0, 15, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 25, -24, 0, 7, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 32, -20, 0, 4, 6, 12, false );
		scourgeGame.addVirtualShape( shape_name, 32, -6, 0, 2, 14, 12, false );
		scourgeGame.addVirtualShape( shape_name, 32, 0, 0, 4, 6, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 4, 6, 12, false );
		scourgeGame.addVirtualShape( shape_name, 4, 0, 0, 28, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 2, -6, 0, 2, 8, 12, false );
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

function setPosition( x, y, z, shape, force ) {
	if( force || scourgeGame.getMission().isFree( x, y, z, shape ) ) {
		scourgeGame.getMission().setMapPosition( x, y, z, shape );
		return true;
	} else {
		return false;
	}
}
