package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 1, 2010
 * Time: 9:16:51 AM
 */
@Root(name = "type")
public class ItemType {
    @Element
    private String name;

    @Element(name="isWeapon", required = false)
    private boolean weapon;

    @Element(name="isRandom", required = false)
    private boolean random;

    @Element(name="isEnchantable", required = false)
    private boolean enchantable;

    @Element(name="isArmor", required = false)
    private boolean armor;

    @Element(name="isRanged", required = false)
    private boolean ranged;

    @Element(required = false)
    private boolean hasSpell;

    @Element(name="defaultDimension")
    private String defaultDimension;

    public String getName() {
        return name;
    }

    public boolean isWeapon() {
        return weapon;
    }

    public boolean isRandom() {
        return random;
    }

    public boolean isEnchantable() {
        return enchantable;
    }

    public boolean isArmor() {
        return armor;
    }

    public boolean isRanged() {
        return ranged;
    }

    public boolean isHasSpell() {
        return hasSpell;
    }

    public String getDefaultDimension() {
        return defaultDimension;
    }

    public String toString() {
        return name + " weapon=" + isWeapon() + " armor=" + isArmor() + " ranged=" + isRanged() + " random=" + isRandom() + " spell=" + isHasSpell() + " enchantable=" + isEnchantable() + " dim=" + getDefaultDimension();
    }
}
