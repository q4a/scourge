package org.scourge.terrain;

/**
 * User: gabor
 * Date: Jun 15, 2010
 * Time: 8:16:25 AM
 */
public abstract class Generator {
    private Region region;
    private int x, y;

    public Generator(Region region, int x, int y) {
        this.region = region;
        this.x = x;
        this.y = y;
    }


    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public Region getRegion() {
        return region;
    }

    public abstract void generate();
}
