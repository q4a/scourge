// =============================================================
// This squirrel file contains the code called during terrain generation.
// Functions here are not story-related and can be used anytime for map building
//

/**
 * The following global variables will exist:
 * villageX, villageY - upper left of area where village is
 * villageWidth, villageHeight - width and height of village area
 * villageRoadX, villageRoadY - x and y of two village roads (running entire width and height of village)
 */
function villageRoads() {
	print( "villageX=" + villageX + " villageY=" + villageY + "\n" );
	print( "villageWidth=" + villageWidth + " villageHeight=" + villageWidth + "\n" );
	print( "villageRoadX=" + villageRoadX + " villageRoadY=" + villageRoadY + "\n" );
	
	i <- 0
	for( i = villageY; i < villageY + villageHeight; i+=4 ) {
		if( i < villageRoadY || i > villageRoadY + 16 ) {
			if( 0 == ( rand() * 5.0 / RAND_MAX ).tointeger() ) {
				scourgeGame.getMission().setMapPosition( villageRoadX, i, 0, "STREETLIGHT" );
			} else if( 0 == ( rand() * 5.0 / RAND_MAX ).tointeger() ) {
				ix <- ( 0 == ( rand() * 2.0 / RAND_MAX ).tointeger() ? villageRoadX + 4 : villageRoadX + 10 );
				scourgeGame.getMission().addItem( "Barrel", ix, i, 0, true );
			}
		}
	}
}
