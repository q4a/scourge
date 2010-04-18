package org.scourge;

import com.jme.math.Vector3f;
import com.jme.system.DisplaySystem;
import org.scourge.io.SaveGame;
import org.scourge.ui.Window;
import org.scourge.ui.WindowListener;

import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Apr 12, 2010
 * Time: 8:51:12 PM
 */
public class Session implements WindowListener {
    private Window mainMenuWindow, gameMenuWindow;
    private SaveGame saveGame;
    private Logger logger = Logger.getLogger(Session.class.toString());

    public Session() {
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
                        saveGame = SaveGame.newGame();
                        startGame();
                    } catch(Exception exc) {
                        Window.showMessage("Error: " + exc.getMessage());
                        logger.log(Level.SEVERE, "Error: " + exc.getMessage(), exc);
                    }
                }
            });
        } else {
            saveGame = SaveGame.newGame();
            startGame();
        }
    }

    private void loadGame() throws Exception {
        saveGame = SaveGame.loadGame();
        startGame();
    }

    private void startGame() {
        mainMenuWindow.setVisible(false);
        float[] pos = saveGame.getPlayerPosition();
        Main.getMain().getPlayer().moveTo(new Vector3f(pos[0], pos[1], pos[2]));
        Main.getMain().getTerrain().gotoPlayer();
    }

    private void saveGame() throws Exception {
        gameMenuWindow.setVisible(false);
        saveGame.setPlayerPosition(new float[] {
                Main.getMain().getPlayer().getX(), 9, Main.getMain().getPlayer().getZ()
        });
        saveGame.save();
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
