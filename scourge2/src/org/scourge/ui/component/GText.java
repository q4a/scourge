package org.scourge.ui.component;

import java.util.LinkedList;

import com.jme.renderer.ColorRGBA;
import com.jme.scene.Node;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.BlendState;
import com.jme.system.DisplaySystem;

/**
 * Original by:
 * @author Victor Porof, blue_veek@yahoo.com
 */
public class GText extends Node {

    /**
     *
     */
    private static final long serialVersionUID = 1L;
    private GFont gFont;
    private String text = "";
    private ColorRGBA fill, foreground;
    private LinkedList<Quad> charQuads = new LinkedList<Quad>();
    private float size;
    private float kerning;
    private float width;
    private float height;

    public GText(GFont gFont, float kerneling, ColorRGBA foreground) {
        this.gFont = gFont;
        this.size = gFont.getSize();
        this.kerning = kerneling;
        this.foreground = foreground;

        BlendState bs = DisplaySystem.getDisplaySystem().getRenderer().createBlendState();
        bs.setBlendEnabled(true);
        bs.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
        bs.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
        bs.setEnabled(true);
        bs.setTestEnabled(true);
        bs.setTestFunction(BlendState.TestFunction.GreaterThan);
        bs.setReference(0);
        setRenderState(bs);

        setText(text);
    }

    private void construct() {
        float spacing = 0;

        if(charQuads.size() > 0) {
            for(Quad q : charQuads) {
                detachChild(q);
            }
            charQuads.clear();
        }

         for (int i = 0; i < text.length(); i++) {
             Quad quad = new Quad(String.valueOf(text.charAt(i)), size, size);
             float positionX = spacing + gFont.getMetricsWidths()[text.charAt(i)] / 2f;
             float positionY = gFont.getTextDescent();
             quad.getLocalTranslation().set(positionX, positionY, 0);
             quad.setDefaultColor(foreground);
             quad.setRenderState(gFont.getChar(text.charAt(i)));
             attachChild(quad);
             
             spacing += gFont.getMetricsWidths()[text.charAt(i)] + kerning;
             charQuads.add(quad);
         }

         this.width = spacing;
         this.height = size;
         updateRenderState();
        lockBounds();
        lockMeshes();
     }

    public void setText(Object text) {
        this.text = String.valueOf(text);
        construct();
    }

    public String getText() {
        return text;
    }

    public void setFill(ColorRGBA fill) {
        this.fill = fill;
        construct();
    }

    public ColorRGBA getFill() {
        return fill;
    }

    public ColorRGBA getForeground() {
        return foreground;
    }

    public void setForeground(ColorRGBA foreground) {
        this.foreground = foreground;
        construct();
    }

    public float getWidth() {
        return width;
    }

    public float getHeight() {
        return height;
    }
}