package org.scourge.terrain;

import java.awt.AlphaComposite;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Logger;

import javax.swing.ImageIcon;

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
    private float[] heights = new float[4];
    private Main main;
    private static Map<String, Texture> textures = new HashMap<String, Texture>();

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

    public void createSpatial(Map<Direction, TileTexType> around, List<Map<String, GroundType>> ground, int x, int y) {
        spatial = type.createSpatial(angle, heights);
        applyTexture(around, ground, x, y);
    }

    protected void applyTexture(Map<Direction, TileTexType> around, List<Map<String, GroundType>> ground, int x, int y) {
        if(tex.getTexturePath() != null && !type.isTexturePreset()) {
            // remove the same textures
            for(Direction dir : Direction.values()) {
                if(around.get(dir) == null || around.get(dir).ordinal() >= tex.ordinal()) {
                    around.remove(dir);
                }
            }

            StringBuffer textureKey = new StringBuffer(tex.name());

            Image background;
            if(!around.isEmpty()) {
                background = createBlendedEdge(tex, around, textureKey);
            } else {
                TexTile tt = new TexTile(tex.getTexturePath());
                background = tt.img;
            }
            int width = background.getWidth(null);
            int height = background.getHeight(null);
            Image img = background;
            for(Map<String, GroundType> groundMap : ground) {
                img = createGround(groundMap, x, y, background, width, height, textureKey);
                background = img;
            }
            String key = textureKey.toString();
            Texture texture = textures.get(key);
            if(texture == null) {
                texture = TextureManager.loadTexture(img,
                                                     Texture.MinificationFilter.Trilinear,
                                                     Texture.MagnificationFilter.Bilinear,
                                                     false);
                textures.put(key, texture);
            }

            TextureState ts = main.getDisplay().getRenderer().createTextureState();
            ts.setTexture(texture);
            spatial.setRenderState(ts);
        }
    }

    private static enum GroundEdge {
        hole("./data/textures/stencil/sm_hole.png"),
        narrow("./data/textures/stencil/sm_narrow.png"),
        edge("./data/textures/stencil/sm_edge.png"),
        tip("./data/textures/stencil/sm_tip.png"),
        corner("./data/textures/stencil/sm_corner.png");

        private String stencilPath;

        GroundEdge(String stencilPath) {
            this.stencilPath = stencilPath;
        }

        public String getStencilPath() {
            return stencilPath;
        }
    }

    protected Image createGround(Map<String, GroundType> ground, int x, int y, Image background, int width, int height, StringBuffer textureKey) {
        BufferedImage tmp =  new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = (Graphics2D) tmp.getGraphics();
        g.drawImage(background, null, null);

        for(int xx = 0; xx < 4; xx++) {
            for(int yy = 0; yy < 4; yy++) {
                int gx = x * 4 + xx;
                int gy = y * 4 + yy;
                GroundType gt = ground.get(GroundType.getGroundKey(gx, gy));
                if(gt == null || gt == GroundType.none) {
                    textureKey.append("-x");
                    continue;
                }

                g.setComposite(AlphaComposite.SrcOver);

                GroundType west = ground.get(GroundType.getGroundKey(gx - 1, gy));
                GroundType east = ground.get(GroundType.getGroundKey(gx + 1, gy));
                GroundType north = ground.get(GroundType.getGroundKey(gx, gy - 1));
                GroundType south = ground.get(GroundType.getGroundKey(gx, gy + 1));
                boolean w = west == gt;
                boolean e = east == gt;
                boolean n = north == gt;
                boolean s = south == gt;


                GroundEdge groundEdge = null;
                float angle = 0;
                if(!w && !e && !n && !s) {
                    groundEdge = GroundEdge.hole;
                    angle = 0;

                } else if(w && !e && !n && !s) {
                    groundEdge = GroundEdge.tip;
                    angle = 180;
                } else if(!w && e && !n && !s) {
                    groundEdge = GroundEdge.tip;
                    angle = 0;
                } else if(!w && !e && n && !s) {
                    groundEdge = GroundEdge.tip;
                    angle = 90;
                } else if(!w && !e && !n && s) {
                    groundEdge = GroundEdge.tip;
                    angle = 270;

                } else if(!w && !e && n && s) {
                    groundEdge = GroundEdge.narrow;
                    angle = 90;
                } else if(w && e && !n && !s) {
                    groundEdge = GroundEdge.narrow;
                    angle = 0;

                } else if(w && !e && n && !s) {
                    groundEdge = GroundEdge.corner;
                    angle = 90;
                } else if(!w && e && !n && s) {
                    groundEdge = GroundEdge.corner;
                    angle = 270;
                } else if(w && !e && !n && s) {
                    groundEdge = GroundEdge.corner;
                    angle = 180;
                } else if(!w && e && n && !s) {
                    groundEdge = GroundEdge.corner;
                    angle = 0;


                } else if(w & e & n & !s) {
                    groundEdge = GroundEdge.edge;
                    angle = 0;
                } else if(w & e & !n & s) {
                    groundEdge = GroundEdge.edge;
                    angle = 180;
                } else if(!w & e & n & s) {
                    groundEdge = GroundEdge.edge;
                    angle = 270;
                } else if(w & !e & n & s) {
                    groundEdge = GroundEdge.edge;
                    angle = 90;
                }

                AffineTransform transform = AffineTransform.getTranslateInstance(xx * width / 4.0,
                                                                                 (4 - 1 - yy) * height / 4.0);
                if(groundEdge != null) {
                    g.setComposite(AlphaComposite.Xor);
                    TexTile stencil = new TexTile(groundEdge.getStencilPath(), true, angle);
                    g.drawImage(stencil.img, transform, null);
                    g.setComposite(AlphaComposite.DstAtop);
                    textureKey.append("-").append(groundEdge.name()).append(":").append(angle);
                }

                TexTile patch = new TexTile(gt.getTexturePath(), true, 0);
                transform.concatenate(AffineTransform.getScaleInstance((width / 4.0f) / (float)patch.width,
                                                                       (height / 4.0f) / (float)patch.height));
                g.drawImage(patch.img, transform, null);
                textureKey.append("-").append(gt.name());
            }
            textureKey.append("|");
        }
        g.dispose();

        return tmp;
    }

    private String createTextureKey(TileTexType tex, Map<Direction, TileTexType> around) {
        StringBuilder sb = new StringBuilder(tex.name());
        for(Direction dir : Direction.values()) {
            sb.append("-");
            sb.append(around.get(dir) == null ? "" : around.get(dir).name());
        }
        return sb.toString();
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

        public Image blend(TexTile background, Map<Direction, TexTile> aroundTex) {
            return blend(background, aroundTex, 0);
        }

        public Image blend(TexTile background, Map<Direction, TexTile> aroundTex, float angle) {
            TexTile stencil = new TexTile(stencilPath, true, angle);
            TexTile texTile = aroundTex.values().iterator().next();

            BufferedImage tmp =  new BufferedImage(stencil.width, stencil.height, BufferedImage.TYPE_INT_ARGB);
            Graphics2D g = (Graphics2D) tmp.getGraphics();
            g.drawImage(background.img, AffineTransform.getScaleInstance(stencil.width / (float)background.width, stencil.height / (float)background.height), null);
            g.setComposite(AlphaComposite.Xor);
            g.drawImage(stencil.img, null, null);
            g.setComposite(AlphaComposite.DstAtop);
            g.drawImage(texTile.img, AffineTransform.getScaleInstance(stencil.width / (float)texTile.width, stencil.height / (float)texTile.height), null);            
            g.dispose();

            return tmp;
        }

    }

    private Image createBlendedEdge(TileTexType tex, Map<Direction, TileTexType> around, StringBuffer textureKey) {
        TexTile background = new TexTile(tex.getTexturePath());
        Map<Direction, TexTile> aroundTex = new HashMap<Direction, TexTile>();
        for(Direction dir : around.keySet()) {
            aroundTex.put(dir, new TexTile(around.get(dir).getTexturePath()));
        }


        TexEdge edge = null;
        float angle = 0;
        if(aroundTex.size() == 4) {
            edge = TexEdge.hole;

        } else if(aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.EAST) &&
                  aroundTex.keySet().contains(Direction.SOUTH)) {
            edge = TexEdge.tip;
            angle = 270;
        } else if(aroundTex.keySet().contains(Direction.EAST) &&
                  aroundTex.keySet().contains(Direction.SOUTH) &&
                  aroundTex.keySet().contains(Direction.WEST)) {
            edge = TexEdge.tip;
            angle = 180;
        } else if(aroundTex.keySet().contains(Direction.SOUTH) &&
                  aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.NORTH)) {
            edge = TexEdge.tip;
            angle = 90;
        } else if(aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.EAST)) {
            edge = TexEdge.tip;
            angle = 0;

        } else if(aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.SOUTH)) {
            edge = TexEdge.narrow;
            angle = 90;
        } else if(aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.EAST)) {
            edge = TexEdge.narrow;

        } else if(aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.WEST)) {
            edge = TexEdge.corner;
            angle = 0;
        } else if(aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.SOUTH)) {
            edge = TexEdge.corner;
            angle = 90;
        } else if(aroundTex.keySet().contains(Direction.SOUTH) &&
                  aroundTex.keySet().contains(Direction.EAST)) {
            edge = TexEdge.corner;
            angle = 180;
        } else if(aroundTex.keySet().contains(Direction.EAST) &&
                  aroundTex.keySet().contains(Direction.NORTH)) {
            edge = TexEdge.corner;
            angle = 270;

        } else if(aroundTex.keySet().contains(Direction.WEST)) {
            edge = TexEdge.edge;
            angle = 0;
        } else if(aroundTex.keySet().contains(Direction.EAST)) {
            edge = TexEdge.edge;
            angle = 180;
        } else if(aroundTex.keySet().contains(Direction.NORTH)) {
            edge = TexEdge.edge;
            angle = 270;
        } else if(aroundTex.keySet().contains(Direction.SOUTH)) {
            edge = TexEdge.edge;
            angle = 90;
            
        }

        if(edge != null) {
            textureKey.append("-").append(edge.name()).append(":").append(angle);
            return edge.blend(background, aroundTex, angle);
        } else {
            return background.img;
        }
    }


    private static class TexTile {
        private int width, height;
        private int[] pixels;
        private BufferedImage img;
        private String name;

        public TexTile(String path) {
            this(path, false, 0);
        }

        public TexTile(String path, boolean hasAlpha, float angle) {
            name = path + ":" + angle;
            ImageIcon image = ShapeUtil.getImageIcon(path);
            int type = hasAlpha ? BufferedImage.TYPE_INT_ARGB : BufferedImage.TYPE_INT_RGB;

            // draw
            width = image.getIconWidth();
            height = image.getIconHeight();
            BufferedImage tmp =  new BufferedImage(width, height, type);
            Graphics2D g = (Graphics2D) tmp.getGraphics();
            g.drawImage(image.getImage(), null, null);
            g.dispose();

            // rotate
            AffineTransform tx = new AffineTransform();
            tx.rotate(FastMath.DEG_TO_RAD * angle, width / 2, height / 2);
            AffineTransformOp op = new AffineTransformOp(tx, AffineTransformOp.TYPE_BILINEAR);
            img = op.filter(tmp, null);

            pixels = ((DataBufferInt) img.getRaster().getDataBuffer()).getData();

        }

        @Override
        public String toString() {
            return name;
        }
    }
        
    private static Logger logger = Logger.getLogger(Tile.class.toString());
    public static void debug() {
        logger.info("* Loaded " + textures.size() + " textures: ");
//        for(String s : textures.keySet()) {
//            logger.info(s);
//        }
    }
}
