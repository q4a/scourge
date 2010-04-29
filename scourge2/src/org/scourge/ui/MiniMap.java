package org.scourge.ui;

import com.jme.math.FastMath;
import com.jme.math.Vector3f;
import com.jme.renderer.ColorRGBA;
import com.jme.renderer.Renderer;
import com.jme.scene.Node;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.BlendState;
import com.jme.scene.state.ColorMaskState;
import com.jme.scene.state.RenderState;
import com.jme.scene.state.StencilState;
import com.jme.system.DisplaySystem;
import org.scourge.Main;
import org.scourge.terrain.Md2Model;
import org.scourge.terrain.Region;
import org.scourge.ui.component.WinUtil;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

/**
 * User: gabor
 * Date: Apr 24, 2010
 * Time: 3:46:15 PM
 */
public class MiniMap {
    private Map<String, Quad> tiles = new HashMap<String, Quad>();
    private Set<String> toBeRemoved = new HashSet<String>();
    private Vector3f centerTrans = new Vector3f();
    private Node node;
    private static final int MINIMAP_TEX_SIZE = 256;
    private static final int MINIMAP_TILE_TEX_SIZE = 128;
    private static final float MINIMAP_TILE_SIZE = 128f;
    private static final int MINIMAP_TILES_PER_ROW = 11;
    private Node map;
    private float[] angles = new float[3];
    private static final float DIRECTION_ADJUSTMENT = FastMath.DEG_TO_RAD * 90f;

    public MiniMap() {
        Renderer renderer = DisplaySystem.getDisplaySystem().getRenderer();

        node = new Node("minimap");
        WinUtil.makeNodeOrtho(node);
        node.clearRenderState(RenderState.StateType.Light);
        node.getLocalTranslation().set(MINIMAP_TEX_SIZE / 2 + 5, renderer.getHeight() - MINIMAP_TEX_SIZE / 2 - 5, 0);

        // stencil
        {
            Quad stencil = WinUtil.createQuad("minimap.map.stencil", MINIMAP_TEX_SIZE, MINIMAP_TEX_SIZE, "data/textures/minimask.png");
            stencil.setSolidColor(ColorRGBA.white);

            ColorMaskState cms = renderer.createColorMaskState();
            cms.setAll(false);
            cms.setEnabled(true);
            stencil.setRenderState(cms);

            StencilState ss = renderer.createStencilState();
            ss.setEnabled(true);
            ss.setUseTwoSided(false);
            ss.setStencilOpFail(StencilState.StencilOperation.Replace);
            ss.setStencilOpZFail(StencilState.StencilOperation.Replace);
            ss.setStencilOpZPass(StencilState.StencilOperation.Replace);
            ss.setStencilFunction(StencilState.StencilFunction.Always);
            ss.setStencilMask(0xffffffff);
            ss.setStencilReference(0x1);
            stencil.setRenderState(ss);

            BlendState bs = renderer.createBlendState();
            bs.setBlendEnabled(false);
            bs.setTestEnabled(true);
            bs.setTestFunction(BlendState.TestFunction.NotEqualTo);
            bs.setReference(0);
            stencil.setRenderState(bs);

            stencil.updateRenderState();
            stencil.updateGeometricState(0, true);
            stencil.setZOrder(2);
            node.attachChild(stencil);
        }

        // the map
        {
            map = new Node("minimap.map.contents");

            StencilState ss = renderer.createStencilState();
            ss.setEnabled(true);
            ss.setUseTwoSided(false);
            ss.setStencilOpFail(StencilState.StencilOperation.Keep);
            ss.setStencilOpZFail(StencilState.StencilOperation.Keep);
            ss.setStencilOpZPass(StencilState.StencilOperation.Keep);
            ss.setStencilFunction(StencilState.StencilFunction.EqualTo);
            ss.setStencilMask(0xffffffff);
            ss.setStencilReference(0x1);
            map.setRenderState(ss);

            map.updateRenderState();
            map.updateGeometricState(0, true);
            map.setZOrder(1);
            node.attachChild(map);
        }

        // the cover
        {
            Quad cover = WinUtil.createQuad("minimap.map.stencil", MINIMAP_TEX_SIZE, MINIMAP_TEX_SIZE, "data/textures/minimap.png");

            BlendState bs = renderer.createBlendState();
            bs.setBlendEnabled(false);
            bs.setTestEnabled(true);
            bs.setTestFunction(BlendState.TestFunction.NotEqualTo);
            bs.setReference(0);
            cover.setRenderState(bs);

            cover.setZOrder(0);
            node.attachChild(cover);
        }


        node.updateGeometricState(0, true);
        node.updateRenderState();
    }

