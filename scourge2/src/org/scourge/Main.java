package org.scourge;

import com.jme.image.Texture;
import com.jme.input.InputHandler;
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
import org.scourge.terrain.Player;
import org.scourge.terrain.Region;
import org.scourge.terrain.Terrain;
import org.scourge.ui.Window;
import org.scourge.ui.WindowListener;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.Random;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Main extends Game implements WindowListener {
    private Player player;
    private InputHandler playerController;
    private Node cameraHolder;
    private CameraNode camNode;
    private Skybox skybox;
    private Terrain terrain;

    private WaterRenderPass waterEffectRenderPass;
    private Quad waterQuad;
    private float farPlane = 10000.0f;
    private float textureScale = 0.02f;
    private Random random = new Random(17L);
    private Text2D positionLabel, loadingLabel;

    Window mainMenuWindow;

    public static void main(String[] args) {
        Main app = new Main();
        app.setConfigShowMode(ConfigShowMode.ShowIfNoConfig);
        app.start();
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
        display.setTitle("Scourge II");
        cam.setLocation(new Vector3f(0,0,0));
        cam.update();

        CullState cs = display.getRenderer().createCullState();
		cs.setCullFace(CullState.Face.Back);
		cs.setEnabled(true);
		rootNode.setRenderState(cs);

        FogState fogState = display.getRenderer().createFogState();
        fogState.setDensity(1.0f);
        fogState.setEnabled(true);
        fogState.setColor(new ColorRGBA(0.65f, 0.65f, 0.75f, 1.0f));
        fogState.setEnd(cam.getFrustumFar() * 0.2f);
        fogState.setStart(cam.getFrustumFar() * 0.15f);
        fogState.setDensityFunction(FogState.DensityFunction.Linear);
        fogState.setQuality(FogState.Quality.PerVertex);
        //fogState.setSource(FogState.CoordinateSource.Depth);
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

        try {
            player = new Player(this, 498, 9, 489);
            player.setKeyFrame(Player.Md2Key.stand);
            terrain = new Terrain(this);
        } catch(IOException exc) {
            throw new RuntimeException(exc);
        }

        cameraHolder = new Node("cam_holder");
        player.getNode().attachChild(cameraHolder);

        toggleCameraAttached();

        playerController = new PlayerController(this);
        input = playerController;

        Node reflectedNode = new Node("reflectNode");

        buildSkyBox();
        reflectedNode.attachChild(skybox);
        reflectedNode.attachChild(terrain.getNode());
        rootNode.attachChild(reflectedNode);

        rootNode.attachChild(player.getNode());



        // setup the water plane
        waterEffectRenderPass = new WaterRenderPass(cam, 4, false, false); // setting last param to false renders faster
        waterEffectRenderPass.setWaterPlane(new Plane(new Vector3f(0.0f, 1.0f, 0.0f), 0.0f));

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

        if(!"true".equalsIgnoreCase(System.getProperty("test.mode"))) {
            showMainMenu();
        }
    }

    public void showMainMenu() {
        mainMenuWindow = new Window(display.getRenderer().getWidth() / 2,
                                    display.getRenderer().getHeight() / 2,
                                    300, 300, this);
        mainMenuWindow.addLabel(0, 90, "Scourge II");
        mainMenuWindow.addButton("new", 0, 30, "New Game");
        mainMenuWindow.addButton("load", 0, -10, "Continue Game");
        mainMenuWindow.addButton("quit", 0, -50, "Quit");
        showWindow(mainMenuWindow);
    }

    @Override
    public void buttonClicked(String name) {
        System.err.println("Button clicked: " + name);
        hideWindow(mainMenuWindow);
    }

    public void showWindow(Window win) {
        win.pack();
        rootNode.attachChild(win.getNode());
        ((PlayerController)input).setUIEnabled(true);
    }

    public void hideWindow(Window win) {
        rootNode.detachChild(win.getNode());
        ((PlayerController)input).setUIEnabled(false);
    }

    public void toggleCameraAttached() {
        if(camNode != null) {
            input = firstPersonHandler;
            cameraHolder.detachChild(camNode);
            camNode.setCamera(null);
            camNode = null;
        } else {
            camNode = new CameraNode("camera node", cam);
            camNode.setLocalTranslation(new Vector3f(-350, 350, 0));
            camNode.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 90.0f, Vector3f.UNIT_Y));
            camNode.getLocalRotation().multLocal(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 45.0f, Vector3f.UNIT_X));
            camNode.updateWorldData(0);
            cameraHolder.attachChild(camNode);
            input = playerController;
        }
    }

    @Override
    protected void simpleUpdate() {
        terrain.update();

        // the world vectors aren't computed until the first update :-/
        terrain.getCurrentRegion().moveToTopOfTerrain();

//        playerController.update(tpf);

        player.moveToTopOfTerrain();

        positionLabel.setText("Player: " + player.getX() + "," + player.getZ() +
                              " (" + (player.getX() % Region.REGION_SIZE) + "," + (player.getZ() % Region.REGION_SIZE) + ")" +
                              " region: " + getTerrain().getCurrentRegion().getX() + "," + getTerrain().getCurrentRegion().getY() +
                              " (" + getTerrain().getCurrentRegion().getX() / Region.REGION_SIZE + "," +
                              getTerrain().getCurrentRegion().getY() / Region.REGION_SIZE + ")");
        positionLabel.updateRenderState();
	    positionLabel.updateModelBound();


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

    public Player getPlayer() {
        return player;
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
}
