package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Quad;
import org.scourge.Main;

import java.io.File;
import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.apache.commons.io.FileUtils.readLines;

/**
 * User: gabor
 * Date: Feb 17, 2010
 * Time: 10:05:51 AM
 */
public class Terrain implements NodeGenerator {
    private Node terrain;
    private Main main;

    private Tile[][] tiles;
    private int rows, cols;
    private Town town;

    public Terrain(Main main) throws IOException {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());

        @SuppressWarnings({"unchecked"})
        List<String> lines = readLines(new File("./data/maps/m32m32.map"));
        rows = lines.size();
        cols = lines.get(0).length();
        boolean[][] map = new boolean[rows][cols];
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                map[y][x] = lines.get(y).charAt(x) != '~';
            }
        }

        tiles = new Tile[rows][cols];
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
                        tiles[y][x].set(main.getRandom().nextFloat() * 3 <= 1 ? TileTexType.MOSS : TileTexType.GRASS, TileType.QUAD, 0);
                    }
                }
            }
        }

        // create some hills
        createHeights(tiles, rows, cols);

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

                tile.createSpatial(around, x, y);

                Spatial sp = tile.getRenderedSpatial();
                sp.getLocalTranslation().set(x * 16, 2, y * 16);
                sp.updateModelBound();
                sp.updateWorldBound();
                terrain.attachChild(sp);
                terrain.updateModelBound();
                terrain.updateWorldBound();
            }
        }

        // copy the NW normal of each quad into the adjacent quads
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                Tile tile = tiles[y][x];
                if(tile.type == TileType.QUAD) {
                    Tile westTile = x > 0 ? tiles[y][x - 1] : null;
                    Tile nwTile = x > 0 && y > 0 ? tiles[y - 1][x - 1] : null;
                    Tile northTile = y > 0 ? tiles[y - 1][x] : null;

                    copyNormal(tile, westTile, Tile.Edge.NE);
                    copyNormal(tile, westTile, Tile.Edge.SE);
                    copyNormal(tile, northTile, Tile.Edge.SE);
                    copyNormal(tile, northTile, Tile.Edge.SW);
                    copyNormal(tile, nwTile, Tile.Edge.SE);
                }
            }
        }

        Tile.debug();
    }

    // hack... this should be in TileType.QUAD but I was lazy
    private void copyNormal(Tile from, Tile to, Tile.Edge edge) {
        if(to == null || to.type != TileType.QUAD ||
           from == null || from.type != TileType.QUAD) return;

        FloatBuffer fromBuf = ((Quad)from.spatial).getNormalBuffer();
        float[] normal = new float[3];
        // get the NW normal; the only one set explicitly
        normal[0] = fromBuf.get(0);
        normal[1] = fromBuf.get(1);
        normal[2] = fromBuf.get(2);

        FloatBuffer toBuf = ((Quad)to.spatial).getNormalBuffer();
        switch(edge) {
            case NE: toBuf.put(9, normal[0]).put(10, normal[1]).put(11, normal[2]); break;
            case SE: toBuf.put(6, normal[0]).put(7, normal[1]).put(8, normal[2]); break;
            case SW: toBuf.put(3, normal[0]).put(4, normal[1]).put(5, normal[2]); break;
            default: throw new IllegalStateException("Can't set NW corner.");
        }

        to.spatial.updateModelBound();
        to.spatial.updateWorldBound();
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
        town = new Town(main, x, 0, z, main.getRandom());
        terrain.attachChild(town.getNode());
        terrain.updateModelBound();
        terrain.updateWorldBound();
    }

    public void flatten(Node node) {
        terrain.updateWorldBound();
        terrain.updateWorldVectors();

        BoundingBox boundingBox = (BoundingBox)node.getWorldBound();
        float dx = boundingBox.getCenter().x - terrain.getWorldTranslation().x;
        float dz = boundingBox.getCenter().z - terrain.getWorldTranslation().z;

        int sx = (int)((dx - boundingBox.xExtent / 2) / ShapeUtil.WALL_WIDTH);
        int sy = (int)((dz - boundingBox.zExtent / 2) / ShapeUtil.WALL_WIDTH);
        int ex = (int)((dx + boundingBox.xExtent / 2) / ShapeUtil.WALL_WIDTH);
        int ey = (int)((dz + boundingBox.zExtent / 2) / ShapeUtil.WALL_WIDTH);

//        System.err.println("house center=" + boundingBox.getCenter());
//        System.err.println("house size=" + boundingBox.xExtent + "," + boundingBox.zExtent);
//        System.err.println("d=" + dx + "," + dz);
//        System.err.println("s=" + sx + "," + sy + " e=" + ex + "," + ey);

        for(int y = sy - 1; y <= ey + 1; y++) {
            for(int x = sx - 1; x <= ex + 1; x++) {
                flattenTile(x, y);
            }
        }
        terrain.updateModelBound();
        terrain.updateWorldBound();
    }

    private void flattenTile(int tx, int ty) {
        if(tx >= 0 && ty >= 0 && tx < cols && ty < rows) {
            Tile tile = tiles[ty][tx];
            if(tile.type != TileType.NONE) {

                //System.err.println("\tflattening: " + tx + "," + ty + " type=" + tile.type.name());

                Tile eastTile = tx < cols - 1 ? tiles[ty][tx + 1] : null;
                Tile westTile = tx > 0 ? tiles[ty][tx - 1] : null;
                Tile southTile = ty < rows - 1 ? tiles[ty + 1][tx] : null;
                Tile northTile = ty > 0 ? tiles[ty - 1][tx] : null;
                Tile nwTile = ty > 0 && tx > 0 ? tiles[ty - 1][tx - 1] : null;
                Tile neTile = ty > 0 && tx < cols - 1 ? tiles[ty - 1][tx + 1] : null;
                Tile seTile = ty < rows - 1 && tx < cols - 1 ? tiles[ty + 1][tx + 1] : null;
                Tile swTile = ty < rows - 1 && tx > 0 ? tiles[ty + 1][tx - 1] : null;

                tile.setHeight(Tile.Edge.NW, 0);
                tile.setHeight(Tile.Edge.SW, 0);
                tile.setHeight(Tile.Edge.SE, 0);
                tile.setHeight(Tile.Edge.NE, 0);

                if(northTile != null) {
                    northTile.setHeight(Tile.Edge.SW, 0);
                    northTile.setHeight(Tile.Edge.SE, 0);
                }
                if(southTile != null) {
                    southTile.setHeight(Tile.Edge.NE, 0);
                    southTile.setHeight(Tile.Edge.NW, 0);
                }
                if(westTile != null) {
                    westTile.setHeight(Tile.Edge.NE, 0);
                    westTile.setHeight(Tile.Edge.SE, 0);
                }
                if(eastTile != null) {
                    eastTile.setHeight(Tile.Edge.NW, 0);
                    eastTile.setHeight(Tile.Edge.SW, 0);
                }
                if(nwTile != null) nwTile.setHeight(Tile.Edge.SE, 0);
                if(swTile != null) swTile.setHeight(Tile.Edge.NE, 0);
                if(neTile != null) neTile.setHeight(Tile.Edge.SW, 0);
                if(seTile != null) seTile.setHeight(Tile.Edge.NW, 0);
            }
        }
    }

    public void moveToTopOfTerrain() {
        if(town != null) {
            town.moveToTopOfTerrain();
        }
    }
}
