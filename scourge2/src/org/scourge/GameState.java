package org.scourge;

import com.jme.system.DisplaySystem;
import com.jme.util.GameTaskQueueManager;
import org.scourge.config.Items;
import org.scourge.io.SaveGame;
import org.scourge.model.Creature;
import org.scourge.model.Session;
import org.scourge.ui.*;
import org.scourge.ui.component.*;

import java.awt.geom.Point2D;
import java.util.concurrent.Callable;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Apr 12, 2010
 * Time: 8:51:12 PM
 */
public class GameState implements WindowListener, ProgressListener {
    private Window mainMenuWindow, gameMenuWindow;
    private CreatureEditor pcEditor;
    private GameUI gameUI;
    private Logger logger = Logger.getLogger(GameState.class.toString());
    private Session session;
    private boolean configInitialized;

    public GameState() {
        mainMenuWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                    (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 4.0f * 3.0f),
                                    300, 300, this);
        mainMenuWindow.addLabel(0, 90, "Scourge II", WinUtil.ScourgeFont.large);
        mainMenuWindow.addButton("new", 0, 30, "New Game");
        mainMenuWindow.addButton("load", 0, -10, "Continue Game");
        mainMenuWindow.addButton("quit", 0, -50, "Quit");
        mainMenuWindow.addProgress("progress", 0, -100, 200, 16);
        mainMenuWindow.pack();

        gameMenuWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                    (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 2.0f),
                                    300, 300, this);
        gameMenuWindow.addLabel(0, 90, "Scourge II", WinUtil.ScourgeFont.large);
        gameMenuWindow.addButton("save", 0, 30, "Save Game");
        gameMenuWindow.addButton("continue", 0, -10, "Continue");
        gameMenuWindow.addButton("back", 0, -50, "to Main Menu");
        gameMenuWindow.pack();

        pcEditor = new CreatureEditor(this);
    }

    public void showMainMenu() {
        Main.getMain().getTerrain().gotoMainMenu();
        mainMenuWindow.setVisible(true);
        if(!configInitialized) {
            initializeConfig();
        }
    }

    @Override
    public void progress(final float percent) {
        GameTaskQueueManager.getManager().update(new Callable<Object>() {
            public Object call() throws Exception {
                mainMenuWindow.setValue("progress", percent);
                return null;
            }
        });
        Thread.yield();
    }

    @Override
    public void done() {
        // effectively hide the progress bar
        progress(0);
    }

    private void initializeConfig() {
        new Thread() {
            public void run() {
                try {
                    logger.info("Loading config...");
                    Items.load(GameState.this);
                    configInitialized = true;
                    logger.info("Config initialized.");
                } catch(Exception exc) {
                    System.err.println("Unable to initialize game: " + exc.getMessage());
                    exc.printStackTrace();
                    System.exit(1);
                }
            }
        }.start();
    }

    @Override
    public void buttonClicked(String name) {
        try {
            if(Window.getWindow() == mainMenuWindow) {
                if("quit".equals(name)) {
                    Main.getMain().finish();
                } else if("load".equals(name)) {
                    startGame();
                } else if("new".equals(name)) {
                    newGame();
                }
            } else if(Window.getWindow() == gameMenuWindow) {
                if("save".equals(name)) {
                    saveGame();
                } else if("continue".equals(name)) {
                    gameMenuWindow.setVisible(false);
                } else {
                    endGame();
                }
            } else if(Window.getWindow() == pcEditor) {
                if(CreatureEditor.START.equals(name)) {
                    savePcAndStartGame();
                }
            }
        } catch(Exception exc) {
            Window.showMessage("Error: " + exc.getMessage());
            logger.log(Level.SEVERE, "Error: " + exc.getMessage(), exc);
        }
    }

    @Override
    public Dragable drag(String name, Point2D point) {
        return null;
    }

    @Override
    public boolean drop(String name, Point2D point, Dragable dragging) {
        return false;
    }

    @Override
    public DragSource getDragSource() {
        return null;
    }

    private void newGame() throws Exception {
        if(SaveGame.hasSavedGame()) {
            Window.confirm("Delete old saved game?", new Runnable() {
                public void run() {
                    try {
                        createPc();
                    } catch(Exception exc) {
                        Window.showMessage("Error: " + exc.getMessage());
                        logger.log(Level.SEVERE, "Error: " + exc.getMessage(), exc);
                    }
                }
            });
        } else {
            createPc();
        }
    }

    private void createPc() throws Exception {
        session = new Session();
        Creature creature = Creature.newPlayer();
        session.getParty().add(creature);
        SaveGame.save(session);
        pcEditor.load(creature);
        pcEditor.setVisible(true);
    }

    public void savePcAndStartGame() throws Exception {
        pcEditor.save();
        pcEditor.setVisible(false);
        SaveGame.save(session);
        startGame();
    }

    private void startGame() throws Exception {
        mainMenuWindow.setVisible(false);
        session = SaveGame.loadGame();
        Main main = Main.getMain();
        main.setPlayer(session.getParty().get(0));
        main.getTerrain().gotoPlayer();
        gameUI = new GameUI(this);
        gameUI.setVisible(true);
        main.setMiniMapVisible(true);
    }

    private void endGame() {
        gameMenuWindow.setVisible(false);
        Main.getMain().setMiniMapVisible(false);
        gameUI.setVisible(false);
        showMainMenu();
    }

    private void saveGame() throws Exception {
        gameMenuWindow.setVisible(false);
        SaveGame.save(session);
    }

    public boolean escapePressed() {
        if(Window.getWindow() == mainMenuWindow) {
            return true;
        } else if(Window.getWindow() == null || Window.getWindow().isAlwaysOpen()) {
            gameMenuWindow.setVisible(true);
        } else if(!Window.getWindow().isAlwaysOpen()) {
            Window.getWindow().setVisible(false);
        }
        return false;
    }

    public Session getSession() {
        return session;
    }
}
