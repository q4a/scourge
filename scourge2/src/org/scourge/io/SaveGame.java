package org.scourge.io;

import org.scourge.model.Session;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import java.io.*;

/**
 * User: gabor
 * Date: Apr 17, 2010
 * Time: 3:27:29 PM
 */
public class SaveGame {

    public static boolean hasSavedGame() {
        return getSaveGameFile().exists();
    }

    public static Session newGame() throws Exception {
        Session session = new Session();
        save(session);
        return session;
    }

    public static Session loadGame() throws Exception {
        if(!hasSavedGame()) {
            return newGame();
        } else {
            Serializer serializer = new Persister();
            Reader reader = new BufferedReader(new FileReader(getSaveGameFile()));
            Session session = serializer.read(Session.class, reader);
            reader.close();
            session.afterLoad();
            return session;
        }
    }

    public static void save(Session session) throws Exception {
        session.beforeSave();
        Serializer serializer = new Persister();
        Writer writer = new BufferedWriter(new FileWriter(getSaveGameFile()));
        serializer.write(session, writer);
        writer.close();
    }

    public static File getDir() {
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
}
