package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.intersection.TrianglePickResults;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Ray;
import com.jme.math.Vector3f;
import com.jme.scene.Controller;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Box;
import com.jme.scene.state.CullState;
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
public class Player implements NodeGenerator {
    private Node player;
    private Main main;
    private Map<Md2Key, Integer[]> keyframes = new HashMap<Md2Key, Integer[]>();
    private final Ray down = new Ray();
    private final Ray forward = new Ray();
    private TrianglePickResults results, noDistanceResults;
    private Quaternion q = new Quaternion();
    private Quaternion p = new Quaternion();
    private Vector3f direction = new Vector3f();
//    private Box debug;

    public enum Md2Key {
        crpain, death, pain, crstnd, run, crdeath, jump, salute, point, stand, crattak, wave, attack, taunt, flip, crwalk
    }


    public Player(Main main, float x, float y, float z) {
        this.main = main;

        // point it down
        down.getDirection().set(new Vector3f(0, -1, 0));
        results = new TrianglePickResults();
        results.setCheckDistance(true);

        noDistanceResults = new TrianglePickResults();
        noDistanceResults.setCheckDistance(false);

        Map<String, Integer[]> frames = new HashMap<String, Integer[]>();
        player = ShapeUtil.loadMd2("./data/models/sfod8/tris.md2", "./data/models/sfod8/Rieger.png", "player", main.getDisplay(), true, frames);
        player.setLocalTranslation(new Vector3f(x * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, z * ShapeUtil.WALL_WIDTH));
        player.setLocalScale(.2f);

        for(String s : frames.keySet()) {
            keyframes.put(Md2Key.valueOf(s), frames.get(s));
        }

//        debug = new Box("debug", new Vector3f(0,0,0), new Vector3f(4, 4, 4));
//        debug.setIsCollidable(false);
//        player.attachChild(debug);
    }

    @Override
    public Node getNode() {
        return player;
    }

    private final static float ABOVE_PLAYER = 0; // Section.SECTION_HEIGHT * ShapeUtil.WALL_WIDTH;
    public void moveToTopOfTerrain() {
        player.setIsCollidable(false);
        down.getOrigin().set(player.getWorldBound().getCenter());
        down.getOrigin().y += ABOVE_PLAYER;
        results.clear();
        main.getTerrain().getNode().findPick(down, results);
        if (results.getNumber() > 0) {
            float dist = results.getPickData(0).getDistance();
            if(!Float.isInfinite(dist) && !Float.isNaN(dist)) {
                player.getLocalTranslation().y -= dist - ABOVE_PLAYER - ((BoundingBox)player.getWorldBound()).yExtent;
            }
        }
        player.setIsCollidable(true);


//        debug.getLocalTranslation().x = ((BoundingBox)player.getWorldBound()).xExtent / 2;
//        debug.getLocalTranslation().y = 0;
//        debug.getLocalTranslation().y -= ((BoundingBox)player.getWorldBound()).yExtent;
//        debug.getLocalTranslation().z = ((BoundingBox)player.getWorldBound()).zExtent / 2;

        player.updateModelBound();
        player.updateWorldBound();
    }

    public boolean canMoveForward(Vector3f proposedLocation) {
        forward.setDirection(getDirection());
        forward.setOrigin(player.getWorldBound().getCenter());
        forward.getOrigin().y -= ((BoundingBox)player.getWorldBound()).yExtent / 2;
        results.clear();
        main.getTerrain().getNode().findPick(forward, results);
        if(results.getNumber() <= 0 || results.getPickData(0).getDistance() >= 6) {
            down.getOrigin().set(proposedLocation);
            down.getOrigin().addLocal(getDirection().normalizeLocal().multLocal(2.0f));
            down.getOrigin().y += ABOVE_PLAYER;
            noDistanceResults.clear();
            main.getTerrain().getNode().findPick(down, noDistanceResults);
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
        return (int)(getNode().getLocalTranslation().x / ShapeUtil.WALL_WIDTH);
    }
        
    public int getZ() {
        return (int)(getNode().getLocalTranslation().z / ShapeUtil.WALL_WIDTH);
    }
}
