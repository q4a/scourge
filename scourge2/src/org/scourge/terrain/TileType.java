package org.scourge.terrain;

import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.TextureState;
import com.jme.system.DisplaySystem;
import org.scourge.Climate;

import java.nio.FloatBuffer;

/**
* User: gabor
* Date: Mar 13, 2010
* Time: 8:55:49 AM
*/
enum TileType {
    NONE {
        @Override
        public Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater) {
            return new Node("empty");
        }

        @Override
        public boolean isTexturePreset() {
            return false;
        }

        @Override
        public void updateHeights(Node node, float[] heights) {
        }
    },
    EDGE_BRIDGE {
        private Model model = Model.edge_bridge;

        @Override
        public Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater) {
            return addEdge(angle, model, level, climate, nextToWater);
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Node node, float[] heights) {
        }
    },
    EDGE_CORNER {
        private Model model = Model.edge_corner;

        @Override
        public Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater) {
            return addEdge(angle, model, level, climate, nextToWater);
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Node node, float[] heights) {
        }
    },
    EDGE_TIP {
        private Model model = Model.edge_tip;

        @Override
        public Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater) {
            return addEdge(angle, model, level, climate, nextToWater);
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Node node, float[] heights) {
        }
    },
    EDGE_SIDE {
        private Model model = Model.edge_side;

        @Override
        public Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater) {
            return addEdge(angle, model, level, climate, nextToWater);
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Node node, float[] heights) {
        }
    },
    EDGE_GATE {
        private Model model = Model.edge_gate;

        @Override
        public Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater) {
            return addEdge(angle, model, level, climate, nextToWater);
        }

        @Override
        public boolean isTexturePreset() {
            return true;
        }

        @Override
        public void updateHeights(Node node, float[] heights) {
        }
    },
    QUAD {
        @Override
        public Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater) {
            Quad ground = createQuad(heights);

            Node groundNode = new Node(ShapeUtil.newShapeName("ground_node"));
            groundNode.attachChild(ground);
            return groundNode;
        }

        @Override
        public boolean isTexturePreset() {
            return false;
        }

        @Override
        public void updateHeights(Node node, float[] heights) {
            FloatBuffer vertexBuf = ((Quad) node.getChild(0)).getVertexBuffer();
            vertexBuf.put(1, heights[0]);
            vertexBuf.put(4, heights[1]);
            vertexBuf.put(7, heights[2]);
            vertexBuf.put(10, heights[3]);
//            node.updateModelBound();
//            node.updateWorldBound();
        }
    },
    ;

    public abstract boolean isTexturePreset();
    public abstract Node createNode(float angle, float[] heights, int level, Climate climate, boolean nextToWater);
    public abstract void updateHeights(Node node, float[] heights);

    protected Node addEdge(float angle, Model model, int level, Climate climate, boolean nextToWater) {
        // create a unique cache key for the model
        model.setNamePrefix(model.name() + "." + climate.name() + "." + level);
        
        Spatial edge = model.createSpatial();
        edge.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * angle, Vector3f.UNIT_Y));

        if(climate == Climate.dungeon) {
            model.assignDungeonTextures(edge, level == 0 ? climate.getBaseTileTex().getTexturePath() : null);
        }

        Node edgeNode = new Node(ShapeUtil.newShapeName("edge_node"));
        edgeNode.attachChild(edge);

        if(level > 0 && !nextToWater) {
            Quad ground = createQuad(new float[] { 0, 0, 0, 0 });
            ground.getLocalTranslation().y -= ShapeUtil.WALL_HEIGHT;
            TextureState ts = DisplaySystem.getDisplaySystem().getRenderer().createTextureState();
            Texture texture = ShapeUtil.loadTexture(climate.getBaseTileTex().getTexturePath());
            texture.setWrap(Texture.WrapMode.Repeat);
            ts.setTexture(texture, 0);
            ground.setRenderState(ts);
            edgeNode.attachChild(ground);
        }

//        edgeNode.setModelBound(new BoundingBox());
//        edgeNode.updateModelBound();
//        edgeNode.updateWorldBound();
        return edgeNode;
    }

    protected Quad createQuad(float[] heights) {
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

//        ground.setModelBound(new BoundingBox());
//        ground.updateModelBound();

        // w/o this magic line splat textures (PassNode) won't work.
        ground.copyTextureCoordinates(0, 1, 1.0f);
        ground.copyTextureCoordinates(1, 2, 1.0f);
        ground.copyTextureCoordinates(2, 3, 1.0f);

        return ground;
    }
}
