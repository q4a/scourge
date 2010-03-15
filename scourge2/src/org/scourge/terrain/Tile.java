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
    private Main main;
    private static final Map<String, Texture> textures = new HashMap<String, Texture>();

    public Tile(Main main) {
        this(main, TileTexType.NONE, TileType.NONE, 0);
    }

    public Tile(Main main, TileTexType tex, TileType type, float angle) {
        this.main = main;
        set(tex, type, angle);
    }

    public void set(TileTexType tex, TileType type, float angle) {
        this.tex = tex;
        this.type = type;
        this.angle = angle;
    }

    public void createSpatial(Map<Direction, TileTexType> around) {
        spatial = type.createSpatial(angle);
        applyTexture(around);
    }

    protected void applyTexture(Map<Direction, TileTexType> around) {
        if(tex.getTexturePath() != null && !type.isTexturePreset()) {
//            Texture texture = TextureManager.loadTexture(tex.getTexturePath(),
//                                                         Texture.MinificationFilter.Trilinear,
//                                                         Texture.MagnificationFilter.Bilinear);
//            texture.setWrap(Texture.WrapMode.Repeat);

            // remove the same textures
            for(Direction dir : Direction.values()) {
                if(around.get(dir) == null || around.get(dir).ordinal() >= tex.ordinal()) {
                    around.remove(dir);
                }
            }
            Texture texture;
            if(around.isEmpty()) {
                // no blending (TextureManager caches)
                texture = TextureManager.loadTexture(tex.getTexturePath(),
                                                     Texture.MinificationFilter.Trilinear,
                                                     Texture.MagnificationFilter.Bilinear); 
            } else {
                // blending (we cache it in textures)
                String key = createTextureKey(tex, around);
                texture = textures.get(key);
                if(texture == null) {
                    texture = createBlendedTexture(tex, around);
                    textures.put(key, texture);
                }
            }
            TextureState ts = main.getDisplay().getRenderer().createTextureState();
            ts.setTexture(texture);
            spatial.setRenderState(ts);
        }
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

    private Texture createBlendedTexture(TileTexType tex, Map<Direction, TileTexType> around) {
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

        return TextureManager.loadTexture(image == null ? background.img : image,
                                          Texture.MinificationFilter.Trilinear,
                                          Texture.MagnificationFilter.Bilinear,
                                          false);
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

    public static void debug() {
        System.err.println(textures.keySet());
    }
}
