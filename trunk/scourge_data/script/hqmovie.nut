function startHqMovie() {
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		key <- "hqMovie";
		value <- scourgeGame.getValue( key );
		//if( value == null ) {
			scourgeGame.setValue( key, "true" );
			
			// start movie mode
			scourgeGame.setMovieMode( true );

			player <- scourgeGame.getPartyMember( 0 );
			player.moveTo( 210, 216 );
			// player.face( 0 );

			uzudil <- findCreatureByType( "Uzudil the Hand" );
			uzudil.setScripted( true );
			uzudil.moveTo( 210, 210 );
			// uzudil.face( 0 );

			// should temporarily "hide" other npc-s...
			
			scourgeGame.moveCamera( 150, 135, 0, 
															0, 85, 260, 
															1.8,
															500 );
			
			scourgeGame.continueAt( "initHq_part2", 2000 );
		//}
	}
}
	
function initHq_part2() {	
	uzudil <- findCreatureByType( "Uzudil the Hand" );
	uzudil.say( _( "Welcome to the S.c.o.u.r.g.e. headquarters!" ) );	
	scourgeGame.continueAt( "initHq_part3", 5000 );
}

function initHq_part3() {	
	scourgeGame.moveCamera( 150, 125, 0, 
													0, 85, 300, 
													1.8,
													5000 );
	uzudil <- findCreatureByType( "Uzudil the Hand" );
	uzudil.say( _( "Come and talk to me anytime, or check the board for new missions." ) );	
	scourgeGame.continueAt( "initHq_part4", 7000 );
}

function initHq_part4() {	
	scourgeGame.moveCamera( 150, 140, 0, 
													0, 85, 240, 
													1.8,
													5000 );
	player <- scourgeGame.getPartyMember( 0 );
	player.say( _( "Thank you. I will report for duty soon. Now where is the beer-hall?" ) );	
	scourgeGame.continueAt( "initHq_part_last", 7000 );
}

function initHq_part_last() {
	scourgeGame.setMovieMode( false );
}

