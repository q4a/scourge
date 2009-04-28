REGION_WIDTH <- ( 74 * 4 );
REGION_DEPTH <- ( 74 * 4 );

/*
 * Return true if squirrel took care of the land generation.
 * Return false to continue with the regular terrain generation.
 */
function generate_land( region_x, region_y, offset_x, offset_y ) {
	if( region_x == 38 && region_y == 20 ) {
		horghh_q1( offset_x, offset_y );
		return true;
	} else if( region_x == 37 && region_y == 18 ) {
		//drawRandomHouse( offset_x + 8 * MAP_UNIT, offset_y + 16 * MAP_UNIT );
		drawHouseTower( offset_x + 8 * MAP_UNIT, offset_y + 16 * MAP_UNIT );
	}
	return false;
}

VILLAGE_OFFSET <- MAP_UNIT

function horghh_q1( offset_x, offset_y ) {
	hx <- offset_x;
	hy <- offset_y + MAP_UNIT;
	print( "Generating city Q1 at: " + hx.tostring() + "," + hy.tostring() + "\n" );
	drawVillage( hx + VILLAGE_OFFSET, hy + VILLAGE_OFFSET, 15, 15 );
}
