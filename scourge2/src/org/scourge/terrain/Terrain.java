package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
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
    private Map<String, Region> loadedRegions = new HashMap<String, Region>();
    private final Map<String, RegionLoaderThread> regionThreads = new HashMap<String, RegionLoaderThread>();
    private byte checkPendingRegions;
    private static Logger logger = Logger.getLogger(Terrain.class.toString());
    private boolean initialized;

    public Terrain(Main main) throws IOException {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());
        mapIO = new MapIO();

        initialized = false;
        loadRegion(main.getPlayer().getX() / Region.REGION_SIZE, main.getPlayer().getZ() / Region.REGION_SIZE);
        // fast start for ui testing
        if(!"true".equals(System.getProperty("ui.test"))) {
            loadRegion();
        }
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
            // todo: this can cause a NPE if the region is not loaded yet
            currentRegion = loadedRegions.get(getRegionKey(px, pz));
        }
    }

    // called from the input-handler thread
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

    // input-handler thread
    public void loadRegion(final int rx, final int ry) {
        final String key = getRegionKey(rx, ry);

        // non-synchronized check (this could cause synchronization problems...)
        if(!loadedRegions.containsKey(key)) {

            synchronized(regionThreads) {
                if(!regionThreads.containsKey(key)) {
                    RegionLoaderThread thread = new RegionLoaderThread(this, rx, ry);
                    regionThreads.put(key, thread);
                    thread.start();

                    if(!initialized) {
                        try {
                            thread.join();
                            update();
                        } catch (InterruptedException e) {
                            // eh
                        }
                    }
                }
            }
        }

        switchRegion();
    }

    // called from the main thread
    public void update() {
        // make an unsynchronized check (a small hack: pendingRegions.isEmpty() would have to be synchronized)
        if(checkPendingRegions > 0) {
            // add/remove regions in the main thread to avoid concurrent mod. exceptions
            synchronized(regionThreads) {
                for(Iterator<String> e = regionThreads.keySet().iterator(); e.hasNext();) {
                    String key = e.next();
                    RegionLoaderThread thread = regionThreads.get(key);
                    if(thread.getRegion() != null) {
                        logger.fine("Attaching region " + key);

                        e.remove();
                        Region region = thread.getRegion();
                        if(currentRegion == null) {
                            currentRegion = region;
                        }
                        loadedRegions.put(key, region);

                        // keep checking if there are threads out there
                        checkPendingRegions--;


                        // below this line doesn't need to be synchronized
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
                    }
                }
            }

            removeFarRegions();
        }
    }

    // main thread
    private void removeFarRegions() {
        int rx = currentRegion.getX() / Region.REGION_SIZE;
        int ry = currentRegion.getY() / Region.REGION_SIZE;

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
            logger.fine("Unloading region: " + s);
            Region region = loadedRegions.remove(s);
            terrain.detachChild(region.getNode());
        }

        logger.info("Current: " + rx + "," + ry + " loaded regions: " + loadedRegions.keySet());
//        System.err.println("terrain has " + terrain.getChildren().size() + " children: ");
//        for(Spatial sp : terrain.getChildren()) {
//            System.err.println("\t" + sp.getName());
//        }
        
        System.gc();
        Thread.yield();
    }

    public Region getRegionAtPoint(Vector3f v) {
        int rx = (int)(v.x / ShapeUtil.WALL_WIDTH / Region.REGION_SIZE);
        int rz = (int)(v.z / ShapeUtil.WALL_WIDTH / Region.REGION_SIZE);
        return loadedRegions.get(getRegionKey(rx, rz));
    }

    public static String getRegionKey(int rx, int rz) {
        return "" + rx + "," + rz;
    }

    public void setRegionPending() {
        checkPendingRegions++;
    }
}
