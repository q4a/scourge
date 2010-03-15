package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector2f;
import com.jme.math.Vector3f;
import com.jme.scene.Spatial;
import com.jme.scene.TexCoords;
import com.jme.scene.shape.Quad;

import java.nio.FloatBuffer;

/**
* User: gabor
* Date: Mar 13, 2010
* Time: 8:55:49 AM
*/
enum TileType {
    NONE {
        @Override
        public Spatial createSpatial(float angle) {
            return null;
        }

        @Override
        public boolean isTexturePreset() {
            return false;
        }
    },
    EDGE_BRIDGE {
        @Override
        public Spatial createSpatial(float angle) {
            return addEdge(angle, "b");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }
    },
    EDGE_CORNER {
        @Override
        public Spatial createSpatial(float angle) {
            return addEdge(angle, "c");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }
    },
    EDGE_TIP {
        @Override
        public Spatial createSpatial(float angle) {
            return addEdge(angle, "t");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }
    },
    EDGE_SIDE {
        @Override
        public Spatial createSpatial(float angle) {
            return addEdge(angle, "s");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }
    },
    QUAD {
        @Override
        public Spatial createSpatial(float angle) {
            Quad ground = new Quad(ShapeUtil.newShapeName("ground"), ShapeUtil.WALL_WIDTH, ShapeUtil.WALL_WIDTH);
            FloatBuffer normBuf = ground.getNormalBuffer();
            normBuf.clear();
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);

//            Vector2f[] coords = new Vector2f[] {
//                new Vector2f(0,    0),
//                new Vector2f(0.5f, 0),
//                new Vector2f(0.5f, 0.5f),
//                new Vector2f(0,    0.5f)
//            };
//            TexCoords tc = TexCoords.makeNew(coords);
//            ground.setTextureCoords(tc);

            ground.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * -90.0f, Vector3f.UNIT_X));
            ground.setModelBound(new BoundingBox());
            return ground;
        }

        @Override
        public boolean isTexturePreset() {
            return false;
        }
    },
    ;

    public abstract Spatial createSpatial(float angle);

    protected Spatial addEdge(float angle, String model) {
        Spatial edge = ShapeUtil.load3ds("./data/3ds/edge-" + model + ".3ds", "./data/textures", "edge");
        edge.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * angle, Vector3f.UNIT_Y));
        edge.setModelBound(new BoundingBox());
        edge.updateModelBound();
        edge.updateWorldBound();
        return edge;
    }

    public abstract boolean isTexturePreset();
}
