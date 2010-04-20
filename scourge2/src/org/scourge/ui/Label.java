package org.scourge.ui;

import com.jme.renderer.ColorRGBA;
import com.jmex.font2d.Text2D;

/**
 * User: gabor
 * Date: Apr 19, 2010
 * Time: 11:58:57 AM
 */
public class Label extends Component {
    private Text2D label;
    private float size;
    private int flags;
    private ColorRGBA color;
    private float scale;

    public Label(Window window, String name, int x, int y, String text, float size, int flags, ColorRGBA color, float scale) {
        super(window, name, x, y, text.length() * Window.FONT_WIDTH, Window.FONT_HEIGHT);
        this.size = size;
        this.flags = flags;
        this.color = color;
        this.scale = scale;
        label = WinUtil.createLabel(0, 0, text, size, flags, color, scale);
        getNode().attachChild(label);
    }

    @Override
    public void setText(String text) {
        getWindow().unpack();
        if(label != null) {
            getNode().detachChild(label);
        }
        label = WinUtil.createLabel(0, 0, text, size, flags, color, scale);
        getNode().attachChild(label);
        getWindow().pack();
    }

    @Override
    public String getText() {
        return label.getText().toString();
    }    
}
