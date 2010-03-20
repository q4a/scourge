package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.scene.Node;
import org.scourge.Main;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * User: gabor
 * Date: Feb 17, 2010
 * Time: 10:05:51 AM
 */
public class Terrain implements NodeGenerator {
    private Node terrain;
    private Main main;

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
                        tiles[y][x].set(TileTexType.GRASS, TileType.QUAD, 0);
                    }
                }
            }
        }

        // create some hills
        createHeights(tiles, rows, cols);

        // create the ground cover
        List<Map<String, GroundType>> ground = new ArrayList<Map<String, GroundType>>();
        ground.add(GroundType.moss.getGround(tiles, main.getRandom()));
        ground.add(GroundType.lichen.getGround(tiles, main.getRandom()));

        // create the shapes and textures
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                Tile tile = tiles[y][x];
                if(tile.type == TileType.NONE) continue;

                Tile eastTile = x < cols - 1 ? tiles[y][x + 1] : null;
                Tile westTile = x > 0 ? tiles[y][x - 1] : null;
                Tile southTile = y < rows - 1 ? tiles[y + 1][x] : null;
                Tile northTile = y > 0 ? tiles[y - 1][x] : null;

                Map<Direction, TileTexType> around = new HashMap<Direction, TileTexType>();
                around.put(Direction.EAST, eastTile != null ? eastTile.tex : null);
                around.put(Direction.WEST, westTile != null ? westTile.tex : null);
                around.put(Direction.SOUTH, southTile != null ? southTile.tex : null);
                around.put(Direction.NORTH, northTile != null ? northTile.tex : null);

                tile.createSpatial(around, ground, x, y);
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

    private void createHeights(Tile[][] tiles, int rows, int cols) {
        // set the heights
        for(int y = 1; y < rows - 1; y++) {
            for(int x = 1; x < cols - 1; x++) {
                float h = main.getRandom().nextFloat() * 10;
                tiles[y - 1][x - 1].setHeight(Tile.Edge.SE, h);
                tiles[y - 1][x].setHeight(Tile.Edge.SW, h);
                tiles[y][x - 1].setHeight(Tile.Edge.NE, h);
                tiles[y][x].setHeight(Tile.Edge.NW, h);
            }
        }

        // clamp heights around the edges
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                Tile tile = tiles[y][x];
                if(tile.type == TileType.NONE) continue;

                Tile eastTile = x < cols - 1 ? tiles[y][x + 1] : null;
                Tile westTile = x > 0 ? tiles[y][x - 1] : null;
                Tile southTile = y < rows - 1 ? tiles[y + 1][x] : null;
                Tile northTile = y > 0 ? tiles[y - 1][x] : null;

                boolean north = northTile != null && northTile.tex == tile.tex;
                boolean south = southTile != null && southTile.tex == tile.tex;
                boolean east = eastTile != null && eastTile.tex == tile.tex;
                boolean west = westTile != null && westTile.tex == tile.tex;

                if(!north) {
                    tile.setHeight(Tile.Edge.NW, 0);
                    if(westTile != null) westTile.setHeight(Tile.Edge.NE, 0);
                    tile.setHeight(Tile.Edge.NE, 0);
                    if(eastTile != null) eastTile.setHeight(Tile.Edge.NW, 0);
                }
                if(!west) {
                    tile.setHeight(Tile.Edge.NW, 0);
                    if(northTile != null) northTile.setHeight(Tile.Edge.SW, 0);
                    tile.setHeight(Tile.Edge.SW, 0);
                    if(southTile != null) southTile.setHeight(Tile.Edge.NW, 0);
                }
                if(!south) {
                    tile.setHeight(Tile.Edge.SW, 0);
                    if(westTile != null) westTile.setHeight(Tile.Edge.SE, 0);
                    tile.setHeight(Tile.Edge.SE, 0);
                    if(eastTile != null) eastTile.setHeight(Tile.Edge.SW, 0);
                }
                if(!east) {
                    tile.setHeight(Tile.Edge.SE, 0);
                    if(southTile != null) southTile.setHeight(Tile.Edge.NE, 0);
                    tile.setHeight(Tile.Edge.NE, 0);
                    if(northTile != null) northTile.setHeight(Tile.Edge.SE, 0);
                }
            }
        }
    }

    @Override
    public Node getNode() {
        return terrain;
    }

    public void addTown(int x, int z) {
        Town town = new Town(main, x, 0, z, main.getRandom());
        terrain.attachChild(town.getNode());
        terrain.updateModelBound();
        terrain.updateWorldBound();
    }

}
