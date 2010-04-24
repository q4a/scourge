package org.scourge.ui.component;

import com.jme.renderer.ColorRGBA;
import com.jme.scene.shape.Quad;
import com.jmex.font2d.Text2D;

/**
 * User: gabor
 * Date: Apr 18, 2010
 * Time: 3:44:23 PM
 */
public class Textfield extends org.scourge.ui.component.Component {
    private Quad quad;
    private static final String BACKGROUND = "./data/textures/ui/text.png";
    private boolean focus;
    private static final ColorRGBA FOCUS_COLOR = new ColorRGBA(1, 0.7f, 0.5f, 1);
    private static final ColorRGBA UNFOCUS_COLOR = new ColorRGBA(1, 1, 1, 1);
    private StringBuffer sb = new StringBuffer();
    private Text2D label;

    private static final int TEXT_FLAGS = 0;
    private static final ColorRGBA TEXT_COLOR = new ColorRGBA(0.4f, 0.2f, 0.15f, 1);
    private static final float TEXT_SCALE = 1;
    private int maxSize;


    public Textfield(org.scourge.ui.component.Window window, String name, int x, int y, String text, int size) {
        super(window, name, x, y, (size + 1) * org.scourge.ui.component.Window.FONT_WIDTH, org.scourge.ui.component.Window.FONT_HEIGHT * 2);
        this.maxSize = size;

        quad = WinUtil.createQuad("button_quad", getW(), getH(), BACKGROUND);
        getNode().attachChild(quad);

        sb = new StringBuffer(text);
        attachLabel();

        setFocus(false);
    }

    public boolean isFocus() {
        return focus;
    }

    public void setFocus(boolean b) {
        focus = b;
        getWindow().unpack();
        quad.setSolidColor(focus ? FOCUS_COLOR : UNFOCUS_COLOR);
        getWindow().pack();
    }

    public void deleteLast() {
        if(sb.length() > 0) {
            sb.deleteCharAt(sb.length() - 1);
            updateText();
        }
    }

    public void addChar(char c) {
        if(sb.length() < maxSize - 1) {
            sb.append(c);
            updateText();
        }
    }

    @Override
    public void setText(String value) {
        sb = new StringBuffer(value);
        updateText();
    }

    @Override
    public String getText() {
        return sb.toString();
    }

    private void updateText() {
        getWindow().unpack();
        if(label != null) {
            getNode().detachChild(label);
        }
        attachLabel();
        getWindow().pack();
    }

    private void attachLabel() {
        String text = sb.toString();
        label = WinUtil.createText((maxSize - text.length()) * -org.scourge.ui.component.Window.FONT_WIDTH / 2, 0, text,
                                    org.scourge.ui.component.Window.FONT_HEIGHT, TEXT_FLAGS, TEXT_COLOR, TEXT_SCALE);
        getNode().attachChild(label);
    }
}
