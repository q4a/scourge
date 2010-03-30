package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.Vector2f;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Quad;
import org.scourge.Main;

import java.io.File;
import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.*;

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
    private List<House> houses = new ArrayList<House>();

    static final float MIN_HEIGHT = 2;

    public Terrain(Main main) throws IOException {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());

        @SuppressWarnings({"unchecked"})
        List<String> lines = readLines(new File("./data/maps/m32m32.map"));
        rows = lines.size();
        cols = lines.get(0).length();

        tiles = new Tile[rows][cols];
        makeTiles(rows, cols, tiles, lines);

        // create some hills
        createHeights(tiles, rows, cols);

        // create the shapes and textures
        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                Tile tile = tiles[y][x];
                if(tile.isEmpty()) continue;

                Tile eastTile = x < cols - 1 ? tiles[y][x + 1] : null;
                Tile westTile = x > 0 ? tiles[y][x - 1] : null;
                Tile southTile = y < rows - 1 ? tiles[y + 1][x] : null;
                Tile northTile = y > 0 ? tiles[y - 1][x] : null;

                Map<Direction, TileTexType> around = new HashMap<Direction, TileTexType>();
                around.put(Direction.EAST, eastTile != null ? eastTile.tex : null);
                around.put(Direction.WEST, westTile != null ? westTile.tex : null);
                around.put(Direction.SOUTH, southTile != null ? southTile.tex : null);
                around.put(Direction.NORTH, northTile != null ? northTile.tex : null);

                tile.createNode(around, tile.getLevel());

                Node node = tile.getNode();
                node.getLocalTranslation().set(x * ShapeUtil.WALL_WIDTH, MIN_HEIGHT + (tile.getLevel() * ShapeUtil.WALL_HEIGHT), y * ShapeUtil.WALL_WIDTH);
                node.updateModelBound();
                node.updateWorldBound();
                terrain.attachChild(node);
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

        for(int x = 0; x < cols; x++) {
            for(int y = 0; y < rows; y++) {
                Tile tile = tiles[y][x];
                if(tile.isEmpty()) continue;
                tile.attachModels();
            }
        }

        ShapeUtil.debug();
    }

    private void makeTiles(int rows, int cols, Tile[][] tiles, List<String> lines) {
        List<Set<Vector2f>> housePoints = new ArrayList<Set<Vector2f>>();
        Set<Vector2f> roadPos = new HashSet<Vector2f>();

        // create tiles and handle empty tiles with models
        for(int y = 0; y < rows; y++) {
            for(int x = 0; x < cols; x++) {
                tiles[y][x] = new Tile(main);
                if(lines.get(y).charAt(x) == 'B') {
                    tiles[y][x].addModel(Model.bridge);
                } else if(lines.get(y).charAt(x) == 'F') {
                    makeForestTile(tiles[y][x]);
                } else if(lines.get(y).charAt(x) == 'H') {
                    storeHousePoisition(housePoints, x, y);
                } else if(lines.get(y).charAt(x) == 'x') {
                    roadPos.add(new Vector2f(x, y));
                }
            }
            // turn some tiles into water
            lines.set(y, lines.get(y).replaceAll("B", "~"));
            lines.set(y, lines.get(y).replaceAll("[xHF]", "*")); // todo: this won't work for higher levels (+,-)
        }

        // the symbols for different levels on the map
        String levels = "~*+-";

        for(int i = 1; i < levels.length(); i++) {
            char c = levels.charAt(i);
            char prevC = levels.charAt(i - 1);
            for(int x = 0; x < cols; x++) {
                for(int y = 0; y < rows; y++) {
                    if(lines.get(y).charAt(x) == c) {

                        tiles[y][x].setLevel(i - 1);

                        if(check(y - 1, x, prevC, lines) && check(y, x - 1, prevC, lines) && check(y, x + 1, prevC, lines) && !check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 180);
                        } else if(!check(y - 1, x, prevC, lines) && check(y, x - 1, prevC, lines) && check(y, x + 1, prevC, lines) && check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 0);
                        } else if(check(y - 1, x, prevC, lines) && !check(y, x - 1, prevC, lines) && check(y, x + 1, prevC, lines) && check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 90);
                        } else if(check(y - 1, x, prevC, lines) && check(y, x - 1, prevC, lines) && !check(y, x + 1, prevC, lines) && check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, -90);

                        } else if(!check(y - 1, x, prevC, lines) && !check(y, x - 1, prevC, lines) && check(y, x + 1, prevC, lines) && check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 90);
                        } else if(check(y - 1, x, prevC, lines) && check(y, x - 1, prevC, lines) && !check(y, x + 1, prevC, lines) && !check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, -90);
                        } else if(!check(y - 1, x, prevC, lines) && check(y, x - 1, prevC, lines) && !check(y, x + 1, prevC, lines) && check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 0);
                        } else if(check(y - 1, x, prevC, lines) && !check(y, x - 1, prevC, lines) && check(y, x + 1, prevC, lines) && !check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 180);

                        } else if(!check(y - 1, x, prevC, lines) && !check(y, x - 1, prevC, lines) && !check(y, x + 1, prevC, lines) && check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 0);
                        } else if(check(y - 1, x, prevC, lines) && !check(y, x - 1, prevC, lines) && !check(y, x + 1, prevC, lines) && !check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 180);
                        } else if(!check(y - 1, x, prevC, lines) && check(y, x - 1, prevC, lines) && !check(y, x + 1, prevC, lines) && !check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, -90);
                        } else if(!check(y - 1, x, prevC, lines) && !check(y, x - 1, prevC, lines) && check(y, x + 1, prevC, lines) && !check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 90);

                        } else if(!check(y - 1, x, prevC, lines) && check(y, x - 1, prevC, lines) && check(y, x + 1, prevC, lines) && !check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_BRIDGE, 90);
                        } else if(check(y - 1, x, prevC, lines) && !check(y, x - 1, prevC, lines) && !check(y, x + 1, prevC, lines) && check(y + 1, x, prevC, lines)) {
                            tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_BRIDGE, 0);

                        } else {
                            Vector2f point = new Vector2f(x, y);
                            if(roadPos.contains(point)) {
                                tiles[y][x].set(TileTexType.ROAD, TileType.QUAD, 0);
                            } else {
                                int type = (int)(main.getRandom().nextFloat() * 5);
                                tiles[y][x].set(type == 0 ? TileTexType.MOSS : (type == 1 ? TileTexType.LYCHEN : TileTexType.GRASS), TileType.QUAD, 0);
                            }
                        }
                    }
                }
            }
        }

        addHouses(housePoints);
    }

    private void storeHousePoisition(List<Set<Vector2f>> housePoints, int x, int y) {
        boolean found = false;
        for(Set<Vector2f> housePoint : housePoints) {
            for(Vector2f point : housePoint) {
                if((point.x == x && (point.y == y - 1 || point.y == y + 1)) ||
                   (point.y == y && (point.x == x - 1 || point.x == x + 1))) {
                    housePoint.add(new Vector2f(x, y));
                    found = true;
                    break;
                }
            }
            if(found) break;
        }
        if(!found) {
            Set<Vector2f> set = new HashSet<Vector2f>();
            set.add(new Vector2f(x, y));
            housePoints.add(set);
        }
    }

    private void addHouses(List<Set<Vector2f>> housePoints) {
        for(Set<Vector2f> housePoint : housePoints) {
            float minx = 1000, miny = 1000, maxx = 0, maxy = -1;
            for(Vector2f point : housePoint) {
                if(point.x < minx) {
                    minx = point.x;
                }
                if(point.x > maxx) {
                    maxx = point.x;
                }
                if(point.y < miny) {
                    miny = point.y;
                }
                if(point.y > maxy) {
                    maxy = point.y;
                }
            }
            int w = (int)(maxx - minx) + 1;
            int h = (int)(maxy - miny) + 1;
            float height = tiles[(int)miny][(int)minx].getLevel();

            House house = new House(main, minx + w / 2.0f - 0.5f, height, miny + h / 2.0f + 0.5f, w, h,
                                    (int)(main.getRandom().nextFloat() * 2) + 1, main.getRandom());
            houses.add(house);
            terrain.attachChild(house.getNode());
        }
    }

    private void makeForestTile(Tile tile) {
        tile.addModel(Model.getRandomTree(main.getRandom()),
                      new Vector3f(8, 0, 8),
                      (main.getRandom().nextFloat() * 0.3f) + 1.0f,
                      main.getRandom().nextFloat() * 360.0f);
    }

    private boolean check(int y, int x, char c, List<String> lines) {
        return (y >= 0 && x >= 0 && y < lines.size() && x < lines.get(0).length() && lines.get(y).charAt(x) != c);
    }

    // hack... this should be in TileType.QUAD but I was lazy
    private void copyNormal(Tile from, Tile to, Tile.Edge edge) {
        if(to == null || to.type != TileType.QUAD ||
           from == null || from.type != TileType.QUAD) return;

        Quad fromQuad = (Quad)from.ground.getChild(0);
        Quad toQuad = (Quad)to.ground.getChild(0);
        FloatBuffer fromBuf = fromQuad.getNormalBuffer();
        float[] normal = new float[3];
        // get the NW normal; the only one set explicitly
        normal[0] = fromBuf.get(0);
        normal[1] = fromBuf.get(1);
        normal[2] = fromBuf.get(2);

        FloatBuffer toBuf = toQuad.getNormalBuffer();
        switch(edge) {
            case NE: toBuf.put(9, normal[0]).put(10, normal[1]).put(11, normal[2]); break;
            case SE: toBuf.put(6, normal[0]).put(7, normal[1]).put(8, normal[2]); break;
            case SW: toBuf.put(3, normal[0]).put(4, normal[1]).put(5, normal[2]); break;
            default: throw new IllegalStateException("Can't set NW corner.");
        }

        toQuad.updateModelBound();
        toQuad.updateWorldBound();
    }

    private void createHeights(Tile[][] tiles, int rows, int cols) {
        // set the heights
        for(int y = 1; y < rows - 1; y++) {
            for(int x = 1; x < cols - 1; x++) {
                float h = 2.0f + main.getRandom().nextFloat() * 8.0f;
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
            tile.clearModels();
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
        for(House house : houses) {
            house.moveToTopOfTerrain();
        }
    }
}
