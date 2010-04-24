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
    private GText label;

    public Label(Window window, String name, int x, int y, String text, ColorRGBA color, float scale, WinUtil.ScourgeFont scourgeFont) {
        super(window, name, x, y, text.length() * Window.FONT_WIDTH, Window.FONT_HEIGHT);
        label = WinUtil.createLabel(0, 0, text, color, scale, scourgeFont);
        getNode().attachChild(label);
    }

    @Override
    public void setText(String text) {
        getWindow().unpack();
        label.setText(text);
        getWindow().pack();
    }

    @Override
    public String getText() {
        return label.getText();
    }    
}
