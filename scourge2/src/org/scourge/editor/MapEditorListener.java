package org.scourge.editor;

/**
 * User: gabor
 * Date: Mar 31, 2010
 * Time: 3:12:18 PM
 */
public interface MapEditorListener {
    void mapScrolled(int sx, int sy, int ex, int ey, int cursorX, int cursorY);
    void keyChanged(char key);
}
