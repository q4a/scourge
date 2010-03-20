package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Box;
import com.jme.scene.shape.Quad;
import com.jme.util.geom.BufferUtils;

import java.nio.FloatBuffer;

/**
* User: gabor
* Date: Mar 13, 2010
* Time: 8:55:49 AM
*/
enum TileType {
    NONE {
        @Override
        public Spatial createSpatial(float angle, float[] heights) {
            return null;
        }

        @Override
        public boolean isTexturePreset() {
            return false;
        }

        @Override
        public void updateHeights(Spatial spatial, float[] heights) {
        }
    },
    EDGE_BRIDGE {
        @Override
        public Spatial createSpatial(float angle, float[] heights) {
            return addEdge(angle, "b");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Spatial spatial, float[] heights) {
        }
    },
    EDGE_CORNER {
        @Override
        public Spatial createSpatial(float angle, float[] heights) {
            return addEdge(angle, "c");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Spatial spatial, float[] heights) {
        }
    },
    EDGE_TIP {
        @Override
        public Spatial createSpatial(float angle, float[] heights) {
            return addEdge(angle, "t");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Spatial spatial, float[] heights) {
        }
    },
    EDGE_SIDE {
        @Override
        public Spatial createSpatial(float angle, float[] heights) {
            return addEdge(angle, "s");
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Spatial spatial, float[] heights) {
        }
    },
    QUAD {
        @Override
        public Spatial createSpatial(float angle, float[] heights) {
            Quad ground = new Quad(ShapeUtil.newShapeName("ground"), ShapeUtil.WALL_WIDTH, ShapeUtil.WALL_WIDTH);
            FloatBuffer normBuf = ground.getNormalBuffer();
            normBuf.clear();
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);

            updateHeights(ground, heights);

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

        @Override
        public void updateHeights(Spatial spatial, float[] heights) {
            FloatBuffer vertexBuf = ((Quad)spatial).getVertexBuffer();
            vertexBuf.put(2, heights[0]);
            vertexBuf.put(5, heights[1]);
            vertexBuf.put(8, heights[2]);
            vertexBuf.put(11, heights[3]);
            spatial.updateModelBound();
            spatial.updateWorldBound();
        }
    },
    ;

    public abstract Spatial createSpatial(float angle, float[] heights);

    protected Spatial addEdge(float angle, String model) {
        Spatial edge = ShapeUtil.load3ds("./data/3ds/edge-" + model + ".3ds", "./data/textures", "edge");
        edge.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * angle, Vector3f.UNIT_Y));
        edge.setModelBound(new BoundingBox());
        edge.updateModelBound();
        edge.updateWorldBound();
        return edge;
    }

    public abstract boolean isTexturePreset();


    public abstract void updateHeights(Spatial spatial, float[] heights);
}
