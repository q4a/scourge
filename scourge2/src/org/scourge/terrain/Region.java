package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.Vector2f;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.shape.Quad;
import org.scourge.io.MapIO;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Feb 17, 2010
 * Time: 10:05:51 AM
 */
public class Region implements NodeGenerator {
    private Terrain terrain;
    private Node region;
    public static final int REGION_SIZE = 20;
    private Tile[][] tiles;
    private int x, y, rows, cols;
    private List<House> houses = new ArrayList<House>();
    public static final float MIN_HEIGHT = 2;
    private static Logger logger = Logger.getLogger(Region.class.toString());
    private static final int EDGE_BUFFER = 2;
    private boolean first = true;

    public Region(Terrain terrain, int x, int y) throws IOException {
        this.terrain = terrain;
        this.x = x;
        this.y = y;
        rows = cols = REGION_SIZE;
        load();
    }

    /**
     * When loading a region, an extra buffer of EDGE_BUFFER tiles is loaded around the edges.
     * This is so the edge detection algorithms draw correctly for the section that is displayed.
     * @throws IOException error loading map
     */
    public void load() throws IOException {
        logger.fine("----------------------------------------------------------------");
        logger.fine("Loading region: " + x + "," + y);
        long start = System.currentTimeMillis();
        long firstStart = start;
        this.region = new Node("region_" + (x / REGION_SIZE) + "_" + (y / REGION_SIZE));
        // this.region.clearRenderState(RenderState.StateType.Texture);
        region.setModelBound(new BoundingBox());

        start = System.currentTimeMillis();
        MapIO.RegionPoint[][] region = terrain.getMapIO().readRegion(x - EDGE_BUFFER, y - EDGE_BUFFER, rows + EDGE_BUFFER * 2, cols + EDGE_BUFFER * 2);
        logger.fine("Loaded data in " + (System.currentTimeMillis() - start) + " millis.");

        start = System.currentTimeMillis();
        tiles = new Tile[rows + EDGE_BUFFER * 2][cols + EDGE_BUFFER * 2];
        makeTiles(region);
        logger.fine("makeTiles in " + (System.currentTimeMillis() - start) + " millis.");

        // create some hills
        start = System.currentTimeMillis();
        createHeights();
        logger.fine("createHeights in " + (System.currentTimeMillis() - start) + " millis.");

        // create the shapes and textures
        start = System.currentTimeMillis();
        Map<Direction, TileTexType> around = new HashMap<Direction, TileTexType>();
        for(int x = 0; x < cols + EDGE_BUFFER * 2; x++) {
            for(int y = 0; y < rows + EDGE_BUFFER * 2; y++) {
                Tile tile = tiles[y][x];
                if(tile.isEmpty()) continue;

                Tile eastTile = x < cols + EDGE_BUFFER * 2 - 1 ? tiles[y][x + 1] : null;
                Tile westTile = x > 0 ? tiles[y][x - 1] : null;
                Tile southTile = y < rows + EDGE_BUFFER * 2 - 1 ? tiles[y + 1][x] : null;
                Tile northTile = y > 0 ? tiles[y - 1][x] : null;

                around.put(Direction.EAST, eastTile != null ? eastTile.tex : null);
                around.put(Direction.WEST, westTile != null ? westTile.tex : null);
                around.put(Direction.SOUTH, southTile != null ? southTile.tex : null);
                around.put(Direction.NORTH, northTile != null ? northTile.tex : null);

                tile.createNode(around, tile.getLevel());
                Thread.yield();
            }
        }
        logger.fine("createNodes in " + (System.currentTimeMillis() - start) + " millis.");

        start = System.currentTimeMillis();
        for(int x = EDGE_BUFFER; x < cols + EDGE_BUFFER; x++) {
            for(int y = EDGE_BUFFER; y < rows + EDGE_BUFFER; y++) {
                Tile tile = tiles[y][x];
                if(tile.isEmpty()) continue;

                Node node = tile.getNode();
                node.getLocalTranslation().set(x * ShapeUtil.WALL_WIDTH, MIN_HEIGHT + (tile.getLevel() * ShapeUtil.WALL_HEIGHT), y * ShapeUtil.WALL_WIDTH);
                this.region.attachChild(node);
                Thread.yield();
            }
        }
        logger.fine("addNodes in " + (System.currentTimeMillis() - start) + " millis.");

        // copy the NW normal of each quad into the adjacent quads
        start = System.currentTimeMillis();
        for(int x = 0; x < cols + EDGE_BUFFER * 2; x++) {
            for(int y = 0; y < rows + EDGE_BUFFER * 2; y++) {
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
        logger.fine("copyNormals in " + (System.currentTimeMillis() - start) + " millis.");

        start = System.currentTimeMillis();
        for(int x = EDGE_BUFFER; x < cols + EDGE_BUFFER; x++) {
            for(int y = EDGE_BUFFER; y < rows + EDGE_BUFFER; y++) {
                Tile tile = tiles[y][x];
                if(tile.isEmpty()) continue;
                tile.attachModels();
                Thread.yield();
            }
        }
        logger.fine("attachModels in " + (System.currentTimeMillis() - start) + " millis.");

        this.region.getLocalTranslation().addLocal(new Vector3f(x * ShapeUtil.WALL_WIDTH, 0, y * ShapeUtil.WALL_WIDTH));

        if(logger.isLoggable(Level.FINE)) {
            ShapeUtil.debug();
            logger.fine("loaded region in " + (System.currentTimeMillis() - firstStart) + " millis.");
        }
    }

    // the symbols for different levels on the map
    private static final String LEVELS = "~*+-";
    private static final String GROUND_LEVELS = LEVELS.substring(1);

    private void makeTiles(MapIO.RegionPoint[][] region) {
        List<Set<Vector2f>> housePoints = new ArrayList<Set<Vector2f>>();
        Set<Vector2f> roadPos = new HashSet<Vector2f>();
        Set<Vector2f> cobblesPos = new HashSet<Vector2f>();
        Set<Vector2f> ladderPos = new HashSet<Vector2f>();

        // create tiles and handle empty tiles with models
        for(int y = 0; y < rows + EDGE_BUFFER * 2; y++) {
            for(int x = 0; x < cols + EDGE_BUFFER * 2; x++) {
                tiles[y][x] = new Tile(terrain.getMain());
                tiles[y][x].setClimate(region[y][x].getClimate());
                tiles[y][x].setLevel(region[y][x].getLevel());

                if(region[y][x].getC() == 'B') {
                    if(check(y - 1, x, region[y][x].getLevel(), region) && check(y + 1, x, region[y][x].getLevel(), region)) {
                        tiles[y][x].addModel(Model.bridge, new Vector3f(0, 0, 0), 1, 90, Vector3f.UNIT_Y);
                    } else {
                        tiles[y][x].addModel(Model.bridge);
                    }
                    region[y][x].setC('~');
                } else if(region[y][x].getC() == 'F') {
                    makeForestTile(tiles[y][x]);
                    region[y][x].setC('*');
                } else if(region[y][x].getC() == 'H') {
                    storeHousePoisition(housePoints, x, y);
                    region[y][x].setC('*');
                } else if(region[y][x].getC() == 'x') {
                    roadPos.add(new Vector2f(x, y));
                    region[y][x].setC('*');
                } else if(region[y][x].getC() == 'X') {
                    cobblesPos.add(new Vector2f(x, y));
                    region[y][x].setC('*');
                } else if(region[y][x].getC() == 'L') {
                    ladderPos.add(new Vector2f(x, y));
                    region[y][x].setC('*');
                }
            }
        }

        char c = '*';
        for(int x = 0; x < cols + EDGE_BUFFER * 2; x++) {
            for(int y = 0; y < rows + EDGE_BUFFER * 2; y++) {
                if(region[y][x].getC() != '~') {

                    int level = tiles[y][x].getLevel();

                    if(check(y - 1, x, level, region) && check(y, x - 1, level, region) && check(y, x + 1, level, region) && !check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 180);
                    } else if(!check(y - 1, x, level, region) && check(y, x - 1, level, region) && check(y, x + 1, level, region) && check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 0);
                    } else if(check(y - 1, x, level, region) && !check(y, x - 1, level, region) && check(y, x + 1, level, region) && check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, 90);
                    } else if(check(y - 1, x, level, region) && check(y, x - 1, level, region) && !check(y, x + 1, level, region) && check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_SIDE, -90);

                    } else if(!check(y - 1, x, level, region) && !check(y, x - 1, level, region) && check(y, x + 1, level, region) && check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 90);
                    } else if(check(y - 1, x, level, region) && check(y, x - 1, level, region) && !check(y, x + 1, level, region) && !check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, -90);
                    } else if(!check(y - 1, x, level, region) && check(y, x - 1, level, region) && !check(y, x + 1, level, region) && check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 0);
                    } else if(check(y - 1, x, level, region) && !check(y, x - 1, level, region) && check(y, x + 1, level, region) && !check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_CORNER, 180);

                    } else if(!check(y - 1, x, level, region) && !check(y, x - 1, level, region) && !check(y, x + 1, level, region) && check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 0);
                    } else if(check(y - 1, x, level, region) && !check(y, x - 1, level, region) && !check(y, x + 1, level, region) && !check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 180);
                    } else if(!check(y - 1, x, level, region) && check(y, x - 1, level, region) && !check(y, x + 1, level, region) && !check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, -90);
                    } else if(!check(y - 1, x, level, region) && !check(y, x - 1, level, region) && check(y, x + 1, level, region) && !check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_TIP, 90);

                    } else if(!check(y - 1, x, level, region) && check(y, x - 1, level, region) && check(y, x + 1, level, region) && !check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_BRIDGE, 90);
                    } else if(check(y - 1, x, level, region) && !check(y, x - 1, level, region) && !check(y, x + 1, level, region) && check(y + 1, x, level, region)) {
                        tiles[y][x].set(TileTexType.ROCK, TileType.EDGE_BRIDGE, 0);

                    } else {
                        Vector2f point = new Vector2f(x, y);
                        if(roadPos.contains(point)) {
                            tiles[y][x].set(TileTexType.ROAD, TileType.QUAD, 0);
                        } else if(cobblesPos.contains(point)) {
                            tiles[y][x].set(TileTexType.COBBLES, TileType.QUAD, 0);                            
                        } else if(x <= EDGE_BUFFER || y <= EDGE_BUFFER ||
                                  x >= cols + EDGE_BUFFER - 1 || y >= rows + EDGE_BUFFER - 1) {
                            // this is so edges meet on the same type
                            tiles[y][x].set(region[y][x].getClimate().getDefaultGround(), TileType.QUAD, 0);
                        } else {
                            tiles[y][x].set(region[y][x].getClimate().getRandomGround(terrain.getMain().getRandom()), TileType.QUAD, 0);
                        }
                    }
                }
            }
        }


        addHouses(housePoints);
        addLadders(ladderPos);
    }

    private boolean check(int y, int x, int level, MapIO.RegionPoint[][] region) {
        return (y >= 0 && x >= 0 && y < REGION_SIZE + EDGE_BUFFER * 2 && x < REGION_SIZE + EDGE_BUFFER * 2 &&
                region[y][x].getC() != '~' && region[y][x].getLevel() >= level);
    }

    private void addLadders(Set<Vector2f> ladderPos) {
        for(Vector2f point : ladderPos) {
            int x = (int)point.getX();
            int y = (int)point.getY();
            
            Tile eastTile = x < cols + EDGE_BUFFER * 2 - 1 ? tiles[y][x + 1] : null;
            Tile westTile = x > 0 ? tiles[y][x - 1] : null;
            Tile southTile = y < rows + EDGE_BUFFER * 2 - 1 ? tiles[y + 1][x] : null;
            Tile northTile = y > 0 ? tiles[y - 1][x] : null;

            float rotation = 0.0f;
            Vector3f trans = new Vector3f(0, 0, 0);
            if(eastTile != null && eastTile.getLevel() > tiles[y][x].getLevel()) {
                rotation = 180.0f;
                trans.x+=2;
            } else if(westTile != null && westTile.getLevel() > tiles[y][x].getLevel()) {
                rotation = 0.0f;
                trans.x+=14;
                trans.z+=ShapeUtil.WALL_WIDTH;
            } else if(southTile != null && southTile.getLevel() > tiles[y][x].getLevel()) {
                rotation = 90.0f;
                trans.x+=ShapeUtil.WALL_WIDTH;
                trans.z+=2;
            } else if(northTile != null && northTile.getLevel() > tiles[y][x].getLevel()) {
                rotation = 270.0f;
                trans.z+=14;                
            }
            tiles[y][x].addModel(Model.ladder, trans, 1, rotation, Vector3f.UNIT_Y);
        }
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
            float minx = 10000, miny = 10000, maxx = -1, maxy = -1;
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

            if(minx < EDGE_BUFFER || miny < EDGE_BUFFER || maxx >= cols + EDGE_BUFFER || maxy >= rows + EDGE_BUFFER) {
                continue;
            }

            int w = (int)(maxx - minx) + 1;
            int h = (int)(maxy - miny) + 1;
            float height = tiles[(int)miny][(int)minx].getLevel();

            House house = new House(terrain.getMain(), minx + w / 2.0f - 0.5f, height, miny + h / 2.0f + 0.5f, w, h,
                                    (int)(terrain.getMain().getRandom().nextFloat() * 2) + 1, terrain.getMain().getRandom());
            houses.add(house);
            region.attachChild(house.getNode());
        }
    }

    private void makeForestTile(Tile tile) {
        Model model = tile.getClimate().getRandomTree(terrain.getMain().getRandom());
        tile.addModel(model,
                      new Vector3f(8, 0, 8),
                      (terrain.getMain().getRandom().nextFloat() * 0.3f) + 1.0f,
                      terrain.getMain().getRandom().nextFloat() * 360.0f,
                      model.getRotationVector());
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
    }

    private void createHeights() {
        // set the heights
        for(int y = EDGE_BUFFER + 1; y < rows + EDGE_BUFFER - 1; y++) {
            for(int x = EDGE_BUFFER + 1; x < cols + EDGE_BUFFER - 1; x++) {
                float h = 2.0f + terrain.getMain().getRandom().nextFloat() * 8.0f;
                tiles[y - 1][x - 1].setHeight(Tile.Edge.SE, h);
                tiles[y - 1][x].setHeight(Tile.Edge.SW, h);
                tiles[y][x - 1].setHeight(Tile.Edge.NE, h);
                tiles[y][x].setHeight(Tile.Edge.NW, h);
            }
        }

        // clamp heights around the edges
        for(int x = 0; x < cols + EDGE_BUFFER * 2; x++) {
            for(int y = 0; y < rows + EDGE_BUFFER * 2; y++) {
                Tile tile = tiles[y][x];
                if(tile.type == TileType.NONE) continue;

                Tile eastTile = x < cols + EDGE_BUFFER * 2 - 1 ? tiles[y][x + 1] : null;
                Tile westTile = x > 0 ? tiles[y][x - 1] : null;
                Tile southTile = y < rows + EDGE_BUFFER * 2 - 1 ? tiles[y + 1][x] : null;
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
        return region;
    }

    public void flatten(Node node) {
        region.updateWorldBound();
        region.updateWorldVectors();

        BoundingBox boundingBox = (BoundingBox)node.getWorldBound();
        float dx = boundingBox.getCenter().x - region.getWorldTranslation().x;
        float dz = boundingBox.getCenter().z - region.getWorldTranslation().z;

        int sx = (int)((dx - boundingBox.xExtent / 2) / ShapeUtil.WALL_WIDTH) + EDGE_BUFFER;
        int sy = (int)((dz - boundingBox.zExtent / 2) / ShapeUtil.WALL_WIDTH) + EDGE_BUFFER;
        int ex = (int)((dx + boundingBox.xExtent / 2) / ShapeUtil.WALL_WIDTH) + EDGE_BUFFER;
        int ey = (int)((dz + boundingBox.zExtent / 2) / ShapeUtil.WALL_WIDTH) + EDGE_BUFFER;

        for(int y = sy - 1; y <= ey + 1; y++) {
            for(int x = sx - 1; x <= ex + 1; x++) {
                flattenTile(x, y);
            }
        }
        region.updateModelBound();
        region.updateWorldBound();
    }

    private void flattenTile(int tx, int ty) {
        if(tx >= 0 && ty >= 0 && tx < cols && ty < rows) {
            Tile tile = tiles[ty][tx];
            tile.clearModels();
            if(tile.type != TileType.NONE) {
                Tile eastTile = tx < cols + EDGE_BUFFER - 1 ? tiles[ty][tx + 1] : null;
                Tile westTile = tx > 0 ? tiles[ty][tx - 1] : null;
                Tile southTile = ty < rows + EDGE_BUFFER - 1 ? tiles[ty + 1][tx] : null;
                Tile northTile = ty > 0 ? tiles[ty - 1][tx] : null;
                Tile nwTile = ty > 0 && tx > 0 ? tiles[ty - 1][tx - 1] : null;
                Tile neTile = ty > 0 && tx < cols + EDGE_BUFFER - 1 ? tiles[ty - 1][tx + 1] : null;
                Tile seTile = ty < rows + EDGE_BUFFER - 1 && tx < cols + EDGE_BUFFER - 1 ? tiles[ty + 1][tx + 1] : null;
                Tile swTile = ty < rows + EDGE_BUFFER - 1 && tx > 0 ? tiles[ty + 1][tx - 1] : null;

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
        if(first) {
            first = false;
            for(House house : houses) {
                flatten(house.getNode());
            }
        }
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public String getRegionKey() {
        return Terrain.getRegionKey(x / REGION_SIZE, y / REGION_SIZE);
    }
}
