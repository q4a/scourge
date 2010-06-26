package org.scourge.terrain;

import com.jme.math.Vector3f;
import org.scourge.Main;
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

    public MonsterGenerator(Region region, int x, int y, String monsterName) {
        super(region, x, y);
        this.monsterName = monsterName;
    }

    @Override
    public void generate() {
        if(monsters.size() < 3 && monsterName != null) {
            // put a new monster on the map
            Monster monster = Monster.valueOf(monsterName);
            MonsterInstance monsterInstance = new MonsterInstance(monster);
            if(getRegion().findSpaceAround(getX(), getY(), monsterInstance, monsterInstance.getCreatureModel().getNode().getLocalTranslation())) {
                monsters.add(monsterInstance);
                monsterInstance.getCreatureModel().getNode().getLocalTranslation().x -= getRegion().getX() * ShapeUtil.WALL_WIDTH;
                monsterInstance.getCreatureModel().getNode().getLocalTranslation().z -= getRegion().getY() * ShapeUtil.WALL_WIDTH;
                getRegion().getNode().attachChild(monsterInstance.getCreatureModel().getNode());
                
                monsterInstance.getCreatureModel().getNode().updateRenderState();
                monsterInstance.getCreatureModel().getNode().updateWorldData(0);
                monsterInstance.getCreatureModel().getNode().updateModelBound();
                monsterInstance.getCreatureModel().getNode().updateWorldBound();
                getRegion().getNode().updateRenderState();
                getRegion().getNode().updateWorldData(0);
                getRegion().getNode().updateModelBound();
                getRegion().getNode().updateWorldBound();
                System.err.println("\t&&& Added " + monster.name() + " on region " + getRegion() + " near " + getX() + "," + getY());
            }
        }
    }

    @Override
    public void update() {
        for(MonsterInstance monsterInstance : monsters) {
            monsterInstance.getCreatureModel().moveToTopOfTerrain();
        }
    }

    private class MonsterInstance implements HasModel {
        private Monster monster;
        private Md2Model model;

        public MonsterInstance(Monster monster) {
            this.monster = monster;
            this.model = monster.createModel();
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

