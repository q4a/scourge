//REGION_WIDTH <- ( 74 * 4 );
//REGION_DEPTH <- ( 74 * 4 );

/*
 * Return true if squirrel took care of the land generation.
 * Return false to continue with the regular terrain generation.
 */
function generate_land( region_x, region_y, offset_x, offset_y ) {
	print( "Generate land at " + region_x.tostring() + "," + region_y.tostring() + 
	       " offset=" + offset_x.tostring() + "," + offset_y.tostring() + "\n" );
	if( region_x == 37 && region_y == 18 ) {
		drawHouseTower( offset_x + 8 * MAP_UNIT, offset_y + 16 * MAP_UNIT );
	} else if( region_x == 41 && region_y == 21 ) {
		drawHorghhHQ( offset_x, offset_y + MAP_UNIT + MAP_UNIT );
	}
	return false;
}

function generate_city( region_x, region_y, offset_x, offset_y, w, h ) {
	drawVillage( offset_x, offset_y, w * 4, h * 4 );
}

function generate_tree( region_x, region_y, offset_x, offset_y, x, y, climate_value, vegetation_value ) {
	climate <- getClimate( climate_value );
	if( climate == null ) return;
	vegetation <- getVegetation( vegetation_value );
	if( vegetation == null ) return;
		
	if( in_excluded_area( region_x, region_y, x, y ) ) return;
	
	tree <- getTreeForClimate(climate);
	xp <- offset_x + x;
	yp <- offset_y + y;
	rate <- ( scourgeGame.getMission().mt_rand() * 100.0 ).tointeger();
	if( rate < vegetation.getTreeRate() && scourgeGame.getMission().isFreeOutdoors( xp, yp, 0, tree ) ) {
		setPosition( xp, yp, 0, tree, false );
	}
}

function get_ground_texture( climate_value, vegetation_value ) {
	climate <- getClimate( climate_value );
	if( climate == null ) return "grass";
	//vegetation <- getVegetation( vegetation_value );
	//if( vegetation == null ) return "grass";
	return climate.getGroundTexture();
}

/**
 * Areas to not cover with trees (towns, roads, etc.)
 */
function in_excluded_area( region_x, region_y, x, y ) {
	return inHq( region_x, region_y, x, y ) || inCity( region_x, region_y, x, y );
}

// Music selection step 1: find the music key for the current location or failing that for the current vegetation or climate. 
// This is done in two steps to avoid switching out the currently playing music if we're going to load a new music for the same key.
function get_music_key( region_x, region_y, x, y, climate_value, vegetation_value ) {
	if( inHq( region_x, region_y, x, y ) ) {
		return "hq"
	} else if( inCity( region_x, region_y, x, y ) ) {
		return "city";
	} else {
		vegetation <- getVegetation( vegetation_value );
		climate <- getClimate( climate_value );
		if( vegetation != null && climate != null ) {
			if( vegetation.getclass() == LightForestVegetation || vegetation.getclass() == DeepForestVegetation ) {
				return "forest";
			}
		}
	}
	return "general";
}

// Music selection step 2: Load a music file for a specific key. 
// All music files are prefixed with scourge_data/sound/music in C++.
// I was going to use a squirrel table with an eval function for this but couldn't get it to work. :-(
function get_music_name( music_key ) {
	print( "get_music_name key=" + music_key + "\n" );
	if( music_key == "hq" ) {
		return pickOne( [ "headquarters.ogg" ] );
	} else if( music_key == "city" ) { 
		return pickOne( [ "town1.ogg", "town2.ogg" ] );
	} else if( music_key == "forest" ) {
		return pickOne( [ "outdoors1.ogg", "outdoors2.ogg", "outdoors3.ogg" ] );
	} else if( music_key == "general" ) { 
		return pickOne( [ "outdoors1.ogg", "outdoors2.ogg", "outdoors3.ogg" ] );
	} else {
		return "";
	}
}

