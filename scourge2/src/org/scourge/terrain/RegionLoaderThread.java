package org.scourge.terrain;

import org.scourge.Main;

import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Apr 4, 2010
 * Time: 8:03:16 PM
 */
public class RegionLoaderThread extends Thread {
    private static Logger logger = Logger.getLogger(RegionLoaderThread.class.toString());
    private int rx;
    private int ry;
    private Region region;
    private Terrain terrain;

    public RegionLoaderThread(Terrain terrain, int rx, int ry) {
        this.terrain = terrain;
        this.rx = rx;
        this.ry = ry;
        setPriority(Thread.MIN_PRIORITY);
        setDaemon(true);
    }

    public void run() {
        String key = Terrain.getRegionKey(rx, ry);
        try {
            terrain.getMain().setLoading(true);
            logger.info("Loading region: " + key);
            region = new Region(terrain, rx * Region.REGION_SIZE, ry * Region.REGION_SIZE);
            terrain.setRegionPending();
        } catch(IOException exc) {
            logger.log(Level.SEVERE, exc.getMessage(), exc);
        } finally {
            terrain.getMain().setLoading(false);
            logger.info("Loaded region: " + key);
        }
    }

    public Region getRegion() {
        return region;
    }
}
