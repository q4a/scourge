// todo: 
// - conversations to show until cleared or changed
// - register a cleanup function to run if movie is Esc-aped
// - make tooltips show as scrolls
// - better tooltips text for new models (houses, etc.)
// - extra decoration in Karzul's 'pen'
// - summon monster spell
// - karzul to summon 'help' (demons)

ch12_x <- 260;
ch12_y <- 400;

function startChapter12Movie() {
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		key <- "chapter12Movie";
		value <- scourgeGame.getValue( key );
		//if( value == null || scourgeGame.getRerunMovies() ) {
			scourgeGame.setValue( key, "true" );
			
			// start movie mode
			scourgeGame.setMovieMode( true );
			
			scourgeGame.setInterruptFunction( "chapter12MovieInterrupt" )

			scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 30, 0.8, 4500 );
			
			player <- scourgeGame.getPartyMember( 0 );
			player.say( _( "I feel a rumbling echo... nearby, yet from deep within the earth..." ) );
			
			scourgeGame.continueAt( "chapter12_part2", 5000 );
			
		//}
	}
}

function chapter12_part2() {
	player <- scourgeGame.getPartyMember( 0 );
	player.say( _( "Like clouds gathering before a storm... An evil presence moves out of the darkness below..." ) );	
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 90, 0.7, 4500 );	
	scourgeGame.continueAt( "chapter12_part3", 5000 );
}

function chapter12_part3() {
	player <- scourgeGame.getPartyMember( 0 );
	player.clearSpeech();
	scourgeGame.getMission().setMapEffect( 325, 444, 4, // map location 
	                                       "EFFECT_RING",  												// effect 
	                                       20, 20, 																	// base size
	                                       0,																			// delay
	                                       true,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);	
	scourgeGame.moveCamera( ch12_x + 5, ch12_y, 0, 0, 45, 100, 0.9, 4500 );	
	scourgeGame.continueAt( "chapter12_part4", 5000 );
}

function chapter12_part4() {
	scourgeGame.moveCamera( ch12_x + 10, ch12_y, 0, 0, 45, 110, 1.0, 4500 );
	karzul <- scourgeGame.getMission().addCreature( 320, 449, 0, "Karzul Agmordexu" );
	karzul.setScripted( true );
	//karzul.setVisible( true );	
	//karzul.setStateMod( 11, false );
	karzul.setOffset( 0.0, 0.0, -8.0 ); // karzul is sunk into the ground
	karzul.setAnimation( MD2_PAIN1 );
	drawEffects();
	scourgeGame.continueAt( "chapter12_part5", 5000 );
}

