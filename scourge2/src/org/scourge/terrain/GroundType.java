package org.scourge.terrain;

/**
 * User: gabor
 * Date: Mar 15, 2010
 * Time: 9:33:02 AM
 */
public enum GroundType {
    none,
    moss("./data/textures/moss.png"),
    ;

    private String texturePath;

    GroundType() {
       this(null);
    }

    GroundType(String texturePath) {
        this.texturePath = texturePath;
    }

    public String getTexturePath() {
        return texturePath;
    }

    public static String getGroundKey(int x, int y) {
        return "" + x + "," + y;
    }
}
