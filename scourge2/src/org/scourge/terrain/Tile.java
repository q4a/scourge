package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.PassNode;
import com.jme.scene.PassNodeState;
import com.jme.scene.Spatial;
import com.jme.scene.state.BlendState;
import com.jme.scene.state.TextureState;
import com.jme.util.TextureManager;
import org.scourge.Main;

import javax.swing.*;
import java.util.*;
import java.util.logging.Logger;

/**
* User: gabor
* Date: Mar 13, 2010
* Time: 8:56:01 AM
*/
class Tile {
    public TileTexType tex;
    public TileType type;
    public float angle;
    public Node node;
    public Node ground;
    private float[] heights = new float[4];
    private Main main;
    private static Logger logger = Logger.getLogger(Tile.class.toString());
    private int level;
    private List<ModelOnTile> models = new ArrayList<ModelOnTile>();

    public void clearModels() {
        models.clear();
    }

    private class ModelOnTile {
        public Model model;
        public Vector3f translate;
        public float scale;
        public float rotate;
        public Vector3f axis;


        public ModelOnTile(Model model, Vector3f translate, float scale, float rotate, Vector3f axis) {
            this.model = model;
            this.translate = translate;
            this.scale = scale;
            this.rotate = rotate;
            this.axis = axis;
        }
    }

    public enum Edge {
        NW, SW, SE, NE
    }


    public Tile(Main main) {
        this(main, TileTexType.NONE, TileType.NONE, 0);
    }

    public Tile(Main main, TileTexType tex, TileType type, float angle) {
        this.main = main;
        for(int i = 0; i < heights.length; i++) {
            heights[i] = 0;
        }
        set(tex, type, angle);
    }

    public void setLevel(int level) {
        this.level = level;
    }

    public int getLevel() {
        return level;
    }

    public void addModel(Model model) {
        addModel(model, new Vector3f(0, 0, 0), 1, 0, Vector3f.UNIT_Z);
    }

    public void addModel(Model model, Vector3f translate, float scale, float rotate, Vector3f axis) {
        models.add(new ModelOnTile(model, translate, scale, rotate, axis));
    }

    public boolean isEmpty() {
        return type == TileType.NONE && models.isEmpty();
    }

    public void set(TileTexType tex, TileType type, float angle) {
        this.tex = tex;
        this.type = type;
        this.angle = angle;
    }

    public void setHeight(Edge edge, float height) {
        heights[edge.ordinal()] = height;
        if(ground != null) {
            type.updateHeights(ground, heights);
        }
    }

    public float getAvgHeight() {
        float sum = 0;
        for(float h : heights) {
            sum += h;
        }
        return sum / (float)heights.length;
    }

    static List<Long> createTimes = new ArrayList<Long>();
    static List<Long> textureTimes = new ArrayList<Long>();

    public void createNode(Map<Direction, TileTexType> around, int level) {
        long start = System.currentTimeMillis();
        node = new Node(ShapeUtil.newShapeName("tile"));
        ground = type.createNode(angle, heights, level);
        node.attachChild(ground);
        node.setModelBound(new BoundingBox());
        //node.updateModelBound();
        createTimes.add(System.currentTimeMillis() - start);

        start = System.currentTimeMillis();
        applyTexture(around);
        textureTimes.add(System.currentTimeMillis() - start);
    }

    public static void debug() {
        long avgCreate = 0;
        long avgTex = 0;
        for(int i = 0; i < createTimes.size(); i++) {
            avgCreate += createTimes.get(i);
            avgTex += textureTimes.get(i);
        }
        avgCreate = (long)(avgCreate / (float)createTimes.size());
        avgTex = (long)(avgTex / (float)createTimes.size());
        System.err.println("Avg time for " + createTimes.size() + " tiles: create=" + avgCreate + " tex=" + avgTex);
        createTimes.clear();
        textureTimes.clear();        
    }

