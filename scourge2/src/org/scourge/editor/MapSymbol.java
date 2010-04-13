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
    ramp('L', Color.magenta);

    private char c;
    private Color color;
    private final static Map<Character, MapSymbol> chars = new HashMap<Character, MapSymbol>();

    static {
        for(MapSymbol ms : MapSymbol.values()) {
            chars.put(ms.getC(), ms);
        }
    }

    MapSymbol(char c, Color color) {
        this.c = c;
        this.color = color;
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
}