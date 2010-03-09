package org.scourge.terrain;

import com.jme.scene.Spatial;

/**
 * User: gabor
 * Date: Mar 6, 2010
 * Time: 2:32:24 PM
 */
public enum Ledge {
    CORNER {
        @Override
        public Spatial getSpatial() {
            return ShapeUtil.load3ds("./data/3ds/ledge-corner.3ds", "./data/textures", "ledge");
        }
    },
    SIDE {
        @Override
        public Spatial getSpatial() {
            return ShapeUtil.load3ds("./data/3ds/ledge-side.3ds", "./data/textures", "ledge");
        }
    },
    NARROW {
        @Override
        public Spatial getSpatial() {
            return ShapeUtil.load3ds("./data/3ds/ledge-narrow.3ds", "./data/textures", "ledge");
        }
    },
    TIP {
        @Override
        public Spatial getSpatial() {
            return ShapeUtil.load3ds("./data/3ds/ledge-tip.3ds", "./data/textures", "ledge");
        }
    };
    public static final float LEDGE_WIDTH = 2;
    public static final float LEDGE_HEIGHT = 1.5f;

    public abstract Spatial getSpatial();
}
