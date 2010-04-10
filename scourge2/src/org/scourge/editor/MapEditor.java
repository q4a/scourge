package org.scourge.editor;

import org.apache.commons.io.FileUtils;
import org.scourge.Climate;
import org.scourge.io.MapIO;
import org.scourge.terrain.Region;

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.zip.GZIPOutputStream;

import static org.apache.commons.io.FileUtils.readLines;

/**
 * User: gabor
 * Date: Mar 29, 2010
 * Time: 9:16:59 PM
 */
public class MapEditor extends JPanel {
    private int version = 1;
    private int rows = 1000, cols = 1000;
    private int[][] point;
    public static final int CHAR_HEIGHT = 11;
    public static final int CHAR_WIDTH = 11;
    private int cursorX = 0, cursorY = 0;
    private int sx, sy, ex, ey;
    private JScrollPane scrollPane;
    private java.util.List<MapEditorListener> listeners = new ArrayList<MapEditorListener>();
    private Editor editor;
    private static final Map<Character, Color> backgrounds = new HashMap<Character, Color>();

    // try to pick pairs of opposite colors
    public static final Color[] borders = new Color[] {
            Color.white,
            Color.yellow,
            Color.blue,
            Color.red,
            Color.green,
            Color.magenta,
            Color.cyan
    };

    static {
        backgrounds.put((char)Climate.alpine.ordinal(), new Color(0x40, 0x35, 0x00));
        backgrounds.put((char)Climate.boreal.ordinal(), new Color(0x00, 0x40, 0x00));
        backgrounds.put((char)Climate.temperate.ordinal(), new Color(0x00, 0x40, 0x35));
        backgrounds.put((char)Climate.subtropical.ordinal(), new Color(0x40, 0x40, 0x35));
        backgrounds.put((char)Climate.tropical.ordinal(), new Color(0x40, 0x00, 0x00));

    }

    public MapEditor(Editor editor) {
        this.editor = editor;
        addMouseMotionListener(new MouseMotionAdapter() {

            @Override
            public void mouseDragged(MouseEvent mouseEvent) {
                updatePoint(mouseEvent);
            }

            @Override
            public void mouseMoved(MouseEvent mouseEvent) {
                moveCursor(mouseEvent);
            }
        });

        addMouseListener(new MouseAdapter() {

            @Override
            public void mouseClicked(MouseEvent mouseEvent) {
                updatePoint(mouseEvent);
            }
        });

        setFocusTraversalKeysEnabled(false);
        requestFocusInWindow();
        requestFocus();
    }

    private void moveCursor(MouseEvent mouseEvent) {
        cursorX = mouseEvent.getX() / CHAR_WIDTH;
        cursorY = mouseEvent.getY() / CHAR_HEIGHT;
//                System.err.println("event=" + mouseEvent.getX() + "," + mouseEvent.getY() +
//                                   " cursor=" + cursorX + "," + cursorY);
        repaint();
    }

    private void updatePoint(MouseEvent mouseEvent) {
        cursorX = mouseEvent.getX() / CHAR_WIDTH;
        cursorY = mouseEvent.getY() / CHAR_HEIGHT;

        for(int x = cursorX; x < cursorX + editor.getBrush().getW(); x++) {
            for(int y = cursorY; y < cursorY + editor.getBrush().getH(); y++) {
                if(editor.getBrush().isRandom() && 0 < (int)(Math.random() * 4.0f)) {
                    continue;
                }
                
                point[y][x] = ((editor.isLevelLocked() ? getLevel(cursorX, cursorY) : editor.getLevel()) << 16) +
                              ((editor.isClimateLocked() ? getClimate(cursorX, cursorY) : editor.getClimate()).ordinal() << 8) +
                              (editor.isSymbolLocked() ? getMapSymbol(cursorX, cursorY) : editor.getMapSymbol()).getC();            
            }
        }
        repaint();
    }

    public void loadMap() throws IOException {
        if(!MapIO.GZIP_FILE.exists()) {
            importPng(new File("/Users/gabor/scourge/trunk/scourge_data/mapgrid/world/map.png"));
        }
        
        MapIO mapIO = new MapIO();
        rows = mapIO.getRows();
        cols = mapIO.getCols();
        point = mapIO.readRaw();
        setPreferredSize(new Dimension(cols * CHAR_WIDTH, rows * CHAR_HEIGHT));
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
        String path = "./data/maps/land.bin.gz";
        System.err.println("Saving " + path + "...");
        DataOutputStream out = new DataOutputStream(new GZIPOutputStream(new FileOutputStream(path)));

        // header
        out.writeInt(version);
        out.writeInt(rows);
        out.writeInt(cols);

        // body: todo: speed this up like MapIO
        for(int y = 0; y < rows; y++) {
            for(int x = 0; x < cols; x++) {
                out.writeInt(point[y][x]);
            }
        }
        out.close();
        System.err.println("Map saved.");

        // remove the unzipped map
        FileUtils.deleteQuietly(new File("./data/maps/land.bin"));
    }

