package org.scourge.editor;

import org.scourge.Climate;
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
    private JLabel point = new JLabel();
    private JComboBox mapSymbol = new JComboBox(MapSymbol.values());
    private JComboBox climate = new JComboBox(Climate.values());
    private JComboBox level = new JComboBox(new String[] { "ground", "level 1", "level 2", "level 3", "level 4"});
    private JCheckBox lockClimate = new JCheckBox("Lock");

    public Editor() throws IOException {
        super("Scourge II Editor");

        mapEditor = new MapEditor(this);
        mapEditor.loadMap();
        JScrollPane sp = new JScrollPane(mapEditor);
        sp.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        sp.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        sp.getHorizontalScrollBar().setUnitIncrement(MapEditor.CHAR_WIDTH);
        sp.getVerticalScrollBar().setUnitIncrement(MapEditor.CHAR_HEIGHT);
        mapEditor.setScrollPane(sp);

        JPanel south = new JPanel(new GridLayout(1, 2, 5, 5));
        south.add(position);
        south.add(point);

        JPanel north = new JPanel(new BorderLayout());
        JButton save = new JButton("Save Map");
        north.add(save, BorderLayout.WEST);
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

        lockClimate.setSelected(true);
        JPanel right = new JPanel(new BorderLayout());
        JPanel pp = new JPanel(new GridLayout(4, 2));
        pp.add(new JLabel("Symbol:"));
        pp.add(mapSymbol);
        pp.add(new JLabel("Climate:"));
        pp.add(climate);
        pp.add(new JLabel("Lock climate?"));
        pp.add(lockClimate);
        pp.add(new JLabel("Level:"));
        pp.add(level);
        pp.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5),
                                                        BorderFactory.createTitledBorder("Properties")));
        right.add(pp, BorderLayout.NORTH);

        JSplitPane split = new JSplitPane();
        split.setLeftComponent(sp);
        split.setRightComponent(right);
        split.setContinuousLayout(true);
        split.setOneTouchExpandable(true);
        split.setDividerLocation(800);
        setLayout(new BorderLayout());
        add(split, BorderLayout.CENTER);
        add(north, BorderLayout.NORTH);
        add(south, BorderLayout.SOUTH);

        pack();
        setSize(1024, 768);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        this.setLocationRelativeTo(null);
        mapEditor.grabFocus();

        mapEditor.addListener(this);
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
        position.setText("" + sx + "," + sy + "-" + ex + "," + ey +
                         " cursor=" + cursorX + "," + cursorY + " (" + (cursorX / Region.REGION_SIZE) + "," + (cursorY / Region.REGION_SIZE) + ")");
        point.setText(mapEditor.getMapSymbol(cursorX, cursorY) + "," +
                      mapEditor.getClimate(cursorX, cursorY) + "," +
                      mapEditor.getLevel(cursorX, cursorY));
    }

    public MapSymbol getMapSymbol() {
        return (MapSymbol)mapSymbol.getSelectedItem();
    }

    public Climate getClimate() {
        return (Climate)climate.getSelectedItem();
    }

    public boolean isClimateLocked() {
        return lockClimate.isSelected();
    }

    public int getLevel() {
        return level.getSelectedIndex();
    }
}
