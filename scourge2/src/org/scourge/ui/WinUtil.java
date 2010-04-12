package org.scourge.ui;

import com.jme.image.Texture;
import com.jme.math.Vector3f;
import com.jme.renderer.ColorRGBA;
import com.jme.scene.Text;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.BlendState;
import com.jme.scene.state.TextureState;
import com.jme.system.DisplaySystem;
import com.jmex.font2d.Font2D;
import com.jmex.font2d.Text2D;
import org.scourge.terrain.ShapeUtil;

import java.nio.FloatBuffer;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 11:29:00 PM
 */
public class WinUtil {
    static Text2D createLabel(int x, int y, String text, float size, int flags, ColorRGBA color, float scale) {
        Font2D font = new Font2D();
        Text2D label = font.createText(text, size, flags);
        label.setLocalScale(scale);
		label.setTextColor(color);
        label.getLocalTranslation().addLocal(new Vector3f(x - label.getWidth() / 2, y - label.getHeight() / 2, 0));
        label.setRenderState(Text.getDefaultFontTextureState());
        label.setRenderState(Text.getFontBlend());
        return label;
    }

    static Quad createQuad(String namePrefix, int w, int h, String texturePath) {
        Quad q = new Quad(ShapeUtil.newShapeName(namePrefix), w, h);

        FloatBuffer normBuf = q.getNormalBuffer();
        normBuf.clear();
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);

        TextureState ts = DisplaySystem.getDisplaySystem().getRenderer().createTextureState();
        Texture texture = ShapeUtil.loadTexture(texturePath);
        texture.setWrap(Texture.WrapMode.Repeat);
        texture.setHasBorder(false);
        texture.setApply(Texture.ApplyMode.Modulate);
        ts.setTexture(texture, 0);
        q.setRenderState(ts);

        BlendState as = DisplaySystem.getDisplaySystem().getRenderer().createBlendState();
        as.setBlendEnabled(true);
        as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
        as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
        as.setTestEnabled(true);
        as.setTestFunction(BlendState.TestFunction.GreaterThan);
        as.setEnabled(true);
        q.setRenderState(as);

        return q;
    }    
}
