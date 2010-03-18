package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Quaternion;
import com.jme.math.Vector2f;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.TexCoords;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.TextureState;
import com.jme.util.TextureManager;
import com.jmex.terrain.TerrainBlock;
import com.jmex.terrain.util.HillHeightMap;
import com.jmex.terrain.util.ProceduralTextureGenerator;
import org.scourge.*;
import org.scourge.terrain.Direction;

import javax.swing.*;
import java.nio.FloatBuffer;

/**
 * User: gabor
 * Date: Feb 11, 2010
 * Time: 11:05:14 AM
 */
public class Section implements NodeGenerator {
    private Node section;
    public static final float SECTION_WIDTH = 8;
    public static final float SECTION_HEIGHT = 3;
    public static final float BLOCK_WIDTH = SECTION_WIDTH / 4.0f;
    public static final float BLOCK_HEIGHT = SECTION_HEIGHT / 2.0f;
    private Main main;

    public Section(Main main, int x, int y, int z) {
        this.main = main;
        section = new Node(ShapeUtil.newShapeName("section"));

        Spatial ground = createGround();
        section.attachChild(ground);

        Spatial spatial = ShapeUtil.load3ds("./data/3ds/block.3ds", "./data/textures", "section");
        section.attachChild(spatial);

        // add rings going down to level 0
        for(int i = 0; i < y; i += SECTION_HEIGHT) {
            spatial = ShapeUtil.load3ds("./data/3ds/ring.3ds", "./data/textures", "ring");
            spatial.setLocalTranslation(new Vector3f(0, (i - y) * ShapeUtil.WALL_WIDTH, 0));
            spatial.updateModelBound();
            spatial.updateWorldBound();
            section.attachChild(spatial);
        }
        section.getLocalTranslation().set(x * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, z * ShapeUtil.WALL_WIDTH);
        section.setModelBound(new BoundingBox());
        section.updateModelBound();
        section.updateWorldBound();
    }

