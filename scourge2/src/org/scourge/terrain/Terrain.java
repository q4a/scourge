package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.scene.Node;
import org.scourge.Main;
import org.scourge.io.MapIO;

import java.io.IOException;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Apr 1, 2010
 * Time: 9:01:16 AM
 */
public class Terrain implements NodeGenerator {
    private Node terrain;
    private MapIO mapIO;
    private Main main;
    private Region currentRegion;
    private Map<String, Region> loadedRegions = new HashMap<String, Region>();
    private static Logger logger = Logger.getLogger(Terrain.class.toString());

    public Terrain(Main main) throws IOException {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());
        mapIO = new MapIO();
        currentRegion = new Region(this, 470, 460);
        terrain.attachChild(currentRegion.getNode());
        terrain.updateModelBound();
    }

    @Override
    public Node getNode() {
        return terrain;
    }

    public MapIO getMapIO() {
        return mapIO;
    }

    public Main getMain() {
        return main;
    }

    public Region getCurrentRegion() {
        return currentRegion;
    }

    public void loadRegion(Direction direction) {
        try {

            int rx = currentRegion.getX();
            int ry = currentRegion.getY();
            switch(direction) {
                case NORTH:
                    ry = currentRegion.getY() - Region.REGION_SIZE;
                    break;
                case SOUTH:
                    ry = currentRegion.getY() + Region.REGION_SIZE;
                    break;
                case WEST:
                    rx = currentRegion.getX() - Region.REGION_SIZE;
                    break;
                case EAST:
                    rx = currentRegion.getX() + Region.REGION_SIZE;
                    break;
            }

            String key = "" + rx + "," + ry;
            if(!loadedRegions.keySet().contains(key)) {
                logger.info("Loading region: " + direction + " coord=" + rx + "," + ry);

                // move the current region and player away
                switch(direction) {
                    case NORTH:
                        currentRegion.getNode().getLocalTranslation().z += Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        main.getPlayer().getNode().getLocalTranslation().z += Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        break;
                    case SOUTH:
                        currentRegion.getNode().getLocalTranslation().z -= Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        main.getPlayer().getNode().getLocalTranslation().z -= Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        break;
                    case WEST:
                        currentRegion.getNode().getLocalTranslation().x += Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        main.getPlayer().getNode().getLocalTranslation().x += Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        break;
                    case EAST:
                        currentRegion.getNode().getLocalTranslation().x -= Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        main.getPlayer().getNode().getLocalTranslation().x -= Region.REGION_SIZE * ShapeUtil.WALL_WIDTH;
                        break;
                }
                currentRegion.getNode().updateModelBound();
                currentRegion.getNode().updateWorldBound();
                main.getPlayer().getNode().updateModelBound();
                main.getPlayer().getNode().updateWorldBound();
                loadedRegions.put("" + currentRegion.getX() + "," + currentRegion.getY(), currentRegion);

                // remove far regions
                Set<String> far = new HashSet<String>();
                for(String s : loadedRegions.keySet()) {
                    String[] ss = s.split(",");
                    int x = Integer.valueOf(ss[0]);
                    int y = Integer.valueOf(ss[1]);
                    if(Math.abs(x - rx) > Region.REGION_SIZE ||
                       Math.abs(y - ry) > Region.REGION_SIZE) {
                        far.add(s);
                    }
                }
                for(String s : far) {
                    logger.info("Unloading region: " + s);
                    Region region = loadedRegions.remove(s);
                    terrain.detachChild(region.getNode());
                }

                // load a new region
                currentRegion = new Region(this, rx, ry);
                terrain.attachChild(currentRegion.getNode());

                // not sure which but one of the following is needed...
                currentRegion.getNode().updateRenderState();
                currentRegion.getNode().updateWorldData(0);
                currentRegion.getNode().updateModelBound();
                currentRegion.getNode().updateWorldBound();
                terrain.updateRenderState();
                terrain.updateWorldData(0);
                terrain.updateModelBound();
                terrain.updateWorldBound();
            }
        } catch(IOException exc) {
            logger.log(Level.SEVERE, exc.getMessage(), exc);
        }
    }
}
