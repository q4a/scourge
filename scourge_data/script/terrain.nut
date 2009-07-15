// =============================================================
// This squirrel file contains the code called during terrain generation.
// Functions here are not story-related and can be used anytime for map building
//

MAP_UNIT <- 16;

function pickOne( arg_list ) {
	selected_index <- ( scourgeGame.getMission().mt_rand() * arg_list.len().tofloat() ).tointeger();
	return arg_list[ selected_index ];
}

/**
 * Return a random npc type (name, really) to use for a village population.
 * This method is called repeatedly by the outdoor terrain generator code.
 */
npc_list <- [ "Male Vagrant", "Male Traveling Wizard", "Male Knight", "Male Rogue", "Female Hobo", "Female Bard", "Female Ranger", "Female Priestess" ];
function getVillageNpcType() {
	// in the future, there should be human towns, dwarven towns, etc.
	return pickOne( npc_list );
}

trees <- [ "tree01", "tree02", "tree03", "tree12", "tree13", "tree14", "tree15", "tree17", "tree20", "tree21", "birch", "bushtree", "cactus", "deadtree", "fern", "fir", "palm", "palm2" ];
function getTree() {
	return pickOne( trees );
}

carts <- [ "CART", "CART2" ];
function getRandomCart() {
	return pickOne( carts );
}

merchant_types <- [ "ARMOR", "FOOD;DRINK", "SCROLL;POTION;WAND;RING;AMULET;STAFF", "SWORD;AXE;BOW;MACE;POLE" ];
function getRandomMerchantType() {
	return pickOne( merchant_types );
}

magic_schools <- [ "Confrontation", "Ambush Trickery and Deceit", "History and Lore", "Life and Death", "Divine Awareness", "Nature" ];
function getRandomMagicSchool() {
	return pickOne( magic_schools );
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
		x = villageX + ( scourgeGame.getMission().mt_rand() * villageWidth.tofloat() ).tointeger();
		y = villageY + ( scourgeGame.getMission().mt_rand() * villageHeight.tofloat() ).tointeger();
		tree <- getTree();
		if( scourgeGame.getMission().isFreeOutdoors( x, y, 0, tree ) ) {
			setPosition( x, y, 0, tree, false );
		}
	}
}

function addRug( x, y, w, h ) {
	rx <- ( x ) / MAP_UNIT + ( scourgeGame.getMission().mt_rand() * w.tofloat() ).tointeger();
	ry <- ( y ) / MAP_UNIT - 1 - ( scourgeGame.getMission().mt_rand() * h.tofloat() ).tointeger();
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
		p <- ( scourgeGame.getMission().mt_rand() * obj_pos.len().tofloat() ).tointeger();
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
		p <- ( scourgeGame.getMission().mt_rand() * obj_pos.len().tofloat() ).tointeger();
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
	pickOne( room_functions ).call( this, x, y, w, h );
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
	return pickOne( HOUSE_POSTFIX );
}

ROOF_POSTFIX <- [ "", "_SLATE", "_RED" ];
function getRoofPostfix() {
	return pickOne( ROOF_POSTFIX );
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
	return pickOne( house_functions ).call( this, x, y, true, null, null );
}

armor <- [ "Horned helmet", "Leather Work Gloves", "Adventuring Hat", "Wooden Shield" ];
function getRandomArmorItem() {
	return pickOne( armor );
}

weapon_list <- [ "Kitchen Knife", "Dagger", "Ornamental officer sword", "Battleaxe", "Smallbow", "Rounded mace" ];
function getRandomWeaponItem() {
	return pickOne( weapon_list );
}

inn <- [ "Beer barrel", "Wine barrel", "Fine wine bottle", "Wine bottle", "Milk", "Apple", "Bread", "Mutton meat", "Chalice" ];
function getRandomInnItem() {
	return pickOne( inn );
}

