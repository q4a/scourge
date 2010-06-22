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
public enum Monster implements HasModel {
    shade("./data/models/phantom/tris.md2", "./data/models/phantom/m10.png");

    private Md2Model model;

    Monster(String model, String skin) {
        this.model = new Md2Model(model, skin);
        this.model.setKeyFrame(Md2Model.Md2Key.stand);
    }

    @Override
    public Md2Model getCreatureModel() {
        return model;
    }
}
