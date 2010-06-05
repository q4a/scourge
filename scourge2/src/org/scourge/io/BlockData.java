package org.scourge.io;

import com.jme.util.export.JMEExporter;
import com.jme.util.export.JMEImporter;
import com.jme.util.export.Savable;
import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;
import org.simpleframework.xml.ElementMap;
import org.simpleframework.xml.Root;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

/**
 * User: gabor
 * Date: May 31, 2010
 * Time: 4:42:11 PM
 */
@Root(name = "block")
public class BlockData implements Savable {
    @Attribute
    private int x;

    @Attribute
    private int y;

    @ElementMap(entry = "data", key = "key", attribute = true, inline = true, required = false)
    private Map<String, String> data = new HashMap<String, String>();

    public BlockData() {
    }

    public BlockData(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public int getX() {
        return x;
    }

    public void setX(int x) {
        this.x = x;
    }

    public int getY() {
        return y;
    }

    public void setY(int y) {
        this.y = y;
    }

    public Map<String, String> getData() {
        return data;
    }

    public void setData(Map<String, String> data) {
        this.data = data;
    }

    public boolean isEmpty() {
        return data == null || data.isEmpty();
    }

    @Override
    public void write(JMEExporter jmeExporter) throws IOException {
        throw new RuntimeException("not used");
    }

    @Override
    public void read(JMEImporter jmeImporter) throws IOException {
        throw new RuntimeException("not used");
    }

    @Override
    public Class getClassTag() {
        throw new RuntimeException("not used");
    }
}