function createTable( x, y ) {
	scourgeGame.getMission().addItem( "Table", x, y, 0, false );
	xx <- mixHouseCoordinates( [ 0, 1, 2 ] );
	yy <- mixHouseCoordinates( [ 0, 1, 2 ] );
	n <- ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger() + 1;
	for( tt <- 0; tt < n; tt++ ) {
		scourgeGame.getMission().addItem( getRandomInnItem(), x + 1 + xx[tt], y - 1 - yy[tt], 3, false );
	}
}

function drawArmorShop( x, y ) {
	print("Making armor shop! " + x.tostring() + "," + y.tostring() + "\n");
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
	print("Making weapon shop! " + x.tostring() + "," + y.tostring() + "\n");
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
	print("Making inn! " + x.tostring() + "," + y.tostring() + "\n");
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
		scourgeGame.getMission().addCreatureAround( x + MAP_UNIT + 2 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 2.0 - 4.0) ).tointeger(),
		                                            y + 2 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 2.0 - 4.0) ).tointeger(),
		                                            0, getVillageNpcType() );
	}
	
	hero_count <- ( scourgeGame.getMission().mt_rand() * 3 ).tointeger();
	print("Adding wandering heros to inn: " + hero_count.tointeger() + "\n");
	for( i <- 0; i < hero_count; i++ ) {
		scourgeGame.getMission().addWanderingHero( x + MAP_UNIT + 2 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 2.0 - 4.0) ).tointeger(),
				                                       y + 2 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 2.0 - 4.0) ).tointeger(),
				                                       0, 
				                                       1 );		
	}
}

function drawChurch( x, y ) {
	print("Making church! " + x.tostring() + "," + y.tostring() + "\n");
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
	print("Making market! " + x.tostring() + "," + y.tostring() + "\n");
	drawFence( x, y );
	
	mx <- 0;
	my <- 0;
	for( mx = 4 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger(); mx < 3 * MAP_UNIT - 8; mx += 8 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger() ) {
		if( mx < MAP_UNIT + 4 || mx >= MAP_UNIT + 12 ) {
			my = ( scourgeGame.getMission().mt_rand() * 4.0 ).tointeger();
			setPosition( x + mx, y - MAP_UNIT + 8 + my, 0, getRandomCart(), false );
		}
	}
	
	for( mx = 4 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger(); mx < 3 * MAP_UNIT - 8; mx += 8 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger() ) {
		if( mx < MAP_UNIT + 4 || mx >= MAP_UNIT + 12 ) {
			my = ( scourgeGame.getMission().mt_rand() * 4.0 ).tointeger();
			setPosition( x + mx, y + MAP_UNIT * 2 - ( 4 + my ), 0, getRandomCart(), false );
		}
	}
	
	for( my = -MAP_UNIT + 10 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger(); my < 2 * MAP_UNIT - 8; my += 8 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger() ) {
		if( my < 4 || my > 12 ) {
			mx = ( scourgeGame.getMission().mt_rand() * 4.0 ).tointeger();
			setPosition( x + 4 + mx, y + my, 0, getRandomCart() + "_90", false );
		}
	}
	
	for( my = -MAP_UNIT + 10 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger(); my < 2 * MAP_UNIT - 8; my += 8 + ( scourgeGame.getMission().mt_rand() * 2.0 ).tointeger() ) {
		if( my < 4 || my > 12 ) {
			mx = ( scourgeGame.getMission().mt_rand() * 4.0 ).tointeger();
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
		merchant = scourgeGame.getMission().addCreatureAround( xp + 5 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 3.0 - 10.0) ).tointeger(),
		                                                       yp - MAP_UNIT + 5 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 3.0 - 10.0) ).tointeger(),
		                                                       0, getVillageNpcType() );
		if( merchant != null ) {
			merchant.setNpcInfo( _("the Traveling Merchant"), "merchant", getRandomMerchantType() );
		}
	}
	
	// todo: add boxes, barrels	
}

