package org.scourge;

import com.jme.math.Vector3f;
import org.scourge.terrain.*;

import java.util.List;
import java.util.Random;

/**
 * User: gabor
 * Date: Mar 31, 2010
 * Time: 9:47:53 AM
 */
public enum Climate {
    none(new TileTexType[] { TileTexType.GRASS }, new Model[] { Model.oak }),
    alpine(new TileTexType[] { TileTexType.ALPINE, TileTexType.ALPINE2, TileTexType.STEPPES }, Model.getAlpineTrees()),
    boreal(new TileTexType[] { TileTexType.BOREAL, TileTexType.BOREAL2, TileTexType.TROPICAL2 }, Model.getBorealTrees()),
    temperate(new TileTexType[] { TileTexType.GRASS, TileTexType.LYCHEN, TileTexType.MOSS }, Model.getTemperateTrees()),
    subtropical(new TileTexType[] { TileTexType.DRY, TileTexType.SAND, TileTexType.MUD }, Model.getSubtropicalTrees()),
    tropical(new TileTexType[] { TileTexType.TROPICAL, TileTexType.TROPICAL2, TileTexType.TROPICAL3 }, Model.getTropicalTrees()),
    dungeon(new TileTexType[] { TileTexType.DUNGEON, TileTexType.DUNGEON2, TileTexType.DUNGEON3 }, new Model[0]) {
        @Override
        public TileTexType getBaseTileTex() {
            return TileTexType.DUNGEON;
        }

        @Override
        public boolean isDungeon() {
            return true;
        }

        @Override
        public String getWallTexture() {
            return "data/textures/mountain.png";
        }

        @Override
        public void decorateWall(List<Direction> dirList, Region region, int x, int y) {
            for(Direction dir : dirList) {
                if((int)(Math.random() * 5) == 0) {
                    addTorch(Model.torch, dir, region, x, y);
                } else {
                    if(dir == Direction.NORTH) {
                        region.getTile(x, y).addModel(Model.dungeonColumn,
                                             new Vector3f(ShapeUtil.WALL_WIDTH / 2 - 2, 0, 0),
                                             1, 0, Vector3f.UNIT_Y);
                    }
                    if(dir == Direction.SOUTH) {
                        region.getTile(x, y).addModel(Model.dungeonColumn,
                                             new Vector3f(ShapeUtil.WALL_WIDTH / 2 - 2, 0, ShapeUtil.WALL_WIDTH + 1),
                                             1, 0, Vector3f.UNIT_Y);
                    }
                    if(dir == Direction.WEST) {
                        region.getTile(x, y).addModel(Model.dungeonColumn,
                                             new Vector3f(-1, 0, ShapeUtil.WALL_WIDTH / 2 - 2),
                                             1, 90, Vector3f.UNIT_Y);
                    }
                    if(dir == Direction.EAST) {
                        region.getTile(x, y).addModel(Model.dungeonColumn,
                                             new Vector3f(ShapeUtil.WALL_WIDTH + 1, 0, ShapeUtil.WALL_WIDTH / 2 - 2),
                                             1, 90, Vector3f.UNIT_Y);
                    }
                }
            }
        }
    },
    dungeon2(new TileTexType[] { TileTexType.DUNGEON4, TileTexType.DUNGEON5, TileTexType.DUNGEON6 }, new Model[0]) {
        @Override
        public TileTexType getBaseTileTex() {
            return TileTexType.DUNGEON4;
        }

        @Override
        public boolean isDungeon() {
            return true;
        }

        @Override
        public String getWallTexture() {
            return "data/textures/bluecave.png";
        }

        @Override
        public void decorateWall(List<Direction> dirList, Region region, int x, int y) {
            for(Direction dir : dirList) {
                if((int)(Math.random() * 5) == 0) {
                    addTorch(Model.torchGreen, dir, region, x, y);
                } else {
                    if(dir == Direction.NORTH) {
                        region.getTile(x, y).addModel(Model.dungeonColumn2,
                                             new Vector3f(ShapeUtil.WALL_WIDTH / 2 - 2, 0, -1),
                                             1, 0, Vector3f.UNIT_Y);
                    }
                    if(dir == Direction.SOUTH) {
                        region.getTile(x, y).addModel(Model.dungeonColumn2,
                                             new Vector3f(ShapeUtil.WALL_WIDTH / 2 - 2, 0, ShapeUtil.WALL_WIDTH + 1),
                                             1, 0, Vector3f.UNIT_Y);
                    }
                    if(dir == Direction.WEST) {
                        region.getTile(x, y).addModel(Model.dungeonColumn2,
                                             new Vector3f(-1, 0, ShapeUtil.WALL_WIDTH / 2 - 2),
                                             1, 0, Vector3f.UNIT_Y);
                    }
                    if(dir == Direction.EAST) {
                        region.getTile(x, y).addModel(Model.dungeonColumn2,
                                             new Vector3f(ShapeUtil.WALL_WIDTH + 1, 0, ShapeUtil.WALL_WIDTH / 2 - 2),
                                             1, 0, Vector3f.UNIT_Y);
                    }
                }
            }
        }
    },

    ;

    private static void addTorch(Model torch, Direction dir, Region region, int x, int y) {
        if(dir == Direction.NORTH) {
            region.getTile(x, y).addModel(torch,
                                 new Vector3f(ShapeUtil.WALL_WIDTH / 2 - 2, 10, 0),
                                 1, 180, Vector3f.UNIT_Y);
        }
        if(dir == Direction.SOUTH) {
            region.getTile(x, y).addModel(torch,
                                 new Vector3f(ShapeUtil.WALL_WIDTH / 2 - 2, 10, ShapeUtil.WALL_WIDTH + 1),
                                 1, 0, Vector3f.UNIT_Y);
        }
        if(dir == Direction.WEST) {
            region.getTile(x, y).addModel(torch,
                                 new Vector3f(-1, 10, ShapeUtil.WALL_WIDTH / 2 - 2),
                                 1, -90, Vector3f.UNIT_Y);
        }
        if(dir == Direction.EAST) {
            region.getTile(x, y).addModel(torch,
                                 new Vector3f(ShapeUtil.WALL_WIDTH + 1, 10, ShapeUtil.WALL_WIDTH / 2 - 2),
                                 1, 90, Vector3f.UNIT_Y);
        }
    }

    private TileTexType[] ground;
    private Model[] trees;

    Climate(TileTexType[] ground, Model[] trees) {
        this.ground = ground;
        this.trees = trees;
    }

    public TileTexType getDefaultGround() {
        return ground[0];
    }

    public TileTexType getRandomGround(Random random) {
        int type = (int)(random.nextFloat() * 4);
        if(type == 0 && ground.length > 1) {
            type = (int)(random.nextFloat() * (ground.length - 1)) + 1;
            return ground[type];
        } else {
            return getDefaultGround();
        }
    }

    public Model getRandomTree(Random random) {
        return trees == null || trees.length == 0 ? null : trees[(int)(random.nextFloat() * trees.length)];
    }

    public TileTexType getBaseTileTex() {
        return TileTexType.ROCK;
    }

    public boolean isDungeon() {
        return false;
    }

    public String getWallTexture() {
        throw new RuntimeException("Implement me! Climate=" + name());
    }

    public void decorateWall(List<Direction> dir, Region region, int x, int y) {
    }
}
