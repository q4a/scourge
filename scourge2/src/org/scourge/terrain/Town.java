package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import org.scourge.Main;
import org.scourge.terrain.NodeGenerator;
import org.scourge.terrain.ShapeUtil;
import org.scourge.terrain.House;

import java.util.ArrayList;
import java.util.List;

/**
 * User: gabor
 * Date: Feb 8, 2010
 * Time: 9:56:19 AM
 */
public class Town implements NodeGenerator {
    private Node town;
    private List<House> houses = new ArrayList<House>();


    public Town(Main main, float x, float y, float z) {
        town = new Node(ShapeUtil.newShapeName("town"));

//        houses.add(new House(main, 0, 0, 0, 3, 3, 3));
//        houses.add(new House(main, 3, 0, 6, 3, 4, 2));
//        houses.add(new House(main, 6, 0, 6, 2, 2, 1));
//        houses.add(new House(main, -4, 0, -4, 3, 2, 1));
//        houses.add(new House(main, -4, 0, 6, 3, 3, 1));

        houses.add(new House(main, 0, 0, 0, 2, 3, 2));
        houses.add(new House(main, 3, 0, 0, 2, 2, 1));
        houses.add(new House(main, 3, 0, -4, 2, 2, 1));

        for(House house : houses) {
            town.attachChild(house.getNode());
        }

        town.getLocalTranslation().x = x * ShapeUtil.WALL_WIDTH;
        town.getLocalTranslation().y = y * ShapeUtil.WALL_WIDTH;
        town.getLocalTranslation().z = z * ShapeUtil.WALL_WIDTH;
        Quaternion q = new Quaternion();
        q.fromAngleAxis(FastMath.DEG_TO_RAD * (float)(45.0f * Math.random()), Vector3f.UNIT_Y);
        town.getLocalRotation().multLocal(q);
        BoundingBox bb = new BoundingBox();
        town.setModelBound(bb);
        town.updateModelBound();
        town.updateWorldBound();
    }

    @Override
    public Node getNode() {
        return town;
    }

    public void moveToTopOfTerrain() {
        for(House house : houses) {
            house.moveToTopOfTerrain();
        }
    }
}
