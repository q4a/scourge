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
	}
	return false;
}

VILLAGE_OFFSET <- MAP_UNIT

function horghh_q1( offset_x, offset_y ) {
	hx <- offset_x;
	hy <- offset_y + MAP_UNIT;
	print( "Generating city Q1 at: " + hx.tostring() + "," + hy.tostring() + "\n" );
	for( xx <- 0; xx < 2; xx++ ) {
		for( yy <- 0; yy < 2; yy++ ) {
			drawRandomHouse( hx + VILLAGE_OFFSET + xx * MAP_UNIT * 4, hy + VILLAGE_OFFSET + yy * MAP_UNIT * 4 );
		}
	}
}
