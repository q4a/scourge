package org.scourge.input;

import com.jme.input.*;
import com.jme.scene.Spatial;
import org.scourge.Main;
import org.scourge.input.MouseAction;
import org.scourge.terrain.Md2Model;
import org.scourge.ui.component.Window;

import java.awt.event.KeyListener;

/**
 * User: gabor
 * Date: Feb 9, 2010
 * Time: 4:33:24 PM
 */
public class PlayerController extends InputHandler implements KeyInputListener, MouseInputListener {
    public final static float PLAYER_ROTATE_STEP = 150.0f;
    public final static float PLAYER_SPEED = 50.0f;
    public static boolean fixed_camera = true;
    private Main main;
    private MouseAction mouseAction;
    private int startX = -1, startY = -1;
    private boolean enabled;

    public PlayerController(final Main main) {
        this.main = main;
        RelativeMouse mouse = new RelativeMouse("Mouse Input");
        mouse.registerWithInputHandler(this);
        mouseAction = new MouseAction(main, mouse);

        KeyInput.get().addListener(this);
        MouseInput.get().addListener(this);

        setEnabled(true);
    }

    public void setEnabled(boolean b) {
        enabled = b;
        KeyBindingManager keyboard = KeyBindingManager.getKeyBindingManager();
        if(b) {
            addAction(new ForwardMoveAction(main), "scourge_forward", true );
            addAction(new LeftTurnAction(main), "scourge_left", true);
            addAction(new RightTurnAction(main), "scourge_right", true);
            addAction(mouseAction);
            keyboard.add( "scourge_left", KeyInput.KEY_A );
            keyboard.add( "scourge_right", KeyInput.KEY_D );
            keyboard.add( "scourge_forward", KeyInput.KEY_W );
        } else {
            keyboard.remove("scourge_left");
            keyboard.remove("scourge_right");
            keyboard.remove("scourge_forward");
            removeAction(mouseAction);
            removeAction("scourge_forward");
            removeAction("scourge_left");
            removeAction("scourge_right");
        }
        MouseInput.get().setCursorVisible(true);
    }

    @Override
    public void onKey( char character, int keyCode, boolean pressed ) {
        if(Window.getWindow() != null && Window.getWindow().onKey(character, keyCode, pressed)) return;

        KeyInput keyInput = KeyInput.get();
        if(keyCode == KeyInput.KEY_F5 && !pressed) {
            main.toggleCameraAttached();
        }
        main.getPlayer().getCreatureModel().setKeyFrame(keyInput.isKeyDown(KeyInput.KEY_W) ? Md2Model.Md2Key.run : Md2Model.Md2Key.stand);
    }

    @Override
    public void onButton(int button, boolean pressed, int x, int y) {
        if(Window.getWindow() != null && Window.getWindow().onButton(button, pressed, x, y)) return;

        if(enabled) {
            if(button == 0) {
                if(pressed) {
                    if(!main.drag() && !(Window.getWindow() != null && Window.getWindow().getRectangle().contains(x, y))) {
                        startX = x;
                        startY = y;
                    }
                } else {
                    startX = startY = -1;
                    if(mouseAction.isEnabled()) {
                        mouseAction.setEnabled(false);
                    } else {
                        main.mouseReleased();
                    }
                }
            }
        }
    }

    @Override
    public void onWheel(int wheelDelta, int x, int y) {
        if(Window.getWindow() != null && Window.getWindow().onWheel(wheelDelta, x, y)) return;
    }

    @Override
    public void onMove(int xDelta, int yDelta, int newX, int newY) {
        if(Window.getWindow() != null && Window.getWindow().onMove(xDelta, yDelta, newX, newY)) return;

        if(enabled) {
            if(!mouseAction.isEnabled() && startX >= 0 && startY >= 0) {
                if(Math.abs(startX - newX) > 5 || Math.abs(startY - newY) > 5) {
                    mouseAction.setEnabled(true);
                }
            }
        }
    }
}
