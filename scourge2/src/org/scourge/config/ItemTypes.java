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
public class ItemTypes {
    @ElementList(name="types")
    private List<ItemType> types = new ArrayList<ItemType>();

    @ElementList(entry="sounds", inline=true, type = String.class)
    private List<String> sounds;

    @ElementList(name="tags")
    private List<ItemTag> tags = new ArrayList<ItemTag>();

    private static ItemTypes itemTypes;

    public static void load() throws Exception {
        Serializer serializer = new Persister();
        Reader reader = new BufferedReader(new FileReader("./data/config/itemtypes.xml"));
        ItemTypes.itemTypes = serializer.read(ItemTypes.class, reader);
        reader.close();
    }

    @Override
    public String toString() {
        return "types=" + types + " sounds=" + sounds + " tags=" + tags;
    }

    public static void main(String[] args) {
        try {
            ItemTypes.load();
            System.err.println(ItemTypes.itemTypes);
        } catch(Exception exc) {
            exc.printStackTrace();
        }
    }
}
