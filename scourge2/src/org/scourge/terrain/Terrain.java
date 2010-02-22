package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import org.scourge.*;
import org.scourge.terrain.Direction;

import java.util.HashMap;
import java.util.Map;

/**
 * User: gabor
 * Date: Feb 17, 2010
 * Time: 10:05:51 AM
 */
public class Terrain implements NodeGenerator {
    private Node terrain;
    private Main main;
    private Map<String, Section> sections = new HashMap<String, Section>();
    private final static float STEP_SIZE = 4;
    private final static float STEP_HEIGHT = 1;

    public Terrain(Main main) {
        this.main = main;
        this.terrain = new Node("terrain");
        terrain.setModelBound(new BoundingBox());
    }

    public Section addSection(int x, int y, int z) {
        Section section = new Section(main, x, y, z);
        sections.put(genKey(x, y, z), section);

        Section north = sections.get(genKey(x, y, z - (int)Section.SECTION_WIDTH));
        Section north_above = sections.get(genKey(x, y + (int)Section.SECTION_HEIGHT, z - (int)Section.SECTION_WIDTH));
        Section north_below = sections.get(genKey(x, y - (int)Section.SECTION_HEIGHT, z - (int)Section.SECTION_WIDTH));
        Section south = sections.get(genKey(x, y, z + (int)Section.SECTION_WIDTH));
        Section south_above = sections.get(genKey(x, y + (int)Section.SECTION_HEIGHT, z + (int)Section.SECTION_WIDTH));
        Section south_below = sections.get(genKey(x, y - (int)Section.SECTION_HEIGHT, z + (int)Section.SECTION_WIDTH));
        Section east = sections.get(genKey(x + (int)Section.SECTION_WIDTH, y, z));
        Section east_above = sections.get(genKey(x + (int)Section.SECTION_WIDTH, y + (int)Section.SECTION_HEIGHT, z));
        Section east_below = sections.get(genKey(x + (int)Section.SECTION_WIDTH, y - (int)Section.SECTION_HEIGHT, z));
        Section west = sections.get(genKey(x - (int)Section.SECTION_WIDTH, y, z));
        Section west_above = sections.get(genKey(x - (int)Section.SECTION_WIDTH, y + (int)Section.SECTION_HEIGHT, z));
        Section west_below = sections.get(genKey(x - (int)Section.SECTION_WIDTH, y - (int)Section.SECTION_HEIGHT, z));

        System.err.println(">>> xz=" + x + "," + z);
        System.err.println("\tnorth=" + (north != null) + " south=" + (south != null) + " east=" + (east != null) + " west=" + (west != null));
        System.err.println("\tabove north=" + (north_above != null) + " south=" + (south_above != null) + " east=" + (east_above != null) + " west=" + (west_above != null));
        System.err.println("\tbelow north=" + (north_below != null) + " south=" + (south_below != null) + " east=" + (east_below != null) + " west=" + (west_below != null));

        if(north == null) {
            if(north_above == null) {
                section.addMountain(Direction.NORTH);
            }
        } else {
            north.removeMountain(Direction.SOUTH);
        }
        if(north_below != null) {
            north_below.removeMountain(Direction.SOUTH);
        }        

        if(south == null) {
            if(south_above == null) {
                section.addMountain(Direction.SOUTH);
            }
        } else {
            south.removeMountain(Direction.NORTH);
        }
        if(south_below != null) {
            south_below.removeMountain(Direction.NORTH);
        }

        if(east == null) {
            if(east_above == null) {
                section.addMountain(Direction.EAST);
            }
        } else {
            east.removeMountain(Direction.WEST);
        }
        if(east_below != null) {
            east_below.removeMountain(Direction.WEST);
        }


        if(west == null) {
            if(west_above == null) {
                section.addMountain(Direction.WEST);
            }
        } else {
            west.removeMountain(Direction.EAST);
        }
        if(west_below != null) {
            west_below.removeMountain(Direction.EAST);
        }

        terrain.attachChild(section.getNode());
        terrain.updateModelBound();
        terrain.updateWorldBound();

        return section;
    }

    private String genKey(int x, int y, int z) {
        return "" + x + "," + y + "," + z;
    }

    @Override
    public Node getNode() {
        return terrain;
    }

    /*
     * Connect two sections via stairs.
     */
    public void addStairs(int x1, int y1, int z1, int x2, int y2, int z2) {
        if(Math.abs(y1 - y2) != Section.SECTION_HEIGHT) throw new IllegalArgumentException("Section heights must be 1 block apart.");
        if(!((x1 == x2 && Math.abs(z1 - z2) == Section.SECTION_WIDTH) ||
             (Math.abs(x1 - x2) == Section.SECTION_WIDTH && z1 == z2))) throw new IllegalArgumentException("Section must be adjacent.");

        // swap so section1 is lower than section2
        if(y1 > y2) {
            int tx = x1;
            int ty = y1;
            int tz = z1;
            x1 = x2;
            y1 = y2;
            z1 = z2;
            x2 = tx;
            y2 = ty;
            z2 = tz;
        }

        Section from = sections.get(genKey(x1, y1, z1));
        Section to = sections.get(genKey(x2, y2, z2));

        Direction dir;
        if(x1 < x2) {
            dir = Direction.EAST;
        } else if(x1 > x2) {
            dir = Direction.WEST;
        } else if(z1 < z2) {
            dir = Direction.SOUTH;
        } else {
            dir = Direction.NORTH;
        }

        from.removeMountain(dir);
        to.removeMountain(dir.opposite());

        Spatial ramp = ShapeUtil.load3ds("./data/3ds/ramp.3ds", "./data/textures", "section");
        ramp.getLocalTranslation().y += Section.SECTION_HEIGHT * ShapeUtil.WALL_WIDTH;

        Quaternion q = new Quaternion();
        q.fromAngleAxis(FastMath.DEG_TO_RAD * (dir.getAngle() + 90), Vector3f.UNIT_Y);
        ramp.getLocalRotation().multLocal(q);

        ramp.getLocalTranslation().x = +Section.SECTION_WIDTH * ShapeUtil.WALL_WIDTH;
//
//        Vector3f direction = dir.getDirVector().clone();
//        ramp.getLocalTranslation().addLocal(direction.mult(Section.SECTION_WIDTH * ShapeUtil.WALL_WIDTH / 2, direction));

        from.getNode().attachChild(ramp);
        from.getNode().updateModelBound();
        from.getNode().updateWorldBound();

    }
}