    public MapSymbol getMapSymbol(int x, int y) {
        return MapSymbol.find((char)(point[y][x] & 0xff));
    }

    public Climate getClimate(int x, int y) {
        return Climate.values()[(point[y][x] & 0xff00) >> 8];
    }

    public int getLevel(int x, int y) {
        return (point[y][x] & 0xff0000) >> 16; 
    }


    @Override
    protected void paintComponent(Graphics g) {
        // why is this so hard?!
        int xx = scrollPane.getHorizontalScrollBar().getValue();
        int yy = scrollPane.getVerticalScrollBar().getValue();
        Rectangle r = scrollPane.getVisibleRect();
        g.setColor(Color.black);
        g.fillRect(xx, yy, r.width, r.height);

        sx = xx / CHAR_WIDTH;
        ex = sx + r.width / CHAR_WIDTH;
        sy = yy / CHAR_HEIGHT;
        ey = sy + r.height / CHAR_HEIGHT;
        fireEvent(sx, sy, ex, ey);

        for(int yp = sy; yp < ey; yp++) {
            if(yp < 0 || yp >= rows) continue;
            int[] line = point[yp];

            for(int xp = sx; xp < ex; xp++) {
                if(xp < 0 || xp >= cols) continue;

                int bg = (line[xp] & 0xff00) >> 8;
                Color background = backgrounds.get((char)bg);
                if(background == null) background = Color.black;
                g.setColor(background);
                g.fillRect(xp * CHAR_WIDTH, yp * CHAR_HEIGHT, CHAR_WIDTH, CHAR_HEIGHT);

                int level = (line[xp] & 0xff0000) >> 16;
                if(level > 0) {
                    Color borderColor = borders[level];
                    g.setColor(borderColor);
                    g.drawRect(xp * CHAR_WIDTH, yp * CHAR_HEIGHT, CHAR_WIDTH - 1, CHAR_HEIGHT - 1);
                }

                char c = (char)(line[xp] & 0xff);
                MapSymbol symbol = MapSymbol.find(c);
                Color color = symbol == null ? Color.gray : symbol.getColor();
                g.setColor(color);
                g.drawString("" + c, xp * CHAR_WIDTH, (yp + 1) * CHAR_HEIGHT);
            }
        }

        // draw the region lines
        g.setColor(Color.green);
        for(int yp = sy; yp < ey; yp++) {
            if(yp % Region.REGION_SIZE == 0) {
                int ypos = yp * CHAR_HEIGHT;
                g.drawLine(sx * CHAR_WIDTH, ypos, sx * CHAR_WIDTH + r.width, ypos);
            }
        }

        for(int xp = sx; xp < ex; xp++) {
            if(xp % Region.REGION_SIZE == 0) {
                int xpos = xp * CHAR_WIDTH;
                g.drawLine(xpos, sy * CHAR_HEIGHT, xpos, sy * CHAR_HEIGHT + r.height);
            }
        }

        // cursor
        g.setColor(Color.yellow);
        g.drawRect(cursorX * CHAR_WIDTH, cursorY * CHAR_HEIGHT,
                   (editor.getBrush().getW() * CHAR_WIDTH) - 1,
                   (editor.getBrush().getH() * CHAR_HEIGHT) - 1);
    }

    private void fireEvent(int sx, int sy, int ex, int ey) {
        for(MapEditorListener listener : listeners) {
            listener.mapScrolled(sx, sy, ex, ey, cursorX, cursorY);
        }
    }

    public void addListener(MapEditorListener listener) {
        listeners.add(listener);
        listener.mapScrolled(sx, sy, ex, ey, cursorX, cursorY);
    }

    public void setScrollPane(JScrollPane sp) {
        this.scrollPane = sp;
    }

    @Override
    public boolean isFocusable() {
        return true;
    }

    public void grabFocus() {
        requestFocusInWindow();
        requestFocus();
    }
}
