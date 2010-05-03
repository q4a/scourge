package org.scourge.config;

import org.simpleframework.xml.Element;
import org.simpleframework.xml.Root;

/**
 * User: gabor
 * Date: May 3, 2010
 * Time: 9:14:44 AM
 */
@Root(name="cth")
public class CthAdjustment {
    @Element
    private String max;

    @Element
    private String min;
}
