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
import com.jme.scene.TriMesh;
import com.jme.scene.shape.Quad;
import com.jme.scene.state.TextureState;
import com.jme.util.TextureManager;
import com.jme.util.geom.BufferUtils;
import org.scourge.Main;

import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.util.Random;

/**
 * User: gabor
 * Date: Feb 7, 2010
 * Time: 5:44:11 PM
 */
public class House implements NodeGenerator {
    protected static final float FINAL_TRANS = 3;
    protected static final float FINAL_SCALE = (ShapeUtil.WALL_WIDTH - FINAL_TRANS) / ShapeUtil.WALL_WIDTH;
    protected static final float ROOF_OVERHANG = 0.2f;
    private Node house;
    private Main main;
    private Random random;

    public House(Main main, float x, float y, float z, float w, float h, float levels, Random random) {
        this.main = main;
        this.random = random;
        house = new Node(ShapeUtil.newShapeName("house_"));
        for(int i = 0; i < levels; i++) {
            drawLevel(house, 0, i, 0, w, h, i == 0);
        }
        drawRoof(house, 0, 0 + levels, 0, w, h);
        house.getLocalTranslation().x = x * ShapeUtil.WALL_WIDTH;
        house.getLocalTranslation().y = y * ShapeUtil.WALL_HEIGHT + Region.MIN_HEIGHT;
        house.getLocalTranslation().z = z * ShapeUtil.WALL_WIDTH;
        Quaternion q = new Quaternion();
        q.fromAngleAxis(FastMath.DEG_TO_RAD * (25.0f * random.nextFloat()), Vector3f.UNIT_Y);
        house.getLocalRotation().multLocal(q);
        house.setModelBound(new BoundingBox());
        house.updateModelBound();
        house.updateWorldBound();
    }

    public void moveToTopOfTerrain() {
        house.updateWorldBound();
        house.updateWorldVectors();
        System.err.println("house=" + house.getName() + " local trans=" + house.getLocalTranslation() + " world trans=" + house.getWorldTranslation() + " world bound=" + house.getWorldBound().getCenter());

        main.getTerrain().getCurrentRegion().flatten(house);

        //float height = main.getGround().getHeight(house.getWorldBound().getCenter());
//        house.getLocalTranslation().y = height;
//        house.updateModelBound();
//        house.updateWorldBound();

        //main.getGround().flatten(house.getWorldBound(), height);
    }

    public Node getNode() {
        return house;
    }

    private void drawRoof(Node house, float x, float y, float z, float w, float h) {
        Node roofNode = new Node(ShapeUtil.newShapeName("roof_"));

        Vector2f[] coords = new Vector2f[] {
            new Vector2f(0, 0),
            new Vector2f(h / 2, 0),
            new Vector2f(h / 2, w / 2),
            new Vector2f(0, w / 2)
        };
        TexCoords tc = TexCoords.makeNew(coords);

        Texture texture = TextureManager.loadTexture("./data/textures/roof-red.png",
                                                     Texture.MinificationFilter.Trilinear,
                                                     Texture.MagnificationFilter.Bilinear);
        texture.setWrap(Texture.WrapMode.Repeat);
        TextureState ts = main.getDisplay().getRenderer().createTextureState();
        ts.setTexture(texture);

        Texture texture2 = texture.createSimpleClone();
        texture2.setRotation(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 90.0f, Vector3f.UNIT_Z));
        TextureState ts2 = main.getDisplay().getRenderer().createTextureState();
        ts2.setTexture(texture2);

