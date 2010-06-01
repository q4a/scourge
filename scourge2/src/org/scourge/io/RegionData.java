package org.scourge.io;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.ElementMap;
import org.simpleframework.xml.Root;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * User: gabor
 * Date: May 31, 2010
 * Time: 4:38:52 PM
 */
@Root(name = "region")
public class RegionData {
    @Attribute(name = "region_x")
    private int regionX;

    @Attribute(name = "region_y")
    private int regionY;

    @ElementMap(entry = "block", key = "coordinates", valueType = BlockData.class, attribute = true, inline = true, required = false)
    private Map<String, BlockData> blocks = new HashMap<String, BlockData>();

    public RegionData() {
    }

    public RegionData(int regionX, int regionY) {
        this.regionX = regionX;
        this.regionY = regionY;
    }

    public BlockData getBlock(int x, int y) {
        String key = "" + x + "." + y;
        return blocks.get(key);
    }

    public void putBlock(int x, int y, BlockData blockData) {
        String key = "" + x + "." + y;
        blocks.put(key, blockData);
    }

    public boolean isEmpty() {
        if(blocks != null) {
            // remove blank keys
            Set<String> keys = new HashSet<String>();
            for(String key : blocks.keySet()) {
                if(blocks.get(key).isEmpty()) {
                    keys.add(key);
                }
            }
            for(String key : keys) {
                blocks.remove(key);
            }
            return blocks.isEmpty();
        }
        return true;
    }

    public int getRegionX() {
        return regionX;
    }

    public int getRegionY() {
        return regionY;
    }
}
