package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.util.GameTaskQueue;
import com.jme.util.GameTaskQueueManager;
import org.scourge.Main;
import org.scourge.io.MapIO;

import java.io.IOException;
import java.util.*;
import java.util.concurrent.Callable;
import java.util.concurrent.Future;
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
    private Map<String, Region> loadedRegions = Collections.synchronizedMap(new HashMap<String, Region>());
    private Map<String, Thread> regionThreads = Collections.synchronizedMap(new HashMap<String, Thread>());
    private boolean checkFarRegions;
    private static Logger logger = Logger.getLogger(Terrain.class.toString());
    private boolean initialized;

    public Terrain(Main main) throws IOException {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());
        mapIO = new MapIO();

        initialized = false;
        loadRegion(main.getPlayer().getX() / Region.REGION_SIZE, main.getPlayer().getZ() / Region.REGION_SIZE);
        loadRegion();
        initialized = true;
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


    protected void switchRegion() {
        // switch current region if needed
        int px = main.getPlayer().getX() / Region.REGION_SIZE;
        int pz = main.getPlayer().getZ() / Region.REGION_SIZE;
        if(px != currentRegion.getX() / Region.REGION_SIZE ||
           pz != currentRegion.getY() / Region.REGION_SIZE) {
            currentRegion = loadedRegions.get("" + px + "," + pz);
        }
    }

    public void loadRegion() {
        switchRegion();
        int rx = currentRegion.getX() / Region.REGION_SIZE;
        int ry = currentRegion.getY() / Region.REGION_SIZE;

        int px = main.getPlayer().getX() % Region.REGION_SIZE;
        int pz = main.getPlayer().getZ() % Region.REGION_SIZE;
        if(px < Region.REGION_SIZE / 2) {
            loadRegion(rx - 1, ry);
        } else {
            loadRegion(rx + 1, ry);
        }

        if(pz < Region.REGION_SIZE / 2) {
            loadRegion(rx, ry - 1);
        } else {
            loadRegion(rx, ry + 1);
        }

        if(px < Region.REGION_SIZE / 2 && pz < Region.REGION_SIZE / 2) {
            loadRegion(rx - 1, ry - 1);
        } else if(px < Region.REGION_SIZE / 2 && pz >= Region.REGION_SIZE / 2) {
            loadRegion(rx - 1, ry + 1);
        } else if(px >= Region.REGION_SIZE / 2 && pz < Region.REGION_SIZE / 2) {
            loadRegion(rx + 1, ry - 1);
        } else if(px >= Region.REGION_SIZE / 2 && pz >= Region.REGION_SIZE / 2) {
            loadRegion(rx + 1, ry + 1);
        }
    }

    public void loadRegion(final int rx, final int ry) {
        final String key = "" + rx + "," + ry;
        if(!loadedRegions.containsKey(key)) {
            if(initialized) {
                if(!regionThreads.containsKey(key)) {
                    Thread thread = new Thread("loader_" + key) {
                        public void run() {
                            doLoadRegion(rx, ry);
                        }
                    };
                    regionThreads.put(key, thread);
                    thread.start();
                }
            } else {
                doLoadRegion(rx, ry);
            }
        }

        if(checkFarRegions) {
            // remove far regions
            Set<String> far = new HashSet<String>();
            for(String s : loadedRegions.keySet()) {
                String[] ss = s.split(",");
                int x = Integer.valueOf(ss[0]);
                int y = Integer.valueOf(ss[1]);
                if(Math.abs(x - rx) > 1 || Math.abs(y - ry) > 1) {
                    far.add(s);
                }
            }
            for(String s : far) {
                logger.info("Unloading region: " + s);
                Region region = loadedRegions.remove(s);
                terrain.detachChild(region.getNode());
            }
            System.gc();
            Thread.yield();

            checkFarRegions = false;
        }

        switchRegion();
    }

    private void doLoadRegion(int rx, int ry) {
        try {
            main.setLoading(true);
            final String key = "" + rx + "," + ry;
            logger.info("Loading region: " + key);

            checkFarRegions = true;

            // load a new region
            final Region region = new Region(this, rx * Region.REGION_SIZE, ry * Region.REGION_SIZE);
            if(currentRegion == null) {
                currentRegion = region;
            }
            loadedRegions.put(key, region);


            GameTaskQueueManager.getManager().getQueue(GameTaskQueue.UPDATE).enqueue(new Callable<Object>() {
                @Override
                public Object call() throws Exception {
                    terrain.attachChild(region.getNode());

                    // not sure which but one of the following is needed...
                    region.getNode().updateRenderState();
                    region.getNode().updateWorldData(0);
                    region.getNode().updateModelBound();
                    region.getNode().updateWorldBound();
                    terrain.updateRenderState();
                    terrain.updateWorldData(0);
                    terrain.updateModelBound();
                    terrain.updateWorldBound();

                    regionThreads.remove(key);
                    logger.info("loaded regions: " + loadedRegions.keySet());
                    return null;
                }
            });

        } catch(IOException exc) {
            logger.log(Level.SEVERE, exc.getMessage(), exc);
        } finally {
            main.setLoading(false);
        }
    }
}
