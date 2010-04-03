package org.scourge.editor;

import org.scourge.terrain.Region;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;

/**
 * User: gabor
 * Date: Mar 29, 2010
 * Time: 9:13:45 PM
 */
public class Editor extends JFrame implements MapEditorListener {
    private MapEditor mapEditor;
    private JLabel position = new JLabel();
    private JLabel key = new JLabel();
    private JButton save = new JButton("Save Map");

    public Editor() throws IOException {
        super("Scourge II Editor");

        mapEditor = new MapEditor();
        mapEditor.loadMap();
        JScrollPane sp = new JScrollPane(mapEditor);
        sp.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        sp.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        sp.getHorizontalScrollBar().setUnitIncrement(MapEditor.CHAR_WIDTH);
        sp.getVerticalScrollBar().setUnitIncrement(MapEditor.CHAR_HEIGHT);
        mapEditor.setScrollPane(sp);
        setLayout(new BorderLayout());
        add(sp, BorderLayout.CENTER);

        JPanel p = new JPanel(new GridLayout(1, 2, 5, 5));
        p.add(position);
        p.add(key);
        add(p, BorderLayout.SOUTH);
        mapEditor.addListener(this);

        JPanel p2 = new JPanel(new BorderLayout());
        p2.add(save, BorderLayout.WEST);
        add(p2, BorderLayout.NORTH);
        save.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                try {
                    mapEditor.saveMap();
                    mapEditor.grabFocus();
                } catch(IOException exc) {
                    exc.printStackTrace();
                }
            }
        });

        pack();
        setSize(800, 600);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setLocationRelativeTo(null);
        mapEditor.grabFocus();
    }

    public static void main(String[] args) {
        try {
            new Editor().setVisible(true);
        } catch (IOException exc) {
            exc.printStackTrace();
        }
    }

    @Override
    public void mapScrolled(int sx, int sy, int ex, int ey, int cursorX, int cursorY) {
        position.setText("" + sx + "," + sy + "-" + ex + "," + ey + " cursor=" + cursorX + "," + cursorY + " (" + (cursorX / Region.REGION_SIZE) + "," + (cursorY / Region.REGION_SIZE) + ")");
    }

    @Override
    public void keyChanged(char key) {
        this.key.setText("Key=" + key);
    }
}
