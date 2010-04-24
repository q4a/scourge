package org.scourge.ui.component;

import com.jme.renderer.ColorRGBA;

/**
 * User: gabor
 * Date: Apr 19, 2010
 * Time: 11:58:57 AM
 */
public class Label extends Component {
    private GText label;

    public Label(Window window, String name, int x, int y, String text, ColorRGBA color, boolean centered, WinUtil.ScourgeFont scourgeFont) {
        super(window, name, x, y, text.length() * Window.FONT_WIDTH, Window.FONT_HEIGHT);
        label = WinUtil.createLabel(0, 0, text, color, scourgeFont, centered);
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
