package org.scourge.model;

import com.jme.bounding.BoundingBox;
import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.state.BlendState;
import com.jme.system.DisplaySystem;
import com.jme.util.TextureManager;
import org.scourge.config.ItemTemplate;
import org.scourge.config.Items;
import org.scourge.terrain.ShapeUtil;
import org.scourge.ui.component.Dragable;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

import javax.swing.*;
import java.util.Arrays;
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
    private Spatial spatial;
    private Texture iconTexture;

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
        if(iconTexture == null) {
            ImageIcon icon = getIcon();
            iconTexture = ShapeUtil.getTexture(getTemplate().getIcon());
            if(iconTexture == null) {
                iconTexture = TextureManager.loadTexture(icon.getImage(),
                                                     Texture.MinificationFilter.NearestNeighborNearestMipMap,
                                                     Texture.MagnificationFilter.Bilinear,
                                                     true);
                iconTexture.setWrap(Texture.WrapMode.Repeat);
                iconTexture.setHasBorder(false);
                iconTexture.setApply(Texture.ApplyMode.Modulate);
                ShapeUtil.storeTexture(getTemplate().getIcon(), iconTexture);
            }
        }
        return iconTexture;
    }

    @Override
    public int getIconWidth() {
        return getIcon().getIconWidth();
    }

    @Override
    public int getIconHeight() {
        return getIcon().getIconHeight();
    }

    @Override
    public Spatial getModel() {
        if(spatial == null) {
            spatial = ShapeUtil.importModel(getTemplate().getModel().getPath(), "./data/textures", getTemplate().getName());
//            if(getTemplate().getModel().getRotate() != null) {
//                spatial.getLocalRotation().set(new Quaternion().fromAngles(getTemplate().getModel().getRotate()[0] * 90 * FastMath.DEG_TO_RAD,
//                                                                           getTemplate().getModel().getRotate()[1] * 90 * FastMath.DEG_TO_RAD,
//                                                                           getTemplate().getModel().getRotate()[2] * 90 * FastMath.DEG_TO_RAD));
//            }
//            spatial.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(90 * FastMath.DEG_TO_RAD, Vector3f.UNIT_Y));

            BlendState as = DisplaySystem.getDisplaySystem().getRenderer().createBlendState();
            as.setBlendEnabled(true);
            as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
            as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
            as.setReference(0);
            as.setTestEnabled(true);
            as.setTestFunction(BlendState.TestFunction.NotEqualTo);
            as.setEnabled(true);
            spatial.setRenderState(as);

            BoundingBox bb = new BoundingBox();
            spatial.setModelBound(bb);
            spatial.updateModelBound();
        }
        return spatial;
    }

    public void scaleModel() {
        BoundingBox bb = (BoundingBox)spatial.getWorldBound();
        float[] size = getTemplate().getModel().getDimensions();
        if(size != null) {
            if(size[0] > 0) spatial.getLocalScale().x = size[0] / bb.xExtent;
            if(size[1] > 0) spatial.getLocalScale().y = size[1] / bb.yExtent;
            if(size[2] > 0) spatial.getLocalScale().z = size[2] / bb.zExtent;
        }
        spatial.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(90 * FastMath.DEG_TO_RAD, Vector3f.UNIT_Y));
        spatial.updateModelBound();
        spatial.updateWorldBound();

    }
}
