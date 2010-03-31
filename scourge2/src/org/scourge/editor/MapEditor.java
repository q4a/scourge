package org.scourge.editor;

import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
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
    private int rows = 1000, cols = 1000;
    private List<String> lines;
    static final int CHAR_HEIGHT = 11;
    static final int CHAR_WIDTH = 11;

    private static Map<Character, Color> colors = new HashMap<Character, Color>();
    private JScrollPane scrollPane;

    static {
        colors.put('~', Color.blue);
        colors.put('*', Color.green);
        colors.put('B', new Color(0xff, 0xe0, 0x00));
        colors.put('x', new Color(0xff, 0xe0, 0x00));
        colors.put('F', new Color(0x00, 0xf0, 0x80));
        colors.put('H', new Color(0x80, 0xff, 0x00));
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


        lines = new ArrayList<String>(rows);
        for(int y = 0; y < rows; y++) {
            StringBuilder sb = new StringBuilder();
            for(int x = 0; x < cols; x++) {
                int red = (pixels[y * cols + x] & 0x00FF0000) >> 16;
                int green = (pixels[y * cols + x] & 0x0000FF00) >> 8;
                int blue = (pixels[y * cols + x] & 0x000000FF);

                
                sb.append(blue > 0 ? "*" : "~");
            }
            lines.add(sb.toString());
        }
        setPreferredSize(new Dimension(cols * CHAR_WIDTH, rows * CHAR_HEIGHT));
    }

    public void importMap(File map) throws IOException {
        rows = 1000;
        cols = 1000;
        //noinspection unchecked
        lines = readLines(map);
        int r = lines.size();
        int c = lines.get(0).length();

        // expand map
        StringBuilder sb = new StringBuilder("");
        for(int x = c; x < cols; x++) {
            sb.append("~");
        }
        for(int y = 0; y < r; y++) {
            lines.set(y, lines.get(y) + sb.toString());
        }

        sb = new StringBuilder("");
        for(int x = 0; x < cols; x++) {
            sb.append("~");
        }
        for(int y = r; y < rows; y++) {
            lines.add(sb.toString());
        }
        setPreferredSize(new Dimension(cols * CHAR_WIDTH, rows * CHAR_HEIGHT));
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
            String line = lines.get(y);

            for(int x = sx; x < ex; x++) {
                if(x < 0 || x >= cols) continue;

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

    public void setScrollPane(JScrollPane sp) {
        this.scrollPane = sp;
    }
}