function chapter12_part5() {
	scourgeGame.moveCamera( ch12_x + 10, ch12_y, 0, 0, 45, 110, 1.0, 4500 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.setOffset( 0.0, 0.0, -6.0 ); // karzul is sunk into the ground
	karzul.setAnimation( MD2_PAIN2 );
	drawEffects();	
	scourgeGame.continueAt( "chapter12_part6", 5000 );
}

function chapter12_part6() {
	scourgeGame.moveCamera( ch12_x + 10, ch12_y, 0, 0, 45, 105, 1.0, 4500 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.setOffset( 0.0, 0.0, -4.0 ); // karzul is sunk into the ground
	karzul.setAnimation( MD2_PAIN3 );
	drawEffects();
	scourgeGame.continueAt( "chapter12_part7", 5000 );
}

function chapter12_part7() {
	scourgeGame.moveCamera( ch12_x + 10, ch12_y, 0, 0, 45, 100, 1.0, 4500 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.setOffset( 0.0, 0.0, -2.0 ); // karzul is sunk into the ground
	karzul.setAnimation( MD2_PAIN1 );
	drawEffects();	
	scourgeGame.continueAt( "chapter12_part8", 5000 );
}

function chapter12_part8() {
	scourgeGame.moveCamera( ch12_x - 10, ch12_y + 5, 0, 0, 70, 100, 1.3, 6500 );
	scourgeGame.getMission().removeMapEffect( 325, 444, 4 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.setOffset( 0.0, 0.0, 0.0 ); // karzul is sunk into the ground
	karzul.setAnimation( MD2_STAND );
	karzul.moveTo( 315, 448 ); // face the camera	
	karzul.say( _( "Ooh it feels good to breathe the air again! I suggest you do so too for a last time, my friends. Yes my puny human worshippers, time to end your insignificant mortal wanderings..." ) );
	scourgeGame.continueAt( "chapter12_part9", 7000 );
}

function chapter12_part9() {
	scourgeGame.moveCamera( ch12_x - 10, ch12_y - 5, 0, 0, 70, 85, 1.3, 6500 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.moveTo( 320, 448 ); // face the camera
	karzul.say( _( "Your quest has ceased to be, for the beginning of a new era is upon us! The era of unbounded evil. With the help of the Spawn of Arcanox, nothing will stand in my way!" ) );
	scourgeGame.continueAt( "chapter12_part10", 7000 );
}

function chapter12_part10() {
	scourgeGame.moveCamera( ch12_x - 10, ch12_y - 10, 0, 0, 70, 70, 1.35, 6500 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.say( _( "Mothrazu served me well. Hells! YOU have served me well! As a reward I will personally see that your souls are well taken care of... by my people below." ) );
	scourgeGame.continueAt( "chapter12_end", 7000 );	
}

function chapter12_end() {
	karzul <- findCreatureByType( "Karzul Agmordexu" );	
	karzul.setNpc( false );
	scourgeGame.setMovieMode( false );
}

function chapter12MovieInterrupt() {
	print( "in chapter12MovieInterrupt\n" )
	scourgeGame.getMission().removeMapEffect( 325, 444, 4 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	if( karzul == null ) {
		karzul = scourgeGame.getMission().addCreature( 320, 449, 0, "Karzul Agmordexu" );		
	}
	karzul.setScripted( true );
	karzul.setOffset( 0.0, 0.0, 0.0 );
	karzul.setNpc( false );
}

function drawEffects() {
	scourgeGame.getMission().thunder();
	scourgeGame.getMission().quake();
	scourgeGame.getMission().setMapEffect( 320, 442, 4, // map location 
	                                       "EFFECT_TELEPORT",  												// effect 
	                                       8, 8, 																	// base size
	                                       0,																			// delay
	                                       false,																	// forever 
	                                       0, 0, 0, 														// offset
	                                       1, 0.2, 0.2 														// color
																				);		
	
	i <- 0;
	x <- 0;
	y <- 0;
	angle <- 0;
	for( i = 0; i < 30; i++ ) {
		angle = ( 360.0 * rand() / RAND_MAX.tofloat() );
		x = 320 + 15.0 * cos( angle );
		y = 444 + 15.0 * sin( angle );
		scourgeGame.getMission().setMapEffect( x, y, 4, // map location 
		                                       ( i < 15 ? "EFFECT_SMOKE" : "EFFECT_FIRE" ),  												// effect 
		                                       3, 3, 																	// base size
		                                       200 * i,																			// delay
		                                       false,																	// forever 
		                                       0, 0, 0, 														// offset
		                                       1, 0.2, 0.2 														// color
																					);
	}
}



// ***********************************************************
// ***********************************************************
// ***********************************************************


game_end_pause <- 8000

function startGameEndMovie() {
	scourgeGame.setWeather( WEATHER_CLEAR );
	scourgeGame.setMovieMode( true );
	scourgeGame.setInterruptFunction( "gameEndMovieInterrupt" )

	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 30, 0.8, game_end_pause - 500 );
	
	c1 <- findCreatureByType( "Norill" );
	if( c1 != null ) {
		c1.setScripted( true );
		c1.moveTo( 325, 445 );
	}
	c2 <- findCreatureByType( "Sarrez" );
	if( c2 != null ) {
		c2.setScripted( true );
		c2.moveTo( 330, 448 );
	}
	c3 <- findCreatureByType( "Rolan" );
	if( c3 != null ) {
		c3.setScripted( true );
		c3.moveTo( 325, 451 );
	}
	
	player <- scourgeGame.getPartyMember( 0 );
	player.moveTo( 325, 448 );
	player.say( _( "The demon was vanquished with the aid of the Guardians of the Realm!" ) );
	
	scourgeGame.continueAt( "gameEndPart2", game_end_pause );
}

function gameEndPart2() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 30, 0.8, game_end_pause - 600 );
	player <- scourgeGame.getPartyMember( 0 );
	player.say( _( "...and yet I feel it will take another century for the stench of corruption to leave this place." ) );
	scourgeGame.continueAt( "gameEndPart3", game_end_pause );
}

function gameEndPart3() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 35, 0.82, game_end_pause - 600 );
	player <- scourgeGame.getPartyMember( 0 );
	player.say( _( "I sense a foreboding that the events that occured here are just the beginning of another tale..." ) );
	scourgeGame.continueAt( "gameEndPart4", game_end_pause );
}

function gameEndPart4() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 40, 0.84, game_end_pause - 600 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.say( _( "Yes mortal... you bested me in your realm. For this you deserve my admiration. Not many can claim victory over Karzul the mighty..." ) );	
	scourgeGame.continueAt( "gameEndPart5", game_end_pause );
}

function gameEndPart5() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 45, 0.86, game_end_pause - 600 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.say( _( "However with great deeds come great responsibilities. For know that my spirit is immortal. You have thwarted my designs on your realm, a fact I did not foresee..." ) );	
	scourgeGame.continueAt( "gameEndPart6", game_end_pause );
}

