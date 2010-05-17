package org.scourge.ui.component;

import org.scourge.ui.component.Dragable;

import java.awt.*;
import java.awt.geom.Point2D;

/**
 * User: gabor
 * Date: Apr 11, 2010
 * Time: 9:49:23 PM
 */
public interface WindowListener {
    /**
     * A button was clicked.
     * @param name the component's name
     */
    public void buttonClicked(String name);

    /**
     * Start the drag-n-drop operation.
     * @param name the active component
     * @param point coordinates relative to the top left of the component
     * @return the object being dragged
     */
    Dragable drag(String name, Point2D point);

    /**
     * End the drag-n-drop operation.
     * @param name the component's name
     * @param point coordinates relative to the top left of the component
     * @param dragging the dragged object
     * @return true if the drop succeeded, false otherwise
     */
    boolean drop(String name, Point2D point, Dragable dragging);

    DragSource getDragSource();
}
