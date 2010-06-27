package org.scourge.terrain;

import com.jme.math.Vector3f;

/**
 * User: gabor
 * Date: Jun 15, 2010
 * Time: 8:16:25 AM
 */
public abstract class Generator {
    private Region region;
    private int x, y;
    private Vector3f location;

    public Generator(Region region, int x, int y) {
        this.region = region;
        this.x = x;
        this.y = y;
        this.location = new Vector3f((getRegion().getX() + x) * ShapeUtil.WALL_WIDTH, 0, (getRegion().getY() + y) * ShapeUtil.WALL_WIDTH);
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

    public Vector3f getLocation() {
        return location;
    }

    public abstract void generate();

    public abstract void update(float tpf);

    public abstract void unloading();
}
