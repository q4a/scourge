package org.scourge;

import com.jme.system.DisplaySystem;
import org.scourge.io.SaveGame;
import org.scourge.model.Creature;
import org.scourge.model.Session;
import org.scourge.ui.Window;
import org.scourge.ui.WindowListener;

import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Apr 12, 2010
 * Time: 8:51:12 PM
 */
public class GameState implements WindowListener {
    private Window mainMenuWindow, gameMenuWindow;
    private Logger logger = Logger.getLogger(GameState.class.toString());
    private Session session;

    public GameState() {
        mainMenuWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                    (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 4.0f * 3.0f),
                                    300, 300, this);
        mainMenuWindow.addLabel(0, 90, "Scourge II");
        mainMenuWindow.addButton("new", 0, 30, "New Game");
        mainMenuWindow.addButton("load", 0, -10, "Continue Game");
        mainMenuWindow.addButton("quit", 0, -50, "Quit");
        mainMenuWindow.pack();


        gameMenuWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                    (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 2.0f),
                                    300, 300, this);
        gameMenuWindow.addLabel(0, 90, "Scourge II");
        gameMenuWindow.addButton("save", 0, 30, "Save Game");
        gameMenuWindow.addButton("continue", 0, -10, "Continue");
        gameMenuWindow.addButton("back", 0, -50, "to Main Menu");
        gameMenuWindow.pack();
    }

    public void showMainMenu() {
        Main.getMain().getTerrain().gotoMainMenu();
        mainMenuWindow.setVisible(true);
    }

    @Override
    public void buttonClicked(String name) {
        try {
            if(Window.getWindow() == mainMenuWindow) {
                if("quit".equals(name)) {
                    Main.getMain().finish();
                } else if("load".equals(name)) {
                    loadGame();
                } else if("new".equals(name)) {
                    newGame();
                }
            } else if(Window.getWindow() == gameMenuWindow) {
                if("save".equals(name)) {
                    saveGame();
                } else if("continue".equals(name)) {
                    gameMenuWindow.setVisible(false);
                } else {
                    gameMenuWindow.setVisible(false);
                    showMainMenu();
                }
            }
        } catch(Exception exc) {
            Window.showMessage("Error: " + exc.getMessage());
            logger.log(Level.SEVERE, "Error: " + exc.getMessage(), exc);
        }
    }

    private void newGame() throws Exception {
        if(SaveGame.hasSavedGame()) {
            Window.confirm("Delete old saved game?", new Runnable() {
                public void run() {
                    try {
                        startNewGame();
                    } catch(Exception exc) {
                        Window.showMessage("Error: " + exc.getMessage());
                        logger.log(Level.SEVERE, "Error: " + exc.getMessage(), exc);
                    }
                }
            });
        } else {
            startNewGame();
        }
    }

    private void startNewGame() throws Exception {
        session = new Session(); 
        session.getParty().add(new Creature());
        SaveGame.save(session);
        startGame();
    }

    private void loadGame() throws Exception {
        session = SaveGame.loadGame();
        startGame();
    }

    private void startGame() {
        mainMenuWindow.setVisible(false);
        Main main = Main.getMain();
        main.setPlayer(session.getParty().get(0));
        main.getTerrain().gotoPlayer();
    }

    private void saveGame() throws Exception {
        gameMenuWindow.setVisible(false);
        SaveGame.save(session);
    }

    public boolean escapePressed() {
        if(Window.getWindow() == mainMenuWindow) {
            return true;
        } else if(Window.getWindow() == gameMenuWindow) {
            gameMenuWindow.setVisible(false);
            return false;
        } else {
            gameMenuWindow.setVisible(true);
            return false;
        }
    }
}
