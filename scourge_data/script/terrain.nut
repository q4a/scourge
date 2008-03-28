// =============================================================
// This squirrel file contains the code called during terrain generation
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
	print( "villageRoadX=" + villageRoadX + " villageRoadY=" + villageRoadX + "\n" );
}
