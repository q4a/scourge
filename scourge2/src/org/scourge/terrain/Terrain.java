package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.scene.Node;
import org.scourge.*;

import java.util.HashMap;
import java.util.Map;

/**
 * User: gabor
 * Date: Feb 17, 2010
 * Time: 10:05:51 AM
 */
public class Terrain implements NodeGenerator {
    private Node terrain;
    private Main main;
    private Map<String, Section> sections = new HashMap<String, Section>();
    private final static float STEP_SIZE = 4;
    private final static float STEP_HEIGHT = 1;

    // make sure there is a border of space around the land
    private static final String[] MAP = new String[] {
            "                          ",
            "        *                 ",
            "       ***   **           ",
            "   **************     *   ",
            "  ****************   ** * ",
            " ********** ******    *** ",
            "   ********   ***     **  ",
            "  ********** *****   **** ",
            " ****************** ***   ",
            "  ********************    ",
            "  *********************   ",
            "  **********************  ",
            "  ********* *********     ",
            "  ********    *********   ",
            "  ********************    ",
            "   *** **  ***  ****      ",
            "    *       *     ***     ",
            "                          ",
    };

    public Terrain(Main main) {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());

        int rows = MAP.length;
        int cols = MAP[0].length();
        boolean[][] map = new boolean[rows][cols];
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                map[y][x] = MAP[y].charAt(x) != ' ';
            }
        }

        Tile[][] tiles = new Tile[rows][cols];
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                tiles[y][x] = new Tile(main);
                if(map[y][x]) {
                    if(map[y - 1][x] && map[y][x - 1] && map[y][x + 1] && !map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 180);
                    } else if(!map[y - 1][x] && map[y][x - 1] && map[y][x + 1] && map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 0);
                    } else if(map[y - 1][x] && !map[y][x - 1] && map[y][x + 1] && map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 90);
                    } else if(map[y - 1][x] && map[y][x - 1] && !map[y][x + 1] && map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, -90);

                    } else if(!map[y - 1][x] && !map[y][x - 1] && map[y][x + 1] && map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 90);
                    } else if(map[y - 1][x] && map[y][x - 1] && !map[y][x + 1] && !map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, -90);
                    } else if(!map[y - 1][x] && map[y][x - 1] && !map[y][x + 1] && map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 0);
                    } else if(map[y - 1][x] && !map[y][x - 1] && map[y][x + 1] && !map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 180);

                    } else if(!map[y - 1][x] && !map[y][x - 1] && !map[y][x + 1] && map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 0);
                    } else if(map[y - 1][x] && !map[y][x - 1] && !map[y][x + 1] && !map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 180);
                    } else if(!map[y - 1][x] && map[y][x - 1] && !map[y][x + 1] && !map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, -90);
                    } else if(!map[y - 1][x] && !map[y][x - 1] && map[y][x + 1] && !map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 90);

                    } else if(!map[y - 1][x] && map[y][x - 1] && map[y][x + 1] && !map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_BRIDGE, 90);
                    } else if(map[y - 1][x] && !map[y][x - 1] && !map[y][x + 1] && map[y + 1][x]) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_BRIDGE, 0);

                    } else {
                        // todo: maybe use a fractal growth pattern here...
                        int gt = (int)(Math.random() * 6.0f);
                        //TileTexType tex = gt == 0 ? TileTexType.GRASS2 : (gt == 1 ? TileTexType.GRASS3 : TileTexType.GRASS);
                        tiles[y][x].set(TileTexType.GRASS, TileType.QUAD, 0);
                    }
                }
            }
        }

        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                Tile tile = tiles[y][x];
                if(tile.type == TileType.NONE) continue;

                Map<Direction, TileTexType> around = new HashMap<Direction, TileTexType>();
                around.put(Direction.EAST, x < cols - 1 ? tiles[y][x + 1].tex : null);
                around.put(Direction.WEST, x > 0 ? tiles[y][x - 1].tex : null);
                around.put(Direction.SOUTH, y < rows - 1 ? tiles[y + 1][x].tex : null);
                around.put(Direction.NORTH, y > 0 ? tiles[y - 1][x].tex : null);
                tile.createSpatial(around);
                tile.spatial.getLocalTranslation().set(x * 16, 2, y * 16);
                tile.spatial.updateModelBound();
                tile.spatial.updateWorldBound();
                terrain.attachChild(tile.spatial);
                terrain.updateModelBound();
                terrain.updateWorldBound();
            }
        }

        Tile.debug();

    }

    @Override
    public Node getNode() {
        return terrain;
    }

}
