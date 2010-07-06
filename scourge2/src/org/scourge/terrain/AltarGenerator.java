package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.scene.Spatial;
import org.scourge.Main;

/**
 * User: gabor
 * Date: Jul 5, 2010
 * Time: 9:25:15 AM
 */
public class AltarGenerator extends Generator {
    String type;
    private Spatial spatial;
    private boolean first = true;
    private boolean generated;

    public AltarGenerator(Region region, int x, int y, String type) {
        super(region, x, y);
        this.type = type;
        try {
            Model model = Model.valueOf(type);
            spatial = model.createSpatial();
            spatial.setModelBound(new BoundingBox());
            spatial.updateModelBound();
            spatial.updateWorldBound();
            spatial.updateGeometricState(0, true);
        } catch(RuntimeException exc) {
            exc.printStackTrace();
            throw exc;
        }
    }

    @Override
    public void generate() {
        if(spatial != null && !generated) {
            generated = true;
            if(getRegion().findSpaceAround(getX(), getY(), spatial, spatial.getLocalTranslation())) {
                spatial.getLocalTranslation().x -= getRegion().getX() * ShapeUtil.WALL_WIDTH;
                spatial.getLocalTranslation().z -= getRegion().getY() * ShapeUtil.WALL_WIDTH;
                getRegion().getNode().attachChild(spatial);
                spatial.updateRenderState();
                spatial.updateWorldData(0);
                spatial.updateModelBound();
                spatial.updateWorldBound();
                getRegion().getNode().updateRenderState();
                getRegion().getNode().updateWorldData(0);
                getRegion().getNode().updateModelBound();
                getRegion().getNode().updateWorldBound();
            }
        }
    }

    @Override
    public void update(float tpf) {
        if(first) {
            first = false;
            // todo: fountain is not on the ground
            spatial.updateGeometricState(0, true);
            Terrain.moveOnTopOfTerrain(spatial);
        }
    }

    @Override
    public void unloading() {
    }
}
