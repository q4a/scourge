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
    private Label label;
    private Quad quad;
    private static final String BUTTON_BACKGROUND = "./data/textures/ui/button.png";
    //private static final ColorRGBA BUTTON_TEXT_COLOR = new ColorRGBA(0.2f, 0.05f, 0, 1);
    private static final ColorRGBA BUTTON_TEXT_COLOR = new ColorRGBA(1, 0.90f, 0.75f, 1);
    private static final float BUTTON_TEXT_SCALE = 0.8f;
    private static final Quaternion CLICK_TEXTURE_ROTATE = new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 180, Vector3f.UNIT_Y);
    private static final Vector3f BUTTON_PRESS_TRANS = new Vector3f(2, -2, 0);

    public Button(Window window, String name, int x, int y, int w, int h, String text) {
        super(window, name, x, y, w, h);

        quad = WinUtil.createQuad("button_quad", w, h, BUTTON_BACKGROUND);
        getNode().attachChild(quad);

        label = new Label(window, ShapeUtil.newShapeName("button_label"), 0, 0, text,
                          BUTTON_TEXT_COLOR, BUTTON_TEXT_SCALE,
                          WinUtil.ScourgeFont.regular);
        getNode().attachChild(label.getNode());
    }

    public void pressButton() {
        quad.getLocalRotation().multLocal(CLICK_TEXTURE_ROTATE);
        label.getNode().getLocalTranslation().addLocal(BUTTON_PRESS_TRANS);
    }

    public void releaseButton() {
        quad.getLocalRotation().multLocal(CLICK_TEXTURE_ROTATE);
        label.getNode().getLocalTranslation().subtractLocal(BUTTON_PRESS_TRANS);
    }

    @Override
    public void setText(String text) {
        label.setText(text);
    }

    @Override
    public String getText() {
        return label.getText();
    }
}
