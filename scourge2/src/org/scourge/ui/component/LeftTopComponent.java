package org.scourge.ui.component;

import java.awt.*;

/**
 * User: gabor
 * Date: May 9, 2010
 * Time: 3:30:59 PM
 */
public class LeftTopComponent extends Component {

    public LeftTopComponent(Window window, String name, int x, int y, int w, int h) {
        super(window, name, x, y, w, h);
    }

    @Override
    public Rectangle getRectangle() {
        return new Rectangle(getWindow().getX() + getX(), getWindow().getY() + getY() - getH(), getW(), getH());
    }
}
