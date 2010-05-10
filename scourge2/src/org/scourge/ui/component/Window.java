package org.scourge.ui.component;

import com.jme.input.KeyInput;
import com.jme.input.KeyInputListener;
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
import org.scourge.Main;
import org.scourge.terrain.NodeGenerator;
import org.scourge.terrain.ShapeUtil;

import java.awt.*;
import java.awt.geom.Point2D;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 1:51:00 PM
 */
public class Window implements NodeGenerator, MouseInputListener, KeyInputListener {
    private Node win;
    private static final String WINDOW_BACKGROUND = "./data/textures/ui/win.png";    
    public static final ColorRGBA TEXT_COLOR = new ColorRGBA(0.3f, 0.15f, 0.1f, 1);

    public static final int FONT_WIDTH = 10;
    public static final int FONT_HEIGHT = 8;

    private Map<String, org.scourge.ui.component.Component> components = new HashMap<String, org.scourge.ui.component.Component>();
    private WindowListener listener;
    private int x, y, w, h;
    private org.scourge.ui.component.Button currentButton;

    private static Stack<Window> windows = new Stack<Window>();
    private static final String MESSAGE_OK = "internal_ok";
    private static Runnable onOk;
    private static final String MESSAGE_CONFIRM_OK = "internal_confirm_ok";
    private static final String MESSAGE_CONFIRM_CANCEL = "internal_confirm_cancel";
    private Textfield currentTextField;
    private Rectangle rectangle;
    private boolean alwaysOpen;
    private boolean visible;

    public Window(int x, int y, int w, int h) {
        this(x, y, w, h, null);
    }

    public Window(int x, int y, int w, int h, WindowListener listener) {
        this.listener = listener;
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;
        win = new Node(ShapeUtil.newShapeName("window"));
        rectangle = new Rectangle(x - w / 2, y - h / 2, w, h);

        Quad q = WinUtil.createQuad("window_quad", w, h, WINDOW_BACKGROUND);
        win.attachChild(q);
        win.getLocalTranslation().addLocal(new Vector3f(x, y, 0));

        WinUtil.makeNodeOrtho(win);
    }

    public void pack() {
        win.updateRenderState();
        win.lockBounds();
        win.lockMeshes();
    }

    public void unpack() {
        win.unlockBounds();
        win.unlockMeshes();
    }

    @Override
    public Node getNode() {
        return win;
    }

    public void addTextfield(String name, int x, int y, String text, int size) {
        Textfield textfield = new Textfield(this, name, x, y, text, size);
        setCurrentTextField(textfield);
        addComponent(textfield);
    }

    public void addImage(String name, String imagePath, int x, int y, int w, int h) {
        addComponent(new ImageComponent(this, name, imagePath, x, y, w, h));
    }

    public void addLabel(int x, int y, String text, WinUtil.ScourgeFont font) {
        addLabel(ShapeUtil.newShapeName("label"), x, y, text, TEXT_COLOR, true, font);
    }

    public void addLabel(int x, int y, String text) {
        addLabel(ShapeUtil.newShapeName("label"), x, y, text);
    }

    public void addLabel(String name, int x, int y, String text) {
        addLabel(name, x, y, text, true);
    }

    public void addLabel(String name, int x, int y, String text, boolean centered) {
        addLabel(name, x, y, text, TEXT_COLOR, centered, WinUtil.ScourgeFont.text);
    }

    public void addLabel(String name, int x, int y, String text, ColorRGBA color, boolean centered, WinUtil.ScourgeFont font) {
        addComponent(new Label(this, name, x, y, text, color, centered, font));
    }

    public void addButton(String name, int x, int y, String text) {
        addButton(name, x, y, 130, 28, text);
    }

    public void addButton(String name, int x, int y, int w, int h, String text) {
        addComponent(new org.scourge.ui.component.Button(this, name, x, y, w, h, text));
    }

    public void addProgress(String name, int x, int y, int w, int h) {
        addComponent(new Progress(this, name, x, y, w, h));
    }

    public void addComponent(org.scourge.ui.component.Component c) {
        components.put(c.getName(), c);
        win.attachChild(c.getNode());
    }

    public Rectangle getRectangle() {
        return rectangle;
    }

