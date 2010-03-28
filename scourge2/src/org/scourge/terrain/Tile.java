package org.scourge.terrain;

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
    public PassNode passNode;
    private float[] heights = new float[4];
    private Main main;
    private static Logger logger = Logger.getLogger(Tile.class.toString());
    private int level;

    public void setLevel(int level) {
        this.level = level;
    }

    public int getLevel() {
        return level;
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

    public void set(TileTexType tex, TileType type, float angle) {
        this.tex = tex;
        this.type = type;
        this.angle = angle;
    }

    public void setHeight(Edge edge, float height) {
        heights[edge.ordinal()] = height;
        if(node != null) {
            type.updateHeights(node, heights);
        }
    }

    public void createNode(Map<Direction, TileTexType> around, int level) {
        node = type.createNode(angle, heights, level);
        applyTexture(around);
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
                node.setRenderState(background);
            } else {
                // alpha used for blending the passnodestates together
                BlendState as = main.getDisplay().getRenderer().createBlendState();
                as.setBlendEnabled(true);
                as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
                as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
                as.setTestEnabled(true);
                as.setTestFunction(BlendState.TestFunction.GreaterThan);
                as.setEnabled(true);

                passNode = new PassNode("SplatPassNode");
                passNode.attachChild(node);

                PassNodeState passNodeState = new PassNodeState();
                passNodeState.setPassState(background);
                passNode.addPass(passNodeState);

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

    public Spatial getNode() {
        return passNode != null ? passNode : node;
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
