package org.scourge.terrain;

import com.jme.input.MouseInput;
import com.jme.intersection.BoundingPickResults;
import com.jme.intersection.PickResults;
import com.jme.math.FastMath;
import com.jme.math.Ray;
import com.jme.math.Vector2f;
import com.jme.math.Vector3f;
import com.jme.scene.Node;
import com.jme.scene.Spatial;
import com.jme.scene.TriMesh;
import com.jme.system.DisplaySystem;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * User: gabor
 * Date: May 16, 2010
 * Time: 1:55:23 PM
 */
public class Selection {
    private final Ray pickRay = new Ray();
    private PickResults pickResults = new BoundingPickResults();
    private Vector2f screenPos = new Vector2f();
    private Vector3f worldCoords = new Vector3f();
    private Vector3f worldCoords2 = new Vector3f();
    private Vector3f[] triangle = new Vector3f[3];
    private Node node;
    private List<Spatial> spatials = new ArrayList<Spatial>();
    private Map<Spatial, Vector3f> points = new HashMap<Spatial, Vector3f>();
    private Vector3f location = new Vector3f(0,0,0);

    public Selection(Node node) {
        this.node = node;
    }

    public boolean testUnderMouse() {
        spatials.clear();
        points.clear();

        //System.err.println("!!! mouse=" + MouseInput.get().getXAbsolute() + "," + MouseInput.get().getYAbsolute());
        // Get the position that the mouse is pointing to
        screenPos.set(MouseInput.get().getXAbsolute(), MouseInput.get().getYAbsolute());
        // Get the world location of that X,Y value
        DisplaySystem.getDisplaySystem().getWorldCoordinates(screenPos, 0, worldCoords);
        DisplaySystem.getDisplaySystem().getWorldCoordinates(screenPos, 1, worldCoords2);

        // Create a ray starting from the camera, and going in the direction
        // of the mouse's location
        pickRay.getOrigin().set(worldCoords);
        pickRay.getDirection().set(worldCoords2.subtractLocal(worldCoords).normalizeLocal());

        pickResults.setCheckDistance(false);
        pickResults.clear();

        node.findPick(pickRay, pickResults);
        boolean triangleWasPicked = false;
        System.err.println("----------------");
        for(int i = 0; i < pickResults.getNumber(); i++) {
            TriMesh mesh = (TriMesh)pickResults.getPickData(i).getTargetMesh();
            System.err.println("\ttarget=" + mesh.getName());
            spatials.add(mesh);
            for( int j = 0; j < mesh.getTriangleCount(); j++ ){
                mesh.getTriangle( j, triangle );
                triangleWasPicked =
                        ( pickRay.intersect( triangle[0].addLocal( mesh.getWorldTranslation() ),
                                             triangle[1].addLocal( mesh.getWorldTranslation() ),
                                             triangle[2].addLocal( mesh.getWorldTranslation() ) ) );
                if( triangleWasPicked ){
//                    System.err.println("location:");
//                    System.err.println("\t" + triangle[0] + "\n\t" + triangle[1] + "\n\t" + triangle[2]);

                    location.set(triangle[0]);
                    location.addLocal(triangle[1]).addLocal(triangle[2]).multLocal(FastMath.ONE_THIRD);

                    points.put(mesh, location.clone());
                }
            }
        }
        return !spatials.isEmpty();
    }

    public List<Spatial> getSpatials() {
        return spatials;
    }

    public Map<Spatial, Vector3f> getPoints() {
        return points;
    }

    public Vector3f getLocation() {
        return points.size() > 0 && spatials.size() > 0 ? points.get(spatials.get(0)) : null;
    }
}
