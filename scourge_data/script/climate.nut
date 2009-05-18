VEGETATION_BARREN <- 0x0010;
VEGETATION_GROVES <- 0x0020;
VEGETATION_LIGHTFOREST <- 0x0040;
VEGETATION_DEEPFOREST <- 0x0080;

class BaseClimate {	
}

class AlpineClimate extends BaseClimate {
	function getPixelValue() {
		return 50;
	}
	
	function getTreeList() {
		return ["tree18", "birch", "deadtree", "fir"];
	}
}

class BorealClimate extends BaseClimate {
	function getPixelValue() {
		return 100;
	}
	
	function getTreeList() {
		return ["tree01", "tree14", "tree16", "tree20", "birch", "fern", "fir"];
	}
}

class TemperateClimate extends BaseClimate {
	function getPixelValue() {
		return 150;
	}
	
	function getTreeList() {
		return ["tree01", "tree02", "tree03", "tree06", "tree10", "tree12", "tree13", "tree14", "tree15", "tree17", "tree20", "tree21", "birch"];
	}	
}

class SubtropicalClimate extends BaseClimate {
	function getPixelValue() {
		return 200;
	}
	
	function getTreeList() {
		return ["tree05", "tree09", "tree11", "tree18", "tree19", "bushtree", "cactus", "deadtree", "palm", "palm2"];
	}		
}

class TropicalClimate extends BaseClimate {
	function getPixelValue() {
		return 250;
	}

	function getTreeList() {
		return ["tree03", "tree19", "bushtree", "fern", "palm", "palm2"];
	}	
}

climates <- [ BorealClimate(), AlpineClimate(), TemperateClimate(), SubtropicalClimate(), TropicalClimate() ];

function getClimate( value ) {
	for( i <- 0; i < climates.len(); i++ ) {
		if( climates[i].getPixelValue() == value ) {
			return climates[i];
		}
	}
	if( value != 0 ) print( "Can't find climate for value=" + value.tostring() + "\n" );
	return null;
}

/* This is so lame... if I add this method to ClimateBase, I get a runtime error (Indexing instance with string).
 * So I gave up and moved it here. :-/
 */
function getTreeForClimate(climate) {
	tree_list <- climate.getTreeList();
	index <- ( scourgeGame.getMission().mt_rand() * tree_list.len().tofloat() ).tointeger();
	return tree_list[index];
}
