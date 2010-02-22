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

import java.io.*;
import java.net.URISyntaxException;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * User: gabor
 * Date: Feb 7, 2010
 * Time: 5:45:08 PM
 */
public class ShapeUtil {
    private static final Md2ToJme CONVERTER_MD2 = new Md2ToJme();
    private static final FormatConverter CONVERTER_3DS = new MaxToJme();
    private static int shapeCount = 0;
    public static final float WALL_WIDTH = 16.0f;

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
	public static Spatial load3ds(String modelPath, String textureDir, String name_prefix) {
		Spatial output = null; // the geometry will go here.
		final ByteArrayOutputStream outStream =
			new ByteArrayOutputStream(); // byte array streams don't have to be closed
		try {
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
			CONVERTER_3DS.convert(rawIn, outStream);
			rawIn.close(); // FileInputStream s must be explicitly closed.

			// prepare outStream for loading.
			final ByteArrayInputStream convertedIn =
				new ByteArrayInputStream(outStream.toByteArray());

			// import the converted stream to jME as a Spatial
			output = (Spatial) BinaryImporter.getInstance().load(convertedIn);
            output.setName(newShapeName(name_prefix));
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			System.err.println("File not found at: " + modelPath);
		} catch (IOException e) {
			e.printStackTrace();
			System.err.println("Unable read model at: " + modelPath);
		} catch (URISyntaxException e) {
			e.printStackTrace();
			System.err.println("Invalid texture location at:" + textureDir);
		}	/*
		* The bounding box is an important optimization.
		* There is no point in rendering geometry outside of the camera's
		* field of view. However, testing whether each individual triangle
		* is visible is nearly as expensive as actually rendering it. So you
		* don't test every triangle. Instead, you just test the bounding box.
		* If the box isn't in view, don't bother looking for triangles inside.
			*/
        if(output != null) {
		    output.setModelBound(new BoundingBox());
		    output.updateModelBound();
        }
		return output;
	}

}
