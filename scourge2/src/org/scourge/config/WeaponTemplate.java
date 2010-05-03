package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 9:09:53 AM
 */
@Root(name = "weapon")
public class WeaponTemplate {

    @Element
    private int ap;

    @Element
    private int damage;

    @Element(name="damage_type", required=false)
    private String damageType;

    @Element(required=false)
    private String skill;

    @Element(required=false)
    private int parry;

    @Element(required=false, name="two_handed")
    private boolean twoHanded;

    @Element(required=false)
    private int range;
}
