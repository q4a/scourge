package org.scourge.ui;

import com.jme.renderer.ColorRGBA;
import com.jme.scene.Node;
import com.jmex.font2d.Text2D;

/**
 * User: gabor
 * Date: Apr 19, 2010
 * Time: 11:58:57 AM
 */
public class Label extends Component {
    private Node label;
    private ColorRGBA color;
    private float scale;
    private String text;

    public Label(Window window, String name, int x, int y, String text, ColorRGBA color, float scale, WinUtil.ScourgeFont scourgeFont) {
        super(window, name, x, y, text.length() * Window.FONT_WIDTH, Window.FONT_HEIGHT);
        this.color = color;
        this.scale = scale;
        label = WinUtil.createLabel(0, 0, text, color, scale, scourgeFont);
        this.text = text;
        getNode().attachChild(label);
    }

    @Override
    public void setText(String text) {
        getWindow().unpack();
        if(label != null) {
            getNode().detachChild(label);
        }
        label = WinUtil.createLabel(0, 0, text, color, scale, WinUtil.ScourgeFont.text);
        this.text = text;
        getNode().attachChild(label);
        getWindow().pack();
    }

    @Override
    public String getText() {
        return text;
    }    
}
