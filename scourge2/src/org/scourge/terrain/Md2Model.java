package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.intersection.BoundingCollisionResults;
import com.jme.intersection.CollisionResults;
import com.jme.intersection.TrianglePickResults;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Ray;
import com.jme.math.Vector3f;
import com.jme.scene.*;
import com.jme.scene.shape.Box;
import com.jme.system.DisplaySystem;
import org.scourge.Main;
import org.scourge.terrain.NodeGenerator;
import org.scourge.terrain.ShapeUtil;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * User: gabor
 * Date: Feb 9, 2010
 * Time: 9:51:05 AM
 */
public class Md2Model implements NodeGenerator {
    private Node node;
    private Map<Md2Key, Integer[]> keyframes = new HashMap<Md2Key, Integer[]>();
    private final Ray down = new Ray();
    private final Ray forward = new Ray();
    private TrianglePickResults noDistanceResults;
    private Quaternion q = new Quaternion();
    private Quaternion p = new Quaternion();
    private Vector3f direction = new Vector3f();
    private CollisionResults collisionResults;
    private static final float MD2_SCALE = .2f;

    public enum Md2Key {
        crpain, death, pain, crstand, run, crdeath, jump, salute, point, stand, crattack, wave, attack, taunt, flip, crwalk, crstnd, crattak
    }


    public Md2Model(String model, String skin, String namePrefix) {
        collisionResults = new BoundingCollisionResults();

        // point it down
        down.getDirection().set(new Vector3f(0, -1, 0));
        noDistanceResults = new TrianglePickResults();
        noDistanceResults.setCheckDistance(false);

        Map<String, Integer[]> frames = new HashMap<String, Integer[]>();
        node = ShapeUtil.loadMd2(model, skin, namePrefix, DisplaySystem.getDisplaySystem(), true, frames);
        node.setLocalScale(MD2_SCALE);

        for(String s : frames.keySet()) {
            keyframes.put(Md2Key.valueOf(s), frames.get(s));
        }
    }

    public void moveTo(Vector3f pos) {
        node.setLocalTranslation(new Vector3f(pos.x * ShapeUtil.WALL_WIDTH, pos.y * ShapeUtil.WALL_WIDTH, pos.z * ShapeUtil.WALL_WIDTH));
    }

    @Override
    public Node getNode() {
        return node;
    }

    public void moveToTopOfTerrain() {
        Terrain.moveOnTopOfTerrain(node);
    }

    private Vector3f backupLocation = new Vector3f();
    @SuppressWarnings({"FieldCanBeLocal"})
    private float backupScaleY;
    public boolean canMoveTo(Vector3f proposedLocation) {
        boolean retValue = false;

        // collisions are sooo simple in jme...
        backupLocation.set(node.getLocalTranslation());
        node.getLocalTranslation().set(proposedLocation);
        backupScaleY = node.getLocalScale().y;
        // Make the player 2 units tall. This "lifts" him off the floor so we can walk over floor irregularities, ramps, bridges, etc.
        // Also this makes him shorter so we can fit thru the dungeon entrance. Yes this is a hack
        node.getLocalScale().y = (2.0f / ((BoundingBox)node.getWorldBound()).yExtent) * node.getLocalScale().y;
        node.updateGeometricState(0,true); // make geometry changes take effect now!

//        System.err.println("proposedLocation=" + proposedLocation + " node=" + node.getName() +
//                           " region=" + ((proposedLocation.x / ShapeUtil.WALL_WIDTH) / Region.REGION_SIZE) + "," + ((proposedLocation.z / ShapeUtil.WALL_WIDTH) / Region.REGION_SIZE) +
//                           " offset=" + ((proposedLocation.x / ShapeUtil.WALL_WIDTH) % Region.REGION_SIZE) + "," + ((proposedLocation.z / ShapeUtil.WALL_WIDTH) % Region.REGION_SIZE));

        // check where the bounding box is
        boolean collisions = checkBoundingCollisions(Main.getMain().getTerrain().getNode());
//        System.err.println("\tbound collisions found anything? " + collisions);

        // check for triangles within the bounding box
        for(int i = 0; i < collisionResults.getNumber(); i++) {
            Geometry g = collisionResults.getCollisionData(i).getTargetMesh();
            Node parentNode = g.getParent();
            if(parentNode != null) {
                collisions = hasTriangleCollision(node, parentNode);
                if(collisions) break;
            }
        }
//        System.err.println("\ttri collisions ok? " + !collisions);

        // check for water below
        if(!collisions) {
            down.getOrigin().set(getNode().getLocalTranslation());
            down.getOrigin().addLocal(getDirection().normalizeLocal().multLocal(2.0f));
            noDistanceResults.clear();
            Main.getMain().getTerrain().getNode().findPick(down, noDistanceResults);
            for(int i = 0; i < noDistanceResults.getNumber(); i++) {
                if(noDistanceResults.getPickData(i).getTargetTris().size() > 0) {
                    retValue = true;
                    break;
                }
            }
//            System.err.println("\ton water? " + !retValue);
        }

        // reset the node's shape and position
        node.getLocalScale().y = backupScaleY;
        node.getLocalTranslation().set(backupLocation);
        node.updateModelBound();
        node.updateGeometricState(0,true); // make geometry changes take effect now!

        return retValue;
    }


    public boolean hasTriangleCollision(Node n1,Node nodeWithSharedNodes) {
		List<Spatial> geosN1 = n1.descendantMatches(TriMesh.class);
		for (Spatial triN1 : geosN1) {
			if (hasTriangleCollision((TriMesh)triN1, nodeWithSharedNodes))
				return true;
		}
		return false;
	}

	public boolean hasTriangleCollision(TriMesh sp,Node nodeWithSharedNodes) {
		List<Spatial> geosN2 = nodeWithSharedNodes.descendantMatches(TriMesh.class);

		for (Spatial triN2 : geosN2) {
            if (((TriMesh)triN2).hasTriangleCollision(sp)) {
//                    System.err.println("collision: " + triN2.getName());
                return true;
            }
		}
		return false;
	}

    public boolean checkBoundingCollisions(Node world) {
        collisionResults.clear();
        node.findCollisions(world, collisionResults);
        return collisionResults.getNumber() > 0;
    }

    public Vector3f getDirection() {
        q.set(node.getLocalRotation());
        q.multLocal(p.fromAngleAxis(FastMath.DEG_TO_RAD * 90.0f, Vector3f.UNIT_Y));
        q.getRotationColumn( 2, direction );
        return direction;
    }

    public void setKeyFrame(Md2Key key) {
        setKeyFrame(key, 10);
    }

    public void setKeyFrame(Md2Key key, float speed) {
        Controller c = getController();
        c.setSpeed(speed);
        c.setActive(true);
        c.setRepeatType(Controller.RT_WRAP);
        Integer[] times = keyframes.get(key);
        c.setMinTime(times[0]);
        c.setMaxTime(times[1] - 1);
    }

    private Controller getController() {
        return node.getChild(0).getController(0);
    }

    public int getX() {
        return Math.round(getNode().getLocalTranslation().x / ShapeUtil.WALL_WIDTH);
    }
        
    public int getZ() {
        return Math.round(getNode().getLocalTranslation().z / ShapeUtil.WALL_WIDTH);
    }
}
