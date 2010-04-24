package org.scourge.ui.component;

import com.jme.scene.shape.Quad;
import com.jme.scene.state.RenderState;
import com.jme.scene.state.TextureState;

/**
 * User: gabor
 * Date: Apr 19, 2010
 * Time: 9:03:04 PM
 */
public class ImageComponent extends Component {
    private Quad quad;

    public ImageComponent(Window window, String name, String imagePath, int x, int y, int w, int h) {
        super(window, name, x, y, w, h);
        quad = WinUtil.createQuad("image_quad", w, h, imagePath);
        getNode().attachChild(quad);
    }

    @Override
    public String getImage() {
        return ((TextureState)quad.getRenderState(RenderState.StateType.Texture)).getTexture().getImageLocation();
    }

    @Override
    public void setImage(String imagePath) {
        getWindow().unpack();
        getNode().detachChild(quad);
        quad = WinUtil.createQuad("image_quad", getW(), getH(), imagePath);
        getNode().attachChild(quad);
        getWindow().pack();
    }
}
