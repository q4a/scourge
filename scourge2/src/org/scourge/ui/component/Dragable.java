package org.scourge.ui.component;

import com.jme.image.Texture;
import com.jme.scene.Spatial;

/**
 * User: gabor
 * Date: May 9, 2010
 * Time: 3:07:39 PM
 */
public interface Dragable {
    public int getIconWidth();
    public int getIconHeight();
    public Texture getIconTexture();
    public Spatial getModel();
    public void scaleModel();
}
