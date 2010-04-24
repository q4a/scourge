package org.scourge.ui;

import org.scourge.GameState;
import org.scourge.model.Creature;

import java.util.List;

/**
 * User: gabor
 * Date: Apr 23, 2010
 * Time: 9:25:44 PM
 */
public class GameUI extends Window implements WindowListener {
    private GameState gameState;
    private static int WIDTH = 200;
    private static int HEIGHT = 60;
    private static final int BUFFER = 10;

    public GameUI(GameState gameState) {
        super(WIDTH / 2, HEIGHT / 2, WIDTH, HEIGHT);
        setListener(this);
        setAlwaysOpen(true);
        int size = (WIDTH - BUFFER * 2) / 4;
        for(int i = 0; i < 4; i++) {
            addImage("pc" + i, null, (i * size) - WIDTH / 2 + size / 2 + BUFFER, (HEIGHT - size) / 2 - 5, size - 5, size - 5);
        }
        pack();
        this.gameState = gameState;
    }


    @Override
    public void setVisible(boolean visible) {
        unpack();
        List<Creature> party = gameState.getSession().getParty();
        for(int i = 0; i < 4; i++) {
            setImage("pc" + i, i < party.size() ? party.get(i).getPortrait() : null);
        }
        pack();
        super.setVisible(visible);    //To change body of overridden methods use File | Settings | File Templates.
    }

    @Override
    public void buttonClicked(String name) {
        //To change body of implemented methods use File | Settings | File Templates.
    }
}
