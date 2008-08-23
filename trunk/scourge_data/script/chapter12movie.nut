function startChapter12Movie() {
	if( scourgeGame.getMission().getDungeonDepth() == 0 ) {
		key <- "chapter12Movie";
		value <- scourgeGame.getValue( key );
		//if( value == null || scourgeGame.getRerunMovies() ) {
			scourgeGame.setValue( key, "true" );
			
			// start movie mode
			scourgeGame.setMovieMode( true );

			scourgeGame.moveCamera( 230, 370, 0, 0, 30, 30, 0.7, 4000 );
			
			player <- scourgeGame.getPartyMember( 0 );
			player.say( _( "I feel a rumbling echo... nearby, yet from deep within the earth..." ) );
			
			scourgeGame.continueAt( "chapter12_part2", 5000 );
			
		//}
	}
}

function chapter12_part2() {
	scourgeGame.moveCamera( 230, 370, 0, 0, 30, 90, 0.7, 2000 );	
	scourgeGame.continueAt( "chapter12_part3", 3000 );
}

function chapter12_part3() {
	scourgeGame.getMission().quake();
	scourgeGame.moveCamera( 230, 370, 0, 0, 30, 100, 0.6, 2000 );	
	scourgeGame.continueAt( "chapter12_part4", 3000 );
}

function chapter12_part4() {
	scourgeGame.moveCamera( 230, 370, 0, 0, 30, 110, 0.5, 2000 );
	karzul <- scourgeGame.getMission().addCreature( 321, 443, 0, "Karzul the Destroyer" );
	karzul.moveTo( 321, 443 );
	karzul.setVisible( true );
	karzul.setScripted( true );
	karzul.setStateMod( 11, false );
	karzul.say( _( "Ooh it feels good to breathe the air again! I suggest you do so too for a last time, my friends..." ) );
	scourgeGame.continueAt( "chapter12_end", 5000 );
}

function chapter12_end() {
	scourgeGame.setMovieMode( false );
}
