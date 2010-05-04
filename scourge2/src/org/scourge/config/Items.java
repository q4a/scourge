package org.scourge.config;

import org.scourge.ui.ProgressListener;
import org.simpleframework.xml.ElementList;
import org.simpleframework.xml.Root;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.Reader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
    private Map<String, ItemTemplate> itemsByName = new HashMap<String, ItemTemplate>();

    public static void load(ProgressListener progressListener) throws Exception {
        String[] files = {
                "./data/config/weapon.xml",
                "./data/config/armor.xml",
                "./data/config/magicitem.xml",
                "./data/config/otheritem.xml"
        };
        int count = 0;
        float max = files.length + 1;
        for(String file : files) {
            instance.items.addAll(loadItems(file).items);
            progressListener.progress((count++) / max);
        }
        for(ItemTemplate item : instance.items) {
            instance.itemsByName.put(item.getName(), item);
        }
        progressListener.done();
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

    public ItemTemplate getItem(String name) {
        return itemsByName.get(name);
    }
}
