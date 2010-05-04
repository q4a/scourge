package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 9:12:29 AM
 */
@Root(name="skill_adjustment")
public class SkillAdjustment {
    @Element
    private String skill;

    @Element(required=false)
    private ApAdjustment ap;

    @Element(required=false)
    private CthAdjustment cth;

    @Element(required=false)
    private DamageAdjustment damage;

    @Element(required=false)
    private ArmorAdjustment armor;

    public String getSkill() {
        return skill;
    }

    public ApAdjustment getAp() {
        return ap;
    }

    public CthAdjustment getCth() {
        return cth;
    }

    public DamageAdjustment getDamage() {
        return damage;
    }

    public ArmorAdjustment getArmor() {
        return armor;
    }
}
