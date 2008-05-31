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

			uzudil <- findCreatureByType( "Uzudil the Hand" );
			uzudil.setScripted( true );
			uzudil.moveTo( 210, 210 );

			// should temporarily "hide" other npc-s...
			hideUnScripted();
			
			scourgeGame.moveCamera( 150, 138, 0, 0, 85, 260, 1.8, 500 );
			
			scourgeGame.continueAt( "initHq_part2", 1000 );
		//}
	}
}

function hideUnScripted(){
	i <- 0;
	creature <- null;
	for( i = 0; i < scourgeGame.getMission().getCreatureCount(); i++ ) {
		creature = scourgeGame.getMission().getCreature( i );
		if( !creature.isScripted() ) {
			creature.setVisible( false );
		}
	}
}

function initHq_part2() {	
	scourgeGame.moveCamera( 150, 120, 0, 0, 85, 320, 1.8, 10000 );
	scourgeGame.continueAt( "initHq_part3", 1000 );
}
	
function initHq_part3() {	
	player <- scourgeGame.getPartyMember( 0 );
	uzudil <- findCreatureByType( "Uzudil the Hand" );
	uzudil.say( player.getName() + _( ", welcome to the S.c.o.u.r.g.e. headquarters!" ) );	
	scourgeGame.continueAt( "initHq_part4", 5000 );
}

function initHq_part4() {	
	uzudil <- findCreatureByType( "Uzudil the Hand" );
	uzudil.say( _( "Come and talk to me anytime, or check the board for new missions." ) );	
	scourgeGame.continueAt( "initHq_part5", 7000 );
}

function initHq_part5() {	
	player <- scourgeGame.getPartyMember( 0 );
	player.say( _( "Thank you. I will report for duty soon. Now where is the beer-hall?" ) );	
	scourgeGame.continueAt( "initHq_part6", 3000 );
}

function initHq_part6() {	
	scourgeGame.moveCamera( 150, 150, 0, 0, 70, 230, 1.2, 6000 );
	scourgeGame.continueAt( "initHq_part7", 6000 );
}

function initHq_part7() {	
	scourgeGame.continueAt( "initHq_part_last", 1000 );
}

function initHq_part_last() {
	scourgeGame.setMovieMode( false );
}

