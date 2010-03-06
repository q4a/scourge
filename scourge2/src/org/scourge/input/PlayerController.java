package org.scourge.input;

import com.jme.input.*;
import com.jme.input.action.InputActionEvent;
import com.jme.input.action.KeyInputAction;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import org.scourge.Main;
import org.scourge.input.MouseAction;
import org.scourge.terrain.Player;

/**
 * User: gabor
 * Date: Feb 9, 2010
 * Time: 4:33:24 PM
 */
public class PlayerController extends InputHandler {
    public final static float PLAYER_ROTATE_STEP = 100.0f;
    public final static float PLAYER_SPEED = 50.0f;
    public static boolean fixed_camera = true;

    private boolean turnLeft, turnRight, move;
    private Main main;

    public PlayerController(final Main main) {
        this.main = main;

        // set up mouse looking around
        RelativeMouse mouse = new RelativeMouse("Mouse Input");
        mouse.registerWithInputHandler( this );
        MouseAction mouseAction = new MouseAction(main, mouse);
        addAction(mouseAction);

        KeyBindingManager keyboard = KeyBindingManager.getKeyBindingManager();
        keyboard.set( "scourge_left", KeyInput.KEY_A );
        keyboard.set( "scourge_right", KeyInput.KEY_D );
        keyboard.set( "scourge_forward", KeyInput.KEY_W );
        keyboard.set( "scourge_look_left", KeyInput.KEY_O );
        keyboard.set( "scourge_look_right", KeyInput.KEY_P );

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

        KeyInput.get().addListener( new KeyInputListener() {
            public void onKey( char character, int keyCode, boolean pressed ) {
//                if(keyCode == KeyInput.KEY_A) {
//                    turnLeft = pressed;
//                    turnRight = false;
//                } else if(keyCode == KeyInput.KEY_D) {
//                    turnRight = pressed;
//                    turnLeft = false;
//                }
//                if(keyCode == KeyInput.KEY_W) {
//                    move = pressed;
//                }

                if(keyCode == KeyInput.KEY_F5 && !pressed) {
                    main.toggleCameraAttached();
                }

//                if(turnLeft || turnRight || move) {
                if(keyCode == KeyInput.KEY_W && pressed) {
                    main.getPlayer().setKeyFrame(Player.Md2Key.run);
                } else {
                    main.getPlayer().setKeyFrame(Player.Md2Key.stand);
                }
            }
        } );
    }

}