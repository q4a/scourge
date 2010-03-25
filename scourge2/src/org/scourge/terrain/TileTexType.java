package org.scourge.terrain;

/**
* User: gabor
* Date: Mar 13, 2010
* Time: 8:55:40 AM
*/
enum TileTexType {
    NONE(null),
    ROCK("./data/textures/surf1.png"),
    GRASS("./data/textures/grass.png"),
    MOSS("./data/textures/moss.png"),
    ;

    private String texturePath;

    TileTexType(String texture_path) {
        this.texturePath = texture_path;
    }

    public String getTexturePath() {
        return texturePath;
    }
}
