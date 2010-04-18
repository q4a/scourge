package org.scourge.input;

import com.jme.input.action.InputActionEvent;
import com.jme.input.action.KeyInputAction;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import org.scourge.Main;

/**
* User: gabor
* Date: Feb 22, 2010
* Time: 8:07:01 AM
*/
class LeftTurnAction extends KeyInputAction {
    private Quaternion q;
    private final Main main;

    public LeftTurnAction(Main main) {
        this.main = main;
        q = new Quaternion();
    }

    @Override
    public void performAction(InputActionEvent event) {
//        if(event.getTriggerPressed() || event.getTime() != 0) {
//            main.getPlayer().setKeyFrame(Player.Md2Key.run);
//        } else {
//            main.getPlayer().setKeyFrame(Player.Md2Key.stand);
//        }
        q.fromAngleAxis(FastMath.DEG_TO_RAD * PlayerController.PLAYER_ROTATE_STEP * event.getTime(), Vector3f.UNIT_Y);
        main.getPlayer().getCreatureModel().getNode().getLocalRotation().multLocal(q);
        if(!PlayerController.fixed_camera) {
            q.fromAngleAxis(FastMath.DEG_TO_RAD * -PlayerController.PLAYER_ROTATE_STEP * event.getTime(), Vector3f.UNIT_Y);
            main.getCameraHolder().getLocalRotation().multLocal(q);
        }
    }
}
