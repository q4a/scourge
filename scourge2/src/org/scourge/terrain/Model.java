package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Spatial;
import com.jme.scene.state.BlendState;
import com.jme.system.DisplaySystem;

import java.util.Random;

/**
 * User: gabor
 * Date: Mar 27, 2010
 * Time: 8:59:30 PM
 */
public enum Model {
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
    ladder("./data/3ds/ladder.3ds") {
        public Spatial createSpatial() {
            return getAlphaSpatial(1, 0);
        }
    }
    ;

    private static final Model[] BOREAL_TREES = new Model[] {
        fir, fir, fir, fir, fir, fir, fir,
        birch, birch, birch, birch, birch, birch, birch, birch, birch,
        birch2, birch2, birch2, birch2,
        fern, fern, fern, fern, fern, fern, fern, fern,
        redOak,
        bush, bush,
        deadTree,
        cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress, cypress
    };
    public static Model[] getBorealTrees() {
        return BOREAL_TREES;
    }

    private static final Model[] ALPINE_TREES = new Model[] {
        oak, oak, oak,
        fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir, fir,
        birch, birch, birch, birch,
        birch2, birch2, birch2, birch2,
        bush, bush, bush, bush, bush, bush, bush, bush,
        deadTree,deadTree,deadTree,deadTree,deadTree,deadTree,deadTree,deadTree,deadTree,deadTree,
        cypress, cypress, cypress, cypress,
        bushtree, bushtree, bushtree, 
    };
    public static Model[] getAlpineTrees() {
        return ALPINE_TREES;
    }

    private static final Model[] TEMPERATE_TREES = new Model[] {
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

    Model(String modelPath) {
        this(modelPath, false);
    }

    Model(String modelPath, boolean ignoreHeightMap) {
        this.modelPath = modelPath;
        this.ignoreHeightMap = ignoreHeightMap;
    }

    public boolean isIgnoreHeightMap() {
        return ignoreHeightMap;
    }

    public String getModelPath() {
        return modelPath;
    }

    public abstract Spatial createSpatial();

    protected Spatial getNoAlphaSpatial() {
        Spatial spatial = ShapeUtil.importModel(getModelPath(), "./data/textures", name());
        spatial.setModelBound(new BoundingBox());
        spatial.updateModelBound();
        return spatial;
    }

    protected Spatial getAlphaSpatial(float scale) {
        return getAlphaSpatial(scale, -90.0f);
    }

    protected Spatial getAlphaSpatial(float scale, float rotateX) {
        Spatial spatial = ShapeUtil.importModel(getModelPath(), "./data/textures", name());
        if(rotateX != 0) {
            spatial.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(rotateX * FastMath.DEG_TO_RAD, Vector3f.UNIT_X));
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
}
