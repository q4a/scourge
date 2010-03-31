package org.scourge.editor;

import javax.swing.*;
import java.awt.*;
import java.io.File;
import java.io.IOException;

/**
 * User: gabor
 * Date: Mar 29, 2010
 * Time: 9:13:45 PM
 */
public class Editor extends JFrame {
    private MapEditor mapEditor;

    public Editor() throws IOException {
        super("Scourge II Editor");

        mapEditor = new MapEditor();
        mapEditor.importPng(new File("/Users/gabor/scourge/trunk/scourge_data/mapgrid/world/map.png"));
        JScrollPane sp = new JScrollPane(mapEditor);
        sp.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        sp.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        sp.getHorizontalScrollBar().setUnitIncrement(MapEditor.CHAR_WIDTH);
        sp.getVerticalScrollBar().setUnitIncrement(MapEditor.CHAR_HEIGHT);
        mapEditor.setScrollPane(sp);
        setLayout(new BorderLayout());
        add(sp, BorderLayout.CENTER);

        pack();
        setSize(400, 400);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setLocationRelativeTo(null);
    }

    public static void main(String[] args) {
        try {
            new Editor().setVisible(true);
        } catch (IOException exc) {
            exc.printStackTrace();
        }
    }

}
