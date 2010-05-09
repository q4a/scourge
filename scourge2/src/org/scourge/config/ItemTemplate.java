package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.ElementList;
import org.simpleframework.xml.Root;

import java.util.ArrayList;
import java.util.List;

/**
 * User: gabor
 * Date: Apr 29, 2010
 * Time: 8:14:49 AM
 */
@Root(name = "item")
public class ItemTemplate {
    @Element
    private String name;

    @Element(required=false)
    private String description;

    @Element(name="display_name")
    private String displayName;

    @Element
    private String shape;

    @Element
    private String icon;

    @Element(name = "inventory_location", required=false)
    private int inventoryLocation;

    @Element(required=false)
    private int price;

    @Element
    private int rareness;

    @Element(name = "short_description", required=false)
    private String shortDescription;

    @Element(required = false)
    private String tags;

    @Element
    private String type;

    @Element
    private float weight;

    @Element(required=false, name="spell_level")
    private int spellLevel;

    @Element(required=false, name="max_charges")
    private int maxCharges;

    @Element(required=false, name="min_level")
    private int minLevel;

    @Element(required=false, name="min_depth")
    private int minDepth;

    @Element(required=false, name="container_width")
    private int containerWidth;

    @Element(required=false, name="container_height")
    private int containerHeight;

    @Element(required=false, name="container_texture")
    private String containerTexture;

    @Element(required=false)
    private WeaponTemplate weapon;

    @Element(required=false)
    private ArmorTemplate armor;

    @Element(required=false)
    private PotionTemplate potion;    

    @ElementList(required=false, inline=true)
    private List<SkillAdjustment> skillAdjustments = new ArrayList<SkillAdjustment>();

    private Model model;

    public String getName() {
        return name;
    }

    public String getDescription() {
        return description;
    }

    public String getDisplayName() {
        return displayName;
    }

    public String getShape() {
        return shape;
    }

    public String getIcon() {
        return icon;
    }

    public int getInventoryLocation() {
        return inventoryLocation;
    }

    public int getPrice() {
        return price;
    }

    public int getRareness() {
        return rareness;
    }

    public String getShortDescription() {
        return shortDescription;
    }

    public String getTags() {
        return tags;
    }

    public String getType() {
        return type;
    }

    public float getWeight() {
        return weight;
    }

    public int getSpellLevel() {
        return spellLevel;
    }

    public int getMaxCharges() {
        return maxCharges;
    }

    public int getMinLevel() {
        return minLevel;
    }

    public int getMinDepth() {
        return minDepth;
    }

    public int getContainerWidth() {
        return containerWidth;
    }

    public int getContainerHeight() {
        return containerHeight;
    }

    public String getContainerTexture() {
        return containerTexture;
    }

    public WeaponTemplate getWeapon() {
        return weapon;
    }

    public ArmorTemplate getArmor() {
        return armor;
    }

    public PotionTemplate getPotion() {
        return potion;
    }

    public List<SkillAdjustment> getSkillAdjustments() {
        return skillAdjustments;
    }

    public Model getModel() {
        return model;
    }

    public void afterLoad() {
        model = Models.getInstance().getModel(shape);
    }
}