    public Spatial createGround() {
        float size = Section.SECTION_WIDTH * ShapeUtil.WALL_WIDTH;
        OutdoorHeightMap heightMap = new OutdoorHeightMap((int)(size / 4.0f), 20, 2.0f, 4.5f, (byte) 1);
        Vector3f terrainScale = new Vector3f(4.14f, 0.03f, 4.14f);
        TerrainBlock groundTerrain = new TerrainBlock(ShapeUtil.newShapeName("terrain"),
                                   heightMap.getSize(),
                                   terrainScale,
                                   heightMap.getHeightMap(),
                                   new Vector3f(0, 0, 0));
		groundTerrain.setDetailTexture(1, 16);

        // Some textures
        ProceduralTextureGenerator pt = new ProceduralTextureGenerator(heightMap);
        pt.addTexture(new ImageIcon("./data/textures/grass-tile.png"), -128, 0, 128);
        pt.addTexture(new ImageIcon("./data/textures/subtrop-tile.png"), 0, 128, 500); // last arg: 255
        //pt.addTexture(new ImageIcon("./data/textures/alpine.png"), 128, 255, 384);

        pt.createTexture(256);

        TextureState ts = main.getDisplay().getRenderer().createTextureState();
        ts.setEnabled(true);
        Texture t1 = TextureManager.loadTexture(pt.getImageIcon().getImage(),
                                                Texture.MinificationFilter.Trilinear,
                                                Texture.MagnificationFilter.Bilinear, true);
        ts.setTexture(t1, 0);

        Texture t2 = TextureManager.loadTexture("./data/textures/detail.png",
                                                Texture.MinificationFilter.Trilinear,
                                                Texture.MagnificationFilter.Bilinear);
        t2.setScale(new Vector3f(0.5f, 0.5f, 0.5f));
        ts.setTexture(t2, 1);
        t2.setWrap(Texture.WrapMode.Repeat);

        t1.setApply(Texture.ApplyMode.Combine);
        t1.setCombineFuncRGB(Texture.CombinerFunctionRGB.Modulate);
        t1.setCombineSrc0RGB(Texture.CombinerSource.CurrentTexture);
        t1.setCombineOp0RGB(Texture.CombinerOperandRGB.SourceColor);
        t1.setCombineSrc1RGB(Texture.CombinerSource.PrimaryColor);
        t1.setCombineOp1RGB(Texture.CombinerOperandRGB.SourceColor);

        t2.setApply(Texture.ApplyMode.Combine);
        t2.setCombineFuncRGB(Texture.CombinerFunctionRGB.AddSigned);
        t2.setCombineSrc0RGB(Texture.CombinerSource.CurrentTexture);
        t2.setCombineOp0RGB(Texture.CombinerOperandRGB.SourceColor);
        t2.setCombineSrc1RGB(Texture.CombinerSource.Previous);
        t2.setCombineOp1RGB(Texture.CombinerOperandRGB.SourceColor);
        groundTerrain.setRenderState(ts);

        groundTerrain.getLocalTranslation().set( -SECTION_WIDTH * ShapeUtil.WALL_WIDTH / 2, 0.05f, -SECTION_WIDTH * ShapeUtil.WALL_WIDTH / 2 );
        BoundingBox bb = new BoundingBox();
        groundTerrain.setModelBound(bb);
        groundTerrain.updateModelBound();
        groundTerrain.updateWorldBound();
        return groundTerrain;
    }

//    private Spatial createGround() {
//        Quad ground = new Quad(ShapeUtil.newShapeName("ground"), SECTION_WIDTH * ShapeUtil.WALL_WIDTH, SECTION_WIDTH * ShapeUtil.WALL_WIDTH);
//        FloatBuffer normBuf = ground.getNormalBuffer();
//        normBuf.clear();
//        normBuf.put(0).put(1).put(0);
//        normBuf.put(0).put(1).put(0);
//        normBuf.put(0).put(1).put(0);
//        normBuf.put(0).put(1).put(0);
//
//        Vector2f[] coords = new Vector2f[] {
//            new Vector2f(0, 0),
//            new Vector2f(SECTION_WIDTH / 2, 0),
//            new Vector2f(SECTION_WIDTH / 2, SECTION_WIDTH / 2),
//            new Vector2f(0, SECTION_WIDTH / 2)
//        };
//        TexCoords tc = TexCoords.makeNew(coords);
//        ground.setTextureCoords(tc);
//        ground.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * -90.0f, Vector3f.UNIT_X));
//        ground.setModelBound(new BoundingBox());
//        Texture texture = TextureManager.loadTexture("./data/textures/grass.png",
//                                                     Texture.MinificationFilter.Trilinear,
//                                                     Texture.MagnificationFilter.Bilinear);
//        texture.setWrap(Texture.WrapMode.Repeat);
//        TextureState ts = main.getDisplay().getRenderer().createTextureState();
//        ts.setTexture(texture);
//        ground.setRenderState(ts);
//        return ground;
//    }

