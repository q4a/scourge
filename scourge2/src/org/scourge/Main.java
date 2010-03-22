package org.scourge;

import com.jme.bounding.BoundingBox;
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
import com.jme.scene.CameraNode;
import com.jme.scene.Node;
import com.jme.scene.Skybox;
import com.jme.scene.Spatial;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.CullState;
import com.jme.scene.state.FogState;
import com.jme.scene.state.ZBufferState;
import com.jme.system.DisplaySystem;
import com.jme.system.JmeException;
import com.jme.util.TextureManager;
import com.jmex.effects.water.WaterRenderPass;
import org.scourge.input.PlayerController;
import org.scourge.terrain.*;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.Random;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;

public class Main extends Game {
    private Player player;
    private Town town;
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

    public static void main(String[] args) {
        Main app = new Main();
        app.setConfigShowMode(ConfigShowMode.AlwaysShow);
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
        fogState.setColor(new ColorRGBA(0.5f, 0.5f, 0.75f, 0.5f));
        fogState.setEnd(cam.getFrustumFar());
        fogState.setStart(cam.getFrustumFar() * 0.25f);
        fogState.setDensityFunction(FogState.DensityFunction.Linear);
        fogState.setQuality(FogState.Quality.PerVertex);
        //fogState.setSource(FogState.CoordinateSource.Depth);
        rootNode.setRenderState(fogState);

        lightState.detachAll();
        DirectionalLight dr = new DirectionalLight();
        dr.setDiffuse(new ColorRGBA(1.0f, 1.0f, 1.0f, 1.0f));
        dr.setAmbient(new ColorRGBA(0.5f, 0.5f, 0.5f, 1.0f));
        dr.setSpecular(new ColorRGBA(1.0f, 0.0f, 0.0f, 1.0f));
        dr.setDirection(new Vector3f(1, -1, 1));
        dr.setEnabled(true);
        lightState.attach(dr);
        dr = new DirectionalLight();
        dr.setDiffuse(new ColorRGBA(0.75f, 0.7f, 0.55f, 1.0f));
        dr.setAmbient(new ColorRGBA(0.5f, 0.5f, 0.5f, 1.0f));
        dr.setSpecular(new ColorRGBA(1.0f, 0.0f, 0.0f, 1.0f));
        dr.setDirection(new Vector3f(-1, -1, 1));
        dr.setEnabled(true);
        lightState.attach(dr);

        player = new Player(this, 8, 9, 8);
        player.setKeyFrame(Player.Md2Key.stand);

        try {
            buildTerrain();
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
    }

    private void buildTerrain() throws IOException {
        // middle
        terrain = new Terrain(this);
        terrain.addTown(14, 14);
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


    private boolean first = true;



    @Override
    protected void simpleUpdate() {
        // the world vectors aren't computed until the first update :-/
        if(first) {
            first = false;
            terrain.moveToTopOfTerrain();
        }

//        playerController.update(tpf);

        player.moveToTopOfTerrain();

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
}