// Music selection step 2: Ambient sound samples selection. Return a list of possible sounds for a music_key.
// Maybe play different sounds during day/night?
function get_ambients( music_key ) {
	print( "get_ambients key=" + music_key + "\n" );
	if( music_key == "hq" || music_key == "city" ) {
		return "city01.ogg,city02.ogg,city03.ogg,city04.ogg,city05.ogg";
	} else if( music_key == "forest" || music_key == "general" ) {
		return "crickets.ogg,eagleowl.wav,frog.wav,grasshopper.wav,windgust01.wav,windgust02.wav,wolf.wav"
	} else {
		return "";
	}
}

function inHq( region_x, region_y, x, y ) {
	return region_x == 41 && region_y == 21 && x > 55;
}

function inCity( region_x, region_y, x, y ) {
	return scourgeGame.getMission().getCityName( region_x, region_y, x, y ) != null || inHorggh( region_x, region_y, x, y );
}

function inHorggh( region_x, region_y, x, y ) {
	return region_x == 40 && region_y == 21 && x >= 163 && x < 275 && y >= 20 && y < 200;
}

function draw_path_chunk( x, y, texture_name, angle ) {
	if( x >= 0 && y >= 0 && x < MAP_WIDTH && y < MAP_DEPTH ) {
		//print("drawing texture " + texture_name + "\n");
		scourgeGame.getMission().flattenChunkWalkable( x + MAP_UNIT / 2, y - MAP_UNIT / 2 );
		scourgeGame.getMission().addOutdoorTexture( x, y, texture_name, angle, false, false );
	}
}

road_turns <- [ "road_turn", "road_cutoff" ];
function get_road_turn() {
	return pickOne( road_turns );
}

last_abs_x <- -1;
last_abs_y <- -1;

function start_draw_path() {
	last_abs_x = -1;
	last_abs_y = -1;	
}

function draw_path( current_rx, current_ry, offs_x, offs_y, rx, ry, x, y, walksX ) {
	abs_x <- rx * ( MAP_WIDTH / 2 ) + x;
	abs_y <- ry * ( MAP_DEPTH / 2 ) + y;
	
	real_x <- offs_x + x;
	real_y <- offs_y + y;
						
	if( rx == current_rx && ry == current_ry ) {
		//print( "adding road, region=" + rx.tostring() + "," + ry.tostring() + " pos=" + x.tostring() + "," + y.tostring() + " walksX=" + walksX.tostring() + "\n" );
			
		if( abs_x == last_abs_x ) {
			//print( "horiz\n" );
			draw_path_chunk( real_x, real_y, "road", 0 );
		} else if( abs_y == last_abs_y ) {
			//print( "vert\n" );
			draw_path_chunk( real_x, real_y, "road", 90 );					
		} else if( abs_y < last_abs_y && abs_x > last_abs_x ) {
			if( walksX ) {
				//print( "1\n" );
				draw_path_chunk( real_x, real_y, get_road_turn(), 180 );
				draw_path_chunk( real_x, real_y + MAP_UNIT, get_road_turn(), 0 );
			} else {
				//print( "2\n" );
				draw_path_chunk( real_x, real_y, get_road_turn(), 90 );
				draw_path_chunk( real_x, real_y + MAP_UNIT, get_road_turn(), 270 );						
			}
		} else if( abs_y > last_abs_y && abs_x < last_abs_x ) {
			//print( "3\n" );
			draw_path_chunk( real_x, real_y, get_road_turn(), 180 );
			draw_path_chunk( real_x + MAP_UNIT, real_y, get_road_turn(), 0 );
		} else if( abs_y > last_abs_y && abs_x > last_abs_x ) {
			if( walksX ) {
				//print( "4\n" );
				draw_path_chunk( real_x, real_y, get_road_turn(), 270 );
				draw_path_chunk( real_x, real_y - MAP_UNIT, get_road_turn(), 90 );
			} else {
				//print( "5\n" );
				draw_path_chunk( real_x, real_y, get_road_turn(), 90 );
				draw_path_chunk( real_x - MAP_UNIT, real_y, get_road_turn(), 270 );
			}
		} else {
			print( "*** Error: unknown road section, region=" + rx.tostring() + "," + ry.tostring() + " pos=" + x.tostring() + "," + y.tostring() + " walksX=" + walksX.tostring() + "\n" );
		}
	}
		
	last_abs_x = abs_x;
	last_abs_y = abs_y;
}

