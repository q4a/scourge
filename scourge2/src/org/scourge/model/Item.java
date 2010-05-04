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

    private ItemTemplate template;

    // explicit default constructor for simple.xml
    public Item() {
    }

    public Item(String name) {
        setName(name);
        this.charges = template.getMaxCharges();
    }

    // no getter only used to resolve the template
    public void setName(String name) {
        this.name = name;
        this.template = Items.getInstance().getItem(name);
    }

    public ItemTemplate getTemplate() {
        return template;
    }

    public int getCharges() {
        return charges;
    }
}
