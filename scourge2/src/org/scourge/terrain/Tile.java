package org.scourge.terrain;

import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.scene.Spatial;
import com.jme.scene.state.TextureState;
import com.jme.util.TextureManager;
import org.scourge.Main;

import javax.swing.*;
import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.image.AffineTransformOp;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.util.*;
import java.util.List;

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


            Image background;
            if(!around.isEmpty()) {
                background = createBlendedEdge(tex, around);
            } else {
                TexTile tt = new TexTile(tex.getTexturePath());
                background = tt.img;
            }
            int width = background.getWidth(null);
            int height = background.getHeight(null);
            Image img = background;
            for(Map<String, GroundType> groundMap : ground) {
                img = createGround(groundMap, x, y, background, width, height);
                background = img;
            }
            Texture texture = TextureManager.loadTexture(img,
                                                 Texture.MinificationFilter.Trilinear,
                                                 Texture.MagnificationFilter.Bilinear,
                                                 false);

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

    protected Image createGround(Map<String, GroundType> ground, int x, int y, Image background, int width, int height) {
        BufferedImage tmp =  new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = (Graphics2D) tmp.getGraphics();
        g.drawImage(background, null, null);

        for(int xx = 0; xx < 4; xx++) {
            for(int yy = 0; yy < 4; yy++) {
                int gx = x * 4 + xx;
                int gy = y * 4 + yy;
                GroundType gt = ground.get(GroundType.getGroundKey(gx, gy));
                if(gt == null || gt == GroundType.none) continue;

                g.setComposite(AlphaComposite.SrcOver);

                GroundType west = ground.get(GroundType.getGroundKey(gx - 1, gy));
                GroundType east = ground.get(GroundType.getGroundKey(gx + 1, gy));
                GroundType north = ground.get(GroundType.getGroundKey(gx, gy - 1));
                GroundType south = ground.get(GroundType.getGroundKey(gx, gy + 1));
                boolean w = west == gt;
                boolean e = east == gt;
                boolean n = north == gt;
                boolean s = south == gt;


                TexTile stencil = null;
                if(!w && !e && !n && !s) {
                    stencil = new TexTile(GroundEdge.hole.getStencilPath(), true, 0);

                } else if(w && !e && !n && !s) {
                    stencil = new TexTile(GroundEdge.tip.getStencilPath(), true, 180);
                } else if(!w && e && !n && !s) {
                    stencil = new TexTile(GroundEdge.tip.getStencilPath(), true, 0);
                } else if(!w && !e && n && !s) {
                    stencil = new TexTile(GroundEdge.tip.getStencilPath(), true, 90);
                } else if(!w && !e && !n && s) {
                    stencil = new TexTile(GroundEdge.tip.getStencilPath(), true, 270);

                } else if(!w && !e && n && s) {
                    stencil = new TexTile(GroundEdge.narrow.getStencilPath(), true, 90);
                } else if(w && e && !n && !s) {
                    stencil = new TexTile(GroundEdge.narrow.getStencilPath(), true, 0);

                } else if(w && !e && n && !s) {
                    stencil = new TexTile(GroundEdge.corner.getStencilPath(), true, 90);
                } else if(!w && e && !n && s) {
                    stencil = new TexTile(GroundEdge.corner.getStencilPath(), true, 270);
                } else if(w && !e && !n && s) {
                    stencil = new TexTile(GroundEdge.corner.getStencilPath(), true, 180);
                } else if(!w && e && n && !s) {
                    stencil = new TexTile(GroundEdge.corner.getStencilPath(), true, 0);


                } else if(w & e & n & !s) {
                    stencil = new TexTile(GroundEdge.edge.getStencilPath(), true, 0);
                } else if(w & e & !n & s) {
                    stencil = new TexTile(GroundEdge.edge.getStencilPath(), true, 180);
                } else if(!w & e & n & s) {
                    stencil = new TexTile(GroundEdge.edge.getStencilPath(), true, 270);
                } else if(w & !e & n & s) {
                    stencil = new TexTile(GroundEdge.edge.getStencilPath(), true, 90);
                }

                AffineTransform transform = AffineTransform.getTranslateInstance(xx * width / 4.0,
                                                                                 (4 - 1 - yy) * height / 4.0);
                if(stencil != null) {
                    g.setComposite(AlphaComposite.Xor);
                    g.drawImage(stencil.img, transform, null);
                    g.setComposite(AlphaComposite.DstAtop);
                }

                TexTile patch = new TexTile(gt.getTexturePath(), true, 0);
                transform.concatenate(AffineTransform.getScaleInstance((width / 4.0f) / (float)patch.width,
                                                                       (height / 4.0f) / (float)patch.height));
                g.drawImage(patch.img, transform, null);
            }
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

    private Image createBlendedEdge(TileTexType tex, Map<Direction, TileTexType> around) {
        TexTile background = new TexTile(tex.getTexturePath());
        Map<Direction, TexTile> aroundTex = new HashMap<Direction, TexTile>();
        for(Direction dir : around.keySet()) {
            aroundTex.put(dir, new TexTile(around.get(dir).getTexturePath()));
        }


        Image image = null;
        if(aroundTex.size() == 4) {
            image = TexEdge.hole.blend(background, aroundTex);

        } else if(aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.EAST) &&
                  aroundTex.keySet().contains(Direction.SOUTH)) {
            image = TexEdge.tip.blend(background, aroundTex, 270);
        } else if(aroundTex.keySet().contains(Direction.EAST) &&
                  aroundTex.keySet().contains(Direction.SOUTH) &&
                  aroundTex.keySet().contains(Direction.WEST)) {
            image = TexEdge.tip.blend(background, aroundTex, 180);
        } else if(aroundTex.keySet().contains(Direction.SOUTH) &&
                  aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.NORTH)) {
            image = TexEdge.tip.blend(background, aroundTex, 90);
        } else if(aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.EAST)) {
            image = TexEdge.tip.blend(background, aroundTex, 0);

        } else if(aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.SOUTH)) {
            image = TexEdge.narrow.blend(background, aroundTex, 90);
        } else if(aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.EAST)) {
            image = TexEdge.narrow.blend(background, aroundTex);

        } else if(aroundTex.keySet().contains(Direction.NORTH) &&
                  aroundTex.keySet().contains(Direction.WEST)) {
            image = TexEdge.corner.blend(background, aroundTex, 0);
        } else if(aroundTex.keySet().contains(Direction.WEST) &&
                  aroundTex.keySet().contains(Direction.SOUTH)) {
            image = TexEdge.corner.blend(background, aroundTex, 90);
        } else if(aroundTex.keySet().contains(Direction.SOUTH) &&
                  aroundTex.keySet().contains(Direction.EAST)) {
            image = TexEdge.corner.blend(background, aroundTex, 180);
        } else if(aroundTex.keySet().contains(Direction.EAST) &&
                  aroundTex.keySet().contains(Direction.NORTH)) {
            image = TexEdge.corner.blend(background, aroundTex, 270);

        } else if(aroundTex.keySet().contains(Direction.WEST)) {
            image = TexEdge.edge.blend(background, aroundTex, 0);
        } else if(aroundTex.keySet().contains(Direction.EAST)) {
            image = TexEdge.edge.blend(background, aroundTex, 180);
        } else if(aroundTex.keySet().contains(Direction.NORTH)) {
            image = TexEdge.edge.blend(background, aroundTex, 270);
        } else if(aroundTex.keySet().contains(Direction.SOUTH)) {
            image = TexEdge.edge.blend(background, aroundTex, 90);
            
        }

        return image == null ? background.img : image;
    }


    private static class TexTile {
        private int width, height;
        private int[] pixels;
        private BufferedImage img;

        public TexTile(String path) {
            this(path, false, 0);
        }

        public TexTile(String path, boolean hasAlpha, float angle) {
            ImageIcon image = new ImageIcon(path);
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
    }
}
