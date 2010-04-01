package org.scourge.input;

import com.jme.input.action.InputActionEvent;
import com.jme.input.action.KeyInputAction;
import com.jme.math.Vector3f;
import org.scourge.Main;
import org.scourge.terrain.Direction;
import org.scourge.terrain.Player;
import org.scourge.terrain.Region;
import org.scourge.terrain.ShapeUtil;

/**
* User: gabor
* Date: Feb 22, 2010
* Time: 8:07:17 AM
*/
class ForwardMoveAction extends KeyInputAction {
    private Vector3f tempVa;
    private Vector3f direction;
    private final Main main;

    public ForwardMoveAction(Main main) {
        this.main = main;
        tempVa = new Vector3f();
        direction = new Vector3f();
    }

    @Override
    public void performAction(InputActionEvent event) {
        if(main.getPlayer().canMoveForward()) {
            direction.set(main.getPlayer().getDirection());
            main.getPlayer().getNode().getLocalTranslation().addLocal(direction.mult(PlayerController.PLAYER_SPEED * event.getTime(), tempVa));

            int px = (int)(main.getPlayer().getNode().getLocalTranslation().x / ShapeUtil.WALL_WIDTH);
            int py = (int)(main.getPlayer().getNode().getLocalTranslation().z / ShapeUtil.WALL_WIDTH);

            if(px < 10) {
                main.getTerrain().loadRegion(Direction.WEST);
            } else if(px >= Region.REGION_SIZE - 10) {
                main.getTerrain().loadRegion(Direction.EAST);
            }

            if(py < 10) {
                main.getTerrain().loadRegion(Direction.NORTH);
            } else if(py >= Region.REGION_SIZE - 10) {
                main.getTerrain().loadRegion(Direction.SOUTH);
            }
        }
    }
}
