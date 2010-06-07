package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.intersection.TrianglePickResults;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Ray;
import com.jme.math.Vector3f;
import com.jme.scene.Controller;
import com.jme.scene.Node;
import com.jme.system.DisplaySystem;
import org.scourge.Main;
import org.scourge.terrain.NodeGenerator;
import org.scourge.terrain.ShapeUtil;

import java.util.HashMap;
import java.util.Map;

/**
 * User: gabor
 * Date: Feb 9, 2010
 * Time: 9:51:05 AM
 */
public class Md2Model implements NodeGenerator {
    private Node player;
    private Map<Md2Key, Integer[]> keyframes = new HashMap<Md2Key, Integer[]>();
    private final Ray down = new Ray();
    private final Ray forward = new Ray();
    private TrianglePickResults results, noDistanceResults;
    private Quaternion q = new Quaternion();
    private Quaternion p = new Quaternion();
    private Vector3f direction = new Vector3f();
//    private Box debug;

    public enum Md2Key {
        crpain, death, pain, crstand, run, crdeath, jump, salute, point, stand, crattack, wave, attack, taunt, flip, crwalk, crstnd, crattak
    }


    public Md2Model(String model, String skin) {
        // point it down
        down.getDirection().set(new Vector3f(0, -1, 0));
        results = new TrianglePickResults();
        results.setCheckDistance(true);

        noDistanceResults = new TrianglePickResults();
        noDistanceResults.setCheckDistance(false);

        Map<String, Integer[]> frames = new HashMap<String, Integer[]>();
        player = ShapeUtil.loadMd2(model, skin, "player", DisplaySystem.getDisplaySystem(), true, frames);
        //moveTo(pos);
        player.setLocalScale(.2f);

        for(String s : frames.keySet()) {
            keyframes.put(Md2Key.valueOf(s), frames.get(s));
        }
    }

    public void moveTo(Vector3f pos) {
        player.setLocalTranslation(new Vector3f(pos.x * ShapeUtil.WALL_WIDTH, pos.y * ShapeUtil.WALL_WIDTH, pos.z * ShapeUtil.WALL_WIDTH));
    }

    @Override
    public Node getNode() {
        return player;
    }

    public void moveToTopOfTerrain() {
        Terrain.moveOnTopOfTerrain(player);
    }

    public boolean canMoveForward(Vector3f proposedLocation) {
        forward.setDirection(getDirection());
        forward.setOrigin(player.getWorldBound().getCenter());
        forward.getOrigin().y -= ((BoundingBox)player.getWorldBound()).yExtent / 2;
        results.clear();
        Main.getMain().getTerrain().getNode().findPick(forward, results);
        if(results.getNumber() <= 0 || results.getPickData(0).getDistance() >= 6) {
            down.getOrigin().set(proposedLocation);
            down.getOrigin().addLocal(getDirection().normalizeLocal().multLocal(2.0f));
            noDistanceResults.clear();
            Main.getMain().getTerrain().getNode().findPick(down, noDistanceResults);
            for(int i = 0; i < noDistanceResults.getNumber(); i++) {
                if(noDistanceResults.getPickData(i).getTargetTris().size() > 0) return true;
            }
            return false;
        } else {
            return false;
        }
    }

    public Vector3f getDirection() {
        q.set(player.getLocalRotation());
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
        return player.getChild(0).getController(0);
    }

    public int getX() {
        return Math.round(getNode().getLocalTranslation().x / ShapeUtil.WALL_WIDTH);
    }
        
    public int getZ() {
        return Math.round(getNode().getLocalTranslation().z / ShapeUtil.WALL_WIDTH);
    }
}
