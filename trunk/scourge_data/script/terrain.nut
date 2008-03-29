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
	for( i = villageY; i < villageY + villageHeight; i+=10 ) {
		scourgeGame.getMission().setMapPosition( villageRoadX, i, 0, "STREETLIGHT" );
	}
}
