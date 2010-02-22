package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.bounding.BoundingVolume;
import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.state.TextureState;
import com.jme.util.TextureManager;
import com.jmex.terrain.TerrainBlock;
import com.jmex.terrain.util.*;
import org.scourge.Main;
import org.scourge.terrain.NodeGenerator;
import org.scourge.terrain.Section;
import org.scourge.terrain.ShapeUtil;

import javax.swing.*;

/**
 * User: gabor
 * Date: Feb 7, 2010
 * Time: 5:53:13 PM
 */
public class Ground implements NodeGenerator {
    private Node ground;
    private AbstractHeightMap heightMap;
    private TerrainBlock terrain;

    public Ground(Main main, float x, float y, float z) {
        ground = new Node(ShapeUtil.newShapeName("ground"));

        // The terrain
		//heightMap = new HillHeightMap(129, 2000, 5.0f, 20.0f, (byte) 2);
        heightMap = new FluidSimHeightMap(32, 2000, 0, 0.001f, 15, 15, 0.033f, 5, System.currentTimeMillis());
        //heightMap = new MidPointHeightMap(128, 1);
        //heightMap = new ParticleDepositionHeightMap(128, 10, 2, 10, 30, 0);
		//heightMap.setHeightScale(0.25f);
        //heightMap.setHeightScale(0.005f);
		//Vector3f terrainScale = new Vector3f(ShapeUtil.WALL_WIDTH / 2, 0.05f, ShapeUtil.WALL_WIDTH / 2);
        float size = Section.SECTION_WIDTH * ShapeUtil.WALL_WIDTH;
        Vector3f terrainScale = new Vector3f(size / heightMap.getSize(),
                                             0.03f,
                                             size / heightMap.getSize());
//        terrain = new TerrainPage("Terrain", 33, heightMap.getSize(), terrainScale, heightMap.getHeightMap());
        terrain = new TerrainBlock(ShapeUtil.newShapeName("terrain"),
                                   heightMap.getSize(),
                                   terrainScale,
                                   heightMap.getHeightMap(),
                                   new Vector3f(0, 0, 0));


		terrain.setDetailTexture(1, 16);
        //terrain.setModelBound(new BoundingBox());
		ground.attachChild(terrain);

		// Some textures
		ProceduralTextureGenerator pt = new ProceduralTextureGenerator(heightMap);
        pt.addTexture(new ImageIcon("./data/textures/grass.png"), -128, 0, 128);
        pt.addTexture(new ImageIcon("./data/textures/subtrop.png"), 0, 128, 255);
        pt.addTexture(new ImageIcon("./data/textures/alpine.png"), 128, 255, 384);

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
		ground.setRenderState(ts);

        ground.getLocalTranslation().x = x * ShapeUtil.WALL_WIDTH - size / 2;
        ground.getLocalTranslation().y = y * ShapeUtil.WALL_WIDTH;
        ground.getLocalTranslation().z = z * ShapeUtil.WALL_WIDTH - size / 2;
        BoundingBox bb = new BoundingBox();
        ground.setModelBound(bb);
        ground.updateModelBound();
        ground.updateWorldBound();
    }

    public Node getNode() {
        return ground;
    }

    public float getHeight(Vector3f location) {
        terrain.worldToLocal(location, calcVec1);
        float height = terrain.getHeight(calcVec1);
        // seems like getHeightFromWorld is incorrect
//        float height = terrain.getHeightFromWorld(location);
        return(Float.isNaN(height) ? 0 : height);
    }


    private Vector3f calcVec1 = new Vector3f();
    public void flatten(BoundingVolume bv, float height) {
        // find the local center
        Vector3f localCenter = calcVec1.set(bv.getCenter()).subtractLocal(ground.getLocalTranslation());

        for(int x = 0; x < heightMap.getSize(); x++) {
            for(int z = 0; z < heightMap.getSize(); z++) {
                float px = x * terrain.getStepScale().x;
                float pz = z * terrain.getStepScale().z;
                
                if(FastMath.abs(localCenter.x - px) < ((BoundingBox)bv).xExtent + terrain.getStepScale().x &&
                   FastMath.abs(localCenter.z - pz) < ((BoundingBox)bv).zExtent + terrain.getStepScale().z) {
                    heightMap.setHeightAtPoint(height / terrain.getStepScale().y, x, z);
                }
            }
        }
        terrain.updateFromHeightMap();
    }
}
