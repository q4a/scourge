package org.scourge.editor;

import org.scourge.io.BlockData;
import org.scourge.util.StringUtil;

import javax.swing.*;
import javax.swing.table.AbstractTableModel;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

/**
 * User: gabor
 * Date: May 29, 2010
 * Time: 2:42:15 PM
 */
public class BlockDataEditor extends JDialog {
    private JTable table;
    private BlockModel model = new BlockModel();

    public BlockDataEditor(Editor editor, BlockData blockData) {
        super(editor, "Property Editor", true);
        
        model.setBlockData(blockData);
        table = new JTable(model);
        setLayout(new BorderLayout());
        add(new JScrollPane(table), BorderLayout.CENTER);
        JPanel p = new JPanel(new GridLayout(1, 2, 5, 5));
        JButton ok = new JButton("OK");
        ok.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                model.save();
                setVisible(false);
            }
        });
        p.add(ok);
        JButton cancel = new JButton("Cancel");
        cancel.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                setVisible(false);
            }
        });
        p.add(cancel);
        JPanel pp = new JPanel(new BorderLayout());
        pp.add(p, BorderLayout.EAST);

        JPanel p2 = new JPanel(new GridLayout(1, 2, 5, 5));
        JButton add = new JButton("+");
        add.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                model.addRow();
                if(model.getRowCount() > 0) {
                    table.setRowSelectionInterval(model.getRowCount() - 1, model.getRowCount() - 1);
                }
            }
        });
        p2.add(add);
        JButton rem = new JButton("-");
        rem.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent actionEvent) {
                int row = table.getSelectedRow();
                model.removeRow(row);
                if(model.getRowCount() > 0) {
                    row = model.getRowCount() >= row ? model.getRowCount() - 1 : row;
                    table.setRowSelectionInterval(row, row);
                }
            }
        });
        p2.add(rem);
        pp.add(p2, BorderLayout.WEST);

        add(pp, BorderLayout.SOUTH);
        pack();
        setSize(350, 350);
        this.setLocationRelativeTo(editor);
        setVisible(true);
    }

    private class BlockModel extends AbstractTableModel {
        private ArrayList<String[]> list = new ArrayList<String[]>();
        private BlockData data;

        @Override
        public int getRowCount() {
            return list == null ? 0 : list.size();
        }

        @Override
        public int getColumnCount() {
            return 2;
        }

        @Override
        public Object getValueAt(int row, int col) {
            return list == null ? "" : list.get(row)[col];
        }

        @Override
        public boolean isCellEditable(int row, int col) {
            return list != null;
        }

        @Override
        public void setValueAt(Object o, int row, int col) {
            list.get(row)[col] = (String)o;
        }

        @Override
        public String getColumnName(int col) {
            return col == 0 ? "key" : "value";
        }

        public void addRow() {
            if(list != null) {
                list.add(new String[] { "", "" });
                refresh();
            }
        }

        public void removeRow(int row) {
            if(list != null && row >= 0 && row < getRowCount()) {
                list.remove(row);
                refresh();
            }
        }

        public void setBlockData(BlockData data) {
            this.data = data;
            list = new ArrayList<String[]>();
            for(String key : data.getData().keySet()) {
                list.add(new String[] { key, data.getData().get(key) });
            }
            refresh();
        }

        protected void refresh() {
            if(table != null) {
                fireTableDataChanged();
                table.updateUI();
                table.repaint();
            }
        }

        public void save() {
            if(list != null) {
                Map<String, String> h = new HashMap<String, String>();
                for(String[] s : list) {
                    if(!StringUtil.isEmpty(s[0]) && !StringUtil.isEmpty(s[1])) {
                        h.put(s[0], s[1]);
                    }
                }
                if(!h.isEmpty()) {
                    data.setData(h);
                }
            }
        }
    }
}