    public void addMountain(Direction dir) {
        Spatial mountain = ShapeUtil.load3ds("./data/3ds/mountain.3ds", "./data/textures", "mountain");
        mountain.setName("mountain_" + dir.name());
        switch(dir) {
            case NORTH:
                mountain.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 180, Vector3f.UNIT_Y));
                mountain.setLocalTranslation(new Vector3f(0, 0, -SECTION_WIDTH * ShapeUtil.WALL_WIDTH / 2));
                break;
            case SOUTH:
                mountain.setLocalTranslation(new Vector3f(0, 0, SECTION_WIDTH * ShapeUtil.WALL_WIDTH / 2));
                break;
            case EAST:
                mountain.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 90, Vector3f.UNIT_Y));
                mountain.setLocalTranslation(new Vector3f(SECTION_WIDTH * ShapeUtil.WALL_WIDTH / 2, 0, 0));
                break;
            case WEST:
                mountain.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * -90, Vector3f.UNIT_Y));
                mountain.setLocalTranslation(new Vector3f(-SECTION_WIDTH * ShapeUtil.WALL_WIDTH / 2, 0, 0));
                break;
        }
        section.attachChild(mountain);
        section.updateModelBound();
        section.updateWorldBound();
    }


    @Override
    public Node getNode() {
        return section;
    }

    public void removeMountain(Direction dir) {
        Spatial mountain = section.getChild("mountain_" + dir.name());
        if(mountain != null) {
            section.detachChild(mountain);
            section.updateModelBound();
            section.updateWorldBound();
        }
    }

    public void addTown() {
        Town town = new Town(main, 0, 0, 0, main.getRandom());
        section.attachChild(town.getNode());
        section.updateModelBound();
        section.updateWorldBound();
    }

    public void addLedge(Ledge ledge, float angle, int x, int y, int z) {
        Spatial spatial = ledge.getSpatial();
        spatial.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * angle, Vector3f.UNIT_Y));
        spatial.getLocalTranslation().set((x - 1.5f) * Ledge.LEDGE_WIDTH * ShapeUtil.WALL_WIDTH,
                                          (y + 1) * Ledge.LEDGE_HEIGHT * ShapeUtil.WALL_WIDTH,
                                          (z - 1.5f) * Ledge.LEDGE_WIDTH * ShapeUtil.WALL_WIDTH);
        spatial.setModelBound(new BoundingBox());
        spatial.updateModelBound();
        spatial.updateWorldBound();

        section.attachChild(spatial);
        section.updateModelBound();
        section.updateWorldBound();
    }

    public void addBlock(float x, float y, float z) {
        Node node = new Node(ShapeUtil.newShapeName("block"));

        Spatial spatial = ShapeUtil.load3ds("./data/3ds/block-qtr.3ds", "./data/textures", "block-model");
        //spatial.getLocalScale().set(0.25f, 0.5f, 0.25f);
        node.attachChild(spatial);

        Spatial ground = createBlockGround();
        node.attachChild(ground);

        node.getLocalTranslation().set(x - (1.5f * BLOCK_WIDTH * ShapeUtil.WALL_WIDTH),
                                       (y + BLOCK_HEIGHT * ShapeUtil.WALL_WIDTH),
                                       z - (1.5f * BLOCK_WIDTH * ShapeUtil.WALL_WIDTH));
        node.setModelBound(new BoundingBox());
        node.updateModelBound();
        node.updateWorldBound();
        section.attachChild(node);
        section.updateModelBound();
        section.updateWorldBound();
    }

    private Spatial createBlockGround() {
//        Quad ground = new Quad(ShapeUtil.newShapeName("ground"), BLOCK_WIDTH * ShapeUtil.WALL_WIDTH, BLOCK_WIDTH * ShapeUtil.WALL_WIDTH);
//        FloatBuffer normBuf = ground.getNormalBuffer();
//        normBuf.clear();
//        normBuf.put(0).put(1).put(0);
//        normBuf.put(0).put(1).put(0);
//        normBuf.put(0).put(1).put(0);
//        normBuf.put(0).put(1).put(0);
//
//        Vector2f[] coords = new Vector2f[] {
//            new Vector2f(0, 0),
//            new Vector2f(BLOCK_WIDTH / 2, 0),
//            new Vector2f(BLOCK_WIDTH / 2, BLOCK_WIDTH / 2),
//            new Vector2f(0, BLOCK_WIDTH / 2)
//        };
//        TexCoords tc = TexCoords.makeNew(coords);
//        ground.setTextureCoords(tc);
//        ground.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * -90.0f, Vector3f.UNIT_X));
//        ground.setModelBound(new BoundingBox());
//        Texture texture = TextureManager.loadTexture("./data/textures/grass.png",
//                                                     Texture.MinificationFilter.Trilinear,
//                                                     Texture.MagnificationFilter.Bilinear);
//        texture.setWrap(Texture.WrapMode.Repeat);
//        TextureState ts = main.getDisplay().getRenderer().createTextureState();
//        ts.setTexture(texture);
//        ground.setRenderState(ts);
//        return ground;

        float size = Section.BLOCK_WIDTH * ShapeUtil.WALL_WIDTH;
        OutdoorHeightMap heightMap = new OutdoorHeightMap((int)(size / 4.0f), 40, 0.5f, 1.0f, (byte) 1);
        Vector3f terrainScale = new Vector3f(4.6f, 0.015f, 4.6f);
        TerrainBlock groundTerrain = new TerrainBlock(ShapeUtil.newShapeName("terrain"),
                                   heightMap.getSize(),
                                   terrainScale,
                                   heightMap.getHeightMap(),
                                   new Vector3f(0, 0, 0));
		groundTerrain.setDetailTexture(1, 16);

        // Some textures
        ProceduralTextureGenerator pt = new ProceduralTextureGenerator(heightMap);
        pt.addTexture(new ImageIcon("./data/textures/grass.png"), -128, 0, 128);
        pt.addTexture(new ImageIcon("./data/textures/subtrop.png"), 0, 128, 500); // last arg: 255
        //pt.addTexture(new ImageIcon("./data/textures/alpine.png"), 128, 255, 384);

        pt.createTexture(256);

        TextureState ts = main.getDisplay().getRenderer().createTextureState();
        ts.setEnabled(true);
        Texture t1 = TextureManager.loadTexture(pt.getImageIcon().getImage(),
                                                Texture.MinificationFilter.Trilinear,
                                                Texture.MagnificationFilter.Bilinear, true);
        ts.setTexture(t1, 0);

        Texture t2 = TextureManager.loadTexture("./data/textures/detail.png",
                                                Texture.MinificationFilter.Trilinear,
                                                Texture.MagnificationFilter.Bilinear);
        t2.setScale(new Vector3f(0.5f, 0.5f, 0.5f));
        ts.setTexture(t2, 1);
        t2.setWrap(Texture.WrapMode.Repeat);

        t1.setApply(Texture.ApplyMode.Combine);
        t1.setCombineFuncRGB(Texture.CombinerFunctionRGB.Modulate);
        t1.setCombineSrc0RGB(Texture.CombinerSource.CurrentTexture);
        t1.setCombineOp0RGB(Texture.CombinerOperandRGB.SourceColor);
        t1.setCombineSrc1RGB(Texture.CombinerSource.PrimaryColor);
        t1.setCombineOp1RGB(Texture.CombinerOperandRGB.SourceColor);

        t2.setApply(Texture.ApplyMode.Combine);
        t2.setCombineFuncRGB(Texture.CombinerFunctionRGB.AddSigned);
        t2.setCombineSrc0RGB(Texture.CombinerSource.CurrentTexture);
        t2.setCombineOp0RGB(Texture.CombinerOperandRGB.SourceColor);
        t2.setCombineSrc1RGB(Texture.CombinerSource.Previous);
        t2.setCombineOp1RGB(Texture.CombinerOperandRGB.SourceColor);
        groundTerrain.setRenderState(ts);

        groundTerrain.getLocalTranslation().set( -BLOCK_WIDTH * ShapeUtil.WALL_WIDTH / 2, 0.05f, -BLOCK_WIDTH * ShapeUtil.WALL_WIDTH / 2 );
        BoundingBox bb = new BoundingBox();
        groundTerrain.setModelBound(bb);
        groundTerrain.updateModelBound();
        groundTerrain.updateWorldBound();
        return groundTerrain;
    }

    public void addStairs(float x, float y, float z, float angle) {
        Spatial stairs = ShapeUtil.load3ds("./data/3ds/stairs.3ds", "./data/textures", "stairs");
        stairs.getLocalTranslation().set(x - 2 * BLOCK_WIDTH * ShapeUtil.WALL_WIDTH, y, z - 1.5f * BLOCK_WIDTH * ShapeUtil.WALL_WIDTH);
        stairs.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * angle, Vector3f.UNIT_Y));
        stairs.setModelBound(new BoundingBox());
        stairs.updateModelBound();
        stairs.updateWorldBound();
        section.attachChild(stairs);
        section.updateModelBound();
        section.updateWorldBound();
    }

}
