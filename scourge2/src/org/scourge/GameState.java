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
    private Window mainMenuWindow, gameMenuWindow, pcEditorWindow;
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

        pcEditorWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                    (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 2.0f),
                                    500, 350, this);
        pcEditorWindow.addLabel(0, 155, "Create a character");
        pcEditorWindow.addLabel(-150, 120, "Name:");
        pcEditorWindow.addTextfield("name", 0, 120, "", 20);

        pcEditorWindow.addLabel(-150, 90, "Level: ");
        pcEditorWindow.addLabel("level", -100, 90, "0");
        pcEditorWindow.addLabel(-150, 60, "Exp.: ");
        pcEditorWindow.addLabel("exp", -100, 60, "0");
        pcEditorWindow.addLabel(-150, 30, "Hp: ");
        pcEditorWindow.addLabel("hp", -100, 30, "0");
        pcEditorWindow.addLabel(-50, 30, "Mp: ");
        pcEditorWindow.addLabel("mp", 0, 30, "0");
        pcEditorWindow.addLabel(-150, 0, "Coins: ");
        pcEditorWindow.addLabel("coins", -100, 0, "0");

        pcEditorWindow.addButton("start", -70, -150, "Start Game");
        pcEditorWindow.addButton("cancel", 70, -150, "Cancel");
        pcEditorWindow.pack();
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
            } else if(Window.getWindow() == pcEditorWindow) {
                if("cancel".equals(name)) {
                    pcEditorWindow.setVisible(false);
                } else if("start".equals(name)) {
                    savePcAndStartGame();
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
        Creature creature = new Creature();
        session.getParty().add(creature);
        SaveGame.save(session);
        pcEditorWindow.setText("name", creature.getName());
        pcEditorWindow.setText("level", String.valueOf(creature.getLevel()));
        pcEditorWindow.setText("exp", String.valueOf(creature.getExperience()));
        pcEditorWindow.setText("hp", String.valueOf(creature.getHp()));
        pcEditorWindow.setText("mp", String.valueOf(creature.getMp()));
        pcEditorWindow.setText("coins", String.valueOf(creature.getCoins()));
        pcEditorWindow.setVisible(true);
    }

    private void savePcAndStartGame() throws Exception {
        Creature creature = session.getParty().get(0);
        creature.setName(pcEditorWindow.getText("name"));
        SaveGame.save(session);
        pcEditorWindow.setVisible(false);
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
        } else if(Window.getWindow() == null) {
            gameMenuWindow.setVisible(true);
        } else {
            Window.getWindow().setVisible(false);
        }
        return false;
    }
}
