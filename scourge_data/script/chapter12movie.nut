function startChapter12Movie() {
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		key <- "chapter12Movie";
		value <- scourgeGame.getValue( key );
		//if( value == null || scourgeGame.getRerunMovies() ) {
			scourgeGame.setValue( key, "true" );
			
			// start movie mode
			scourgeGame.setMovieMode( true );

			scourgeGame.moveCamera( 230, 370, 0, 0, 50, 30, 0.7, 4000 );
			
			player <- scourgeGame.getPartyMember( 0 );
			player.say( _( "I feel a rumbling echo... nearby, yet from deep within the earth..." ) );
			
			scourgeGame.continueAt( "chapter12_part2", 5000 );
			
		//}
	}
}

function chapter12_part2() {
	scourgeGame.moveCamera( 230, 370, 0, 0, 50, 90, 0.7, 2000 );	
	scourgeGame.continueAt( "chapter12_part3", 3000 );
}

function chapter12_part3() {
	scourgeGame.getMission().quake();
	scourgeGame.getMission().setMapEffect( 320, 430, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);
	scourgeGame.getMission().setMapEffect( 315, 433, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       500,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);			
	scourgeGame.getMission().setMapEffect( 325, 444, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       1000,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);
	scourgeGame.moveCamera( 235, 370, 0, 0, 45, 100, 0.9, 2000 );	
	scourgeGame.continueAt( "chapter12_part4", 3000 );
}

function chapter12_part4() {
	scourgeGame.moveCamera( 240, 370, 0, 0, 45, 110, 1.0, 2000 );
	karzul <- scourgeGame.getMission().addCreature( 321, 437, 0, "Karzul Agmordexu" );
	karzul.moveTo( 321, 443 );
	karzul.setVisible( true );
	karzul.setScripted( true );
	karzul.setStateMod( 11, false );
	scourgeGame.getMission().setOffset( karzul.getX(), karzul.getY(), 0, 0, 0, -8 ); // karzul is sunk into the ground
	karzul.say( _( "Ooh it feels good to breathe the air again! I suggest you do so too for a last time, my friends..." ) );
	scourgeGame.getMission().setMapEffect( 320, 430, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);
	scourgeGame.getMission().setMapEffect( 315, 433, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       500,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);			
	scourgeGame.getMission().setMapEffect( 325, 444, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       1000,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);	
	scourgeGame.continueAt( "chapter12_part5", 5000 );
}

function chapter12_part5() {
	scourgeGame.moveCamera( 240, 370, 0, 0, 45, 110, 1.0, 2000 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	scourgeGame.getMission().setOffset( karzul.getX(), karzul.getY(), 0, 0, 0, -6 );
	karzul.say( _( "Yes my puny human worshippers, it is time to end your quest of insignificant mortal wanderings..." ) );
	scourgeGame.getMission().quake();
	scourgeGame.getMission().setMapEffect( 320, 430, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);
	scourgeGame.getMission().setMapEffect( 315, 433, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       500,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);			
	scourgeGame.getMission().setMapEffect( 325, 444, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       1000,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);	
	scourgeGame.continueAt( "chapter12_part6", 5000 );
}

function chapter12_part6() {
	scourgeGame.moveCamera( 240, 370, 0, 0, 45, 110, 1.0, 2000 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	scourgeGame.getMission().setOffset( karzul.getX(), karzul.getY(), 0, 0, 0, -4 );
	karzul.say( _( "For the beginning of a new era is upon us! The era of unbounded evil. With the help of the Spawn of Arcanox, nothing will stand in my way!" ) );
	scourgeGame.getMission().quake();
	scourgeGame.getMission().setMapEffect( 320, 430, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);
	scourgeGame.getMission().setMapEffect( 315, 433, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       500,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);			
	scourgeGame.getMission().setMapEffect( 325, 444, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       1000,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);	
	scourgeGame.continueAt( "chapter12_part7", 5000 );
}

function chapter12_part7() {
	scourgeGame.moveCamera( 240, 370, 0, 0, 45, 110, 1.0, 2000 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	scourgeGame.getMission().setOffset( karzul.getX(), karzul.getY(), 0, 0, 0, -2 );
	karzul.say( _( "Mothrazu served me well. Hells! YOU have served me well! As a reward I will personally see that your souls are well taken care of... by my people below." ) );
	scourgeGame.getMission().quake();
	scourgeGame.getMission().setMapEffect( 320, 430, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       0,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);
	scourgeGame.getMission().setMapEffect( 315, 433, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       500,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);			
	scourgeGame.getMission().setMapEffect( 325, 444, 4, // map location 
	                                       "EFFECT_SMOKE",  												// effect 
	                                       3, 3, 																	// base size
	                                       1000,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);	
	scourgeGame.continueAt( "chapter12_end", 5000 );
}

function chapter12_end() {
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	scourgeGame.getMission().setOffset( karzul.getX(), karzul.getY(), 0, 0, 0, 0 );	
	karzul.setNpc( false );
	scourgeGame.setMovieMode( false );
}
