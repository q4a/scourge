package org.scourge.io;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import java.io.*;

/**
 * User: gabor
 * Date: Apr 17, 2010
 * Time: 3:27:29 PM
 */
@Root(name = "save_game")
public class SaveGame {
    @Attribute(name = "version")
    private int version;

    @Element(name = "player_position")
    private float[] playerPosition;

    public static boolean hasSavedGame() {
        return getSaveGameFile().exists();
    }

    public static SaveGame newGame() throws Exception {
        SaveGame saveGame = new SaveGame();
        saveGame.save();
        return saveGame;
    }

    public static SaveGame loadGame() throws Exception {
        if(!hasSavedGame()) {
            return newGame();
        } else {
            Serializer serializer = new Persister();
            Reader reader = new BufferedReader(new FileReader(getSaveGameFile()));
            SaveGame saveGame = serializer.read(SaveGame.class, reader);
            reader.close();
            return saveGame;
        }
    }

    // set default values here
    private SaveGame() {
        version = 1;
        //693, 9, 151);
        //389, 9, 349);
        //498, 9, 489);
        playerPosition = new float[] { 498, 9, 489 };
    }

    public void save() throws Exception {
        Serializer serializer = new Persister();
        Writer writer = new BufferedWriter(new FileWriter(getSaveGameFile()));
        serializer.write(this, writer);
        writer.close();
    }

    private static File getDir() {
        File dir = new File(System.getProperty("user.home"), ".scourge2");
        if(!dir.exists()) {
            //noinspection ResultOfMethodCallIgnored
            dir.mkdir();
        }
        return dir;
    }

    public static File getSaveGameFile() {
        return new File(getDir(), "savegame.xml");
    }

    public float[] getPlayerPosition() {
        return playerPosition;
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public void setPlayerPosition(float[] playerPosition) {
        this.playerPosition = playerPosition;
    }
}
