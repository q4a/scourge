package org.scourge.terrain;

import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import org.scourge.Main;
import org.scourge.input.PlayerController;
import org.scourge.model.HasModel;
import org.scourge.model.Monster;

import java.util.ArrayList;
import java.util.List;

/**
 * User: gabor
 * Date: Jun 15, 2010
 * Time: 8:17:17 AM
 */
public class MonsterGenerator extends Generator {
    private String monsterName;
    private List<MonsterInstance> monsters = new ArrayList<MonsterInstance>();
    private Vector3f proposedLocation = new Vector3f();
    private Vector3f tempVa = new Vector3f();
    private Quaternion q = new Quaternion();
    private static final float RADIUS = 4f * ShapeUtil.WALL_WIDTH;
    private static final float RADIUS_SQUARED = RADIUS * RADIUS;

    public MonsterGenerator(Region region, int x, int y, String monsterName) {
        super(region, x, y);
        this.monsterName = monsterName;
    }

    @Override
    public void generate() {
        while(monsters.size() < 3 && monsterName != null) {
            // put a new monster on the map
            Monster monster = Monster.valueOf(monsterName);
            MonsterInstance monsterInstance = new MonsterInstance(monster);
            if(getRegion().findSpaceAround(getX(), getY(), monsterInstance, monsterInstance.getCreatureModel().getNode().getLocalTranslation())) {
                monsters.add(monsterInstance);
                Terrain terrain = Main.getMain().getTerrain();
                terrain.getNode().attachChild(monsterInstance.getCreatureModel().getNode());
                monsterInstance.getCreatureModel().getNode().updateRenderState();
                monsterInstance.getCreatureModel().getNode().updateWorldData(0);
                monsterInstance.getCreatureModel().getNode().updateModelBound();
                monsterInstance.getCreatureModel().getNode().updateWorldBound();
                terrain.getNode().updateRenderState();
                terrain.getNode().updateWorldData(0);
                terrain.getNode().updateModelBound();
                terrain.getNode().updateWorldBound();
                System.err.println("\t&&& Added " + monster.name() + " on region " + getRegion() + " near " + getX() + "," + getY());
            }
        }
    }

    @Override
    public void update(float tpf) {
        for(MonsterInstance monsterInstance : monsters) {
            monsterInstance.getCreatureModel().moveToTopOfTerrain();

            // move around
            proposedLocation.set(monsterInstance.getCreatureModel().getNode().getLocalTranslation());
            proposedLocation.addLocal(monsterInstance.getCreatureModel().getDirection().mult(monsterInstance.getMonster().getSpeed() * tpf, tempVa));
            if(proposedLocation.distanceSquared(getLocation()) < RADIUS_SQUARED && monsterInstance.getCreatureModel().canMoveTo(proposedLocation)) {
                monsterInstance.getCreatureModel().getNode().getLocalTranslation().set(proposedLocation);
            } else {
                q.fromAngleAxis(FastMath.DEG_TO_RAD * (180f + (float)(90f * Math.random()) - 45f), Vector3f.UNIT_Y);
                monsterInstance.getCreatureModel().getNode().getLocalRotation().multLocal(q);
            }
        }
    }

    @Override
    public void unloading() {
        Terrain terrain = Main.getMain().getTerrain();
        for(MonsterInstance monsterInstance : monsters) {
            terrain.getNode().detachChild(monsterInstance.getCreatureModel().getNode());
        }
        monsters.clear();
        terrain.getNode().updateRenderState();
        terrain.getNode().updateWorldData(0);
        terrain.getNode().updateModelBound();
        terrain.getNode().updateWorldBound();
    }

    private class MonsterInstance implements HasModel {
        private Monster monster;
        private Md2Model model;

        public MonsterInstance(Monster monster) {
            this.monster = monster;
            this.model = monster.createModel();
            this.model.setKeyFrame(Md2Model.Md2Key.run);
        }

        @Override
        public Md2Model getCreatureModel() {
            return model;
        }

        public Monster getMonster() {
            return monster;
        }
    }
}

