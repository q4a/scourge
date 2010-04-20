package org.scourge.ui;

import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.renderer.ColorRGBA;
import com.jme.scene.Node;
import com.jme.scene.shape.Quad;
import com.jmex.font2d.Text2D;
import org.scourge.terrain.ShapeUtil;

import java.awt.*;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 10:14:46 PM
 */
class Button extends Component {
    private Text2D label;
    private Quad quad;
    private static final String BUTTON_BACKGROUND = "./data/textures/ui/button.png";
    private static final float BUTTON_TEXT_SIZE = Window.FONT_HEIGHT;
    private static final int BUTTON_TEXT_FLAGS = 0;
    private static final ColorRGBA BUTTON_TEXT_COLOR = new ColorRGBA(1, 0.8f, 0.75f, 1);
    private static final float BUTTON_TEXT_SCALE = 0.8f;
    private static final Quaternion CLICK_TEXTURE_ROTATE = new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 180, Vector3f.UNIT_Y);
    private static final Vector3f BUTTON_PRESS_TRANS = new Vector3f(2, -2, 0);

    public Button(Window window, String name, int x, int y, int w, int h, String text) {
        super(window, name, x, y, w, h);

        quad = WinUtil.createQuad("button_quad", w, h, BUTTON_BACKGROUND);
        getNode().attachChild(quad);

        label = WinUtil.createLabel(0, 0, text, BUTTON_TEXT_SIZE, BUTTON_TEXT_FLAGS, BUTTON_TEXT_COLOR, BUTTON_TEXT_SCALE);
        getNode().attachChild(label);
    }

    public Text2D getLabel() {
        return label;
    }

    public Quad getQuad() {
        return quad;
    }

    public void pressButton() {
        quad.getLocalRotation().multLocal(CLICK_TEXTURE_ROTATE);
        label.getLocalTranslation().addLocal(BUTTON_PRESS_TRANS);
    }

    public void releaseButton() {
        quad.getLocalRotation().multLocal(CLICK_TEXTURE_ROTATE);
        label.getLocalTranslation().subtractLocal(BUTTON_PRESS_TRANS);
    }

    @Override
    public void setText(String text) {
        getWindow().unpack();
        if(label != null) {
            getNode().detachChild(label);
        }
        label = WinUtil.createLabel(0, 0, text, BUTTON_TEXT_SIZE, BUTTON_TEXT_FLAGS, BUTTON_TEXT_COLOR, BUTTON_TEXT_SCALE);
        getNode().attachChild(label);
        getWindow().pack();
    }

    @Override
    public String getText() {
        return label.getText().toString();
    }
}
