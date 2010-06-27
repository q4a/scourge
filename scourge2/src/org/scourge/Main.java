package org.scourge;

import com.jme.image.Texture;
import com.jme.input.InputHandler;
import com.jme.input.MouseInput;
import com.jme.light.DirectionalLight;
import com.jme.math.*;
import com.jme.renderer.Camera;
import com.jme.renderer.ColorRGBA;
import com.jme.renderer.Renderer;
import com.jme.renderer.pass.RenderPass;
import com.jme.scene.*;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.*;
import com.jme.system.DisplaySystem;
import com.jme.system.JmeException;
import com.jme.util.TextureManager;
import com.jmex.effects.water.WaterRenderPass;
import com.jmex.font2d.Font2D;
import com.jmex.font2d.Text2D;
import org.scourge.editor.MapSymbol;
import org.scourge.input.PlayerController;
import org.scourge.io.BlockData;
import org.scourge.model.Creature;
import org.scourge.terrain.*;
import org.scourge.ui.MiniMap;
import org.scourge.ui.component.DragSource;
import org.scourge.ui.component.Dragable;
import org.scourge.ui.component.WinUtil;
import org.scourge.ui.component.Window;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Main extends Game {
    private InputHandler playerController;
    private Node cameraHolder;
    private CameraNode camNode;
    private Skybox skybox;
    private Terrain terrain;
    private Quad draggingIcon;

    private FogState fogState;
    private WaterRenderPass waterEffectRenderPass;
    private Quad waterQuad;
    private float farPlane = 10000.0f;
    private float textureScale = 0.02f;
    private Random random = new Random(17L);
    private Text2D positionLabel, loadingLabel;
    private GameState gameState;
    private Creature player;
    private MiniMap miniMap;

    private static Main main;
    private RenderPass mapPass;
    private Dragable dragging;
    private int dragOffsetX, dragOffsetY;
    private Set<Dragable> firsts = new HashSet<Dragable>();
    private Selection dropSelection, dragSelection;
    private Map<String, Dragable> dragables = new HashMap<String, Dragable>();
    private DragSource dragSource;
    private boolean inDungeon, inUpDown;
    private boolean updateRoof;
    private boolean fogOnWater;

    public static void main(String[] args) {
        main = new Main();
        main.setConfigShowMode(ConfigShowMode.ShowIfNoConfig);
        main.start();
    }

    @Override
    protected void initSystem() throws JmeException {
        super.initSystem();
        Logger.getLogger("com.jme").setLevel(Level.SEVERE);
        //cameraParallel();
    }

    protected void cleanup() {
        super.cleanup();
        waterEffectRenderPass.cleanup();
    }

    protected void simpleInitGame() {
        MouseInput.get().setCursorVisible(true);

        display.setTitle("Scourge II");
        cam.setLocation(new Vector3f(0,0,0));
        cam.update();

        CullState cs = display.getRenderer().createCullState();
		cs.setCullFace(CullState.Face.Back);
		cs.setEnabled(true);
		rootNode.setRenderState(cs);

        fogState = display.getRenderer().createFogState();
        fogState.setDensity(1.0f);
        fogState.setEnabled(true);
        fogState.setDensityFunction(FogState.DensityFunction.Linear);
        fogState.setQuality(FogState.Quality.PerVertex);
        rootNode.setRenderState(fogState);

        lightState.detachAll();
        DirectionalLight dr = new DirectionalLight();
        dr.setDiffuse(new ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        dr.setAmbient(new ColorRGBA(0.5f, 0.5f, 0.5f, 1.0f));
        dr.setSpecular(new ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        dr.setDirection(new Vector3f(1, -1, 1));
        dr.setEnabled(true);
        lightState.attach(dr);
        dr = new DirectionalLight();
        dr.setDiffuse(new ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        dr.setAmbient(new ColorRGBA(0.5f, 0.5f, 0.5f, 1.0f));
        dr.setSpecular(new ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        dr.setDirection(new Vector3f(-1, -1, 1));
        dr.setEnabled(true);
        lightState.attach(dr);

        Font2D font = new Font2D();
		ZBufferState zbs = DisplaySystem.getDisplaySystem().getRenderer().createZBufferState();
		zbs.setFunction(ZBufferState.TestFunction.Always);
        
        positionLabel = font.createText("", 8, 0);
		positionLabel.setRenderQueueMode(Renderer.QUEUE_ORTHO);
		positionLabel.setRenderState(zbs);
        positionLabel.setRenderState( Text.getDefaultFontTextureState() );
        positionLabel.setRenderState( Text.getFontBlend() );
        positionLabel.setCullHint(Spatial.CullHint.Never);
        positionLabel.setLocalScale(.8f);
		positionLabel.setTextColor(new ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        positionLabel.getLocalTranslation().addLocal(250, 0, 0);
		positionLabel.updateRenderState();
		rootNode.attachChild(positionLabel);

        loadingLabel = font.createText("Loading...", 8, 0);
		loadingLabel.setRenderQueueMode(Renderer.QUEUE_ORTHO);
		loadingLabel.setRenderState(zbs);
        loadingLabel.setRenderState( Text.getDefaultFontTextureState() );
        loadingLabel.setRenderState( Text.getFontBlend() );
        loadingLabel.setLocalScale(.8f);
		loadingLabel.setTextColor(new ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
		loadingLabel.updateRenderState();
        loadingLabel.setCullHint(Spatial.CullHint.Never);
        loadingLabel.getLocalTranslation().set((display.getRenderer().getWidth() - loadingLabel.getWidth()) / 2, (display.getRenderer().getHeight() - loadingLabel.getHeight()) / 2, 0);
		rootNode.attachChild(loadingLabel);
        miniMap = new MiniMap();
        
        gameState = new GameState();
        try {
            terrain = new Terrain(this);
        } catch(IOException exc) {
            throw new RuntimeException(exc);
        }

        cameraHolder = new Node("cam_holder");

        playerController = new PlayerController(this);
        input = playerController;

        Node reflectedNode = new Node("reflectNode");

        buildSkyBox();
        reflectedNode.attachChild(skybox);
        reflectedNode.attachChild(terrain.getNode());
        rootNode.attachChild(reflectedNode);

        dropSelection = new Selection(rootNode);
        dragSelection = new Selection(rootNode); 

        // setup the water plane
        waterEffectRenderPass = new WaterRenderPass(cam, 4, false, false); // setting last param to false renders faster
        waterEffectRenderPass.setWaterPlane(new Plane(new Vector3f(0.0f, 1.0f, 0.0f), 0.0f));
        waterEffectRenderPass.useFadeToFogColor(true);

        waterQuad = new Quad("waterQuad", 1, 1);
        FloatBuffer normBuf = waterQuad.getNormalBuffer();
        normBuf.clear();
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);
        normBuf.put(0).put(1).put(0);

        waterEffectRenderPass.setWaterEffectOnSpatial(waterQuad);
        rootNode.attachChild(waterQuad);

        waterEffectRenderPass.setReflectedScene(reflectedNode);
        waterEffectRenderPass.setSkybox(skybox);

        pManager.add(waterEffectRenderPass);

        RenderPass rootPass = new RenderPass();
        rootPass.add(rootNode);
        pManager.add(rootPass);

        mapPass = new RenderPass();
        mapPass.setEnabled(false);
        mapPass.add(miniMap.getNode());
        pManager.add(mapPass);

        RenderPass statPass = new RenderPass();
        statPass.add(statNode);
        pManager.add(statPass);

        rootNode.setCullHint(Spatial.CullHint.Dynamic);
        rootNode.setRenderQueueMode(Renderer.QUEUE_OPAQUE);

        setFogOnWater(true);

        if(!"true".equalsIgnoreCase(System.getProperty("test.mode"))) {
            gameState.showMainMenu();
        }
    }

    public void showWindow(Window win) {
        rootNode.attachChild(win.getNode());
    }

    public void hideWindow(Window win) {
        rootNode.detachChild(win.getNode());
    }

    public void setCameraFollowsPlayer(boolean follows) {
        if(follows) {
            if(camNode == null) {
                toggleCameraAttached();
            }
            playerController.setEnabled(true);
            input = playerController;
        } else {
            if(camNode != null) {
                toggleCameraAttached();
            }
            playerController.setEnabled(false);
            input = null;
        }
    }

    public void toggleCameraAttached() {
        if(camNode != null) {
            input = firstPersonHandler;
            cameraHolder.detachChild(camNode);
            camNode.setCamera(null);
            camNode = null;
        } else {
            camNode = new CameraNode("camera node", cam);
            camNode.setLocalTranslation(new Vector3f(-380, 350, 0));
            camNode.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 90.0f, Vector3f.UNIT_Y));
            camNode.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 35.0f, Vector3f.UNIT_X));
            camNode.updateWorldData(0);
            cameraHolder.attachChild(camNode);
            input = playerController;
        }
    }

    @Override
    protected void simpleUpdate() {
        if(updateRoof) {
            updateRoof = false;
            terrain.setRoofVisible(!inDungeon);
            updateFog();
        }        
        terrain.update(tpf);

        // the world vectors aren't computed until the first update :-/
        if(terrain.getCurrentRegion() != null) {
            terrain.getCurrentRegion().moveToTopOfTerrain();

            if(player != null) {
                player.getCreatureModel().moveToTopOfTerrain();

                Tile tile = getTerrain().getCurrentRegion().getTile(
                    player.getCreatureModel().getX() % Region.REGION_SIZE,
                    player.getCreatureModel().getZ() % Region.REGION_SIZE);

                positionLabel.setText("Player: " + player.getCreatureModel().getX() + "," + player.getCreatureModel().getZ() +
                                      " (" + (player.getCreatureModel().getX() % Region.REGION_SIZE) + "," + (player.getCreatureModel().getZ() % Region.REGION_SIZE) + ")" +
                                      " region: " + getTerrain().getCurrentRegion().getX() + "," + getTerrain().getCurrentRegion().getY() +
                                      " (" + getTerrain().getCurrentRegion().getX() / Region.REGION_SIZE + "," + getTerrain().getCurrentRegion().getY() / Region.REGION_SIZE + ")" +
                                      " inDungeon=" + inDungeon +
                                      " inUpDown=" + inUpDown +
                                      " symbol=" + tile.getC() +
                                      " block=" + tile.getBlockData());
                positionLabel.updateRenderState();
                positionLabel.updateModelBound();
            }
        }

        // so lame... this can't be done until the bounding box is calculated
        for(Dragable d : firsts) {
            d.scaleModel();
            Terrain.moveOnTopOfTerrain(d.getModel());
        }
        firsts.clear();

        skybox.getLocalTranslation().set(cam.getLocation());
        skybox.updateGeometricState(0.0f, true);

        if(draggingIcon != null) {
            MouseInput mouseInput = MouseInput.get();
            draggingIcon.getLocalTranslation().set(mouseInput.getXAbsolute() + dragOffsetX, mouseInput.getYAbsolute() - dragOffsetY, 0);
        }

        Vector3f transVec = new Vector3f(cam.getLocation().x,
                waterEffectRenderPass.getWaterHeight(), cam.getLocation().z);

        setTextureCoords(0, transVec.x, -transVec.z, textureScale);

        // vertex coords
        setVertexCoords(transVec.x, transVec.y, transVec.z);
    }

    private void setVertexCoords(float x, float y, float z) {
        FloatBuffer vertBuf = waterQuad.getVertexBuffer();
        vertBuf.clear();

        vertBuf.put(x - farPlane).put(y).put(z - farPlane);
        vertBuf.put(x - farPlane).put(y).put(z + farPlane);
        vertBuf.put(x + farPlane).put(y).put(z + farPlane);
        vertBuf.put(x + farPlane).put(y).put(z - farPlane);
    }

    private void setTextureCoords(int buffer, float x, float y,
            float textureScale) {
        x *= textureScale * 0.5f;
        y *= textureScale * 0.5f;
        textureScale = farPlane * textureScale;
        FloatBuffer texBuf;
        texBuf = waterQuad.getTextureCoords(buffer).coords;
        texBuf.clear();
        texBuf.put(x).put(textureScale + y);
        texBuf.put(x).put(y);
        texBuf.put(textureScale + x).put(y);
        texBuf.put(textureScale + x).put(textureScale + y);
    }
    

    public DisplaySystem getDisplay() {
        return display;
    }

    public Creature getPlayer() {
        return player;
    }

    public void setPlayer(Creature newPlayer) {
        if(player != null) {
            player.getCreatureModel().getNode().detachChild(cameraHolder);
            rootNode.detachChild(player.getCreatureModel().getNode());
        }
        player = newPlayer;
        player.getCreatureModel().getNode().attachChild(cameraHolder);

        // todo: this is not the right place for this
        rootNode.attachChild(player.getCreatureModel().getNode());
        player.getCreatureModel().getNode().updateWorldBound();
        player.getCreatureModel().getNode().updateRenderState();
    }

    public Terrain getTerrain() {
        return terrain;
    }

    public Node getCameraHolder() {
        return cameraHolder;
    }

    public Camera getCamera() {
        return cam;
    }

    public CameraNode getCameraNode() {
        return camNode;
    }

    private void buildSkyBox() {
        skybox = new Skybox("skybox", 10, 10, 10);

        String dir = "./data/textures/skybox1";
        Texture north = TextureManager.loadTexture(dir + "/1.jpg",
                Texture.MinificationFilter.BilinearNearestMipMap,
                Texture.MagnificationFilter.Bilinear);
        Texture south = TextureManager.loadTexture(dir + "/3.jpg",
                Texture.MinificationFilter.BilinearNearestMipMap,
                Texture.MagnificationFilter.Bilinear);
        Texture east = TextureManager.loadTexture(dir + "/2.jpg",
                Texture.MinificationFilter.BilinearNearestMipMap,
                Texture.MagnificationFilter.Bilinear);
        Texture west = TextureManager.loadTexture(dir + "/4.jpg",
                Texture.MinificationFilter.BilinearNearestMipMap,
                Texture.MagnificationFilter.Bilinear);
        Texture up = TextureManager.loadTexture(dir + "/6.jpg",
                Texture.MinificationFilter.BilinearNearestMipMap,
                Texture.MagnificationFilter.Bilinear);
        Texture down = TextureManager.loadTexture(dir + "/5.jpg",
                Texture.MinificationFilter.BilinearNearestMipMap,
                Texture.MagnificationFilter.Bilinear);

        skybox.setTexture(Skybox.Face.North, north);
        skybox.setTexture(Skybox.Face.West, west);
        skybox.setTexture(Skybox.Face.South, south);
        skybox.setTexture(Skybox.Face.East, east);
        skybox.setTexture(Skybox.Face.Up, up);
        skybox.setTexture(Skybox.Face.Down, down);
        skybox.preloadTextures();

        CullState cullState = display.getRenderer().createCullState();
        cullState.setCullFace(CullState.Face.None);
        cullState.setEnabled(true);
        skybox.setRenderState(cullState);

        ZBufferState zState = display.getRenderer().createZBufferState();
        zState.setEnabled(false);
        skybox.setRenderState(zState);

        FogState fs = display.getRenderer().createFogState();
        fs.setEnabled(false);
        skybox.setRenderState(fs);

        skybox.setLightCombineMode(Spatial.LightCombineMode.Off);
        skybox.setCullHint(Spatial.CullHint.Never);
        skybox.setTextureCombineMode(Spatial.TextureCombineMode.Replace);
        skybox.updateRenderState();

        skybox.lockBounds();
        skybox.lockMeshes();
    }

    public Random getRandom() {
        return random;
    }

    private int loadingCounter;
    public void setLoading(boolean loading) {
        if(loading) {
            loadingCounter++;
            loadingLabel.setCullHint(Spatial.CullHint.Never);
        } else {
            loadingCounter--;
            if(loadingCounter == 0) {
                loadingLabel.setCullHint(Spatial.CullHint.Always);
            }
        }
    }

    public boolean isLoading() {
        return loadingCounter > 0;
    }

    protected void escapePressed() {
        if(gameState.escapePressed()) finish();
    }

    public PlayerController getPlayerController() {
        return (PlayerController)playerController;
    }

    public static Main getMain() {
        return main;
    }

    public void setFogOnWater(boolean b) {
        fogOnWater = b;
        updateFog();
    }

    public void updateFog() {
        waterEffectRenderPass.useFadeToFogColor(fogOnWater);
        if(fogOnWater) {
            if(inDungeon) {
                fogState.setColor(ColorRGBA.black);
                fogState.setEnd(cam.getFrustumFar() * 0.16f);
                fogState.setStart(cam.getFrustumFar() * 0.02f);
            } else {
                fogState.setColor(new ColorRGBA(0.65f, 0.65f, 0.75f, 1.0f));
                fogState.setEnd(cam.getFrustumFar() * 0.25f);
                fogState.setStart(cam.getFrustumFar() * 0.175f);
            }
        } else {
            fogState.setColor(new ColorRGBA(0.65f, 0.65f, 0.75f, 1.0f));
            fogState.setEnd(cam.getFrustumFar() * 0.2f);
            fogState.setStart(cam.getFrustumFar() * 0.15f);
        }
    }

    public GameState getGameState() {
        return gameState;
    }

    public void setMiniMapVisible(boolean b) {
        mapPass.setEnabled(b);
    }

    public MiniMap getMiniMap() {
        return miniMap;
    }

    /**
     * The ui released the dragged object outside of a window.
     * @param dragging the dragged object
     * @return true if the drop succeeded, false otherwise
     */
    public boolean drop(Dragable dragging) {
        if(dropSelection.testUnderMouse()) {
            Spatial model = dragging.getModel();
            dragables.put(model.getName(), dragging);
            Vector3f pos = dropSelection.getLocation();
            if(pos != null) {
                model.getLocalTranslation().set(pos);
                model.getLocalTranslation().y = 9;
                model.updateModelBound();
                model.updateWorldBound();
                model.updateGeometricState(0, true);
                rootNode.attachChild(model);
                firsts.add(dragging);
                return true;
            }
        }
        return false;
    }

    /**
     * A drag is started on the map.
     * @return true if the drag is started, false is there is nothing to drag
     */
    public boolean drag() {
        if(dragSelection.testUnderMouse()) {
            for(Spatial spatial : dragSelection.getSpatials()) {
                while(spatial.getParent() != null) {

                    // check for items
                    Dragable dragable = dragables.remove(spatial.getName());
                    if(dragable != null) {
                        rootNode.detachChild(spatial);
                        setDragging(dragable, new DragSource() {
                            @Override
                            public void returnDragable(Dragable dragable) {
                                // it still has the world position
                                rootNode.attachChild(dragable.getModel());
                            }
                        });
                        return true;
                    }
                    spatial = spatial.getParent();
                }
            }
        }

        return false;
    }

    public boolean mouseReleased() {
        Spatial spatial = findInteractiveSpatialClicked();
        if(spatial != null) {
            Model model = (Model)spatial.getUserData(Tile.MODEL);
            BlockData blockData = (BlockData)spatial.getUserData(Tile.BLOCK_DATA);
            if(model == Model.sign) {
                Window.showMessage(blockData.getData().get(MapSymbol.sign.getBlockDataKeys()[0]),
                                   blockData.getData().get(MapSymbol.sign.getBlockDataKeys()[1]));
                return true;
            }
        }
        return false;
    }

    private Spatial findInteractiveSpatialClicked() {
        if(dragSelection.testUnderMouse()) {
            for(Spatial spatial : dragSelection.getSpatials()) {
                while(spatial.getParent() != null) {
                    if(spatial.getUserData(Tile.MODEL) != null && (BlockData)spatial.getUserData(Tile.BLOCK_DATA) != null) {
                        return spatial;
                    }
                    spatial = spatial.getParent();
                }
            }
        }
        return null;
    }

    public void returnDragable() {
        dragSource.returnDragable(dragging);
        setDragging(null, null);
    }

    public void setDragging(Dragable dragging, DragSource dragSource) {
        this.dragging = dragging;
        this.dragSource = dragSource;
        if(draggingIcon != null) {
            rootNode.detachChild(draggingIcon);
            draggingIcon = null;
        }
        if(dragging != null) {
            draggingIcon = WinUtil.createQuad("dragging", dragging.getIconWidth(), dragging.getIconHeight(), dragging.getIconTexture());
            WinUtil.makeNodeOrtho(draggingIcon);
            draggingIcon.setZOrder(-5000);
            dragOffsetX = dragging.getIconWidth() / 2;
            dragOffsetY = dragging.getIconHeight() / 2;
            rootNode.attachChild(draggingIcon);
        }
        rootNode.updateRenderState();
        rootNode.updateGeometricState(0, true);
    }

    public Dragable getDragging() {
        return dragging;
    }

    public Quad getDraggingIcon() {
        return draggingIcon;
    }

    public void checkRoof() {
        Tile tile = player.getTile();
        if(tile != null) {
            boolean inDungeon = tile.getClimate().isDungeon();
            if(inDungeon != this.inDungeon) {
                this.inDungeon = inDungeon;
                updateRoof();
            }
            boolean inUpDown = (tile.getC() == MapSymbol.up.getC() || tile.getC() == MapSymbol.down.getC());
            if(inUpDown != this.inUpDown) {
                System.err.println("teleporting...");
                this.inUpDown = inUpDown;

                // teleport to the location
                BlockData blockData = tile.getBlockData();
                System.err.println("\tblockData=" + blockData);
                if(blockData != null) {
                    String location = blockData.getData().get(tile.getC() == MapSymbol.up.getC() ? MapSymbol.up.getBlockDataKeys()[0] : MapSymbol.down.getBlockDataKeys()[0]);
                    System.err.println("\tlocation=" + location);
                    try {
                        String[] s = location.trim().split(",");
                        getPlayer().getCreatureModel().moveTo(new Vector3f(Float.parseFloat(s[0]) + Region.EDGE_BUFFER, 1f, Float.parseFloat(s[1]) + Region.EDGE_BUFFER));
                        getTerrain().teleport();
                    } catch(RuntimeException exc) {
                        exc.printStackTrace();
                    }
                }
                System.err.println("\tdone.");
            }
        }
    }

    public void updateRoof() {
        updateRoof = true;
    }
}
