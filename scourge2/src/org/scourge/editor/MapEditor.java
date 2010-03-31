package org.scourge.editor;

import com.sun.tools.javac.resources.version;
import org.scourge.Climate;

import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.io.*;
import java.util.HashMap;
import java.util.Map;

import static org.apache.commons.io.FileUtils.readLines;

/**
 * User: gabor
 * Date: Mar 29, 2010
 * Time: 9:16:59 PM
 */
public class MapEditor extends JPanel {
    private int version = 1;
    private int rows = 1000, cols = 1000;
    int[][] point;
    static final int CHAR_HEIGHT = 11;
    static final int CHAR_WIDTH = 11;

    private static Map<Character, Color> colors = new HashMap<Character, Color>();
    private static Map<Character, Color> backgrounds = new HashMap<Character, Color>();
    private JScrollPane scrollPane;

    static {
        colors.put('~', Color.blue);
        colors.put('*', Color.green);
        colors.put('B', new Color(0xff, 0xe0, 0x00));
        colors.put('x', new Color(0xff, 0xe0, 0x00));
        colors.put('F', new Color(0x00, 0xf0, 0x80));
        colors.put('H', new Color(0x80, 0xff, 0x00));

        backgrounds.put((char)Climate.alpine.ordinal(), new Color(0x40, 0x35, 0x00));
        backgrounds.put((char)Climate.boreal.ordinal(), new Color(0x00, 0x40, 0x00));
        backgrounds.put((char)Climate.temperate.ordinal(), new Color(0x00, 0x40, 0x35));
        backgrounds.put((char)Climate.subtropical.ordinal(), new Color(0x40, 0x40, 0x35));
        backgrounds.put((char)Climate.tropical.ordinal(), new Color(0x40, 0x00, 0x00));

    }

    public void importPng(File png) throws IOException {
        System.err.println("Importing png: " + png + "...");
        ImageIcon image = new ImageIcon(png.getPath());
        cols = image.getIconWidth();
        rows = image.getIconHeight();
        System.err.println("size: " + cols + "x" + rows);
        BufferedImage img = new BufferedImage(cols, rows, BufferedImage.TYPE_INT_RGB);
        Graphics2D g = (Graphics2D) img.getGraphics();
        g.drawImage(image.getImage(), null, null);
        g.dispose();

        DataBufferInt data = (DataBufferInt) img.getRaster().getDataBuffer();
        int[] pixels = data.getData();


        point = new int[rows][cols];
        for(int y = 0; y < rows; y++) {
            StringBuilder sb = new StringBuilder();
            for(int x = 0; x < cols; x++) {
                int red = (pixels[y * cols + x] & 0x00FF0000) >> 16;
                int green = (pixels[y * cols + x] & 0x0000FF00) >> 8;
                int blue = (pixels[y * cols + x] & 0x000000FF);

                // green is: 50-wastelands, 100-groves, 150-light forest, 200-deep forest
                // blue: 50-alpine, 100-boreal, 150-temperate, 200-subtropical, 250-tropical
                if(blue == 0) {
                    point[y][x] = '~';
                } else {
                    point[y][x] = ((int)((20 - (green / 10)) * Math.random()) == 0 ? 'F' : '*');
                    point[y][x] += (blue / 50 << 8);
                }
            }
        }

        saveMap();

        setPreferredSize(new Dimension(cols * CHAR_WIDTH, rows * CHAR_HEIGHT));
    }

    public void saveMap() throws IOException {
        String path = "./data/maps/land.bin";
        System.err.println("Saving " + path + "...");
        DataOutputStream out = new DataOutputStream(new FileOutputStream(path));
        // header
        out.writeInt(version);
        out.writeInt(rows);
        out.writeInt(cols);

        // body
        for(int y = 0; y < rows; y++) {
            for(int x = 0; x < cols; x++) {
                out.writeInt(point[y][x]);
            }
        }
        out.close();
        System.err.println("Map saved.");
    }

    @Override
    protected void paintComponent(Graphics g) {
        // why is this so hard?!
        int xx = scrollPane.getHorizontalScrollBar().getValue();
        int yy = scrollPane.getVerticalScrollBar().getValue();
        Rectangle r = scrollPane.getVisibleRect();
        g.setColor(Color.black);
        g.fillRect(xx, yy, r.width, r.height);
        int sx = xx / CHAR_WIDTH;
        int ex = sx + r.width / CHAR_WIDTH;
        int sy = yy / CHAR_HEIGHT;
        int ey = sy + r.height / CHAR_HEIGHT;
        // System.err.println("drawing: " + sx + "," + sy + " - " + ex + "," + ey);
        for(int y = sy; y < ey; y++) {
            if(y < 0 || y >= rows) continue;
            int[] line = point[y];

            for(int x = sx; x < ex; x++) {
                if(x < 0 || x >= cols) continue;

                char c = (char)(line[x] & 0xff);
                int bg = (line[x] & 0xff00) >> 8;
                Color color = colors.get(c);
                if(color == null) color = Color.gray;
                Color background = backgrounds.get((char)bg);
                if(background == null) background = Color.black;
                g.setColor(background);
                g.fillRect(x * CHAR_WIDTH, y * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);
                g.setColor(color);
                g.drawString("" + c, x * CHAR_WIDTH, (y + 1) * CHAR_HEIGHT);
            }
        }
    }

    public void setScrollPane(JScrollPane sp) {
        this.scrollPane = sp;
    }
}
