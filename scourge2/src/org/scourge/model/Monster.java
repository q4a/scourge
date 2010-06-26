package org.scourge.model;

import com.jme.bounding.BoundingBox;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import org.scourge.terrain.Md2Model;
import org.scourge.terrain.NodeGenerator;

/**
 * User: gabor
 * Date: Jun 15, 2010
 * Time: 8:21:39 AM
 */
public enum Monster {
    shade("./data/models/phantom/tris.md2", "./data/models/phantom/m10.png");

    private String modelPath, skinPath;

    Monster(String modelPath, String skinPath) {
        this.modelPath = modelPath;
        this.skinPath = skinPath;
    }

    public String getModelPath() {
        return modelPath;
    }

    public String getSkinPath() {
        return skinPath;
    }

    public Md2Model createModel() {
        Md2Model model = new Md2Model(modelPath, skinPath, name());
        model.setKeyFrame(Md2Model.Md2Key.stand);

        // create world bounds, etc.
        model.getNode().updateGeometricState(0,true);

        return model;
    }
}
