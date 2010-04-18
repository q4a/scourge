package org.scourge.ui;

import com.jme.input.MouseInput;
import com.jme.input.MouseInputListener;
import com.jme.math.Vector3f;
import com.jme.renderer.ColorRGBA;
import com.jme.renderer.Renderer;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.CullState;
import com.jme.scene.state.FogState;
import com.jme.scene.state.ZBufferState;
import com.jme.system.DisplaySystem;
import com.jmex.font2d.Text2D;
import org.scourge.Main;
import org.scourge.terrain.NodeGenerator;
import org.scourge.terrain.ShapeUtil;

import java.awt.*;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 1:51:00 PM
 */
public class Window implements NodeGenerator, MouseInputListener {
    private Node win;
    private static final String WINDOW_BACKGROUND = "./data/textures/ui/win.png";    
    public static final ColorRGBA TEXT_COLOR = new ColorRGBA(0, 0, 0, 1);


    private Map<Rectangle, Button> buttons = new HashMap<Rectangle, Button>();
    private WindowListener listener;
    private int x, y, w, h;
    private Button currentButton;
    public static final float LABEL_FONT_SIZE = 8;

    private static Stack<Window> windows = new Stack<Window>();
    private static final String MESSAGE_OK = "internal_ok";
    private static Runnable onOk;
    private static final String MESSAGE_CONFIRM_OK = "internal_confirm_ok";
    private static final String MESSAGE_CONFIRM_CANCEL = "internal_confirm_cancel";

    public Window(int x, int y, int w, int h, WindowListener listener) {
        this.listener = listener;
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;
        win = new Node(ShapeUtil.newShapeName("window"));

        Quad q = WinUtil.createQuad("window_quad", w, h, WINDOW_BACKGROUND);
        win.attachChild(q);
        win.getLocalTranslation().addLocal(new Vector3f(x, y, 0));

        makeNodeOrtho(win);

        MouseInput.get().addListener(this);
    }

    private void makeNodeOrtho(Node node) {
        ZBufferState zbs = DisplaySystem.getDisplaySystem().getRenderer().createZBufferState();
		zbs.setFunction(ZBufferState.TestFunction.Always);
		node.setRenderQueueMode(Renderer.QUEUE_ORTHO);
		node.setRenderState(zbs);
        node.setCullHint(Spatial.CullHint.Never);
		node.updateRenderState();

        CullState cullState = DisplaySystem.getDisplaySystem().getRenderer().createCullState();
        cullState.setCullFace(CullState.Face.None);
        cullState.setEnabled(true);
        node.setRenderState(cullState);

        FogState fs = DisplaySystem.getDisplaySystem().getRenderer().createFogState();
        fs.setEnabled(false);
        node.setRenderState(fs);

        node.setLightCombineMode(Spatial.LightCombineMode.Off);
        node.setTextureCombineMode(Spatial.TextureCombineMode.Replace);
    }

    public void pack() {
        win.updateRenderState();
        win.lockBounds();
        win.lockMeshes();
    }

    @Override
    public Node getNode() {
        return win;
    }

    public void addLabel(int x, int y, String text) {
        addLabel(x, y, text, LABEL_FONT_SIZE, 0, TEXT_COLOR, 1);
    }

    public void addLabel(int x, int y, String text, float size, int flags, ColorRGBA color, float scale) {
        Text2D label = WinUtil.createLabel(x, y, text, size, flags, color, scale);
		win.attachChild(label);
    }

    public void addButton(String name, int x, int y, String text) {
        addButton(name, x, y, 130, 28, text);
    }

    public void addButton(String name, int x, int y, int w, int h, String text) {
        Button button = new Button(name, x, y, w, h, text, this);
        buttons.put(button.getRectangle(), button);
        win.attachChild(button.getNode());
    }

    @Override
    public void onButton(int mouseButton, boolean pressed, int x, int y) {
        if(Window.getWindow() == this) {
            if(mouseButton == 0) {
                for(Rectangle r : buttons.keySet()) {
                    if(r.contains(x, y)) {
                        Button button = buttons.get(r);
                        if(pressed) {
                            currentButton = button;
                            button.pressButton();
                        } else {
                            currentButton = null;
                            button.releaseButton();
                            if(MESSAGE_OK.equals(button.getName())) {
                                getWindow().setVisible(false);
                            } else if(MESSAGE_CONFIRM_OK.equals(button.getName())) {
                                getWindow().setVisible(false);
                                if(onOk != null) {
                                    onOk.run();
                                    onOk = null;
                                }
                            } else if(MESSAGE_CONFIRM_CANCEL.equals(button.getName())) {
                                getWindow().setVisible(false);
                            } else {
                                listener.buttonClicked(button.getName());
                            }
                        }
                    }
                }
            }
        }
    }

    @Override
    public void onWheel(int wheelDelta, int x, int y) {
    }

    @Override
    public void onMove(int xDelta, int yDelta, int newX, int newY) {
        if(currentButton != null && !currentButton.getRectangle().contains(newX, newY)) {
            currentButton.releaseButton();
            currentButton = null;
        }
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public int getW() {
        return w;
    }

    public int getH() {
        return h;
    }

    public static void showMessage(String message) {
        Window messageWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                          (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() * 0.5f),
                                          Math.max(300, message.length() * 10 + 40), 120, null);
        messageWindow.addLabel(0, 30, message);
        messageWindow.addButton(MESSAGE_OK, 0, -20, "OK");
        messageWindow.pack();
        messageWindow.setVisible(true);
    }

    public static void confirm(String message, Runnable onOk) {
        Window.onOk = onOk;
        Window messageWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                          (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() * 0.5f),
                                          Math.max(350, message.length() * 10 + 40), 120, null);
        messageWindow.addLabel(0, 30, message);
        messageWindow.addButton(MESSAGE_CONFIRM_OK, -70, -20, "OK");
        messageWindow.addButton(MESSAGE_CONFIRM_CANCEL, 70, -20, "Cancel");
        messageWindow.pack();
        messageWindow.setVisible(true);
    }

    public void setVisible(boolean visible) {
        if(visible) {
            windows.push(this);
            Main.getMain().showWindow(this);
        } else {
            Main.getMain().hideWindow(this);
            windows.pop();
        }
        // drive with the mouse if no window is open
        Main.getMain().getPlayerController().toggleMouseDrive(getWindow() == null);
    }

    public static Window getWindow() {
        return windows.isEmpty() ? null : windows.peek();
    }
}
