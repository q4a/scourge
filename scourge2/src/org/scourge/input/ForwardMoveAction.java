package org.scourge.input;

import com.jme.input.action.InputActionEvent;
import com.jme.input.action.KeyInputAction;
import com.jme.math.Vector3f;
import org.scourge.Main;

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
        proposedLocation.set(main.getPlayer().getCreatureModel().getNode().getLocalTranslation());
        proposedLocation.addLocal(direction.mult(PlayerController.PLAYER_SPEED * event.getTime(), tempVa));
        if(main.getPlayer().getCreatureModel().canMoveForward(proposedLocation)) {
            direction.set(main.getPlayer().getCreatureModel().getDirection());
            main.getPlayer().getCreatureModel().getNode().getLocalTranslation().set(proposedLocation);

            main.getTerrain().loadRegion();
            main.checkRoof();
        }
    }
}
