package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.state.BlendState;
import com.jme.scene.state.TextureState;
import com.jme.system.DisplaySystem;
import com.jme.util.export.JMEExporter;
import com.jme.util.export.JMEImporter;
import com.jme.util.export.Savable;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Mar 27, 2010
 * Time: 8:59:30 PM
 */
public enum Model implements Savable {
    bridge("./data/3ds/bridge.3ds", true) {
        public Spatial createSpatial() {
            Spatial sp = getNoAlphaSpatial();
            sp.getLocalTranslation().addLocal(new Vector3f(ShapeUtil.WALL_WIDTH / 2, 0, ShapeUtil.WALL_WIDTH / 2));
            sp.updateModelBound();
            return sp;
        }
    },
    fir("./data/3ds/fir.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(5);
        }
    },
    oak("./data/3ds/tree02.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    willow("./data/3ds/tree03.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    birch("./data/3ds/birch.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(4);
        }
    },
    birch2("./data/3ds/tree13.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    redOak("./data/3ds/tree14.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    bigOak("./data/3ds/tree15.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    redLeaf("./data/3ds/tree16.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    bush("./data/3ds/tree17.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(3);
        }
    },
    deadTree("./data/3ds/tree18.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    palm1("./data/3ds/tree19.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    multiColor("./data/3ds/tree20.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    cypress("./data/3ds/tree21.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    bushtree("./data/3ds/bushtree.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(3);
        }
    },
    cactus("./data/3ds/cactus.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(7);
        }
    },
    fern("./data/3ds/fern.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(6);
        }
    },
    palm2("./data/3ds/palm2.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(4);
        }
    },
    bigpalm("./data/3ds/palm.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(4);
        }
    },
    oldoak("./data/md3/oak/oak1.md3") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(0.12f, 0, 0, 90);
        }

        @Override
        public Vector3f getRotationVector() {
            return Vector3f.UNIT_Y;
        }

        @Override
        public void onLoad(Spatial spatial) {
            Map<String, String> textures = new HashMap<String, String>();
            textures.put("stamm", "data/md3/oak/oakstamm.jpg");
            textures.put("bruch", "data/md3/oak/oakstamm.jpg");
            textures.put("blaetter", "data/md3/oak/oakblaetter.tga");
            assignTextures(spatial, textures);
        }
    },
    oldoak2("./data/md3/oak/oak2.md3") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(0.12f, 0, 0, 90);
        }

        @Override
        public Vector3f getRotationVector() {
            return Vector3f.UNIT_Y;
        }

        @Override
        public void onLoad(Spatial spatial) {
            Map<String, String> textures = new HashMap<String, String>();
            textures.put("stamm", "data/md3/oak/oakstamm2.jpg");
            textures.put("bruch", "data/md3/oak/oakstamm2.jpg");
            textures.put("blaetter", "data/md3/oak/oakblaetter2.tga");
            assignTextures(spatial, textures);
        }
    },
    bigfir("./data/md3/jkm_trees/nadelbaum.md3") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(0.15f, 0, 0, 90);
        }

        @Override
        public Vector3f getRotationVector() {
            return Vector3f.UNIT_Y;
        }

        @Override
        public void onLoad(Spatial spatial) {
            Map<String, String> textures = new HashMap<String, String>();
            textures.put("stumpf", "data/md3/jkm_trees/stamm.jpg");
            textures.put("aeste", "data/md3/jkm_trees/nadel.tga");
            textures.put("stumpf2", "data/md3/jkm_trees/stamm.jpg");
            assignTextures(spatial, textures);
        }
    },
    normal_yellow("./data/md3/jkm_trees/tree1.md3") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(0.11f, 0, 0, 90);
        }

        @Override
        public Vector3f getRotationVector() {
            return Vector3f.UNIT_Y;
        }

        @Override
        public void onLoad(Spatial spatial) {
            Map<String, String> textures = new HashMap<String, String>();
            textures.put("stumpf", "data/md3/jkm_trees/stamm.jpg");
            textures.put("aeste", "data/md3/jkm_trees/tree.tga");
            textures.put("stumpf2", "data/md3/jkm_trees/stamm.jpg");
            assignTextures(spatial, textures);
        }
    },
    normal_green("./data/md3/jkm_trees/tree2.md3") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(0.11f, 0, 0, 90);
        }

        @Override
        public Vector3f getRotationVector() {
            return Vector3f.UNIT_Y;
        }

        @Override
        public void onLoad(Spatial spatial) {
            Map<String, String> textures = new HashMap<String, String>();
            textures.put("stumpf", "data/md3/jkm_trees/stamm.jpg");
            textures.put("aeste", "data/md3/jkm_trees/tree6.tga");
            textures.put("stumpf2", "data/md3/jkm_trees/stamm.jpg");
            assignTextures(spatial, textures);
        }
    },
    normal_red("./data/md3/jkm_trees/tree3.md3") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(0.11f, 0, 0, 90);
        }

        @Override
        public Vector3f getRotationVector() {
            return Vector3f.UNIT_Y;
        }

        @Override
        public void onLoad(Spatial spatial) {
            Map<String, String> textures = new HashMap<String, String>();
            textures.put("stumpf", "data/md3/jkm_trees/stamm.jpg");
            textures.put("aeste", "data/md3/jkm_trees/tree3.tga");
            textures.put("stumpf2", "data/md3/jkm_trees/stamm.jpg");
            assignTextures(spatial, textures);
        }
    },
    normal_green2("./data/md3/jkm_trees/tree4.md3") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(0.11f, 0, 0, 90);
        }

        @Override
        public Vector3f getRotationVector() {
            return Vector3f.UNIT_Y;
        }

        @Override
        public void onLoad(Spatial spatial) {
            Map<String, String> textures = new HashMap<String, String>();
            textures.put("stumpf", "data/md3/jkm_trees/stamm.jpg");
            textures.put("aeste", "data/md3/jkm_trees/tree4.tga");
            textures.put("stumpf2", "data/md3/jkm_trees/stamm.jpg");
            assignTextures(spatial, textures);
        }
    },

    ladder("./data/3ds/ladder.3ds") {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(1, 0, 0, 0);
        }
    },
    mountain("./data/3ds/mtn.3ds") {
        @Override
        public Spatial createSpatial() {
            return getNoAlphaSpatial();
        }
    },
    dungeonColumn("./data/3ds/wood_col.3ds", true) {
        @Override
        public Spatial createSpatial() {
            return getNoAlphaSpatial();
        }
    },
    edge_corner("./data/3ds/edge-c.3ds", true) {
        @Override
        public Spatial createSpatial() {
            return getNoAlphaSpatial();
        }
    },
    edge_tip("./data/3ds/edge-t.3ds", true) {
        @Override
        public Spatial createSpatial() {
            return getNoAlphaSpatial();
        }
    },
    edge_side("./data/3ds/edge-s.3ds", true) {
        @Override
        public Spatial createSpatial() {
            return getNoAlphaSpatial();
        }
    },
    edge_gate("./data/3ds/edge-g.3ds", true) {
        @Override
        public Spatial createSpatial() {
            return getNoAlphaSpatial();
        }
    },
    edge_bridge("./data/3ds/edge-b.3ds", true) {

        @Override
        public Spatial createSpatial() {
            return getNoAlphaSpatial();
        }
    },
    sign("./data/3ds/sign.3ds", false) {
        @Override
        public Spatial createSpatial() {
            return getAlphaSpatial(1f);
        }
    },
    ;

    private static final Model[] BOREAL_TREES = new Model[] {
        normal_green, normal_green, normal_green, normal_green, normal_green, normal_green, normal_green, normal_green,
        normal_green2, normal_green2, normal_green2, normal_green2, normal_green2, normal_green2, normal_green2, normal_green2,
        normal_yellow, normal_yellow,
        normal_red,
        oldoak, oldoak, oldoak, oldoak, oldoak, oldoak, oldoak, oldoak,
        oldoak2, oldoak2, oldoak2,
        fir, fir, fir, fir, fir, fir, fir,
        birch, birch, birch, birch, birch, birch, birch, birch, birch,
        birch2, birch2, birch2, birch2,
        fern, fern, fern, fern, fern, fern, fern, fern,
        redOak,
        bush, bush,
        deadTree,
        cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress
    };
    private String namePrefix;

    public static Model[] getBorealTrees() {
        return BOREAL_TREES;
    }

    private static final Model[] ALPINE_TREES = new Model[] {
        fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir,
        bigfir, bigfir, bigfir, bigfir, bigfir, bigfir, bigfir, bigfir, bigfir, bigfir, bigfir, bigfir,
        birch, birch, birch, birch,
        bush, bush,
        deadTree,deadTree,deadTree,
    };
    public static Model[] getAlpineTrees() {
        return ALPINE_TREES;
    }

    private static final Model[] TEMPERATE_TREES = new Model[] {
        oldoak, oldoak, oldoak, oldoak, oldoak, oldoak, oldoak, oldoak, oldoak, oldoak,
        oldoak2, oldoak2, oldoak2,
        normal_green, normal_green, normal_green, normal_green, normal_green, normal_green, normal_green, normal_green,
        normal_green2, normal_green2, normal_green2, normal_green2, normal_green2, normal_green2, normal_green2, normal_green2,            
        normal_yellow, normal_yellow,
        normal_red,
        oak, oak, oak, oak, oak, oak, oak, oak, oak, oak, oak, oak, oak, oak, oak, oak, oak,
        fir, fir, fir, fir, fir, fir, fir,
        willow, willow,
        birch, birch, birch, birch, birch, birch, birch, birch, birch,
        birch2, birch2, birch2, birch2,
        redOak,
        bigOak, bigOak, bigOak, bigOak, bigOak, bigOak, bigOak, bigOak,
        bush, bush, bush, bush, bush, bush, bush, bush,
        deadTree,
        cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress,
        fern, fern, fern, fern, fern, fern, fern
    };
    public static Model[] getTemperateTrees() {
        return TEMPERATE_TREES;
    }


    private static final Model[] SUBTROPICAL_TREES = new Model[] {
        deadTree,
        bush, bush, bush, bush, bush, bush, bush, bush,
        palm1,palm1,palm1,palm1,palm1,palm1,
        palm2,palm2,palm2,palm2,palm2,palm2,palm2,palm2,
        bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,
        cactus,cactus,cactus,cactus,cactus,cactus,cactus,cactus,cactus,cactus,cactus,
        bushtree, bushtree, bushtree, bushtree, bushtree, bushtree, bushtree, bushtree,  
    };
    public static Model[] getSubtropicalTrees() {
        return SUBTROPICAL_TREES;
    }

    private static final Model[] TROPICAL_TREES = new Model[] {
        bush, bush, bush,
        palm1,palm1,palm1,palm1,palm1,palm1,
        palm2,palm2,palm2,palm2,palm2,palm2,palm2,palm2,
        bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,bigpalm,
    };
    public static Model[] getTropicalTrees() {
        return TROPICAL_TREES;
    }

    private boolean ignoreHeightMap;
    private String modelPath;
    private String texturePath;

    Model(String modelPath) {
        this(modelPath, "./data/textures");
    }

    Model(String modelPath, String texturePath) {
        this(modelPath, texturePath, false);
    }

    Model(String modelPath, boolean ignoreHeightMap) {
        this(modelPath, "./data/textures", ignoreHeightMap);
    }

    Model(String modelPath, String texturePath, boolean ignoreHeightMap) {
        this.modelPath = modelPath;
        this.texturePath = texturePath;
        this.ignoreHeightMap = ignoreHeightMap;
    }

    public boolean isIgnoreHeightMap() {
        return ignoreHeightMap;
    }

    public String getModelPath() {
        return modelPath;
    }

    public String getTexturePath() {
        return texturePath;
    }

    public abstract Spatial createSpatial();

    protected Spatial getNoAlphaSpatial() {
        Spatial spatial = ShapeUtil.importModel(getModelPath(), getTexturePath(), namePrefix == null ? name() : namePrefix, this);
        spatial.setModelBound(new BoundingBox());
        spatial.updateModelBound();
        return spatial;
    }

    protected Spatial getAlphaSpatial(float scale) {
        return getAlphaSpatial(scale, -90.0f, 0, 0);
    }

    protected Spatial getAlphaSpatial(float scale, float rotateX, float rotateY, float rotateZ) {
        Spatial spatial = ShapeUtil.importModel(getModelPath(), getTexturePath(), name(), this);
        if(rotateX != 0) {
            //spatial.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(rotateX * FastMath.DEG_TO_RAD, Vector3f.UNIT_X));
            spatial.getLocalRotation().set(new Quaternion().fromAngles(rotateX * FastMath.DEG_TO_RAD,
                                                                       rotateY * FastMath.DEG_TO_RAD,
                                                                       rotateZ * FastMath.DEG_TO_RAD));
        }

        spatial.setLocalScale(scale);

        BlendState as = DisplaySystem.getDisplaySystem().getRenderer().createBlendState();
        as.setBlendEnabled(true);
        as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
        as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
        as.setReference(0);
        as.setTestEnabled(true);
        as.setTestFunction(BlendState.TestFunction.NotEqualTo);
        as.setEnabled(true);
        spatial.setRenderState(as);

        spatial.setModelBound(new BoundingBox());
        spatial.updateModelBound();
        return spatial;

    }

    public Vector3f getRotationVector() {
        return Vector3f.UNIT_Z;
    }

    public void onLoad(Spatial spatial) {
        // implement to do post-load processing on model
    }

    protected void assignTextures(Spatial spatial, Map<String, String> textures) {
        Node node = (Node)spatial;
        for(String name : textures.keySet()) {
            TextureState ts = DisplaySystem.getDisplaySystem().getRenderer().createTextureState();
            ts.setEnabled(true);
            Texture t = ShapeUtil.loadTexture(textures.get(name));
            t.setWrap(Texture.WrapMode.Repeat);
            t.setHasBorder(false);
            //t.setApply(Texture.ApplyMode.Modulate);
            ts.setTexture(t);
            Spatial child = findChild(node, name);
            if(child != null) child.setRenderState(ts);
            else {
                Logger.getLogger(getClass().toString()).severe("Can't find node child named " + name + " in model " + name());
                ShapeUtil.debugNode(node, "");
            }
        }
        node.updateRenderState();
    }

    private Spatial findChild(Node node, String name) {
        for(Spatial spatial : node.getChildren()) {
            if(spatial.getName().startsWith(name)) {
                return spatial;
            } else if(spatial instanceof Node) {
                Spatial s = findChild((Node)spatial, name);
                if(s != null) return s;
            }
        }
        return null;
    }

    protected void assignDungeonTextures(Spatial spatial, String topTexturePath) {
        Map<String, String> textures = new HashMap<String, String>();

        if(topTexturePath != null) {
            // top
            textures.put("block##0", topTexturePath);
        }

        // walls
        textures.put("wall##0", "data/textures/mountain.png");
        assignTextures(spatial, textures);
    }

    public void setNamePrefix(String namePrefix) {
        this.namePrefix = namePrefix;
    }

    @Override
    public void write(JMEExporter jmeExporter) throws IOException {
        throw new RuntimeException("not used");
    }
    @Override
    public void read(JMEImporter jmeImporter) throws IOException {
        throw new RuntimeException("not used");
    }
    @Override
    public Class getClassTag() {
        throw new RuntimeException("not used");
    }
}