    public void attachModels() {
        for(ModelOnTile model : models) {
            Spatial spatial = model.model.createSpatial();
            spatial.getLocalTranslation().addLocal(model.translate);
            if(!model.model.isIgnoreHeightMap()) {
                spatial.getLocalTranslation().y = getAvgHeight();
            }
            spatial.getLocalTranslation().x -= ShapeUtil.WALL_WIDTH / 2;
            spatial.getLocalTranslation().z -= ShapeUtil.WALL_WIDTH / 2;
            spatial.getLocalScale().multLocal(model.scale);
            spatial.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * model.rotate, model.axis));
            spatial.updateModelBound();
            node.attachChild(spatial);
        }
//        node.updateModelBound();
//        node.updateWorldBound();
    }

    protected void applyTexture(Map<Direction, TileTexType> around) {
        if(tex.getTexturePath() != null && !type.isTexturePreset()) {
            for(Direction dir : Direction.values()) {
                if(around.get(dir) == null || around.get(dir).ordinal() >= tex.ordinal()) {
                    around.remove(dir);
                }
            }

            TextureState background = createSplatTextureState(tex.getTexturePath(), null);
            if(around.isEmpty()) {
                ground.setRenderState(background);
            } else {
                // alpha used for blending the passnodestates together
                BlendState as = main.getDisplay().getRenderer().createBlendState();
                as.setBlendEnabled(true);
                as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
                as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
                as.setTestEnabled(true);
                as.setTestFunction(BlendState.TestFunction.GreaterThan);
                as.setEnabled(true);

                node.detachChild(ground);
                PassNode passNode = new PassNode("SplatPassNode");
                passNode.attachChild(ground);

                PassNodeState passNodeState = new PassNodeState();
                passNodeState.setPassState(background);
                passNode.addPass(passNodeState);
                node.attachChild(passNode);

                for(TileTexType ttt : TileTexType.values()) {
                    Set<Direction> set = new HashSet<Direction>();
                    for(Direction dir : around.keySet()) {
                        if(around.get(dir) == ttt) {
                            set.add(dir);
                        }
                    }
                    if(!set.isEmpty()) {
                        Stencil stencil = getStencil(set);
                        if(stencil.edge != null) {
                            TextureState splat = createSplatTextureState(ttt.getTexturePath(), stencil);
                            passNodeState = new PassNodeState();
                            passNodeState.setPassState(splat);
                            passNodeState.setPassState(as);
                            passNode.addPass(passNodeState);
                        }
                    }
                }
            }
        }
    }

    private void addAlphaSplat(TextureState ts, Stencil stencil) {
        String path = stencil.edge.getStencilPath(main.getRandom());
        String key = path + "_" + stencil.angle;
        Texture t1 = ShapeUtil.getTexture(key);

        if(t1 == null) {
            ImageIcon icon = ShapeUtil.loadImageIcon(path);
        	t1 = TextureManager.loadTexture(icon.getImage(),
                                            Texture.MinificationFilter.Trilinear,
                                            Texture.MagnificationFilter.Bilinear,
                                            true);
	        t1.setRotation(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * stencil.angle, Vector3f.UNIT_Z));
	        t1.setWrap(Texture.WrapMode.Repeat);
	        t1.setApply(Texture.ApplyMode.Combine);
	        t1.setCombineFuncRGB(Texture.CombinerFunctionRGB.Replace);
	        t1.setCombineSrc0RGB(Texture.CombinerSource.Previous);
	        t1.setCombineOp0RGB(Texture.CombinerOperandRGB.SourceColor);
	        t1.setCombineFuncAlpha(Texture.CombinerFunctionAlpha.Replace);
	        ShapeUtil.storeTexture(key, t1);
        }
        ts.setTexture(t1, ts.getNumberOfSetTextures());
    }
    

    private TextureState createSplatTextureState(String texture, Stencil stencil) {
        TextureState ts = main.getDisplay().getRenderer().createTextureState();

        Texture t0 = ShapeUtil.loadTexture(texture);
        t0.setWrap(Texture.WrapMode.Repeat);
        t0.setApply(Texture.ApplyMode.Modulate);
        ts.setTexture(t0, 0);

        if (stencil != null && stencil.edge != null) {
            addAlphaSplat(ts, stencil);
        }

        return ts;
    }

    public Node getNode() {
        return node;
    }

    public Node getGround() {
        return ground;
    }

    private static enum TexEdge {
        hole("./data/textures/stencil/hole2.png"),
        narrow("./data/textures/stencil/narrow2.png"),
        edge("./data/textures/stencil/edge2.png", "./data/textures/stencil/edge3.png"),
        tip("./data/textures/stencil/tip2.png"),
        corner("./data/textures/stencil/corner2.png", "./data/textures/stencil/corner3.png");

        private String[] stencilPath;

        TexEdge(String ... stencilPath) {
            this.stencilPath = stencilPath;
        }

        public String getStencilPath(Random random) {
            int index = (int)(random.nextFloat() * stencilPath.length);
            return stencilPath[index];
        }
    }

    private Stencil getStencil(Set<Direction> dirs) {
        Stencil stencil = new Stencil();
        if(dirs.size() == 4) {
            stencil.edge = TexEdge.hole;

        } else if(dirs.contains(Direction.NORTH) &&
                  dirs.contains(Direction.EAST) &&
                  dirs.contains(Direction.SOUTH)) {
            stencil.edge = TexEdge.tip;
            stencil.angle = 270;
        } else if(dirs.contains(Direction.EAST) &&
                  dirs.contains(Direction.SOUTH) &&
                  dirs.contains(Direction.WEST)) {
            stencil.edge = TexEdge.tip;
            stencil.angle = 0;
        } else if(dirs.contains(Direction.SOUTH) &&
                  dirs.contains(Direction.WEST) &&
                  dirs.contains(Direction.NORTH)) {
            stencil.edge = TexEdge.tip;
            stencil.angle = 90;
        } else if(dirs.contains(Direction.WEST) &&
                  dirs.contains(Direction.NORTH) &&
                  dirs.contains(Direction.EAST)) {
            stencil.edge = TexEdge.tip;
            stencil.angle = 180;

        } else if(dirs.contains(Direction.NORTH) &&
                  dirs.contains(Direction.SOUTH)) {
            stencil.edge = TexEdge.narrow;
            stencil.angle = 90;
        } else if(dirs.contains(Direction.WEST) &&
                  dirs.contains(Direction.EAST)) {
            stencil.edge = TexEdge.narrow;

        } else if(dirs.contains(Direction.NORTH) &&
                  dirs.contains(Direction.WEST)) {
            stencil.edge = TexEdge.corner;
            stencil.angle = 90;
        } else if(dirs.contains(Direction.WEST) &&
                  dirs.contains(Direction.SOUTH)) {
            stencil.edge = TexEdge.corner;
            stencil.angle = 0;
        } else if(dirs.contains(Direction.SOUTH) &&
                  dirs.contains(Direction.EAST)) {
            stencil.edge = TexEdge.corner;
            stencil.angle = 270;
        } else if(dirs.contains(Direction.EAST) &&
                  dirs.contains(Direction.NORTH)) {
            stencil.edge = TexEdge.corner;
            stencil.angle = 180;

        } else if(dirs.contains(Direction.WEST)) {
            stencil.edge = TexEdge.edge;
            stencil.angle = 0;
        } else if(dirs.contains(Direction.EAST)) {
            stencil.edge = TexEdge.edge;
            stencil.angle = 180;
        } else if(dirs.contains(Direction.NORTH)) {
            stencil.edge = TexEdge.edge;
            stencil.angle = 90;
        } else if(dirs.contains(Direction.SOUTH)) {
            stencil.edge = TexEdge.edge;
            stencil.angle = 270;
        }

        return stencil;
    }


    private class Stencil {
        TexEdge edge = null;
        float angle = 0;
    }    
}
