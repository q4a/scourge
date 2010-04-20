package org.scourge.input;

import com.jme.input.*;
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

    private SwitchCameraListener switchCameraListener = new SwitchCameraListener();

    public PlayerController(final Main main) {
        this.main = main;

        RelativeMouse mouse = new RelativeMouse("Mouse Input");
        mouse.registerWithInputHandler(this);
        mouseAction = new MouseAction(main, mouse);

        setEnabled(true);
    }

    public void setEnabled(boolean b) {
        KeyBindingManager keyboard = KeyBindingManager.getKeyBindingManager();
        KeyInput keyInput = KeyInput.get();
        if(b) {
            addAction(new ForwardMoveAction(main), "scourge_forward", true );
            addAction(new LeftTurnAction(main), "scourge_left", true);
            addAction(new RightTurnAction(main), "scourge_right", true);

            keyInput.addListener(switchCameraListener);

            keyboard.add( "scourge_left", KeyInput.KEY_A );
            keyboard.add( "scourge_right", KeyInput.KEY_D );
            keyboard.add( "scourge_forward", KeyInput.KEY_W );
        } else {
            keyboard.remove("scourge_left");
            keyboard.remove("scourge_right");
            keyboard.remove("scourge_forward");

            keyInput.removeListener(switchCameraListener);

            removeAction("scourge_forward");
            removeAction("scourge_left");
            removeAction("scourge_right");
        }
    }

    private class SwitchCameraListener implements KeyInputListener {
        public void onKey( char character, int keyCode, boolean pressed ) {
            KeyInput keyInput = KeyInput.get();
            if(keyCode == KeyInput.KEY_F5 && !pressed) {
                main.toggleCameraAttached();
            }
            main.getPlayer().getCreatureModel().setKeyFrame(keyInput.isKeyDown(KeyInput.KEY_W) ? Md2Model.Md2Key.run : Md2Model.Md2Key.stand);
        }
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
