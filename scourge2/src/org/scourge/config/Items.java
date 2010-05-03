package org.scourge.config;

import org.simpleframework.xml.ElementArray;
import org.simpleframework.xml.ElementList;
import org.simpleframework.xml.Root;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;

/**
 * User: gabor
 * Date: May 1, 2010
 * Time: 2:47:16 PM
 */
@Root(name="items")
public class Items {
    @ElementList(inline=true)
    private List<ItemTemplate> items = new ArrayList<ItemTemplate>();

    private static Items instance = new Items();

    public static void load() throws Exception {
        instance.items.addAll(loadItems("./data/config/weapon.xml").items);
        instance.items.addAll(loadItems("./data/config/armor.xml").items);
        instance.items.addAll(loadItems("./data/config/magicitem.xml").items);
        instance.items.addAll(loadItems("./data/config/otheritem.xml").items);
    }

    private static Items loadItems(String path) throws Exception {
        Serializer serializer = new Persister();
        Reader reader = new BufferedReader(new FileReader(path));
        Items items = serializer.read(Items.class, reader);
        reader.close();
        return items;
    }

    public static Items getInstance() {
        return instance;
    }

    public List<ItemTemplate> getItems() {
        return items;
    }

    public static void main(String[] args) {
        try {
            Items.load();
            System.err.println("Loaded " + getInstance().getItems().size() + " items.");
        } catch(Exception exc) {
            exc.printStackTrace();
        }
    }
}