    public Node getNode() {
        return node;
    }

    public void update(boolean terrainChanged) {
        // move to player position
        Md2Model pm = Main.getMain().getPlayer().getCreatureModel();
        float px = -pm.getX() / MINIMAP_TILE_SIZE * MINIMAP_TILE_TEX_SIZE;
        float py = pm.getZ() / MINIMAP_TILE_SIZE * MINIMAP_TILE_TEX_SIZE;
        map.getLocalTranslation().set(px, py, 0);

        pm.getNode().getLocalRotation().toAngles(angles);
        node.getLocalRotation().fromAngles(0, 0, -angles[1] + DIRECTION_ADJUSTMENT);

        node.updateRenderState();
        node.updateGeometricState(0, true);
        centerTrans.set(-px, -py, 0);

        if(terrainChanged) {
            // synchronize the loaded regions with the minimap
            Map<String, Region> loadedRegions = Main.getMain().getTerrain().getLoadedRegions();
            for(Region region : loadedRegions.values()) {
                int mx = Math.round(region.getX() / MINIMAP_TILE_SIZE);
                int my = Math.round(region.getY() / MINIMAP_TILE_SIZE);
                for(int xx = mx - 1; xx <= mx + 1; xx++) {
                    for(int yy = my - 1; yy <= my + 1; yy++) {
                        String key = "map_" + (yy * MINIMAP_TILES_PER_ROW + xx);
                        if(tiles.get(key) == null) {
                            System.err.println(">>> ADDING: " + key + " at " + (xx * MINIMAP_TILE_TEX_SIZE + px) + "," + (-yy * MINIMAP_TILE_TEX_SIZE + py));
                            Quad quad = WinUtil.createQuad(key, MINIMAP_TILE_TEX_SIZE, MINIMAP_TILE_TEX_SIZE, "data/textures/minimap/" + key + ".png");
                            quad.getLocalTranslation().set(xx * MINIMAP_TILE_TEX_SIZE, -yy * MINIMAP_TILE_TEX_SIZE, 0);
                            quad.setDefaultColor(new ColorRGBA(1, 1, 1, 0.8f));

                            BlendState as = DisplaySystem.getDisplaySystem().getRenderer().createBlendState();
                            as.setBlendEnabled(true);
                            as.setSourceFunction(BlendState.SourceFunction.SourceAlpha);
                            as.setDestinationFunction(BlendState.DestinationFunction.OneMinusSourceAlpha);
                            as.setEnabled(true);
                            quad.setRenderState(as);

                            tiles.put(key, quad);
                            map.attachChild(quad);
                        }
                    }
                }
            }

            // unload the ones no longer needed
            toBeRemoved.clear();
            for(String key : tiles.keySet()) {
                Quad quad = tiles.get(key);
                if(quad.getLocalTranslation().distance(centerTrans) > MINIMAP_TILE_TEX_SIZE * 3) {
                    toBeRemoved.add(key);
                }
            }
            for(String key : toBeRemoved) {
                System.err.println(">>> REMOVING: " + key);
                map.detachChild(tiles.remove(key));
            }
            map.updateRenderState();
            map.updateGeometricState(0, true);
        }
    }
}
