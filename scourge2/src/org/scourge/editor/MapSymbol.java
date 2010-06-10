package org.scourge.editor;

import java.awt.*;
import java.util.HashMap;
import java.util.Map;

/**
 * User: gabor
 * Date: Apr 9, 2010
 * Time: 9:13:12 PM
 */
public enum MapSymbol {
    water('~', Color.blue),
    ground('*', Color.green),
    tree('F', new Color(0x00, 0xf0, 0x80)),
    house('H', new Color(0x80, 0xff, 0x00)),
    bridge('B', new Color(0xff, 0xe0, 0x00)),
    road('x', new Color(0xff, 0x80, 0x00)),
    paved_road('X', new Color(0xff, 0xe0, 0x00)),
    ramp('L', Color.magenta),
    gate('g', Color.cyan),
    sign('s', new Color(0xff, 0x30, 0x30), new String[] { "label", "label2" }),
    up('u', Color.PINK, new String[] { "down_location" }),
    down('d', Color.PINK, new String[] { "up_location" }),
    room('o', new Color(0xff, 0x80, 0x00)),
    room2('O', new Color(0xff, 0x80, 0x00)),
    room3('q', new Color(0xff, 0x80, 0x00)),
    room4('Q', new Color(0xff, 0x80, 0x00)),
    room5('z', new Color(0xff, 0x80, 0x00)),
    ;

    private char c;
    private Color color;
    private String[] blockDataKeys;
    private final static Map<Character, MapSymbol> chars = new HashMap<Character, MapSymbol>();

    static {
        for(MapSymbol ms : MapSymbol.values()) {
            chars.put(ms.getC(), ms);
        }
    }

    MapSymbol(char c, Color color) {
        this(c, color, new String[0]);
    }

    MapSymbol(char c, Color color, String[] blockDataKeys) {
        this.c = c;
        this.color = color;
        this.blockDataKeys = blockDataKeys;
    }

    public char getC() {
        return c;
    }

    public Color getColor() {
        return color;
    }

    public static MapSymbol find(char c) {
        return chars.get(c);
    }

    public String[] getBlockDataKeys() {
        return blockDataKeys;
    }
}
