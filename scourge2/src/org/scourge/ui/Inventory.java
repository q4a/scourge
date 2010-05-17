package org.scourge.ui;

import com.jme.system.DisplaySystem;
import org.scourge.model.Creature;
import org.scourge.ui.component.*;
import org.scourge.ui.component.Window;

import java.awt.geom.Point2D;

/**
 * User: gabor
 * Date: May 5, 2010
 * Time: 9:27:17 AM
 */
public class Inventory extends Window implements WindowListener {
    private static final int WIDTH = 450;
    private static final int HEIGHT = 400;
    private static final int PORTRAIT_SIZE = 128;
    private Creature creature;
    private ItemContainerUI itemContainer;

    public Inventory() {
        super(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
              DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 2,
              WIDTH, HEIGHT);
        setListener(this);

        addImage("inventory.portrait", null,
                 -WIDTH / 2 + 25 + PORTRAIT_SIZE / 2,
                 HEIGHT / 2 - (15 + PORTRAIT_SIZE / 2),
                 PORTRAIT_SIZE, PORTRAIT_SIZE);
        addLabel("inventory.name",
                 -WIDTH / 2 + 25 + PORTRAIT_SIZE + 5,
                 HEIGHT / 2 - 55, "", Window.TEXT_COLOR, false,
                 WinUtil.ScourgeFont.large);
        addLabel("inventory.level",
                 -WIDTH / 2 + 25 + PORTRAIT_SIZE + 5,
                 HEIGHT / 2 - 55, "", false);
        addLabel("inventory.stats",
                 -WIDTH / 2 + 25 + PORTRAIT_SIZE + 5,
                 HEIGHT / 2 - 70, "", false);

        itemContainer = new ItemContainerUI(this, "inventory.items", -WIDTH / 2 + 25, HEIGHT / 2 - 20 - PORTRAIT_SIZE, 12, 6);
        addComponent(itemContainer);

        addButton("inventory.close", -WIDTH / 2 + 85, -HEIGHT / 2 + 30, "OK");

        pack();
    }

    public void setCreature(Creature creature) {
        if(this.creature != creature) {
            this.creature = creature;
            setImage("inventory.portrait", creature.getPortrait());
            setText("inventory.name", creature.getName().substring(0, 1).toUpperCase() + creature.getName().substring(1));
            setText("inventory.level", "Level:" + creature.getLevel() + " Exp:" + creature.getExperience());
            setText("inventory.stats", "HP:" + creature.getHp() + "/" + creature.getMaxHp() +
                                       " MP:" + creature.getMp() + "/" + creature.getMaxMp() +
                                       " Coin:" + creature.getCoins());
            itemContainer.setItems(creature);
        }
    }

    @Override
    public void buttonClicked(String name) {
        if(name.equals("inventory.close")) {
            setVisible(false);
        }
    }

    @Override
    public Dragable drag(String name, Point2D point) {
        if(name.equals("inventory.items")) {
            return itemContainer.drag(point);
        }
        return null;
    }

    @Override
    public boolean drop(String name, Point2D point, Dragable dragging) {
        return itemContainer.drop(point, dragging);
    }

    @Override
    public DragSource getDragSource() {
        return itemContainer;
    }
}
