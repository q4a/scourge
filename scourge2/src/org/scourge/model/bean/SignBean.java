package org.scourge.model.bean;

import org.simpleframework.xml.Attribute;
import org.simpleframework.xml.Element;

/**
 * User: gabor
 * Date: May 29, 2010
 * Time: 2:39:45 PM
 */
public class SignBean {
    @Attribute()
    private int version;

    @Element()
    private String message;

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }
}
