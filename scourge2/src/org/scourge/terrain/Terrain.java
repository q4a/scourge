package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.input.MouseInput;
import com.jme.intersection.BoundingPickResults;
import com.jme.intersection.PickResults;
import com.jme.intersection.TrianglePickResults;
import com.jme.math.FastMath;
import com.jme.math.Ray;
import com.jme.math.Vector2f;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.TriMesh;
import com.jme.system.DisplaySystem;
import com.sun.xml.internal.ws.util.QNameMap;
import org.scourge.Main;
import org.scourge.io.MapIO;
import org.scourge.ui.component.Dragable;

import java.io.IOException;
import java.util.*;
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
    private boolean loadAsynchronously;

    public Terrain(Main main) throws IOException {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());
        mapIO = new MapIO();
    }

    public void gotoMainMenu() {
        clearLoadedRegions();
        loadAsynchronously = false;
        loadRegion(449 / Region.REGION_SIZE, 509 / Region.REGION_SIZE);
        loadAsynchronously = true;

        main.setFogOnWater(false);
        main.setCameraFollowsPlayer(false);
        Vector3f pos = new Vector3f(currentRegion.getX() * ShapeUtil.WALL_WIDTH,
                                           2 * ShapeUtil.WALL_WIDTH,
                                           currentRegion.getY() * ShapeUtil.WALL_WIDTH);
        main.getCamera().getLocation().set(pos);
        main.getCamera().lookAt(pos.addLocal(new Vector3f(10, 1, 10)), Vector3f.UNIT_Y);
    }

    public void gotoPlayer() {
        clearLoadedRegions();
        loadAsynchronously = false;
        loadRegion(main.getPlayer().getCreatureModel().getX() / Region.REGION_SIZE, main.getPlayer().getCreatureModel().getZ() / Region.REGION_SIZE);
        loadRegion();
        loadAsynchronously = true;

        main.setFogOnWater(true);
        main.setCameraFollowsPlayer(true);
    }

    private void clearLoadedRegions() {
        synchronized(regionThreads) {
            for (String key : regionThreads.keySet()) {
                RegionLoaderThread thread = regionThreads.get(key);
                try {
                    thread.join();
                } catch (InterruptedException e1) {
                    // eh
                }
            }
            regionThreads.clear();
        }
        for(Region region : loadedRegions.values()) {
            terrain.detachChild(region.getNode());
        }
        loadedRegions.clear();
        currentRegion = null;
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
        if(main.getPlayer() != null) {
            int px = main.getPlayer().getCreatureModel().getX() / Region.REGION_SIZE;
            int pz = main.getPlayer().getCreatureModel().getZ() / Region.REGION_SIZE;
            if(px != currentRegion.getX() / Region.REGION_SIZE ||
               pz != currentRegion.getY() / Region.REGION_SIZE) {
                Region region = loadedRegions.get(getRegionKey(px, pz));
                if(region != null) currentRegion = region;
            }
        }
    }

    // called from the input-handler thread
    public void loadRegion() {
        switchRegion();
        boolean changed;

        int rx = currentRegion.getX() / Region.REGION_SIZE;
        int ry = currentRegion.getY() / Region.REGION_SIZE;

        int px = main.getPlayer().getCreatureModel().getX() % Region.REGION_SIZE;
        int pz = main.getPlayer().getCreatureModel().getZ() % Region.REGION_SIZE;
        if(px < Region.REGION_SIZE / 2) {
            changed = loadRegion(rx - 1, ry);
        } else {
            changed = loadRegion(rx + 1, ry);
        }

        if(pz < Region.REGION_SIZE / 2) {
            if(!changed && loadRegion(rx, ry - 1)) changed = true;
        } else {
            if(!changed && loadRegion(rx, ry + 1)) changed = true;
        }

        if(px < Region.REGION_SIZE / 2 && pz < Region.REGION_SIZE / 2) {
            if(!changed && loadRegion(rx - 1, ry - 1)) changed = true;
        } else if(px < Region.REGION_SIZE / 2 && pz >= Region.REGION_SIZE / 2) {
            if(!changed && loadRegion(rx - 1, ry + 1)) changed = true;
        } else if(px >= Region.REGION_SIZE / 2 && pz < Region.REGION_SIZE / 2) {
            if(!changed && loadRegion(rx + 1, ry - 1)) changed = true;
        } else if(px >= Region.REGION_SIZE / 2 && pz >= Region.REGION_SIZE / 2) {
            if(!changed && loadRegion(rx + 1, ry + 1)) changed = true;
        }

        Main.getMain().getMiniMap().update(changed);
    }

    // input-handler thread
    public boolean loadRegion(final int rx, final int ry) {
        boolean changed = false;
        final String key = getRegionKey(rx, ry);

        // non-synchronized check (this could cause synchronization problems...)
        if(!loadedRegions.containsKey(key)) {
            synchronized(regionThreads) {
                if(!regionThreads.containsKey(key)) {
                    changed = true;
                    System.err.println(">>> loading: " + rx + "," + ry);
                    RegionLoaderThread thread = new RegionLoaderThread(this, rx, ry);
                    regionThreads.put(key, thread);
                    thread.start();

                    if(!loadAsynchronously) {
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
        return changed;
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

            if(loadAsynchronously) {
                removeFarRegions();
            }
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

    public Map<String, Region> getLoadedRegions() {
        return loadedRegions;
    }

    private static TrianglePickResults results = new TrianglePickResults();
    private static final Ray down = new Ray();

    static {
        down.getDirection().set(new Vector3f(0, -1, 0));
        results.setCheckDistance(true);
    }

    public static void moveOnTopOfTerrain(Spatial spatial) {
        spatial.setIsCollidable(false);
        down.getOrigin().set(spatial.getWorldBound().getCenter());
        results.clear();
        Main.getMain().getTerrain().getNode().findPick(down, results);
        if (results.getNumber() > 0) {
            float dist = results.getPickData(0).getDistance();
            if(!Float.isInfinite(dist) && !Float.isNaN(dist)) {
                spatial.getLocalTranslation().y -= dist - ((BoundingBox)spatial.getWorldBound()).yExtent;
            }
        }
        spatial.setIsCollidable(true);
        spatial.updateModelBound();
        spatial.updateWorldBound();
    }


    private final Ray pickRay = new Ray();
    private PickResults pickResults = new BoundingPickResults();
    private Vector2f screenPos = new Vector2f();
    private Vector3f worldCoords = new Vector3f();
    private Vector3f worldCoords2 = new Vector3f();
    private Vector3f[] triangle = new Vector3f[3];

    public Vector3f getDropLocation(Vector3f location) {
        //System.err.println("!!! mouse=" + MouseInput.get().getXAbsolute() + "," + MouseInput.get().getYAbsolute());
        // Get the position that the mouse is pointing to
        screenPos.set(MouseInput.get().getXAbsolute(), MouseInput.get().getYAbsolute());
        // Get the world location of that X,Y value
        DisplaySystem.getDisplaySystem().getWorldCoordinates(screenPos, 0, worldCoords);
        DisplaySystem.getDisplaySystem().getWorldCoordinates(screenPos, 1, worldCoords2);

        // Create a ray starting from the camera, and going in the direction
        // of the mouse's location
        pickRay.getOrigin().set(worldCoords);
        pickRay.getDirection().set(worldCoords2.subtractLocal(worldCoords).normalizeLocal());
        //pickRay.getDirection().set(cam.getDirection());
        //System.err.println("world pos=" + worldCoords + " dir=" + worldCoords2 + " cam dir=" + cam.getDirection());

        pickResults.setCheckDistance(false);
        pickResults.clear();

        getNode().findPick(pickRay, pickResults);
        boolean triangleWasPicked = false;
        for(int i = 0; i < pickResults.getNumber(); i++) {
            TriMesh mesh = (TriMesh)pickResults.getPickData(i).getTargetMesh();
            for( int j = 0; j < mesh.getTriangleCount(); j++ ){
                mesh.getTriangle( j, triangle );
                triangleWasPicked =
                        ( pickRay.intersect( triangle[0].addLocal( mesh.getWorldTranslation() ),
                                             triangle[1].addLocal( mesh.getWorldTranslation() ),
                                             triangle[2].addLocal( mesh.getWorldTranslation() ) ) );
                if( triangleWasPicked ){
                    System.err.println("location:");
                    System.err.println("\t" + triangle[0] + "\n\t" + triangle[1] + "\n\t" + triangle[2]);

                    location.set(triangle[0]);
                    location.addLocal(triangle[1]).addLocal(triangle[2]).multLocal(FastMath.ONE_THIRD);
                    return location;
                }
            }
        }
        return null;
    }
}
