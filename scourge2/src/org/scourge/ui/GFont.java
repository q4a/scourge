package org.scourge.ui;

import java.awt.*;
import java.awt.image.BufferedImage;
import java.util.LinkedList;

import com.jme.image.Texture;
import com.jme.scene.state.TextureState;
import com.jme.system.DisplaySystem;
import com.jme.util.TextureManager;

/**
 * Original by:
 * @author Victor Porof, blue_veek@yahoo.com
 */
public class GFont {

    private LinkedList<TextureState> charList;
    private static final int CHARS_COUNT = 256;
    private int[] tWidths;
    private int tDescent;
    private int size;

    public GFont(Font fontProto, float glyphSize, int repeat, boolean shadow) {
        Font font = fontProto.deriveFont(glyphSize);
        BufferedImage tempImage = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = (Graphics2D) tempImage.getGraphics();
        g.setFont(font);
        FontMetrics fm = g.getFontMetrics();

        // in order to get a crisp font, use a power of 2 and GL_LINEAR in texture
        size = fm.getMaxAscent() + fm.getMaxDescent() + fm.getLeading();
        size = nextPowerOfTwo(size);

        int tLeading = g.getFontMetrics().getLeading();
        int tAscent = g.getFontMetrics().getMaxAscent();
        this.tDescent = g.getFontMetrics().getMaxDescent();

        this.charList = new LinkedList<TextureState>();
        this.tWidths = new int[CHARS_COUNT];

        for (int i = 0; i < CHARS_COUNT; i++) {
            int fontWidth = g.getFontMetrics().charWidth((char) i);
            if (fontWidth < 1) fontWidth = 1;
            tWidths[i] = fontWidth + repeat;

            float posX = size / 2f - fontWidth / 2f;
            float posY = tLeading + tAscent;
            BufferedImage bImage = new BufferedImage(size, size, BufferedImage.TYPE_INT_ARGB);
            Graphics2D gt = (Graphics2D) bImage.getGraphics();
            gt.setFont(font);

            if(shadow) {
                gt.setColor(Color.black);
                for(int ss = 0; ss < repeat + 1; ss++) {
                    gt.drawString(String.valueOf((char) i), posX + ss, posY + 1);
                }
            }
            gt.setColor(Color.white);
            if(repeat > 0) {
                for(int ss = 0; ss < repeat; ss++) {
                    gt.drawString(String.valueOf((char) i), posX + ss, posY);
                }
            }
            gt.drawString(String.valueOf((char) i), posX, posY);

            TextureState cTextureState = DisplaySystem.getDisplaySystem().getRenderer().createTextureState();
            Texture cTexure = TextureManager.loadTexture(bImage,
                                                         Texture.MinificationFilter.BilinearNoMipMaps,
                                                         Texture.MagnificationFilter.Bilinear,
                                                         true);
            cTextureState.setTexture(cTexure);

            charList.add(cTextureState);
            gt.dispose();
        }

        g.dispose();
    }

    int[] powers = { 0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 };
    private int nextPowerOfTwo(int n) {
        for(int p : powers) {
            if(p > n) return p;
        }
        throw new RuntimeException("Can't find next power of 2 for " + n);
    }

    public TextureState getChar(int charCode) {
        return charList.get(charCode);
    }

    public float getTextDescent() {
        return tDescent;
    }

    public int[] getMetricsWidths() {
        return tWidths;
    }

    public int getSize() {
        return size;
    }
}