///*
// * Return the name of the map to load at this position in the land.
// * If null is returned, a random map is generated.
// */
//function descend_dungeon( region_x, region_y, offset_x, offset_y ) {
//	print( "descending in region: " + region_x.tostring() + "," + region_y.tostring() + " offset: " + offset_x.tostring() + "," + offset_y.tostring() + "\n" );
//	if( region_x == 41 && region_y == 21 ) {
//		return "hq";
//	}
//	return "";
//}

//*************************************************************
// Horggh HQ
//
function drawHorghhHQ( x, y ) {
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
			drawRoadTile( vx, vy, "street_end_180" );
		} else {
			drawRoadTile( vx, vy, "street" );
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
	furnishArea( x - MAP_UNIT, y - MAP_UNIT, 2, 2 );
	drawHouse2x2Corner( x, y + 4 * MAP_UNIT, 180 )
	furnishArea( x, y + 4 * MAP_UNIT, 2, 2 );
	drawHouse2x2Corner( x + 4 * MAP_UNIT, y - 2 * MAP_UNIT, 0 )
	furnishArea( x + 4 * MAP_UNIT, y - 2 * MAP_UNIT, 2, 2 );	
	drawHouse2x2Corner( x + 5 * MAP_UNIT, y + 3 * MAP_UNIT, 270 )
	furnishArea( x + 5 * MAP_UNIT, y + 3 * MAP_UNIT, 2, 2 );
	
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
	
	//setPosition( x + 4 * MAP_UNIT, y - MAP_UNIT, 0, "GATE_DOWN", false );
	
	// the trail
	drawTrail( x + MAP_UNIT + 4, y - MAP_UNIT - 12, 
	           [ "e0", "p0", "t90",
	             "p90", "p90", "p90", "p90", "p90", "p90", "p90", "p90",
	             "t270", "p0", "p0", "p0", "t90",
	             "p90", "p90", "p90", "t270", "e180" ] );
	
	// add some npc-s
	uzudil <- scourgeGame.getMission().addCreature( x - 6 * MAP_UNIT + 88, y - 7 * MAP_UNIT + 75, 0, "Uzudil the Hand");
	uzudil.setNpcInfo( _("Uzudil the Hand"), "sage", "" );
	uzudil.setLevel( 4 );
	uzudil.setConversation( "hq" );
	melathor <- scourgeGame.getMission().addCreature( x - 6 * MAP_UNIT + 180, y - 7 * MAP_UNIT + 58, 0, "Melathor of Allovium");
	melathor.setNpcInfo( _("Melathor of Allovium"), "trainer", "Magician" );
	melathor.setLevel( 15 );
	melathor.setConversation( "hq" );

	npc <- null
	for( i <- 0; i < 10; i++ ) {
		npc = scourgeGame.getMission().addCreatureAround( x - 6 * MAP_UNIT + 85 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 8.0) ).tointeger(),
		                                                  y - 7 * MAP_UNIT + 70 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 8.0) ).tointeger(),
		                                                  0, getVillageNpcType() );
		npc.setConversation( "hq" );
	}
	
	// add some wandering heros
	for( i <- 0; i < 10; i++ ) {
		scourgeGame.getMission().addWanderingHero( x - 6 * MAP_UNIT + 85 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 8.0) ).tointeger(),
		                                           y - 7 * MAP_UNIT + 70 + ( scourgeGame.getMission().mt_rand() * (MAP_UNIT * 8.0) ).tointeger(),
		                                           0, 
		                                           1 );
	}
	
}

