package org.scourge.input;

import com.jme.input.*;
import org.scourge.Main;
import org.scourge.input.MouseAction;
import org.scourge.terrain.Md2Model;
import org.scourge.ui.Window;

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
    private SwitchCameraListener switchCameraListener = new SwitchCameraListener();
    private MouseInputListener mouseListener = new DriveTogglingListener();

    public PlayerController(final Main main) {
        this.main = main;
        RelativeMouse mouse = new RelativeMouse("Mouse Input");
        mouse.registerWithInputHandler(this);
        mouseAction = new MouseAction(main, mouse);

        setEnabled(true);
    }

    public void setEnabled(boolean b) {
        MouseInput mouseInput = MouseInput.get();
        KeyBindingManager keyboard = KeyBindingManager.getKeyBindingManager();
        KeyInput keyInput = KeyInput.get();
        if(b) {
            addAction(new ForwardMoveAction(main), "scourge_forward", true );
            addAction(new LeftTurnAction(main), "scourge_left", true);
            addAction(new RightTurnAction(main), "scourge_right", true);

            keyInput.addListener(switchCameraListener);
            mouseInput.addListener(mouseListener);
            addAction(mouseAction);

            keyboard.add( "scourge_left", KeyInput.KEY_A );
            keyboard.add( "scourge_right", KeyInput.KEY_D );
            keyboard.add( "scourge_forward", KeyInput.KEY_W );
        } else {
            keyboard.remove("scourge_left");
            keyboard.remove("scourge_right");
            keyboard.remove("scourge_forward");

            keyInput.removeListener(switchCameraListener);
            mouseInput.removeListener(mouseListener);
            removeAction(mouseAction);

            removeAction("scourge_forward");
            removeAction("scourge_left");
            removeAction("scourge_right");
        }
        MouseInput.get().setCursorVisible(true);
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

    private class DriveTogglingListener implements MouseInputListener {

        @Override
        public void onButton(int button, boolean pressed, int x, int y) {
            if(button == 0) {
                if(pressed) {
                    if(!(Window.getWindow() != null && Window.getWindow().getRectangle().contains(x, y))) {
                        mouseAction.setEnabled(true);
                    }
                } else {
                    mouseAction.setEnabled(false);
                }
            }
        }

        @Override
        public void onWheel(int wheelDelta, int x, int y) {
        }

        @Override
        public void onMove(int xDelta, int yDelta, int newX, int newY) {
        }
    }
}
