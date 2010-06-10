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
public enum TileTexType {
    NONE(null),
    ROCK("./data/textures/surf1.png"),

    DUNGEON("./data/textures/sup082.png"),
    DUNGEON2("./data/textures/sup074.png"),
    DUNGEON3("./data/textures/sup063.png"),

    DUNGEON4("./data/textures/sue094.png"),
    DUNGEON5("./data/textures/sup016.png"),
    DUNGEON6("./data/textures/sup044.png"),

    ROAD("./data/textures/path.png"),
    COBBLES("./data/textures/cobbles.png"),

    ROOM("./data/textures/floor-1.png"),
    ROOM2("./data/textures/floor-2.png"),
    ROOM3("./data/textures/floor-8.png"),
    ROOM4("./data/textures/floor.png"),
    ROOM5("./data/textures/passage.png"),

    GRASS("./data/textures/grass.png"),
    MOSS("./data/textures/moss2.png"),
    LYCHEN("./data/textures/grass2.png"),
    DRY("./data/textures/dry.png"),
    SAND("./data/textures/sand.png"),
    TROPICAL("./data/textures/tropical.png"),
    TROPICAL2("./data/textures/tropical2.png"),
    TROPICAL3("./data/textures/tropical3.png"),
    MUD("./data/textures/mud.png"),
    BOREAL("./data/textures/boreal.png"),
    BOREAL2("./data/textures/boreal2.png"),
    ALPINE("./data/textures/alpine.png"),
    ALPINE2("./data/textures/alpine2.png"),
    STEPPES("./data/textures/steppes.png"),
    ;

    private String texturePath;

    TileTexType(String texture_path) {
        this.texturePath = texture_path;
    }

    public String getTexturePath() {
        return texturePath;
    }
}
    