function drawGarden( x, y ) {
	print("Making garden! " + x.tostring() + "," + y.tostring() + "\n");
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
				drawRoadTile( vx, vy, "street_end" );
			} else if ( i >= village_width - 1 ) {
				drawRoadTile( vx, vy, "street_end_180" );
			} else {
				drawRoadTile( vx, vy, "street" );
			}
		}
	}
	for( xx <- 3; xx <= village_height - 1; xx += 4 ) {
		vx = x + ( xx * MAP_UNIT );
		for ( i <- 1; i <= village_height; i++ ) {
			vy = y + ( i * MAP_UNIT );
			if ( i == 1 ) {
				drawRoadTile( vx, vy, "street_end_270" );
			} else if ( i % 4 == 0 ) {
				drawRoadTile( vx, vy, "street_cross" );
			} else if ( i >= village_height  ) {
				drawRoadTile( vx, vy, "street_end_90" );
			} else {
				drawRoadTile( vx, vy, "street_90" );
			}
		}
	}		
}

function drawRoadTile( x, y, ref ) {
	scourgeGame.getMission().flattenChunk( x, y - MAP_UNIT );
	scourgeGame.getMission().addOutdoorTexture( x, y, ref, 0, false, false );
}

function drawVillage( x, y, village_width, village_height ) {
	print( "Starting to draw village...\n" );
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
		
		vxx <- vx * 4;
		vyy <- (( vy * 4 ) + 1 );
		
		print( "\th=" + h.tostring() + " vx/y=" + vx.tostring() + "," + vy.tostring() + " vxx/yy=" + vxx.tostring() + "," + vyy.tostring() + "\n" ); 
		
		if( vxx >= village_width || vyy >= village_height ) continue;
		
		xp = x + vxx * MAP_UNIT;
		yp = y + vyy * MAP_UNIT;
		
		
		if( h == 0 ) {
			print( "amor shop at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawArmorShop( xp, yp )
		} else if( h == 3 ) {
			print( "weapon shop at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawWeaponShop( xp, yp )
		} else if( h == 6 ) {
			print( "inn at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawInn( xp, yp )
		} else if( h == 9 ) {
			print( "market at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawMarket( xp, yp );
		} else if( h == 12 ) {
			print( "garden at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawGarden( xp, yp );			
		} else if( h == 15 ) {
			print( "church at " + vx.tostring() + "," + vy.tostring() + "\n" );
			drawChurch( xp, yp );			
		} else {
			drawRandomHouse( xp, yp );
		}
		
		// add some peeps
		for( i <- 0; i < 5; i++ ) {
			scourgeGame.getMission().addCreatureAround( xp + 5 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 3.0 - 10.0) ).tointeger(),
			                                            yp - MAP_UNIT + 5 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 3.0 - 10.0) ).tointeger(),
			                                            0, getVillageNpcType() );
		}		
	}
	print( "Done drawing village.\n" );
}

