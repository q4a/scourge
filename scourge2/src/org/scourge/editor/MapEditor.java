package org.scourge.editor;

import javax.swing.*;
import java.awt.*;
import java.io.File;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import static org.apache.commons.io.FileUtils.readLines;

/**
 * User: gabor
 * Date: Mar 29, 2010
 * Time: 9:16:59 PM
 */
public class MapEditor extends JPanel {
    private int rows, cols;
    private List<String> lines;
    private static final int CHAR_HEIGHT = 11;
    private static final int CHAR_WIDTH = 11;

    private static Map<Character, Color> colors = new HashMap<Character, Color>();
    private Dimension dimension;

    static {
        colors.put('~', Color.blue);
        colors.put('*', Color.green);
        colors.put('B', new Color(0xff, 0xe0, 0x00));
        colors.put('x', new Color(0xff, 0xe0, 0x00));
        colors.put('F', new Color(0x00, 0xf0, 0x80));
        colors.put('H', new Color(0x80, 0xff, 0x00));
    }

    public MapEditor(File map) throws IOException {
        //noinspection unchecked
        lines = readLines(map);
        rows = lines.size();
        cols = lines.get(0).length();
        dimension = new Dimension(getWidth(), getHeight());
    }

    @Override
    protected void paintComponent(Graphics g) {
        g.setColor(Color.black);
        g.fillRect(0, 0, getWidth(), getHeight());
        for(int y = 0; y < lines.size(); y++) {
            String line = lines.get(y);
            for(int x = 0; x < line.length(); x++) {
                char c = line.charAt(x);
                Color color = colors.get(c);
                if(color == null) color = Color.gray;
                Color background = new Color(color.getRed() / 4, color.getGreen() / 4, color.getBlue() / 4);
                g.setColor(background);
                g.fillRect(x * CHAR_WIDTH, y * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
                g.setColor(color);
                g.drawString("" + c, x * CHAR_WIDTH, (y + 1) * CHAR_HEIGHT);
            }
        }
    }

    @Override
    public int getWidth() {
        return cols * CHAR_WIDTH;
    }

    @Override
    public int getHeight() {
        return rows * CHAR_HEIGHT;
    }

    @Override
    public Dimension getMinimumSize() {
        return dimension;
    }

    @Override
    public Dimension getMaximumSize() {
        return dimension;
    }

    @Override
    public Dimension getPreferredSize() {
        return dimension;
    }
}
