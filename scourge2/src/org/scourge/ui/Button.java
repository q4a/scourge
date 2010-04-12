package org.scourge.ui;

import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.renderer.ColorRGBA;
import com.jme.scene.Node;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.BlendState;
import com.jme.scene.state.RenderState;
import com.jme.scene.state.TextureState;
import com.jme.system.DisplaySystem;
import com.jmex.font2d.Text2D;
import org.scourge.terrain.ShapeUtil;

import java.awt.*;
import java.nio.FloatBuffer;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 10:14:46 PM
 */
class Button {
    private Node node;
    private Text2D label;
    private Quad quad;
    private Rectangle rectangle;
    private Window window;
    private String name;
    private int x, y, w, h;
    private static final String BUTTON_BACKGROUND = "./data/textures/ui/button.png";
    private static final float BUTTON_TEXT_SIZE = 8;
    private static final int BUTTON_TEXT_FLAGS = 0;
    private static final ColorRGBA BUTTON_TEXT_COLOR = new ColorRGBA(0.3f, 0.25f, 0.15f, 1);
    private static final float BUTTON_TEXT_SCALE = 0.8f;
    private static final Quaternion CLICK_TEXTURE_ROTATE = new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 180, Vector3f.UNIT_Y);
    private static final Vector3f BUTTON_PRESS_TRANS = new Vector3f(2, -2, 0);

    public Button(String name, int x, int y, int w, int h, String text, Window window) {
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;
        this.name = name;
        this.window = window;
        node = new Node(ShapeUtil.newShapeName("button"));
        node.getLocalTranslation().addLocal(new Vector3f(x, y, 0));

        quad = new Quad(ShapeUtil.newShapeName("button_quad"), w, h);

        FloatBuffer normBuf = quad.getNormalBuffer();
        normBuf.clear();
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);

        TextureState ts = DisplaySystem.getDisplaySystem().getRenderer().createTextureState();
        Texture texture = ShapeUtil.loadTexture(BUTTON_BACKGROUND);
        texture.setWrap(Texture.WrapMode.Repeat);
        texture.setHasBorder(false);
        texture.setApply(Texture.ApplyMode.Modulate);
        texture.setRotation(new Quaternion());
        ts.setTexture(texture, 0);
        quad.setRenderState(ts);

        BlendState as = DisplaySystem.getDisplaySystem().getRenderer().createBlendState();
        as.setBlendEnabled(true);
        as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
        as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
        as.setTestEnabled(true);
        as.setTestFunction(BlendState.TestFunction.GreaterThan);
        as.setEnabled(true);
        quad.setRenderState(as);
        node.attachChild(quad);

        label = window.createLabel(0, 0, text, BUTTON_TEXT_SIZE, BUTTON_TEXT_FLAGS, BUTTON_TEXT_COLOR, BUTTON_TEXT_SCALE);
        node.attachChild(label);

        rectangle = new Rectangle(window.getX() + x - w / 2, window.getY() + y - h / 2, w, h);
    }

    public Rectangle getRectangle() {
        return rectangle;
    }

    public Node getNode() {
        return node;
    }

    public Text2D getLabel() {
        return label;
    }

    public Quad getQuad() {
        return quad;
    }

    public Window getWindow() {
        return window;
    }

    public void pressButton() {
        quad.getLocalRotation().multLocal(CLICK_TEXTURE_ROTATE);
        label.getLocalTranslation().addLocal(BUTTON_PRESS_TRANS);
    }

    public void releaseButton() {
        quad.getLocalRotation().multLocal(CLICK_TEXTURE_ROTATE);
        label.getLocalTranslation().subtractLocal(BUTTON_PRESS_TRANS);
    }


    public String getName() {
        return name;
    }
}
