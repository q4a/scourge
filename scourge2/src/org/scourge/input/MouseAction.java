package org.scourge.input;

import com.jme.input.Mouse;
import com.jme.input.MouseInput;
import com.jme.input.action.InputActionEvent;
import com.jme.input.action.MouseInputAction;
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
    private Main main;
    private Quaternion q = new Quaternion();
    private InputActionEvent event = new InputActionEvent();
    private RightTurnAction rotateRight;
    private LeftTurnAction rotateLeft;

    public MouseAction(Main main, Mouse mouse) {
        this.main = main;
        this.mouse = mouse;
        rotateRight = new RightTurnAction(main);
        rotateLeft = new LeftTurnAction(main);
    }

    @Override
    public void performAction(InputActionEvent evt) {
        if(MouseInput.get().isButtonDown(0)) {
            if(mouse.getLocalTranslation().x != 0) {
                q.fromAngleAxis(FastMath.DEG_TO_RAD * 0.01f * -mouse.getLocalTranslation().x * PlayerController.PLAYER_ROTATE_STEP, Vector3f.UNIT_Y);
                main.getCameraHolder().getLocalRotation().multLocal(q);
                main.getCamera().normalize();
                main.getCamera().update();
            }
            if(mouse.getLocalTranslation().y != 0) {
                q.fromAngleAxis(FastMath.DEG_TO_RAD * 0.01f * -mouse.getLocalTranslation().y * PlayerController.PLAYER_ROTATE_STEP, Vector3f.UNIT_X);
                p.set(main.getCameraNode().getLocalRotation());
                p.multLocal(q);
                p.toAngles(angles);

                float angle = angles[0] * FastMath.RAD_TO_DEG;
                if(angle > 0 && angle < 50) {
                    main.getCameraNode().getLocalRotation().set(p);
                    main.getCamera().normalize();
                    main.getCamera().update();
                }
            }
        } else {
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
}
