package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.PassNode;
import com.jme.scene.PassNodeState;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.BlendState;
import com.jme.scene.state.TextureState;
import com.jme.system.DisplaySystem;
import com.jme.util.TextureManager;

import java.nio.FloatBuffer;
import java.util.Map;

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

            Vector3f a = new Vector3f(-ShapeUtil.WALL_WIDTH / 2, heights[0], -ShapeUtil.WALL_WIDTH / 2);
            Vector3f b = new Vector3f(-ShapeUtil.WALL_WIDTH / 2, heights[1], ShapeUtil.WALL_WIDTH / 2);
            Vector3f c = new Vector3f(ShapeUtil.WALL_WIDTH / 2, heights[2], ShapeUtil.WALL_WIDTH / 2);
            Vector3f d = new Vector3f(ShapeUtil.WALL_WIDTH / 2, heights[3], -ShapeUtil.WALL_WIDTH / 2);

            FloatBuffer vertexBuf = ground.getVertexBuffer();
            vertexBuf.clear();
            vertexBuf.put(a.x).put(a.y).put(a.z);
            vertexBuf.put(b.x).put(b.y).put(b.z);
            vertexBuf.put(c.x).put(c.y).put(c.z);
            vertexBuf.put(d.x).put(d.y).put(d.z);

            Vector3f e1 = b.subtract(a);
            Vector3f e2 = c.subtract(a);
            Vector3f normal = e1.cross(e2).normalizeLocal();

            FloatBuffer normBuf = ground.getNormalBuffer();
            normBuf.clear();
            normBuf.put(normal.x).put(normal.y).put(normal.z);
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);
            normBuf.put(0).put(1).put(0);

            //updateHeights(ground, heights);

//            ground.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * -90.0f, Vector3f.UNIT_X));
            ground.setModelBound(new BoundingBox());
            ground.updateModelBound();

            ground.copyTextureCoordinates(0, 1, 1.0f);
            return ground;
        }

        @Override
        public boolean isTexturePreset() {
            return false;
        }

        @Override
        public void updateHeights(Spatial spatial, float[] heights) {
            FloatBuffer vertexBuf = ((Quad)spatial).getVertexBuffer();
            vertexBuf.put(1, heights[0]);
            vertexBuf.put(4, heights[1]);
            vertexBuf.put(7, heights[2]);
            vertexBuf.put(10, heights[3]);
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
