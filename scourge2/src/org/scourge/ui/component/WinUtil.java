package org.scourge.ui.component;

import com.jme.image.Texture;
import com.jme.math.Vector3f;
import com.jme.renderer.ColorRGBA;
import com.jme.renderer.Renderer;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.Text;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.*;
import com.jme.system.DisplaySystem;
import com.jmex.font2d.Font2D;
import com.jmex.font2d.Text2D;
import org.scourge.terrain.ShapeUtil;

import java.awt.*;
import java.io.File;
import java.nio.FloatBuffer;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 11:29:00 PM
 */
public class WinUtil {
    public static void makeNodeOrtho(Spatial node) {
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

    public enum ScourgeFont {
        regular(12, "data/fonts/DejaVuLGCSans.ttf", 2, true, 0),
        mono(12, "data/fonts/DejaVuLGCSansMono.ttf", 2, false, 0),
        text(12, "data/fonts/DejaVuLGCSans.ttf", 2, false, 0),
        large(32, "data/fonts/GentiumArchaic.ttf", 1, false, 0),
        rune(16, "data/fonts/ScourgeRunes.ttf", 1, false, 0),
        ;

        private float size;
        private String file;
        private GFont gfont;
        private float kerning;
        private int repeat;
        private boolean shadow;

        ScourgeFont(float size, String file, int repeat, boolean shadow, float kerning) {
            this.size = size;
            this.file = file;
            this.repeat = repeat;
            this.shadow = shadow;
            this.kerning = kerning;
        }

        public float getSize() {
            return size;
        }

        public String getFile() {
            return file;
        }

        public GFont getGFont() {
            if(gfont == null) {
                try {
                    Font font = Font.createFont(Font.TRUETYPE_FONT, new File(file));
                    gfont = new GFont(font, size, repeat, shadow);
                } catch(Exception exc) {
                    throw new RuntimeException(exc);
                }
            }
            return gfont;
        }

        public float getKerning() {
            return kerning;
        }
    }

    public static GText createLabel(int x, int y, String text, ColorRGBA color, ScourgeFont scourgeFont, boolean centered) {
        GText label = new GText(scourgeFont.getGFont(), scourgeFont.getKerning(), color);
        label.setText(text);
        label.getLocalTranslation().addLocal(new Vector3f(x - (centered ? label.getWidth() / 2 : 0), y, 0));
        return label;
    }

    public static Text2D createText(int x, int y, String text, float size, int flags, ColorRGBA color, float scale) {
        Font2D font = new Font2D();
        Text2D label = font.createText(text, size, flags);
        label.setLocalScale(scale);
		label.setTextColor(color);
        label.getLocalTranslation().addLocal(new Vector3f(x - label.getWidth() / 2, y - label.getHeight() / 2, 0));
        label.setRenderState(Text.getDefaultFontTextureState());
        label.setRenderState(Text.getFontBlend());
        return label;
    }

    public static Quad createQuad(String namePrefix, int w, int h) {
        return createQuad(namePrefix, w, h, (Texture)null);
    }


    public static Quad createQuad(String namePrefix, int w, int h, String texturePath) {
        Texture texture = null;
        if(texturePath != null) {
            texture = ShapeUtil.loadTexture(texturePath);
            texture.setWrap(Texture.WrapMode.Repeat);
            texture.setHasBorder(false);
            texture.setApply(Texture.ApplyMode.Modulate);
        }
        return createQuad(namePrefix, w, h, texture);
    }

    public static Quad createQuad(String namePrefix, int w, int h, Texture texture) {
        Quad q = new Quad(ShapeUtil.newShapeName(namePrefix), w, h);

        FloatBuffer normBuf = q.getNormalBuffer();
        normBuf.clear();
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);

        if(texture != null) {
            TextureState ts = DisplaySystem.getDisplaySystem().getRenderer().createTextureState();
            ts.setTexture(texture, 0);
            q.setRenderState(ts);
        }

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
