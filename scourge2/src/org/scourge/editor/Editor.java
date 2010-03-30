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
        super("Scourge II editor");

        JPanel panel = new JPanel(new BorderLayout());
        panel.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

        mapEditor = new MapEditor(new File("./data/maps/m32m32.map"));
        panel.add(new JScrollPane(mapEditor));
        getContentPane().add(panel);

        pack();
        // setSize(panel.getWidth(), panel.getHeight());
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setVisible(true);
    }

    public static void main(String[] args) {
        try {
            new Editor();
        } catch (IOException exc) {
            exc.printStackTrace();
        }
    }

}
