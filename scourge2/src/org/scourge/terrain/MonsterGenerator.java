package org.scourge.terrain;

import com.jme.math.Vector3f;
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
    private List<Monster> monsters = new ArrayList<Monster>();

    public MonsterGenerator(Region region, int x, int y, String monsterName) {
        super(region, x, y);
        this.monsterName = monsterName;
    }

    @Override
    public void generate() {
        if(monsters.size() < 3) {
            // put a new monster on the map
            Monster monster = Monster.valueOf(monsterName);
            if(getRegion().findSpaceAround(getX(), getY(), monster)) {
                // add the node
                monster.getCreatureModel().getNode().updateModelBound();
                getRegion().getNode().attachChild(monster.getCreatureModel().getNode());
                getRegion().getNode().updateModelBound();
                getRegion().getNode().updateWorldBound();
                System.err.println("&&& Added " + monster.name() + " on region " + getRegion() + " near " + getX() + "," + getY());
            }
        }
    }
}

