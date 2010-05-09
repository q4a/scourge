package org.scourge.ui;

import com.jme.image.Texture;
import com.jme.renderer.ColorRGBA;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.RenderState;
import com.jme.scene.state.WireframeState;
import com.jme.system.DisplaySystem;
import com.jme.util.TextureManager;
import org.scourge.model.Item;
import org.scourge.model.ItemList;
import org.scourge.terrain.ShapeUtil;
import org.scourge.ui.component.Component;
import org.scourge.ui.component.WinUtil;
import org.scourge.ui.component.Window;

import javax.swing.*;
import java.awt.*;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: May 8, 2010
 * Time: 9:08:16 AM
 */
public class ItemContainerUI extends Component {
    private static final int SLOT_SIZE = 32;
    private ItemList itemList;
    private int slotWidth, slotHeight;
    private Map<Item, Rectangle> rectangles = new HashMap<Item, Rectangle>();
    private Logger logger = Logger.getLogger(ItemContainerUI.class.toString());

    public ItemContainerUI(Window window, String name, int x, int y, int slotWidth, int slotHeight) {
        super(window, name, x, y, slotWidth * SLOT_SIZE, slotHeight * SLOT_SIZE);

        this.slotWidth = slotWidth;
        this.slotHeight = slotHeight;

        ColorRGBA color = new ColorRGBA(1, 0.85f, 0.75f, 1);
        WireframeState wireState = DisplaySystem.getDisplaySystem().getRenderer().createWireframeState();
        wireState.setEnabled(true);
        for(int sx = 0; sx < slotWidth; sx++) {
            for(int sy = 0; sy < slotHeight; sy++) {
                Quad q = WinUtil.createQuad(name + ".slot", SLOT_SIZE, SLOT_SIZE);
                q.getLocalTranslation().set((sx + 0.5f) * SLOT_SIZE, -(sy + 0.5f) * SLOT_SIZE, 0);
                q.setSolidColor(color);
                q.setRenderState(wireState);
                getNode().attachChild(q);
            }
        }
        getNode().updateRenderState();
    }

    public void setItems(ItemList itemList) {
        if(itemList == this.itemList) {
            return;
        }

        this.itemList = itemList;
        clear();
        if(itemList != null) {
            for(Item item : itemList.getItems()) {
                ImageIcon icon = ShapeUtil.loadImageIcon(item.getTemplate().getModel().getIcon());
                if(icon == null) {
                    logger.log(Level.SEVERE, "Can't load icon for " + item.getTemplate().getName());
                    continue;
                }
                Texture texture = ShapeUtil.getTexture(item.getTemplate().getIcon());
                if(texture == null) {
                    texture = TextureManager.loadTexture(icon.getImage(),
                                                         Texture.MinificationFilter.NearestNeighborNearestMipMap,
                                                         Texture.MagnificationFilter.Bilinear,
                                                         true);
                    texture.setWrap(Texture.WrapMode.Repeat);
                    texture.setHasBorder(false);
                    texture.setApply(Texture.ApplyMode.Modulate);
    	            ShapeUtil.storeTexture(item.getTemplate().getIcon(), texture);
                }

                int itemWidth = icon.getIconWidth() / SLOT_SIZE;
                int itemHeight = icon.getIconHeight() / SLOT_SIZE;

                if(item.getContainerPosition() == null ||
                   item.getContainerPosition()[0] > slotWidth ||
                   item.getContainerPosition()[1] > slotHeight) {
                    if(!findPlace(item, itemWidth, itemHeight)) {
                        // assume this can't happen
                        logger.severe("Couldn't fit item into container: " + item.getTemplate().getName() +
                                      " dimensions=" + itemWidth + "," + itemHeight);
                    }
                }

                Quad q = WinUtil.createQuad(getName() + ".item", icon.getIconWidth(), icon.getIconHeight(), texture);
                int[] pos = item.getContainerPosition();
                q.getLocalTranslation().set((pos[0] + itemWidth / 2f) * SLOT_SIZE,
                                            -(pos[1] + itemHeight / 2f) * SLOT_SIZE, 0);
                getNode().attachChild(q);
                getNode().updateRenderState();
                rectangles.put(item, new Rectangle(pos[0], pos[1], itemWidth, itemHeight));
            }
        }
    }

    private void clear() {
        rectangles.clear();
        Set<Spatial> toBeRemoved = new HashSet<Spatial>();
        for(Spatial s : getNode().getChildren()) {
            if(s.getName().startsWith(getName() + ".item")) {
                toBeRemoved.add(s);
            }
        }
        for(Spatial s : toBeRemoved) {
            getNode().detachChild(s);
        }
    }

    /**
     * Find a place for this item. If no place can be found, false is returned.
     * @param itemToPlace the item to put into this container
     * @param itemWidth the item's width (in slots)
     * @param itemHeight the item's height (in slots)
     * @return true if a place was found, false if it won't fit
     */
    private boolean findPlace(Item itemToPlace, int itemWidth, int itemHeight) {
        Rectangle test = new Rectangle(0, 0, itemWidth, itemHeight);
        for(int sx = 0; sx < slotWidth - itemWidth; sx++) {
            for(int sy = 0; sy < slotHeight - itemHeight; sy++) {
                test.setLocation(sx, sy);
                boolean occupied = false;
                for(Rectangle rectangle : rectangles.values()) {
                    if(test.intersects(rectangle)) {
                        occupied = true;
                        break;
                    }
                }
                if(!occupied) {
                    itemToPlace.setContainerPosition(new int[] { sx, sy });
                    return true;
                }
            }
        }
        return false;
    }
}
