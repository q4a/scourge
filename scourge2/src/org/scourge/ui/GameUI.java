package org.scourge.ui;

import com.jme.renderer.ColorRGBA;
import com.jme.scene.shape.Quad;
import org.scourge.GameState;
import org.scourge.model.Creature;
import org.scourge.ui.component.*;

import java.util.List;

/**
 * User: gabor
 * Date: Apr 23, 2010
 * Time: 9:25:44 PM
 */
public class GameUI extends Window implements WindowListener {
    private GameState gameState;
    private PcStatus[] status = new PcStatus[4];
    private static final int WIDTH = 200;
    private static final int BUFFER = 10;
    private final static int PORTRAIT_SIZE = 50;
    private Inventory inventory = new Inventory();

    public GameUI(GameState gameState) {
        super(WIDTH / 2, (gameState.getSession().getParty().size() * (PORTRAIT_SIZE + 5) + 5) / 2,
              WIDTH, gameState.getSession().getParty().size() * (PORTRAIT_SIZE + 5) + 5);
        setListener(this);
        setAlwaysOpen(true);
        int xpos, ypos;
        for(int i = 0; i < gameState.getSession().getParty().size(); i++) {
            xpos = -((WIDTH - PORTRAIT_SIZE) / 2) + BUFFER;
            ypos = (i * (PORTRAIT_SIZE)) - getH() / 2 + PORTRAIT_SIZE / 2 + 5;
            status[i] = new PcStatus(this,
                                     String.valueOf(i),
                                     xpos,
                                     ypos,
                                     WIDTH - BUFFER * 2,
                                     PORTRAIT_SIZE,
                                     PORTRAIT_SIZE - 5);
            status[i].setCreature(gameState.getSession().getParty().get(i));
        }
        pack();
        this.gameState = gameState;
    }

    @Override
    public void buttonClicked(String name) {
        System.err.println("Clicked: " + name);
        if(name.startsWith("pc.")) {
            int index = Integer.valueOf(name.substring(name.lastIndexOf(".") + 1));
            inventory.setCreature(gameState.getSession().getParty().get(index));
            inventory.setVisible(true);
        }
    }

    private class PcStatus extends Component {
        private ImageComponent portrait;
        private Label nameLabel;
        private Quad hpQuad, mpQuad;
        private int lineWidth;
        private int portraitSize;
        private final ColorRGBA hpColor = new ColorRGBA(0.5f, 0.15f, 0.1f, 0.85f);
        private final ColorRGBA mpColor = new ColorRGBA(0.1f, 0.15f, 0.5f, 0.85f);

        public PcStatus(Window window, String name, int x, int y, int w, int h, int portraitSize) {
            super(window, "pc." + name, x, y, w, h);
            this.portraitSize = portraitSize;
            portrait = new ImageComponent(window, "pc.portrait." + name, null, x, y, portraitSize, portraitSize);
            nameLabel = new Label(window, "pc.name." + name, x + portraitSize / 2 + 5, y + portraitSize / 2 - 10, "", Window.TEXT_COLOR, false, WinUtil.ScourgeFont.text);
            lineWidth =  w - portraitSize - 10;
            hpQuad = WinUtil.createQuad("pc.hp." + name, lineWidth, 5);
            hpQuad.setSolidColor(hpColor);
            mpQuad = WinUtil.createQuad("pc.mp." + name, lineWidth, 5);
            mpQuad.setSolidColor(mpColor);
            window.addComponent(portrait);
            window.addComponent(nameLabel);
            window.getNode().attachChild(hpQuad);
            window.getNode().attachChild(mpQuad);
            window.addComponent(this);
        }

        public void setCreature(Creature creature) {
            float mpScale, hpScale;
            if(creature == null) {
                portrait.setImage(null);
                nameLabel.setText("");
                mpScale = hpScale = 0f;
            } else {
                portrait.setImage(creature.getPortrait());
                nameLabel.setText(creature.getName());
                hpScale = creature.getHp() / (float)creature.getMaxHp();
                mpScale = creature.getMp() / (float)creature.getMaxMp();
            }

            hpQuad.getLocalScale().set(hpScale, 1, 1);
            mpQuad.getLocalScale().set(mpScale, 1, 1);
            hpQuad.getLocalTranslation().set(getX() + lineWidth * hpScale / 2 + portraitSize / 2 + 5,
                                             getY(), 0);
            mpQuad.getLocalTranslation().set(getX() + lineWidth * mpScale / 2 + portraitSize / 2 + 5,
                                             getY() - 6, 0);
        }
    }    
}
