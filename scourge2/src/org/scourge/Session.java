package org.scourge;

import com.jme.math.Vector3f;
import com.jme.system.DisplaySystem;
import org.scourge.ui.Window;
import org.scourge.ui.WindowListener;

/**
 * User: gabor
 * Date: Apr 12, 2010
 * Time: 8:51:12 PM
 */
public class Session implements WindowListener {
    private Window mainMenuWindow;
    private Main main;
    private boolean inMainMenu;
    public static final Vector3f PLAYER_START_LOCATION = new Vector3f(693, 9, 151);
            //498, 9, 489);

    public Session(Main main) {
        this.main = main;

        mainMenuWindow = new Window(DisplaySystem.getDisplaySystem().getRenderer().getWidth() / 2,
                                    (int)(DisplaySystem.getDisplaySystem().getRenderer().getHeight() / 4.0f * 3.0f),
                                    300, 300, this);
        mainMenuWindow.addLabel(0, 90, "Scourge II");
        mainMenuWindow.addButton("new", 0, 30, "New Game");
        mainMenuWindow.addButton("load", 0, -10, "Continue Game");
        mainMenuWindow.addButton("quit", 0, -50, "Quit");
    }

    public void showMainMenu() {
        inMainMenu = true;
        main.getTerrain().gotoMainMenu();
        main.showWindow(mainMenuWindow);
    }

    @Override
    public void buttonClicked(String name) {
        inMainMenu = false;
        if("quit".equals(name)) {
            main.finish();
        } else if("load".equals(name)) {
            startGame();
        } else {
            startGame();
        }
    }

    private void startGame() {
        main.hideWindow(mainMenuWindow);
        main.getPlayer().moveTo(PLAYER_START_LOCATION);
        main.getTerrain().gotoPlayer();
    }

    public boolean inMainMenu() {
        return inMainMenu;
    }

    public boolean escapePressed() {
        if(inMainMenu) {
            return true;
        } else {
            showMainMenu();
            return false;
        }
    }
}
