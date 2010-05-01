package org.scourge.config;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 1, 2010
 * Time: 2:59:24 PM
 */
@Root(name="tag")
public class ItemTag {
    @Element
    private String name;

    @Attribute
    private boolean translate;

    @Element
    private String value;

    public String getName() {
        return name;
    }

    public boolean isTranslate() {
        return translate;
    }

    public String getValue() {
        return value;
    }

    @Override
    public String toString() {
        return "name=" + name + " value=" + value + " translate=" + translate;
    }
}
