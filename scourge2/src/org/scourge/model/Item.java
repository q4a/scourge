package org.scourge.model;

import org.scourge.config.ItemTemplate;
import org.scourge.config.Items;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 8:24:55 PM
 */
@Root(name="item")
public class Item {
    @Element
    private String name;

    @Element
    private int charges;

    @Element(required = false)
    private int[] containerPosition;

    private ItemTemplate template;

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
}
