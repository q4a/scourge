package org.scourge.terrain;

/**
* User: gabor
* Date: Mar 13, 2010
* Time: 8:55:40 AM
 *
 * Order means rendering order. Lower textures blend into higher ones. This has means for example
 * if ROAD and GRASS meet, you will see more ROAD than GRASS. ROCK should always be first, since
 * it's the texture of cliffs and cliffs cannot be (currently) texture blended.
*/
enum TileTexType {
    NONE(null),
    ROCK("./data/textures/surf1.png"),
    ROAD("./data/textures/path.png"),
    GRASS("./data/textures/grass.png"),
    MOSS("./data/textures/moss2.png"),
    LYCHEN("./data/textures/grass2.png"),
    ;

    private String texturePath;

    TileTexType(String texture_path) {
        this.texturePath = texture_path;
    }

    public String getTexturePath() {
        return texturePath;
    }
}
    