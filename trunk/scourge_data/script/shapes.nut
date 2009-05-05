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
	} else if( startsWith( shape_name, "P_ROOF_1x3_OPEN_90" ) ) {
		scourgeGame.getMission().setMapEffect( x + 4, y - 6, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
		scourgeGame.getMission().setMapEffect( x + 30, y - 14, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
	} else if( startsWith( shape_name, "P_ROOF_1x3_OPEN" ) ) {
		scourgeGame.getMission().setMapEffect( x + 4, y - 12, z, // map location 
	                                       "EFFECT_SMOKE",												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0.5, 0, 7, 														// offset
	                                       0.2, 0.2, 0.5 														// color
																				);
		scourgeGame.getMission().setMapEffect( x + 13, y - 38, z, // map location 
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
	if( startsWith( shape_name, "P_BASE_2x2_CORNER_270" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -30, 0, 7, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 13, -30, 0, 19, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -0, 0, 32, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 28, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -11, 0, 2, 19, 12, false );
	} else if( startsWith( shape_name, "P_BASE_2x2_CORNER_180" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -30, 0, 5, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 11, -30, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 32, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 28, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 17, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -25, 0, 2, 5, 12, false );
	} else if( startsWith( shape_name, "P_BASE_2x2_CORNER_90" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -30, 0, 32, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 0, -0, 0, 19, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 25, -0, 0, 7, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 19, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -27, 0, 2, 3, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 28, 12, false );
	} else if( startsWith( shape_name, "P_BASE_2x2_CORNER" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -30, 0, 32, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 21, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 27, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -2, 0, 2, 5, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -13, 0, 2, 17, 12, false );
		scourgeGame.addVirtualShape( shape_name, 30, -2, 0, 2, 28, 12, false );
	} else if( startsWith( shape_name, "P_BASE_2x2" ) ) {
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
	} else if( startsWith( shape_name, "P_BASE_1x3_OPEN_90" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -14, 0, 37, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 43, -14, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 5, 2, 12, false );
		scourgeGame.addVirtualShape( shape_name, 11, 0, 0, 37, 2, 12, false );
	} else if( startsWith( shape_name, "P_BASE_1x3_OPEN" ) ) {
		scourgeGame.addVirtualShape( shape_name, 0, -46, 0, 2, 2, 12, true );
		scourgeGame.addVirtualShape( shape_name, 0, 0, 0, 2, 5, 12, false );
		scourgeGame.addVirtualShape( shape_name, 0, -11, 0, 2, 35, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, 0, 0, 2, 37, 12, false );
		scourgeGame.addVirtualShape( shape_name, 14, -43, 0, 2, 5, 12, false );		
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
