package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 9:20:04 AM
 */
@Root(name="armor")
public class ArmorTemplate {
    @Element(required=false, name="pierce_defense")
    private int pierceDefense;

    @Element(required=false, name="slash_defense")
    private int slashDefense;

    @Element(required=false, name="crush_defense")
    private int crushDefense;

    @Element(required=false, name="dodge_penalty")
    private int dodgePenalty;

    @Element(required=false)
    private String skill;
}
