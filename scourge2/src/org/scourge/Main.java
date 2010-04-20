package org.scourge;

import com.jme.image.Texture;
import com.jme.input.InputHandler;
import com.jme.input.MouseInput;
import com.jme.light.DirectionalLight;
import com.jme.math.FastMath;
import com.jme.math.Plane;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.renderer.Camera;
import com.jme.renderer.ColorRGBA;
import com.jme.renderer.Renderer;
import com.jme.renderer.pass.RenderPass;
import com.jme.scene.*;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.CullState;
import com.jme.scene.state.FogState;
import com.jme.scene.state.ZBufferState;
import com.jme.system.DisplaySystem;
import com.jme.system.JmeException;
import com.jme.util.TextureManager;
import com.jmex.effects.water.WaterRenderPass;
import com.jmex.font2d.Font2D;
import com.jmex.font2d.Text2D;
import org.scourge.input.PlayerController;
import org.scourge.model.Creature;
import org.scourge.terrain.Md2Model;
import org.scourge.terrain.Region;
import org.scourge.terrain.Terrain;
import org.scourge.ui.Window;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Main extends Game {
    private InputHandler playerController;
    private Node cameraHolder;
    private CameraNode camNode;
    private Skybox skybox;
    private Terrain terrain;

    private FogState fogState;
    private WaterRenderPass waterEffectRenderPass;
    private Quad waterQuad;
    private float farPlane = 10000.0f;
    private float textureScale = 0.02f;
    private Random random = new Random(17L);
    private Text2D positionLabel, loadingLabel;
    private GameState gameState;
    private Creature player;

    private static Main main;

    public static void main(String[] args) {
        main = new Main();
        main.setConfigShowMode(ConfigShowMode.ShowIfNoConfig);
        main.start();
    }

    @Override
    protected void initSystem() throws JmeException {
        super.initSystem();
        Logger.getLogger("com.jme").setLevel(Level.WARNING);
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
        terrain.update();

        // the world vectors aren't computed until the first update :-/
        if(terrain.getCurrentRegion() != null) {
            terrain.getCurrentRegion().moveToTopOfTerrain();

            if(player != null) {
                player.getCreatureModel().moveToTopOfTerrain();

                positionLabel.setText("Player: " + player.getCreatureModel().getX() + "," + player.getCreatureModel().getZ() +
                                      " (" + (player.getCreatureModel().getX() % Region.REGION_SIZE) + "," + (player.getCreatureModel().getZ() % Region.REGION_SIZE) + ")" +
                                      " region: " + getTerrain().getCurrentRegion().getX() + "," + getTerrain().getCurrentRegion().getY() +
                                      " (" + getTerrain().getCurrentRegion().getX() / Region.REGION_SIZE + "," +
                                      getTerrain().getCurrentRegion().getY() / Region.REGION_SIZE + ")");
                positionLabel.updateRenderState();
                positionLabel.updateModelBound();
            }
        }


        skybox.getLocalTranslation().set(cam.getLocation());
        skybox.updateGeometricState(0.0f, true);

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
        waterEffectRenderPass.useFadeToFogColor(b);
        if(b) {
            fogState.setColor(new ColorRGBA(0.65f, 0.65f, 0.75f, 1.0f));
            fogState.setEnd(cam.getFrustumFar() * 0.25f);
            fogState.setStart(cam.getFrustumFar() * 0.175f);
        } else {
            fogState.setColor(new ColorRGBA(0.65f, 0.65f, 0.75f, 1.0f));
            fogState.setEnd(cam.getFrustumFar() * 0.2f);
            fogState.setStart(cam.getFrustumFar() * 0.15f);
        }
    }
}
