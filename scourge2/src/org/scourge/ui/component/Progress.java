package org.scourge.ui.component;

import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.renderer.ColorRGBA;
import com.jme.scene.shape.Quad;
import org.scourge.terrain.ShapeUtil;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 10:14:46 PM
 */
class Progress extends Component {
    private Quad quad;
    private static final String BACKGROUND = "./data/textures/ui/progress.png";
    private float value;

    public Progress(Window window, String name, int x, int y, int w, int h) {
        super(window, name, x, y, w, h);

        quad = WinUtil.createQuad("progress_quad", w, h, BACKGROUND);
        getNode().attachChild(quad);

        this.value = 0;
    }

    @Override
    public void setValue(float value) {
        this.value = value;
        getWindow().unpack();
        quad.getLocalScale().set((getW() / 100.0f) * value, 1, 1);
        getWindow().pack();
    }

    @Override
    public float getValue() {
        return value;
    }
}
