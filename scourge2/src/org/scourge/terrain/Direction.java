package org.scourge.terrain;

import com.jme.math.Vector3f;

/**
* User: gabor
* Date: Feb 7, 2010
* Time: 5:46:11 PM
*/
public enum Direction {
    NORTH(-90, new Vector3f(0, 0, -1)) {
        public Direction opposite() {
            return SOUTH;
        }
    },
    EAST(0, new Vector3f(-1, 0, 0)) {
        public Direction opposite() {
            return WEST;
        }
    },
    SOUTH(90, new Vector3f(0, 0, 1)) {
        public Direction opposite() {
            return NORTH;
        }
    },
    WEST(180, new Vector3f(1, 0, 0)) {
       public Direction opposite() {
            return EAST;
        }
    };

    private float angle;
    private Vector3f dirVector;

    Direction(float angle, Vector3f dirVector) {
        this.angle = angle;
        this.dirVector = dirVector;
    }

    public float getAngle() {
        return angle;
    }

    public Vector3f getDirVector() {
        return dirVector;
    }

    public abstract Direction opposite();
}