function gameEndPart6() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 50, 0.88, game_end_pause - 600 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.say( _( "Rest now mortal and I will see you soon enough... I will leave you with a question to ponder until we meet again:" ) );	
	scourgeGame.continueAt( "gameEndPart7", game_end_pause );
}

function gameEndPart7() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 55, 0.9, game_end_pause - 600 );
	karzul <- findCreatureByType( "Karzul Agmordexu" );
	karzul.say( _( "With your precious Guardians gone... who will you hide behind the next time we meet?" ) );	
	scourgeGame.continueAt( "gameEndPart8", game_end_pause );
}

function gameEndPart8() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 60, 0.92, game_end_pause - 600 );
	rolan <- findCreatureByType( "Rolan" );
	rolan.say( _( "Begone fiend! Naught have you become, bringer of ails as of naught you were conceived in your cold lair of void." ) );	
	scourgeGame.continueAt( "gameEndPart9", game_end_pause );
}

function gameEndPart9() {
	scourgeGame.moveCamera( ch12_x, ch12_y, 0, 0, 50, 65, 0.94, game_end_pause - 600 );
	rolan <- findCreatureByType( "Rolan" );
	rolan.say( _( "Pay no heed to that chaotic windbag! Celebrate instead for the land has new heroes tonight: ...you!" ) );	
	scourgeGame.continueAt( "gameEndPartLast", game_end_pause );
}

function gameEndPartLast() {
	scourgeGame.setMovieMode( false );
	//scourgeGame.getMission().setCompleted( true );
	scourgeGame.finale( _( "You notice the sun rising in the east. Around you the land slowly wakes, unaware of the catastrophy the most unlikely heroes have just averted. Truly you are heroes of lesser renown.||The demon's corpse disintegrates in front of your eyes. Its corrupted flesh becomes sand which the wind quickly scatters.||The monastery's walls stand like quiet sentinels, the aged stones showing no understanding of the monks' sins finally atoned.||As promised, the Guardians depart without another word.||Congratulations! You have completed the game!" ),
	                   "finale.png" );
	//scourgeGame.showTextMessage( _( "You notice the sun rising in the east. Around you the land slowly wakes, unaware of the catastrophy the most unlikely heroes have just averted. Truly you are heroes of lesser renown.||The demon's corpse disintegrates in front of your eyes. Its corrupted flesh becomes sand which the wind quickly scatters.||The monastery's walls stand like quiet sentinels, the aged stones showing no understanding of the monks' sins finally atoned.||As promised, the Guardians depart without another word.||Congratulations! You have completed the game!" ) );
}

function gameEndMovieInterrupt() {
	gameEndPartLast();
}
