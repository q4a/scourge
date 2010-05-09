package org.scourge.config;

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
 * Date: May 8, 2010
 * Time: 1:23:31 PM
 */
@Root
public class Models {

    @ElementList(inline=true)
    private List<Model> models = new ArrayList<Model>();

    private static Models instance;
    private Map<String, Model> modelsByName = new HashMap<String, Model>();

    public static void load() throws Exception {
        Serializer serializer = new Persister();
        Reader reader = new BufferedReader(new FileReader("./data/config/models.xml"));
        instance = serializer.read(Models.class, reader);
        reader.close();
        for(Model model : instance.models) {
            instance.modelsByName.put(model.getName(), model);
        }
    }

    public List<Model> getModels() {
        return models;
    }

    public static Models getInstance() {
        return instance;
    }

    public Model getModel(String name) {
        return modelsByName.get(name);
    }
}
