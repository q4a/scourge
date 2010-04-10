package org.scourge.editor;

import org.scourge.Climate;
import org.scourge.terrain.Region;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;
import java.util.Vector;

/**
 * User: gabor
 * Date: Mar 29, 2010
 * Time: 9:13:45 PM
 */
public class Editor extends JFrame implements MapEditorListener {
    private MapEditor mapEditor;
    private JLabel position = new JLabel();
    private JLabel point = new JLabel();
    private JComboBox mapSymbol = new JComboBox();
    private JComboBox climate = new JComboBox(Climate.values());
    private JComboBox level = new JComboBox();
    private JCheckBox lockSymbol = new JCheckBox("Lock");
    private JCheckBox lockClimate = new JCheckBox("Lock");
    private JCheckBox lockLevel = new JCheckBox("Lock");
    private Brush brush = Brush.single;
    private static final String HOW_TO_TEXT = "Notes on the map:\n" +
                                              "\n" +
                                              "The map has some limitations, including:\n" +
                                              "- Make sure there is 1 tile space on each side of a house otherwise the door may not be accessible.\n" +
                                              "- Do not put houses across or at the edge of a region boundary.\n" +
                                              "- You can only have a single-level cliff edge. If you want to make a cliff at level 4, levels 1,2,3 must be in front of it like a staircase. " +
                                              "In other words, you can not put two levels next to each other where the difference in levels is more than 1.\n" +
                                              "- Water tiles can only go on the ground level.\n" +
                                              "- Bridges can only span 1 water tile.\n" +
                                              "- There is no undo, don't forget to save!\n" +
                                              "- Ramps should be right next to the cliff and need at least 2x3 space. See map for examples.\n" +
                                              "\nSome of these limitations may be fixed in later versions.";

    public Editor() throws IOException {
        super("Scourge II Editor");

        Vector<String> levels = new Vector<String>();
        for(int i = 0; i < MapEditor.borders.length; i++) {
            if(i == 0) {
                levels.add("Ground level");
            } else {
                levels.add("Level " + i);
            }
        }
        level.setModel(new DefaultComboBoxModel(levels));

        Vector<String> symbols = new Vector<String>();
        for(MapSymbol symbol : MapSymbol.values()) {
            symbols.add(symbol.name() + " (" + symbol.getC() + ")");
        }
        mapSymbol.setModel(new DefaultComboBoxModel(symbols));

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
        lockLevel.setSelected(true);
        JPanel right = new JPanel(new BorderLayout());
        JPanel rightTop = new JPanel();
        right.add(rightTop, BorderLayout.NORTH);
        rightTop.setLayout(new BoxLayout(rightTop, BoxLayout.Y_AXIS));
        JPanel pp = new JPanel(new GridLayout(6, 2));
        pp.add(new JLabel("Symbol:"));
        pp.add(mapSymbol);
        pp.add(new JLabel("Lock symbol?"));
        pp.add(lockSymbol);
        pp.add(new JLabel("Climate:"));
        pp.add(climate);
        pp.add(new JLabel("Lock climate?"));
        pp.add(lockClimate);
        pp.add(new JLabel("Level:"));
        pp.add(level);
        pp.add(new JLabel("Lock level?"));
        pp.add(lockLevel);
        pp.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5),
                                                        BorderFactory.createTitledBorder("Properties")));
        rightTop.add(pp);

        JPanel pp2 = new JPanel(new GridLayout(Brush.values().length, 2));
        pp2.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5),
                                                        BorderFactory.createTitledBorder("Brushes")));
        rightTop.add(pp2);
        final JToggleButton[] brushButtons = new JToggleButton[Brush.values().length];
        ActionListener listener = new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent event) {
                for(JToggleButton brushButton : brushButtons) {
                    if(brushButton != event.getSource()) {
                        brushButton.setSelected(!((JToggleButton) event.getSource()).isSelected());
                    } else {
                        brush = Brush.valueOf(brushButton.getText());
                    }
                }
            }
        };
        int i = 0;
        for(Brush brush : Brush.values()) {
            JToggleButton brushButton = new JToggleButton(brush.name());
            pp2.add(brushButton);
            brushButton.addActionListener(listener);
            if(i == 0) {
                brushButton.setSelected(true);
            }
            brushButtons[i++] = brushButton;            
        }

        JTextArea ta = new JTextArea(HOW_TO_TEXT);
        ta.setEditable(false);
        ta.setWrapStyleWord(true);
        ta.setLineWrap(true);
        ta.setFont(new Font(Font.SANS_SERIF, Font.PLAIN, 9));
        JScrollPane sp2 = new JScrollPane(ta);
        JPanel ppp = new JPanel(new BorderLayout());
        ppp.add(sp2);
        ppp.setBorder(BorderFactory.createCompoundBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5),
                                                        BorderFactory.createTitledBorder("Notes")));
        right.add(ppp);


        JSplitPane split = new JSplitPane();
        split.setLeftComponent(sp);
        split.setRightComponent(right);
        split.setContinuousLayout(true);
        split.setOneTouchExpandable(true);
        split.setDividerLocation(700);
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
        return MapSymbol.values()[mapSymbol.getSelectedIndex()];
    }

    public Climate getClimate() {
        return (Climate)climate.getSelectedItem();
    }

    public boolean isClimateLocked() {
        return lockClimate.isSelected();
    }

    public boolean isSymbolLocked() {
        return lockSymbol.isSelected();
    }

    public boolean isLevelLocked() {
        return lockLevel.isSelected();
    }

    public int getLevel() {
        return level.getSelectedIndex();
    }

    public Brush getBrush() {
        return brush;
    }
}