    @Override
    public void onButton(int mouseButton, boolean pressed, int x, int y) {
        Main main = Main.getMain();
        if(!pressed && main.getDragging() != null) {
            // drop over a window?
            if(getRectangle().contains(x, y)) {
                for(org.scourge.ui.component.Component c : components.values()) {
                    if(c.getRectangle().contains(x, y)) {
                        listener.drop(c.getName(), getComponentCoordinates(x, y, c), main.getDragging());
                        main.setDragging(null);
                    }
                }
            }

            if(main.getDragging() != null) {
                // is there any window to receive it?
                boolean found = false;
                for(Window window : windows) {
                    if(window.getRectangle().contains(x, y)) {
                        found = true;
                        break;
                    }
                }

                // drop over no window
                if(!found) {
                    main.drop(x, y, main.getDragging());
                    main.setDragging(null);
                }
            }
        }

        if(Window.getWindow() == this && getRectangle().contains(x, y)) {
            if(mouseButton == 0) {
                for(org.scourge.ui.component.Component c : components.values()) {
                    if(c.getRectangle().contains(x, y)) {
                        if(c instanceof Textfield) {
                            setCurrentTextField((Textfield)c);
                            break;
                        } else if(c instanceof org.scourge.ui.component.Button) {
                            org.scourge.ui.component.Button button = (org.scourge.ui.component.Button)c;
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
                                } else if(listener != null) {
                                    listener.buttonClicked(button.getName());
                                }
                            }
                        } else if(pressed) {
                            main.setDragging(listener.drag(c.getName(), getComponentCoordinates(x, y, c)));
                        } else {
                            listener.buttonClicked(c.getName());
                        }
                    }
                }
            }
        }
    }

    private Point2D getComponentCoordinates(int x, int y, Component c) {
        return new Point2D.Float(x - c.getRectangle().x,
                                 c.getRectangle().height - (y - c.getRectangle().y));
    }

    @Override
    public void onKey(char character, int keyCode, boolean pressed) {
        if(Window.getWindow() == this) {
            if(currentTextField != null) {
                if(keyCode == KeyInput.KEY_BACK) {
                    if(!pressed) currentTextField.deleteLast();
                } else if (character > 0) {
                    currentTextField.addChar(character);
                }
            }
        }
    }

    private void setCurrentTextField(Textfield textfield) {
        currentTextField = textfield;
        for(org.scourge.ui.component.Component c : components.values()) {
            if(c instanceof Textfield) {
                ((Textfield)c).setFocus(c == currentTextField);
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
        messageWindow.addLabel("internal_message", 0, 30, message);
        messageWindow.addButton(MESSAGE_OK, 0, -20, "OK");
        messageWindow.pack();
        messageWindow.setVisible(true);
    }

    public static void confirm(String message, Runnable onOk) {
        Window.onOk = onOk;
        Window messageWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                          (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() * 0.5f),
                                          Math.max(350, message.length() * FONT_WIDTH + 40), 120, null);
        messageWindow.addLabel("internal_confirm", 0, 30, message);
        messageWindow.addButton(MESSAGE_CONFIRM_OK, -70, -20, "OK");
        messageWindow.addButton(MESSAGE_CONFIRM_CANCEL, 70, -20, "Cancel");
        messageWindow.pack();
        messageWindow.setVisible(true);
    }

    public void setVisible(boolean visible) {
        // guard against adding the window twice to windows
        if(this.visible == visible) {
            return;
        }
        this.visible = visible;

        if(visible) {
            MouseInput.get().addListener(this);
            KeyInput.get().addListener(this);
            windows.push(this);
            Main.getMain().showWindow(this);
        } else {
            windows.pop();
            Main.getMain().hideWindow(this);
            MouseInput.get().removeListener(this);
            KeyInput.get().removeListener(this);
        }
    }

    public static Window getWindow() {
        return windows.isEmpty() ? null : windows.peek();
    }

    public void setText(String name, String value) {
        org.scourge.ui.component.Component c = components.get(name);
        if(c != null) {
            c.setText(value);
        }
    }

    public String getText(String name) {
        org.scourge.ui.component.Component c = components.get(name);
        return (c == null ? null : c.getText());
    }

    public void setValue(String name, float value) {
        org.scourge.ui.component.Component c = components.get(name);
        if(c != null) {
            c.setValue(value);
        }
    }

    public float getValue(String name) {
        org.scourge.ui.component.Component c = components.get(name);
        return (c == null ? 0f : c.getValue());
    }

    public String getImage(String name) {
        org.scourge.ui.component.Component c = components.get(name);
        return (c == null ? null : c.getImage());
    }

    public void setImage(String name, String imagePath) {
        org.scourge.ui.component.Component c = components.get(name);
        if(c != null) {
            c.setImage(imagePath);
        }
    }

    public WindowListener getListener() {
        return listener;
    }

    public void setListener(WindowListener listener) {
        this.listener = listener;
    }

    public void setAlwaysOpen(boolean b) {
        alwaysOpen = b;
    }

    public boolean isAlwaysOpen() {
        return alwaysOpen;
    }
}
