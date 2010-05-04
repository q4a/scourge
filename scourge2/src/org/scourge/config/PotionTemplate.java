package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 9:43:20 AM
 */
@Root(name="potion")
public class PotionTemplate {
    @Element
    private int power;

    @Element(required = false)
    private int time;

    @Element
    private String skill;

    public int getPower() {
        return power;
    }

    public int getTime() {
        return time;
    }

    public String getSkill() {
        return skill;
    }
}
