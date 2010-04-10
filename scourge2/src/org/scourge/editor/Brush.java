package org.scourge.editor;

/**
* User: gabor
* Date: Apr 10, 2010
* Time: 12:56:52 PM
*/
public enum Brush {
    single(1, 1, false),
    two_by_two(2, 2, false), 
    three_by_three(3, 3, false),
    small_cloud(4, 4, true),
    medium_cloud(6, 6, true),
    large_cloud(8, 8, true);

    private int w, h;
    private boolean random;

    Brush(int w, int h, boolean random) {
        this.w = w;
        this.h = h;
        this.random = random;
    }

    public int getW() {
        return w;
    }

    public int getH() {
        return h;
    }

    public boolean isRandom() {
        return random;
    }
}
