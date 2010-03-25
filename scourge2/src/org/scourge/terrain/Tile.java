package org.scourge.terrain;

import java.awt.AlphaComposite;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.logging.Logger;

import javax.swing.ImageIcon;

import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.PassNode;
import com.jme.scene.PassNodeState;
import com.jme.scene.state.BlendState;
import org.scourge.Main;

import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.scene.Spatial;
import com.jme.scene.state.TextureState;
import com.jme.util.TextureManager;

/**
* User: gabor
* Date: Mar 13, 2010
* Time: 8:56:01 AM
*/
class Tile {
    public TileTexType tex;
    public TileType type;
    public float angle;
    public Spatial spatial;
    public PassNode passNode;
    private float[] heights = new float[4];
    private Main main;
    private static Logger logger = Logger.getLogger(Tile.class.toString());

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

    public void set(TileTexType tex, TileType type, float angle) {
        this.tex = tex;
        this.type = type;
        this.angle = angle;
    }

    public void setHeight(Edge edge, float height) {
        heights[edge.ordinal()] = height;
        if(spatial != null) {
            type.updateHeights(spatial, heights);
        }
    }

    public void createSpatial(Map<Direction, TileTexType> around, int x, int y) {
        spatial = type.createSpatial(angle, heights);
        applyTexture(around, x, y);
    }

    protected void applyTexture(Map<Direction, TileTexType> around, int x, int y) {
        if(tex.getTexturePath() != null && !type.isTexturePreset()) {
            // remove the same textures
            for(Direction dir : Direction.values()) {
                if(around.get(dir) == null || around.get(dir).ordinal() >= tex.ordinal()) {
                    around.remove(dir);
                }
            }

            TextureState background = createSplatTextureState(tex.getTexturePath(), null);
            Stencil stencil = null;
            if(!around.isEmpty()) {
                stencil = getStencil(around.keySet());
            }
            if(stencil == null || stencil.edge == null) {
                spatial.setRenderState(background);
            } else {
                TextureState splat = createSplatTextureState("data/textures/surf1.png", stencil);

                // alpha used for blending the passnodestates together
                BlendState as = main.getDisplay().getRenderer().createBlendState();
                as.setBlendEnabled(true);
                as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
                as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
                as.setTestEnabled(true);
                as.setTestFunction(BlendState.TestFunction.GreaterThan);
                as.setEnabled(true);

                passNode = new PassNode("SplatPassNode");
                passNode.attachChild(spatial);

                PassNodeState passNodeState = new PassNodeState();
                passNodeState.setPassState(background);
                passNode.addPass(passNodeState);

                passNodeState = new PassNodeState();
                passNodeState.setPassState(splat);
                passNodeState.setPassState(as);
                passNode.addPass(passNodeState);
            }
        }
    }


    private void addAlphaSplat(TextureState ts, Stencil stencil) {
        Texture t1 = TextureManager.loadTexture(stencil.edge.getStencilPath(),
                                                Texture.MinificationFilter.Trilinear,
                                                Texture.MagnificationFilter.Bilinear);
        t1.setRotation(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * stencil.angle, Vector3f.UNIT_Z));
        t1.setWrap(Texture.WrapMode.Repeat);
        t1.setApply(Texture.ApplyMode.Combine);
        t1.setCombineFuncRGB(Texture.CombinerFunctionRGB.Replace);
        t1.setCombineSrc0RGB(Texture.CombinerSource.Previous);
        t1.setCombineOp0RGB(Texture.CombinerOperandRGB.SourceColor);
        t1.setCombineFuncAlpha(Texture.CombinerFunctionAlpha.Replace);
        ts.setTexture(t1, ts.getNumberOfSetTextures());
    }

    private TextureState createSplatTextureState(String texture, Stencil stencil) {
        TextureState ts = main.getDisplay().getRenderer().createTextureState();

        Texture t0 = TextureManager.loadTexture(texture,
                                                Texture.MinificationFilter.Trilinear,
                                                Texture.MagnificationFilter.Bilinear);
        t0.setWrap(Texture.WrapMode.Repeat);
        t0.setApply(Texture.ApplyMode.Modulate);
        //t0.setScale(new Vector3f(0.5f, 0.5f, 1.0f));
        ts.setTexture(t0, 0);

        if (stencil != null && stencil.edge != null) {
            addAlphaSplat(ts, stencil);
        }

        return ts;
    }

    public Spatial getRenderedSpatial() {
        return passNode != null ? passNode : spatial;
    }

    private static enum TexEdge {
        hole("./data/textures/stencil/hole.png"),
        narrow("./data/textures/stencil/narrow.png"),
        edge("./data/textures/stencil/edge.png"),
        tip("./data/textures/stencil/tip.png"),
        corner("./data/textures/stencil/corner.png");

        private String stencilPath;

        TexEdge(String stencilPath) {
            this.stencilPath = stencilPath;
        }

        public String getStencilPath() {
            return stencilPath;
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

    
    public static void debug() {
//        logger.info("* Loaded " + textures.size() + " textures: ");
//        for(String s : textures.keySet()) {
//            logger.info(s);
//        }
    }
}
