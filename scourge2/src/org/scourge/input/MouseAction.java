package org.scourge.input;

import com.jme.input.Mouse;
import com.jme.input.MouseInput;
import com.jme.input.action.InputActionEvent;
import com.jme.input.action.MouseInputAction;
import com.jme.input.dummy.DummyMouseInput;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import org.scourge.Main;
import org.scourge.input.PlayerController;

/**
* User: gabor
* Date: Feb 21, 2010
* Time: 9:26:20 AM
*/
public class MouseAction extends MouseInputAction {
    private Quaternion p = new Quaternion();
    private float[] angles = new float[3];
    private Mouse mouse;
    private Quaternion q = new Quaternion();
    private InputActionEvent event = new InputActionEvent();
    private RightTurnAction rotateRight;
    private LeftTurnAction rotateLeft;
    private boolean enabled;

    public MouseAction(Main main, Mouse mouse) {
        this.mouse = mouse;
        rotateRight = new RightTurnAction(main);
        rotateLeft = new LeftTurnAction(main);
    }

    @Override
    public void performAction(InputActionEvent evt) {
        if(enabled) {
            float time = 0.001f * PlayerController.PLAYER_ROTATE_STEP;
            if (mouse.getLocalTranslation().x > 0) {
                event.setTime(time * FastMath.DEG_TO_RAD * mouse.getLocalTranslation().x);
                rotateRight.performAction(event);
            } else if (mouse.getLocalTranslation().x < 0) {
                event.setTime(time * FastMath.DEG_TO_RAD * mouse.getLocalTranslation().x * -1);
                rotateLeft.performAction(event);
            }
        }
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
        MouseInput.get().setCursorVisible(!enabled);
    }
}
