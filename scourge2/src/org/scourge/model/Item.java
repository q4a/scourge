package org.scourge.model;

import com.jme.image.Texture;
import com.jme.util.TextureManager;
import org.scourge.config.ItemTemplate;
import org.scourge.config.Items;
import org.scourge.terrain.ShapeUtil;
import org.scourge.ui.component.Dragable;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

import javax.swing.*;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 8:24:55 PM
 */
@Root(name="item")
public class Item implements Dragable {
    @Element
    private String name;

    @Element
    private int charges;

    @Element(required = false)
    private int[] containerPosition;

    private ItemTemplate template;
    private Logger logger = Logger.getLogger(Item.class.toString());
    private ImageIcon icon;

    // explicit default constructor for simple.xml
    public Item() {
    }

    // constructor to create a new item in game
    public Item(String name) {
        this.name = name;
        afterLoad();
    }

    public void afterLoad() {
        this.template = Items.getInstance().getItem(name);
        if(template == null) {
            throw new IllegalArgumentException("Can't find item template " + name);
        }
        this.charges = template.getMaxCharges();
    }

    public ItemTemplate getTemplate() {
        return template;
    }

    public int getCharges() {
        return charges;
    }

    public int[] getContainerPosition() {
        return containerPosition;
    }

    public void setContainerPosition(int[] containerPosition) {
        this.containerPosition = containerPosition;
    }

    public ImageIcon getIcon() {
        if(icon == null) {
            icon = ShapeUtil.loadImageIcon(getTemplate().getModel().getIcon());
            if(icon == null) {
                logger.severe("Can't load icon for " + getTemplate().getName());
            }
        }
        return icon;
    }

    @Override
    public Texture getIconTexture() {
        ImageIcon icon = getIcon();
        Texture texture = ShapeUtil.getTexture(getTemplate().getIcon());
        if(texture == null) {
            texture = TextureManager.loadTexture(icon.getImage(),
                                                 Texture.MinificationFilter.NearestNeighborNearestMipMap,
                                                 Texture.MagnificationFilter.Bilinear,
                                                 true);
            texture.setWrap(Texture.WrapMode.Repeat);
            texture.setHasBorder(false);
            texture.setApply(Texture.ApplyMode.Modulate);
            ShapeUtil.storeTexture(getTemplate().getIcon(), texture);
        }
        return texture;
    }

    @Override
    public int getIconWidth() {
        return getIcon().getIconWidth();
    }

    @Override
    public int getIconHeight() {
        return getIcon().getIconHeight();
    }
}