function mixHouseCoordinates( pos ) {
  tmp <- 0;
  p <- 0;
  for( im <- 0; im < pos.len(); im++ ) {
  	p = ( scourgeGame.getMission().mt_rand() * pos.len().tofloat() ).tointeger();
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
		scourgeGame.getMission().setMapPosition( x + 2, y + h - 4, 0, "HOUSE_2_BASE" );
		
		// the door
		scourgeGame.getMission().setMapPosition( x + 2 + 1, y + h - 4 - 10, 0, "EW_DOOR" );
		
		// the top
		scourgeGame.getMission().setMapPosition( x + 2 - 2, y + h - 4 + 4, 12, "HOUSE_2_TOP" );
		
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

function drawHouse2x2Corner( x, y, angle ) {
	postfix <- "";
	roof_postfix <- "_RED";
	scourgeGame.getMission().startHouse();
	switch( angle ) {
	case 0:
		setPosition( x, y, 0, "P_BASE_2x2_CORNER" + postfix, true );
		setPosition( x + 21, y - 1, 0, "NS_DOOR", true );
		setPosition( x + 1, y - 7, 0, "EW_DOOR", true );
		break;
	case 90:
		setPosition( x, y, 0, "P_BASE_2x2_CORNER_90" + postfix, true );
		setPosition( x + 19, y - 1, 0, "NS_DOOR", true );
		setPosition( x + 30, y - 21, 0, "EW_DOOR", true );
		break;
	case 180:
		setPosition( x, y, 0, "P_BASE_2x2_CORNER_180" + postfix, true );
		setPosition( x + 5, y - 30, 0, "NS_DOOR", true );
		setPosition( x + 30, y - 19, 0, "EW_DOOR", true );
		break;
	case 270:
		setPosition( x, y, 0, "P_BASE_2x2_CORNER_270" + postfix, true );
		setPosition( x + 7, y - 30, 0, "NS_DOOR", true );
		setPosition( x + 1, y - 5, 0, "EW_DOOR", true );
		break;
	}
	scourgeGame.getMission().setMapFloorPosition( x,            y, "ROOM_FLOOR_TILE" );
	scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y, "ROOM_FLOOR_TILE" );
	scourgeGame.getMission().setMapFloorPosition( x,            y - MAP_UNIT, "ROOM_FLOOR_TILE" );
	scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y - MAP_UNIT, "ROOM_FLOOR_TILE" );
	setPosition( x - 2, y + 2, 12, "P_ROOF_2x2" + roof_postfix, true );
	scourgeGame.getMission().endHouse();
}

function drawHouse1x3Open( x, y, angle ) {
	scourgeGame.getMission().startHouse();
	switch( angle ) {
	case 0:
		setPosition( x, y, 0, "P_BASE_1x3_OPEN", true );
		setPosition( x + 1, y - 5, 0, "EW_DOOR", true );
		setPosition( x + 14, y - 37, 0, "EW_DOOR", true );
		scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - MAP_UNIT, "FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x, y - ( 2 * MAP_UNIT ), "FLOOR_TILE" );		
		setPosition( x - 2, y - 2, 12, "P_ROOF_1x3_OPEN", true );
		break;
	case 90:
		setPosition( x, y, 0, "P_BASE_1x3_OPEN_90", true );
		setPosition( x + 5, y - 1, 0, "NS_DOOR", true );
		setPosition( x + 37, y - 14, 0, "NS_DOOR", true );
		scourgeGame.getMission().setMapFloorPosition( x, y, "FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + MAP_UNIT, y, "FLOOR_TILE" );
		scourgeGame.getMission().setMapFloorPosition( x + 2 * MAP_UNIT, y, "FLOOR_TILE" );		
		setPosition( x + 2, y + 2, 12, "P_ROOF_1x3_OPEN_90", true );
		break;
	}
	scourgeGame.getMission().endHouse();
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

function drawTrail( x_start, y_start, path ) {
	tx <- x_start;
	ty <- y_start;
	ref <- "trail_end";
	angle <- 0;
	for( i <- 0; i < path.len(); i++ ) {
		if( path[i].slice(0, 1) == "e" ) {
			ref = "trail_end";
		} else if( path[i].slice(0, 1) == "p" ) {
			ref = "trail";
		} else if( path[i].slice(0, 1) == "t" ) {
			ref = "trail_turn";
		}
		angle = path[i].slice(1).tointeger();
		//print( "ref=" + ref.tostring() + " angle=" + angle.tostring() + " " + tx.tostring() + "," + ty.tostring() + "\n" );
		scourgeGame.getMission().addOutdoorTexture( tx, ty, ref, angle, false, false );
		
		// change direction: this is likely incorrect but works ok for now
		if( angle == 0 || angle == 270 ) {
			ty += 4;
		} else if( angle == 90 || angle == 180 ) {
			tx += 4;
		}
	}
}