        boolean orientation = h > w;
        Quad roof = new Quad(ShapeUtil.newShapeName("roof_side_"));
        FloatBuffer fb = BufferUtils.createFloatBuffer(12);
        if(orientation) {
            fb.put(new float[] {
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
            });
        } else {
            fb.put(new float[] {
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z - 1) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z - 1) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
            });
        }
        roof.setVertexBuffer(fb);
        roof.setTextureCoords(tc);
        roof.rotateNormals(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 180, Vector3f.UNIT_X));
        roof.setRenderState(orientation ? ts : ts2);
        roof.setModelBound(new BoundingBox());
        roof.updateModelBound();
        roof.updateWorldBound();
        roofNode.attachChild(roof);

        roof = new Quad(ShapeUtil.newShapeName("roof_side_"));
        fb = BufferUtils.createFloatBuffer(12);
        if(orientation) {
            fb.put(new float[] {
                    (x) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
            });
        } else {
            fb.put(new float[] {
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z - 1) * ShapeUtil.WALL_WIDTH,
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z - 1) * ShapeUtil.WALL_WIDTH,
            });
        }
        roof.setVertexBuffer(fb);
        roof.setTextureCoords(tc);
        //roof.rotateNormals(new Quaternion().fromAngleAxis(FastMath.DEG_TO_RAD * 180, Vector3f.UNIT_X));
        roof.setRenderState(orientation ? ts : ts2);
        roof.setModelBound(new BoundingBox());
        roof.updateModelBound();
        roof.updateWorldBound();
        roofNode.attachChild(roof);

        texture = TextureManager.loadTexture("./data/textures/floor2.png",
                                                     Texture.MinificationFilter.Trilinear,
                                                     Texture.MagnificationFilter.Bilinear);
        texture.setWrap(Texture.WrapMode.Repeat);
        ts = main.getDisplay().getRenderer().createTextureState();
        ts.setTexture(texture);
        if(orientation) {
            fb = BufferUtils.createFloatBuffer(9);
            fb.put(new float[] {
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1) * ShapeUtil.WALL_WIDTH,
                    (x) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1) * ShapeUtil.WALL_WIDTH,
            });
            coords = new Vector2f[] {
                new Vector2f(0, 0),
                new Vector2f(h / 2, w / 2),
                new Vector2f(0, w / 2)
            };
            tc = TexCoords.makeNew(coords);

            IntBuffer ib = BufferUtils.createIntBuffer(3);
            ib.put(new int[] { 0, 1, 2 });
            FloatBuffer nb = fb.duplicate();
            TriMesh tri = new TriMesh(ShapeUtil.newShapeName("roof_tri_"), fb, nb, null, tc, ib);
            tri.setRenderState(ts);
            tri.setModelBound(new BoundingBox());
            roofNode.attachChild(tri);

            fb = BufferUtils.createFloatBuffer(9);
            fb.put(new float[] {
                    (x - w / 2 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1) * ShapeUtil.WALL_WIDTH,
                    (x) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1) * ShapeUtil.WALL_WIDTH,
            });
            ib = BufferUtils.createIntBuffer(3);
            ib.put(new int[] { 2, 1, 0 });
            nb = fb.duplicate();
            tri = new TriMesh(ShapeUtil.newShapeName("roof_tri_"), fb, nb, null, tc, ib);
            tri.setRenderState(ts);
            tri.setModelBound(new BoundingBox());
            roofNode.attachChild(tri);
        } else {
            fb = BufferUtils.createFloatBuffer(9);
            fb.put(new float[] {
                    (x - w / 2) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x - w / 2) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z / 2 - 1) * ShapeUtil.WALL_WIDTH,
                    (x - w / 2) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
            });
            coords = new Vector2f[] {
                new Vector2f(0, 0),
                new Vector2f(w / 2, h / 2),
                new Vector2f(0, h / 2)
            };
            tc = TexCoords.makeNew(coords);
            IntBuffer ib = BufferUtils.createIntBuffer(3);
            ib.put(new int[] { 2, 1, 0 });
            FloatBuffer nb = fb.duplicate();
            TriMesh tri = new TriMesh(ShapeUtil.newShapeName("roof_tri_"), fb, nb, null, tc, ib);
            tri.setRenderState(ts);
            tri.setModelBound(new BoundingBox());
            roofNode.attachChild(tri);

            fb = BufferUtils.createFloatBuffer(9);
            fb.put(new float[] {
                    (x + w / 2) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z - h / 2 - 1 - ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2) * ShapeUtil.WALL_WIDTH, (y + 1) * ShapeUtil.WALL_WIDTH, (z / 2 - 1) * ShapeUtil.WALL_WIDTH,
                    (x + w / 2) * ShapeUtil.WALL_WIDTH, y * ShapeUtil.WALL_WIDTH, (z + h / 2 - 1 + ROOF_OVERHANG) * ShapeUtil.WALL_WIDTH,
            });
            ib = BufferUtils.createIntBuffer(3);
            ib.put(new int[] { 0, 1, 2 });
            nb = fb.duplicate();
            tri = new TriMesh(ShapeUtil.newShapeName("roof_tri_"), fb, nb, null, tc, ib);
            tri.setRenderState(ts);
            tri.setModelBound(new BoundingBox());
            roofNode.attachChild(tri);
        }

        house.attachChild(roofNode);
    }

    private void drawLevel(Node house, float x, float y, float z, float w, float h, boolean has_door) {

        Direction door = Direction.values()[(int)((float)Direction.values().length * random.nextFloat())];
        drawWall(house, x - (w / 2),     y, z - (h / 2 + 1), Direction.EAST, w, w - 1, door == Direction.EAST && has_door);
        drawWall(house, x - (w / 2 - 1), y, z + (h / 2 - 1), Direction.WEST, w, 0, door == Direction.WEST && has_door);
        drawWall(house, x - (w / 2),     y, z - (h / 2), Direction.SOUTH, h, 0, door == Direction.SOUTH && has_door);
        drawWall(house, x + (w / 2),     y, z - (h / 2 + 1), Direction.NORTH, h, h - 1, door == Direction.NORTH && has_door);
        drawFloor(house, x, y, z, w, h);
    }


    private void drawFloor(Node house, float x, float y, float z, float w, float h) {
        Quaternion q = new Quaternion();
        q.fromAngleAxis(FastMath.DEG_TO_RAD * 90, Vector3f.UNIT_X);

        Quad floor = new Quad(ShapeUtil.newShapeName("floor_"), w * ShapeUtil.WALL_WIDTH, h * ShapeUtil.WALL_WIDTH);
        floor.getLocalTranslation().x = x * ShapeUtil.WALL_WIDTH;
        floor.getLocalTranslation().y = y * ShapeUtil.WALL_WIDTH;
        floor.getLocalTranslation().z = (z - 1) * ShapeUtil.WALL_WIDTH;
        floor.getLocalRotation().multLocal(q);
        Vector2f[] coords = new Vector2f[] {
            new Vector2f(0, 0),
            new Vector2f(h / 2, 0),
            new Vector2f(h / 2, w / 2),
            new Vector2f(0, w / 2)
        };
        TexCoords tc = TexCoords.makeNew(coords);
        floor.setTextureCoords(tc);
        Texture texture = TextureManager.loadTexture("./data/textures/floor.png",
                                                     Texture.MinificationFilter.Trilinear,
                                                     Texture.MagnificationFilter.Bilinear);
        texture.setWrap(Texture.WrapMode.Repeat);
        TextureState ts = main.getDisplay().getRenderer().createTextureState();
        ts.setTexture(texture);
        floor.setRenderState(ts);
        floor.setModelBound(new BoundingBox());
        floor.updateModelBound();
        floor.updateWorldBound();
        house.attachChild(floor);

    }


    private void drawWall(Node house, float x, float y, float z, Direction dir, float length, float finalPos, boolean has_door) {
        int door_pos = has_door ? 1 + (int)((float)(length - 2) * random.nextFloat()) : -1;
        for(int i = 0; i < length; i++) {
            if(i == door_pos) {
                addWallPiece(getDoorFrame(), i, house, x, y, z, dir, length, finalPos);
                addWallPiece(getDoor(), i, house, x, y, z, dir, length, finalPos);
            } else {
                addWallPiece(0 == (int)(8.0f * random.nextFloat()) ? getWindow() : getWall(), i, house, x, y, z, dir, length, finalPos);
            }
        }
    }


    protected void addWallPiece(Spatial spatial, int i, Node house, float x, float y, float z, Direction dir, float length, float finalPos) {
        Quaternion q = new Quaternion();
        q.fromAngleAxis(FastMath.DEG_TO_RAD * dir.getAngle(), Vector3f.UNIT_Y);
        spatial.getLocalRotation().multLocal(q);
            switch(dir) {
                case EAST:
                case WEST:
                    spatial.getLocalTranslation().x = (x + i) * ShapeUtil.WALL_WIDTH;
                    spatial.getLocalTranslation().y = y * ShapeUtil.WALL_WIDTH;
                    spatial.getLocalTranslation().z = z * ShapeUtil.WALL_WIDTH;
                    break;
                case NORTH:
                case SOUTH:
                    spatial.getLocalTranslation().x = x * ShapeUtil.WALL_WIDTH;
                    spatial.getLocalTranslation().y = y * ShapeUtil.WALL_WIDTH;
                    spatial.getLocalTranslation().z = (z + i) * ShapeUtil.WALL_WIDTH;
                    break;
            }

            if(i == finalPos) {
                spatial.getLocalScale().x = FINAL_SCALE;
            }
            spatial.updateModelBound();
            spatial.updateWorldBound();
            house.attachChild(spatial);
    }


    protected Spatial getWall() {
        return ShapeUtil.importModel("./data/3ds/wall.3ds", "./data/textures", "wall");
    }

    protected Spatial getWindow() {
        return ShapeUtil.importModel("./data/3ds/win.3ds", "./data/textures", "wall");
    }

    protected Spatial getDoorFrame() {
        return ShapeUtil.importModel("./data/3ds/dframe.3ds", "./data/textures", "wall");
    }

    protected Spatial getDoor() {
        return ShapeUtil.importModel("./data/3ds/door.3ds", "./data/textures", "wall");
    }
}
