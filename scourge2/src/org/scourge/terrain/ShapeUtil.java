package org.scourge.terrain;

import com.jme.bounding.BoundingBox;
import com.jme.image.Texture;
import com.jme.math.FastMath;
import com.jme.math.Plane;
import com.jme.math.Quaternion;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.TriMesh;
import com.jme.scene.state.CullState;
import com.jme.scene.state.TextureState;
import com.jme.system.DisplaySystem;
import com.jme.util.TextureManager;
import com.jme.util.export.binary.BinaryImporter;
import com.jme.util.geom.BufferUtils;
import com.jme.util.resource.ResourceLocatorTool;
import com.jme.util.resource.SimpleResourceLocator;
import com.jmex.model.animation.KeyframeController;
import com.jmex.model.converters.FormatConverter;
import com.jmex.model.converters.MaxToJme;
import com.jmex.model.converters.Md2ToJme;
import com.jmex.model.converters.ObjToJme;

import javax.swing.*;
import java.io.*;
import java.net.URISyntaxException;
import java.nio.FloatBuffer;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * User: gabor
 * Date: Feb 7, 2010
 * Time: 5:45:08 PM
 */
public class ShapeUtil {
    private static final Md2ToJme CONVERTER_MD2 = new Md2ToJme();
    private static final FormatConverter CONVERTER_3DS = new MaxToJme();
    private static final FormatConverter CONVERTER_OBJ = new ObjToJme();
    private static int shapeCount = 0;
    public static final float WALL_WIDTH = 16.0f;
    public static final float WALL_HEIGHT = 24.0f;
    private static Logger logger = Logger.getLogger(ShapeUtil.class.toString());
    private static WeakHashMap<String, Texture> textures = new WeakHashMap<String, Texture>();
    private static WeakHashMap<String, ImageIcon> images = new WeakHashMap<String, ImageIcon>();
    private static WeakHashMap<String, byte[]> models = new WeakHashMap<String, byte[]>();

    public static String newShapeName(String prefix) {
        return prefix + "_" + (shapeCount++);
    }

    public static Node loadMd2(String modelPath, String texturePath, String name_prefix, DisplaySystem display, boolean invertNormals, Map<String, Integer[]> frames) {
        ByteArrayOutputStream bytearrayoutputstream = new ByteArrayOutputStream(); //For loading the raw file
        Node node;
        try {
            CONVERTER_MD2.convert(new FileInputStream(modelPath), bytearrayoutputstream, frames);
            BinaryImporter binaryImporter = new BinaryImporter();
            node = (Node)binaryImporter.load(new ByteArrayInputStream(bytearrayoutputstream.toByteArray()));
            node.setName(newShapeName(name_prefix));

            for(int i = 0; i < node.getChild(0).getControllerCount(); i++) {
                KeyframeController kc = (KeyframeController)node.getChild(0).getController(i);
                if(invertNormals) {
                    for(KeyframeController.PointInTime pit : kc.keyframes) {
                        pit.newShape.rotateNormals(new Quaternion().fromAngleAxis(180.0f * FastMath.DEG_TO_RAD, Vector3f.UNIT_Z));
                    }
                }
            }
        } catch(IOException exc) {
            throw new RuntimeException(exc);
        }

        TextureState ts = display.getRenderer().createTextureState();
        ts.setEnabled(true);
        ts.setTexture(TextureManager.loadTexture(texturePath, Texture.MinificationFilter.Trilinear, Texture.MagnificationFilter.Bilinear, 0.0f, false));
        node.setRenderState(ts);

        if(invertNormals) {
            CullState cs = display.getRenderer().createCullState();
            cs.setCullFace(CullState.Face.Front);
            cs.setEnabled(true);
            node.setRenderState(cs);
        }

        node.setModelBound(new BoundingBox());
		node.updateModelBound();
        return node;
    }

    /**
	 * Imports a .3ds model from file system.
	 * @param modelPath the path to the model file.
	 * Can be relative to the project directory.
	 * @param textureDir the path to the directory with the model's
	 *  textures. If null, this will attempt to infer textureDir from
	 *  modelPath, which assumes that the texture file(s) are in the same
	 *  directory as the model file.
	 * @param name_prefix the unique name of the resulting spatial will start with this
     * @return a Spatial containing the model geometry
	 * (with provided texture, if any) that can be attached to
	 *  the scenegraph, or null instead if unable to load geometry.
	 */
	public static Spatial importModel(String modelPath, String textureDir, String name_prefix) {
        Spatial output = null;
        try {
            byte[] bytes = models.get(modelPath);
            if(bytes == null) {
                final File textures;
                if(textureDir != null) { // set textureDir location
                    textures = new File( textureDir );
                } else {// try to infer textureDir from modelPath.
                    textures = new File(modelPath.substring(0, modelPath.lastIndexOf('/')));
                }	// Add texture URL to auto-locator
                final SimpleResourceLocator location = new SimpleResourceLocator(textures.toURI().toURL());
                ResourceLocatorTool.addResourceLocator(ResourceLocatorTool.TYPE_TEXTURE, location );

                // read .3ds file into memory & convert it to a jME usable format.
                final FileInputStream rawIn = new FileInputStream(modelPath);
                final ByteArrayOutputStream outStream = new ByteArrayOutputStream(); // byte array streams don't have to be closed
                if(modelPath.endsWith(".3ds")) {
                    CONVERTER_3DS.convert(rawIn, outStream);
                } else if(modelPath.endsWith(".obj")) {
                    CONVERTER_OBJ.convert(rawIn, outStream);
                } else {
                    throw new IllegalStateException("Can't convert model: " + modelPath);
                }
                rawIn.close(); // FileInputStream s must be explicitly closed.
                bytes = outStream.toByteArray();

                // prepare outStream for loading.
                models.put(modelPath, bytes);
            }

            // import the converted stream to jME as a Spatial
            output = (Spatial) BinaryImporter.getInstance().load(new ByteArrayInputStream(bytes));
            output.setName(newShapeName(name_prefix));
            output.setModelBound(new BoundingBox());
            output.updateModelBound();
            return output;
        } catch (Exception exc) {
            logger.log(Level.SEVERE, exc.getMessage(), exc);
            throw new RuntimeException(exc);
        }
	}

    public static ImageIcon loadImageIcon(String path) {
        ImageIcon icon = images.get(path);
        if(icon == null) {
            icon = new ImageIcon(path);
            images.put(path, icon);
        }
        return icon;
    }

    public static Texture loadTexture(String path) {
        return loadTexture(path, path);
    }

    public static Texture loadTexture(String path, String textureKey) {
        Texture t0 = textures.get(textureKey);
        if (t0 == null) {
        	t0 = TextureManager.loadTexture(path,
                                            Texture.MinificationFilter.Trilinear,
                                            Texture.MagnificationFilter.Bilinear);
	        textures.put(textureKey, t0);
        }
        return t0;
    }

    public static void debug() {
        logger.info("loaded " + textures.size() + " textures and " + images.size() + " images and " + models.size() + " models.");
    }

    public static boolean isTextureLoaded(String key) {
        return textures.keySet().contains(key);
    }

    public static void storeTexture(String key, Texture texture) {
        textures.put(key, texture);
    }

    public static Texture getTexture(String key) {
        return textures.get(key);
    }
}
