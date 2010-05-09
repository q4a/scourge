package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;
import org.simpleframework.xml.Serializer;
import org.simpleframework.xml.core.Persister;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.Reader;

/**
 * User: gabor
 * Date: May 8, 2010
 * Time: 1:24:46 PM
 */
@Root
public class Model {
    @Element
    private String name;

    @Element
    private String path;

    @Element
    private float[] dimensions;

    @Element(required = false)
    private boolean interactive;

    @Element(required = false)
    private String icon;

    @Element(required = false, name="icon_rotate")
    private float[] iconRotate;

    @Element(required = false)
    private String effect;

    @Element(required = false, name="outdoors_weight")
    private float outdoorWeight;

    @Element(required = false)
    private float[] offset;

    @Element(required = false, name="not_flatten")
    private boolean notFlatten;

    @Element(required = false, name="no_floor")
    private boolean noFloor;

    @Element(required = false, name="wind")
    private boolean wind;

    @Element(name="rotate_3d", required = false)
    private float[] rotate;

    @Element(required = false)
    private int[] base;

    @Element(required = false)
    private boolean stencil;

    @Element(name="light_blocking", required = false)
    private boolean lightBlocking;

    @Element(name="ignores_height_map", required = false)
    private boolean ignoresHeightMap;

    @Element(required = false)
    private boolean roof;

    @Element(name="uses_alpha", required = false)
    private boolean useAlpha;
        
    @Element(name="outdoor_shadow", required = false)
    private boolean outdoorShadow;
    
    public String getName() {
        return name;
    }

    public String getPath() {
        return path;
    }

    public float[] getDimensions() {
        return dimensions;
    }

    public boolean isInteractive() {
        return interactive;
    }

    public String getIcon() {
        return icon;
    }

    public float[] getRotate() {
        return rotate;
    }

    public int[] getBase() {
        return base;
    }

    public boolean isStencil() {
        return stencil;
    }

    public boolean isLightBlocking() {
        return lightBlocking;
    }

    public boolean isIgnoresHeightMap() {
        return ignoresHeightMap;
    }

    public boolean isRoof() {
        return roof;
    }

    public boolean isUseAlpha() {
        return useAlpha;
    }

    public boolean isOutdoorShadow() {
        return outdoorShadow;
    }
}
