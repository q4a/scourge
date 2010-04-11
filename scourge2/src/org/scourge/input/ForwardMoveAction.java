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
    private Vector3f proposedLocation;

    public ForwardMoveAction(Main main) {
        this.main = main;
        tempVa = new Vector3f();
        direction = new Vector3f();
        proposedLocation = new Vector3f();
    }

    @Override
    public void performAction(InputActionEvent event) {
        proposedLocation.set(main.getPlayer().getNode().getLocalTranslation());
        proposedLocation.addLocal(direction.mult(PlayerController.PLAYER_SPEED * event.getTime(), tempVa));
        if(main.getPlayer().canMoveForward(proposedLocation)) {
            direction.set(main.getPlayer().getDirection());
            main.getPlayer().getNode().getLocalTranslation().set(proposedLocation);

            main.getTerrain().loadRegion();
        }
    }
}
