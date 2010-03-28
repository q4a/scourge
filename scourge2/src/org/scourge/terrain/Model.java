package org.scourge.terrain;

/**
 * User: gabor
 * Date: Mar 27, 2010
 * Time: 8:59:30 PM
 */
public enum Model {
    bridge("./data/3ds/bridge.3ds");

    private String modelPath;

    Model(String modelPath) {
        this.modelPath = modelPath;
    }

    public String getModelPath() {
        return modelPath;
    }
}
