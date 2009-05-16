REGION_WIDTH <- ( 74 * 4 );
REGION_DEPTH <- ( 74 * 4 );

/*
 * Return true if squirrel took care of the land generation.
 * Return false to continue with the regular terrain generation.
 */
function generate_land( region_x, region_y, offset_x, offset_y ) {
	print( "Generate land at " + region_x.tostring() + "," + region_y.tostring() + 
	       " offset=" + offset_x.tostring() + "," + offset_y.tostring() + "\n" );
	if( region_x == 38 && region_y == 20 ) {
		drawVillage( offset_x + MAP_UNIT, offset_y + MAP_UNIT + MAP_UNIT, 15, 15 );
		return true;
	} else if( region_x == 37 && region_y == 18 ) {
		//drawRandomHouse( offset_x + 8 * MAP_UNIT, offset_y + 16 * MAP_UNIT );
		drawHouseTower( offset_x + 8 * MAP_UNIT, offset_y + 16 * MAP_UNIT );
	} else if( region_x == 40 && region_y == 21 ) {
		drawHorghh_part1( offset_x + 6 * MAP_UNIT, offset_y + MAP_UNIT + MAP_UNIT );
		return true;
	} else if( region_x == 41 && region_y == 21 ) {
		drawHorghh_part2( offset_x, offset_y + MAP_UNIT + MAP_UNIT );
		return true;
	}
	return false;
}

/*
 * Return the name of the map to load at this position in the land.
 * If null is returned, a random map is generated.
 */
function descend_dungeon( region_x, region_y, offset_x, offset_y ) {
	return null;
}

//*************************************************************
// Horggh
//
function drawHorghh_part1( x, y ) {
	print( ">>> Drawing Horghh part 1 " + x.tostring() + "," + y.tostring() + "\n");
	// one road goes thru Horghh
	vx <- x + 7 * MAP_UNIT;
	vy <- 0;
	for( yy <- 0; yy <= 9; yy ++ ) {
		vy = y + ( yy * MAP_UNIT );
		if ( yy == 0 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_END_270 );
		} else if ( yy >= 9  ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_END_90 );
		} else if( yy == 3 || yy == 7 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_CROSS );			
		} else {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_90 );
		}
	}
	vy = y + ( 3 * MAP_UNIT );
	for( xx <- 4; xx <= 11; xx ++ ) {
		vx = x + ( xx * MAP_UNIT );
		if ( xx == 4 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_END );
		} else if ( xx >= 11 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_END_180 );
		} else if ( xx != 7 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET );

		}
	}
	vy = y + ( 7 * MAP_UNIT );
	for( xx <- 6; xx <= 11; xx ++ ) {
		vx = x + ( xx * MAP_UNIT );
		if ( xx == 6 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_END );
		} else if ( xx != 7 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET );

		}
	}
	
	// some houses
	drawRandomHouse( x + 8 * MAP_UNIT, y );
	drawRandomHouse( x + 8 * MAP_UNIT, y + 4 * MAP_UNIT );
	drawInn( x + 4 * MAP_UNIT, y + 4 * MAP_UNIT );
	//drawRandomHouse( x + 11 * MAP_UNIT, y + 8 * MAP_UNIT );
	drawGarden( x + 4 * MAP_UNIT, y + 8 * MAP_UNIT );
}

function drawHorghh_part2( x, y ) {
	print( ">>> Drawing Horghh part 2 " + x.tostring() + "," + y.tostring() + "\n");
	// one road goes thru Horghh
	vx <- 0;
	vy <- y + ( 7 * MAP_UNIT );
	
	// the bridge
	for( i <- x + 2 * MAP_UNIT; i < x + 5 * MAP_UNIT; i += MAP_UNIT ) {
		setPosition( i, y + 6 * MAP_UNIT + 5, 0, "BRIDGE_90", false );
		setPosition( i, y + 7 * MAP_UNIT - 2, 0, "BRIDGE_270", false );
	}
	
	// hq building
	drawHQ( x + 6 * MAP_UNIT, y + 5 * MAP_UNIT );
	
	// the road to hq
	for( xx <- 0; xx <= 5; xx ++ ) {
		vx = x + ( xx * MAP_UNIT );
		if ( xx >= 5 ) {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET_END_180 );
		} else {
			drawRoadTile( vx, vy, OUTDOOR_THEME_REF_STREET );
		}
	}
	
	// the isthmus leading to hq
	for( xx <- x + 2 * MAP_UNIT + 8; xx < x + 5 * MAP_UNIT; xx++ ) {
		scourgeGame.getMission().setHeightMap( xx, vy - 20, -10 );
		scourgeGame.getMission().setHeightMap( xx, vy - 16, -10 );	
		scourgeGame.getMission().setHeightMap( xx, vy + 2, -10 );
		scourgeGame.getMission().setHeightMap( xx, vy + 6, -10 );
	}
}

