package org.scourge.input;

import com.jme.input.*;
import com.jme.input.action.InputActionEvent;
import com.jme.input.action.KeyInputAction;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import org.scourge.Main;
import org.scourge.input.MouseAction;
import org.scourge.terrain.Md2Model;

/**
 * User: gabor
 * Date: Feb 9, 2010
 * Time: 4:33:24 PM
 */
public class PlayerController extends InputHandler {
    public final static float PLAYER_ROTATE_STEP = 150.0f;
    public final static float PLAYER_SPEED = 50.0f;
    public static boolean fixed_camera = true;

    private Main main;

    private MouseAction mouseAction;
    private boolean mouseAdded = false;

    public PlayerController(final Main main) {
        this.main = main;

        // set up mouse looking around

        RelativeMouse mouse = new RelativeMouse("Mouse Input");
        mouse.registerWithInputHandler( this );
        mouseAction = new MouseAction(main, mouse);

        addAction(new ForwardMoveAction(main), "scourge_forward", true );
        addAction(new LeftTurnAction(main), "scourge_left", true);
        addAction(new RightTurnAction(main), "scourge_right", true);

        addAction(new KeyInputAction() {
            private Quaternion q = new Quaternion();

            @Override
            public void performAction(InputActionEvent event) {
                q.fromAngleAxis(FastMath.DEG_TO_RAD * PLAYER_ROTATE_STEP * event.getTime(), Vector3f.UNIT_Y);
                main.getCameraHolder().getLocalRotation().multLocal(q);
            }
        }, "scourge_look_left", true);

        addAction(new KeyInputAction() {
            private Quaternion q = new Quaternion();

            @Override
            public void performAction(InputActionEvent event) {
                q.fromAngleAxis(FastMath.DEG_TO_RAD * -PLAYER_ROTATE_STEP * event.getTime(), Vector3f.UNIT_Y);
                main.getCameraHolder().getLocalRotation().multLocal(q);
            }
        }, "scourge_look_right", true);

        final KeyInput keyInput = KeyInput.get();
        keyInput.addListener( new KeyInputListener() {
            public void onKey( char character, int keyCode, boolean pressed ) {
                if(keyCode == KeyInput.KEY_F5 && !pressed) {
                    main.toggleCameraAttached();
                }
                main.getPlayer().getCreatureModel().setKeyFrame(keyInput.isKeyDown(KeyInput.KEY_W) ? Md2Model.Md2Key.run : Md2Model.Md2Key.stand);
            }
        } );

        KeyBindingManager keyboard = KeyBindingManager.getKeyBindingManager();
        keyboard.add( "scourge_left", KeyInput.KEY_A );
        keyboard.add( "scourge_right", KeyInput.KEY_D );
        keyboard.add( "scourge_forward", KeyInput.KEY_W );
        keyboard.add( "scourge_look_left", KeyInput.KEY_O );
        keyboard.add( "scourge_look_right", KeyInput.KEY_P );
    }

    public void toggleMouseDrive(boolean desiredValue) {
        if(mouseAdded != desiredValue) {
            toggleMouseDrive();
        }
    }

    public void toggleMouseDrive() {
        if(mouseAdded) {
            removeAction(mouseAction);
            mouseAdded = false;
            MouseInput.get().setCursorVisible(true);
        } else {
            addAction(mouseAction);
            mouseAdded = true;
            MouseInput.get().setCursorVisible(false);
        }
    }
}
