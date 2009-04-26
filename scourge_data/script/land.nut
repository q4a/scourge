REGION_WIDTH <- ( 74 * 4 );
REGION_DEPTH <- ( 74 * 4 );

function land_generated( region_x, region_y ) {
	if( region_x == 38 && region_y == 20 ) {
		horghh_q1();
	}
}

function horghh_q1() {
	print( "Generating city Q1\n" );
	//flatten_region();
}

// *** NOTE: calling flattenChunk causes errors in texture's ref counting ***
//function flatten_region() {
//	for ( vx <- 0; vx < REGION_WIDTH; vx++ ) {
//		for ( vy <- 0; vy < REGION_DEPTH; vy++ ) {
//			scourgeGame.getMission().flattenChunk( vx, vy );
//		}
//	}
//}