function drawHQ( x, y ) {
	for( xx <- x + MAP_UNIT; xx < x + 5 * MAP_UNIT; xx++ ) {
		for( yy <- y - 2 * MAP_UNIT; yy < y + 4 * MAP_UNIT; yy++ ) {
			scourgeGame.getMission().setHeightMap( xx, yy, 0 );	
		}
	}
	drawHouse2x2Corner( x - MAP_UNIT, y - MAP_UNIT, 90 )
	drawHouse2x2Corner( x, y + 4 * MAP_UNIT, 180 )
	drawHouse2x2Corner( x + 4 * MAP_UNIT, y - 2 * MAP_UNIT, 0 )
	drawHouse2x2Corner( x + 5 * MAP_UNIT, y + 3 * MAP_UNIT, 270 )
	
	drawHouse1x3Open( x, y + 2 * MAP_UNIT, 0 );
	furnishArea( x, y + 2 * MAP_UNIT, 1, 3 );
	drawHouse1x3Open( x + 5 * MAP_UNIT, y + MAP_UNIT, 0 );
	furnishArea( x + 5 * MAP_UNIT, y + MAP_UNIT, 1, 3 );
	drawHouse1x3Open( x + MAP_UNIT, y - 2 * MAP_UNIT, 90 );
	furnishArea( x + MAP_UNIT, y - 2 * MAP_UNIT, 3, 1 );
	drawHouse1x3Open( x + 2 * MAP_UNIT, y + 3 * MAP_UNIT, 90 );
	furnishArea( x + 2 * MAP_UNIT, y + 3 * MAP_UNIT, 3, 1 );
	
	drawHousePart( "", "_RED",  x + MAP_UNIT, y, 2, 1, 180, true );
	//drawHousePart( "", "_RED",  x + 2 * MAP_UNIT, y + MAP_UNIT, 1, 1, 270, true );
	drawHousePart( "", "_RED",  x + 4 * MAP_UNIT, y + MAP_UNIT, 1, 1, 0, true );
	//setPosition( x - MAP_UNIT, y + MAP_UNIT, 0, getTree(), false );
	setPosition( x - MAP_UNIT + 3, y + MAP_UNIT - 12, 0, getTree(), false );
	setPosition( x - 2 * MAP_UNIT, y - MAP_UNIT, 0, getTree(), false );
	setPosition( x - 2 * MAP_UNIT + 5, y - MAP_UNIT - 12, 0, getTree(), false );
	setPosition( x - MAP_UNIT, y + 4 * MAP_UNIT, 0, getTree(), false );
	setPosition( x - MAP_UNIT + 4, y + 4 * MAP_UNIT - 14, 0, getTree(), false );
	setPosition( x + MAP_UNIT + 9, y + 2 * MAP_UNIT - 8, 0, getTree(), false );
	setPosition( x + MAP_UNIT + 25, y + 2 * MAP_UNIT - 17, 0, getTree(), true );
	
	setPosition( x + 4 * MAP_UNIT, y - MAP_UNIT, 0, "GATE_DOWN", false );
	
	// the trail
	drawTrail( x + MAP_UNIT + 4, y - MAP_UNIT - 12, 
	           [ "e0", "p0", "t90",
	             "p90", "p90", "p90", "p90", "p90", "p90", "p90", "p90",
	             "t270", "p0", "p0", "p0", "t90",
	             "p90", "p90", "p90", "t270", "e180" ] );
}

