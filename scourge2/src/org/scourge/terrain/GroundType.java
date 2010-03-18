package org.scourge.terrain;

import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Mar 15, 2010
 * Time: 9:33:02 AM
 */
public enum GroundType {
    none,
    moss("./data/textures/moss.png", 3.0f, 100, 7),
    lichen("./data/textures/grass2.png", 2.5f, 5, 4),
    ;

    private String texturePath;
    private float occurance;
    private int growthIterations;
    private float holeOccurance;
    private Logger logger = Logger.getLogger(GroundType.class.toString());

    GroundType() {
       this(null, 0, 0, 0);
    }

    GroundType(String texturePath, float occurance, int growthIterations, float holeOccurance) {
        this.texturePath = texturePath;
        this.occurance = occurance;
        this.growthIterations = growthIterations;
        this.holeOccurance = holeOccurance;
    }

    public String getTexturePath() {
        return texturePath;
    }

    public static String getGroundKey(int x, int y) {
        return "" + x + "," + y;
    }

    protected Map<String, GroundType> getGround(Tile[][] tiles, Random random) {
        int rows = tiles.length * 4;
        int cols = tiles[0].length * 4;

        logger.info("Creating ground cover: " + name());

        // create some random points
        Map<String, GroundType> ground = new HashMap<String, GroundType>();
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                Tile tile = tiles[y / 4][x / 4];
                if(tile.type == TileType.QUAD) {
                    ground.put(getGroundKey(x, y), random.nextFloat() * occurance <= 1.0f ? GroundType.none : this);
                }
            }
        }
        grow(ground, rows, cols);
//        debugGround(rows, cols, ground);

        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                if(random.nextFloat() * holeOccurance <= 1.0f) {
                    ground.remove(getGroundKey(x, y));
                }
            }
        }
//        debugGround(rows, cols, ground);

        grow(ground, rows, cols);
//        debugGround(rows, cols, ground);

        return ground;
    }

    // grow using cellular automaton: more then 5 neighbors = live, less than 4 = die
    private void grow(Map<String, GroundType> ground, int rows, int cols) {
        for(int i = 0; i < growthIterations; i++) {
            for(int x = 0; x < cols; x++) {
                for(int y = 0; y < rows; y++) {
                    String key = getGroundKey(x, y);
                    if(ground.get(key) == null) continue;

                    int score = 0;
                    if(ground.get(getGroundKey(x - 1, y)) == this) score++;
                    if(ground.get(getGroundKey(x - 1, y - 1)) == this) score++;
                    if(ground.get(getGroundKey(x, y - 1)) == this) score++;
                    if(ground.get(getGroundKey(x + 1, y - 1)) == this) score++;
                    if(ground.get(getGroundKey(x + 1, y)) == this) score++;
                    if(ground.get(getGroundKey(x + 1, y + 1)) == this) score++;
                    if(ground.get(getGroundKey(x, y + 1)) == this) score++;
                    if(ground.get(getGroundKey(x - 1, y + 1)) == this) score++;
                    if(score > 5) {
                        ground.put(key, this);
                    } else if(score < 4) {
                        ground.put(key, GroundType.none);
                    }
                }
            }
            //debugGround(rows, cols, ground);
        }
    }

    private void debugGround(int rows, int cols, Map<String, GroundType> ground) {
        System.err.println("------------------------------------------------");
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                GroundType gt = ground.get(getGroundKey(x, y));
                System.err.print(gt == null ? " " : (gt == GroundType.none ? "." : gt.name().toUpperCase().subSequence(0, 1)));
            }
            System.err.println();
        }

    }
}
